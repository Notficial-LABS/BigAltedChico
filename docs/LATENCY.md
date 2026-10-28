# BigAltedChico — latencia, calidad y validación

Fecha: 25 de octubre de 2026. Marca y fabricante del plugin: **Notficial LABS**. Desarrollo: **Leonardo Marcelo**. C++/JUCE; no se cambió de lenguaje.

## Resultado

**LIVE encendido: 30 ms. LIVE apagado: Quality, 150 ms. Reducción: 80 %.** Son retardos de procesamiento reales y no incluyen ADC/DAC, buffers del dispositivo, otros efectos ni ruteos del DAW. No hay modo de 0 ms.

| Frecuencia | Quality reportado / medido | Live reportado / medido | Tiempo Quality / Live |
|---|---:|---:|---:|
| 44.100 Hz | 6615 / 6615 muestras | 1323 / 1323 muestras | 150 / 30 ms |
| 48.000 Hz | 7200 / 7200 muestras | 1440 / 1440 muestras | 150 / 30 ms |
| 96.000 Hz | 14400 / 14400 muestras | 2880 / 2880 muestras | 150 / 30 ms |

Se midieron 192 combinaciones: dos motores, tres frecuencias, bloques de 64/127/257/1024 muestras, mono/estéreo y cuatro rutas (wet neutro, dry, bypass del plugin y callback de bypass del host). Se inyectó un impulso de amplitud 0,5 tras 250 ms de silencio y se buscó el pico de salida. Error máximo entre pico y latencia declarada: **0 muestras**. Datos completos: `Working/Tests/latency.csv`.

Un impulso con transposición neutra mide el retardo de transporte sin confundirlo con el cambio de forma de onda propio de una transposición. Para transposición activa también se midió la distribución temporal de energía de ataques, más abajo. No se afirma que toda onda transformada conserve su pico exactamente en el mismo punto.

## De dónde salían los 150 ms

`Source/VoiceEngine.cpp`, en `prepare()`, llamaba a:

```cpp
stretch.presetDefault(channels, sampleRate, true);
latencySamples = stretch.inputLatency() + stretch.outputLatency();
```

El header incluido, `ThirdParty/signalsmith-stretch.h`, define ese preset como ventana de 120 ms, intervalo de 30 ms y `splitComputation=true`. `inputLatency()` devuelve la latencia de análisis; `outputLatency()` devuelve la de síntesis **más un intervalo si se reparte el cálculo**.

A 48 kHz: ventana = 5760 muestras; análisis = 2880; síntesis = 2880; cálculo repartido = 1440. Total = **2880 + 2880 + 1440 = 7200 = 150 ms**. `signalsmith-linear/stft.h` sitúa los offsets de análisis/síntesis en la mitad de la ventana.

El retardo seco se ejecuta en paralelo y alinea Dry/Mix/Bypass con el wet: no añade otros 150 ms en serie. Los chunks internos de 256 muestras tampoco constituyen otro FIFO de 256 muestras: se consumen y entregan dentro de la misma llamada. El tamaño de bloque del host no cambia la latencia algorítmica medida.

El YIN original reúne 1024 muestras a una frecuencia de análisis cercana a 12 kHz: 85,33 ms de ventana y pasos de 21,33 ms a 48 kHz. Es un **retardo de reacción del detector**, no otro buffer sumado a los 150 ms. El suavizado de controles de 15 ms también modifica la reacción, no la latencia fija de transporte.

La distinción análisis/síntesis coincide con la [documentación oficial de Signalsmith Stretch](https://signalsmith-audio.com/code/stretch/). Para esta entrega la autoridad final es el header local y las mediciones del binario; no se actualizó la dependencia.

## Implementación

| Parámetro | Quality | Live |
|---|---:|---:|
| Ventana espectral | 120 ms, preset original | 30 ms, configuración manual |
| Intervalo | 30 ms | 5 ms |
| Repartir cálculo | Sí, añade 30 ms | No |
| Ventana YIN a 48 kHz | 1024 / 85,33 ms | 512 / 42,67 ms |
| Hop YIN a 48 kHz | 256 / 21,33 ms | 64 / 5,33 ms |
| Suavizado de controles | 15 ms | 5 ms |

Ambos motores se configuran y reservan en `prepareToPlay()`. Solo se procesa el seleccionado. Cambiar LIVE limpia el historial del motor entrante y llama a `setLatencySamples()` con la latencia de ese motor; JUCE notifica el cambio a VST3/AU. El dry usa el mismo valor. No se fuerza artificialmente a cero el reporte del host.

Cambiar LIVE con el transporte detenido: el historial se vacía y el host debe recalcular compensación. No se ofrece una transición musical continua entre retardos diferentes. Processing se guarda en sesiones y presets, pero no se anuncia como parámetro automatizable. Los presets de fábrica cambian el sonido sin cambiar LIVE. Los estados 0.1 sin Processing se cargan en Quality.

**LINK corregido:** al encenderlo copia Pitch a Formant; cambiar cualquiera actualiza el otro, también desde parámetros del host. Pitch, Formant y Link se declaran como metaparámetros para que AU sepa que pueden modificar otro parámetro. Las notificaciones del control compañero se reenvían desde un timer de 60 Hz en el hilo de mensajes: VST3 suprime las notificaciones anidadas dentro de un cambio del host. El valor usado por el audio se sincroniza inmediatamente; el reflejo en el host puede tardar un tick del timer. Al apagarlo son independientes. El motor interpreta Formant como desplazamiento absoluto y compensa Pitch para evitar duplicarlo. Se mantienen IDs de parámetros existentes y códigos de plugin para conservar identificación de sesiones. La semántica de LINK cambia intencionadamente: un preset antiguo con LINK y valores distintos se normaliza a Pitch. El preset de fábrica Tape Creature mantiene su desplazamiento natural de envolvente (-5 st) y ahora muestra -5 en ambas perillas.

## Comparación de audio

42 renders de salida y siete entradas WAV de 24 bits, 48 kHz, mono, tres segundos, bloque de 128. Cada entrada se procesa con los dos motores y Transpose/Quantize/Robot. Entradas: seno grave 82,4069 Hz, agudo 880 Hz, graves/agudos desafinados (85/852 Hz), secuencia 85/230/455/852 Hz con notas de 80 ms, ataques repetidos y voz armónica sintética de 175 Hz con dos regiones formantes. Transpose usa +7 st; los otros modos, Pitch=0. Drive=0, Mix=100 %, Formant=0, LINK apagado. No son grabaciones de una persona.

`Tools/analyse_audio.py` analiza frecuencia dominante en 0,8–2,2 s, alineando antes por la latencia reportada. El error en cents es `1200*log2(medido/objetivo)`. No es una valoración perceptiva ni demuestra calidad con toda voz real.

| Entrada / modo | Objetivo | Quality medido | Live medido | Error Quality / Live |
|---|---:|---:|---:|---:|
| 82,4069 Hz / Transpose +7 | 123,471 Hz | 124,190 Hz | 125,599 Hz | +10,06 / +29,58 cents |
| 82,4069 Hz / Quantize | 82,4069 Hz | 82,4067 Hz | 82,4065 Hz | ~0 / -0,01 cents |
| 82,4069 Hz / Robot | 523,251 Hz | 518,947 Hz | 456,144 Hz | -14,30 / **-237,62 cents** |
| 880 Hz / Transpose +7 | 1318,51 Hz | 1318,750 Hz | 1320,346 Hz | +0,31 / +2,41 cents |
| 880 Hz / Quantize | 880 Hz | 879,710 Hz | 879,583 Hz | -0,57 / -0,82 cents |
| 880 Hz / Robot | 523,251 Hz | 522,787 Hz | 525,824 Hz | -1,53 / +8,49 cents |
| 85 Hz / Quantize | 87,3071 Hz | 87,545 Hz | 87,277 Hz | +4,70 / -0,59 cents |
| 852 Hz / Quantize | 830,609 Hz | 828,557 Hz | 829,232 Hz | -4,28 / -2,87 cents |

El caso grave de Robot es una limitación real: mover 82 Hz a 523 Hz requiere unos 32 semitonos. La salida presenta componentes y fluctuaciones; citar solo su pico principal no resume toda la distorsión. En esa prueba el detector sí estima unos 82,408 Hz: el error no se explica por confundir la nota de entrada. Quality también presenta artefactos en esa transformación extrema. Live no es un sustituto de Quality para todos los registros. Datos completos, incluyendo las otras notas y relación tono/residuo: `Working/Tests/steady-pitch.csv`.

En notas de 80 ms se analizaron ventanas Hann de 40 ms centradas 25/40/55 ms después de cada cambio, alineadas por la latencia declarada. No se cuenta la latencia fija como error de afinación:

| Modo | Mediana del error absoluto Quality / Live | Ventanas a menos de 50 cents Quality / Live |
|---|---:|---:|
| Transpose | 2,87 / 17,93 cents | 100 / 100 % |
| Quantize | 9,61 / 5,07 cents | 92,75 / 100 % |
| Robot | 154,43 / 31,59 cents | 49,28 / 57,97 % |

Por separado, el tiempo de detección medido desde la entrada, con tolerancia de frecuencia del 2 %, fue 91,33/38 ms para el primer grave y 48,67/27,33 ms para el primer agudo (Quality/Live). En la secuencia rápida, Live llegó a la frecuencia correcta dentro de los 27 segmentos, con mediana de 43,33 ms. Quality no llegó al objetivo dentro de esos segmentos de 80 ms; su ventana es más larga que cada nota. Esto no equivale a 43 ms adicionales de buffer ni garantiza que Robot ya haya estabilizado la salida.

En ataques repetidos, la anchura entre el 5 % y el 95 % de energía fue:

| Modo | Quality | Live |
|---|---:|---:|
| Transpose | 4,146 ms | 4,271 ms |
| Quantize | 4,792 ms | 4,771 ms |
| Robot | 4,000 ms | 3,042 ms |

El centro de energía quedó entre 0,718 y 1,497 ms después del tiempo nominal compensado. La transformación altera el ataque; estas cifras describen esa señal de prueba, no una latencia adicional constante. Ningún WAV de prueba tuvo muestras no finitas ni alcanzó 0 dBFS. Formant, Drive, LINK, silencio, estados, presets y attachments se comprobaron en ambos motores.

Menos ventana reduce la resolución espectral y la estabilidad de armónicos/formantes graves, aunque mejora la reacción temporal. Live hace cálculos más frecuentes y sin repartirlos: menor latencia no significa menor carga. En esta ejecución, render completo con trazas incluidas: promedio 1.19 % Quality y 1.40 % Live del tiempo real en M1 Max. Son tiempos offline, no una garantía de picos de CPU en un proyecto de Ableton.

## Validación del host y límites

`Working/Tests/vst3-host.txt`: un host JUCE de pruebas carga el **VST3 compilado**, no solo la clase del procesador. A 48 kHz verifica Quality → Live → Quality, notificaciones de latencia y sus impulsos: **7200 → 1440 → 7200 muestras**, todos coincidentes. También verifica las notificaciones bidireccionales de LINK.

Ableton Live 12.4.7 reconoció `Notficial LABS / BigAltedChico / 0.2.0`; su barra de estado mostró **“Latency: 7200 samples (150 ms)”** en Quality. La captura automática de la ventana flotante del plugin devolvió una imagen en blanco y Processing no está expuesto por el conector porque no es automatizable. La lectura directa de **Live=30 ms dentro de Ableton sigue pendiente de activar LIVE en la instancia recargada**. No se presenta la prueba del host JUCE como una medición realizada en Ableton. No se hizo un loopback ADC/DAC ni un render de compensación de Ableton.

`Working/Tests/auval.txt`: **AU VALIDATION SUCCEEDED**, fabricante Notficial LABS, versión 0.2.0, propiedad Latency válida. La primera consulta antes de actualizar el registro de Audio Units no encontró el componente; una vez registrado, la validación completa pasó.

Se corrigió además un fallo de empaquetado: `moduleinfo.json` se generaba después de la firma. Ableton rechazó ese bundle por recurso sellado inválido. CMake ahora firma al final de la generación del bundle y el VST3 instalado pasa `codesign --verify --deep --strict` y el escaneo de Ableton. Firma local ad hoc, no notarización para distribución pública.

## Repetir y escuchar

1. Ejecutar `./build.sh check`: pruebas C++, renders WAV y validación del binario VST3.
2. Instalar `numpy scipy soundfile` en un entorno Python y ejecutar `python Tools/analyse_audio.py` para generar los CSV de análisis.
3. Escuchar `Working/Tests/audio/<entrada>-Quality-<modo>.wav` y `...-Live-...wav`. Los archivos conservan el retardo inicial. Para comparar timbre alineado, adelantar Quality 7200 muestras y Live 1440 a 48 kHz. No normalizar cada archivo por separado si se compara nivel.
4. En Ableton, cargar una instancia nueva, detener transporte, activar/desactivar LIVE y pasar el puntero por el título del dispositivo. Esperado: 1440/7200 muestras a 48 kHz. El engranaje muestra también muestras y milisegundos.
5. Para comprobar PDC de Ableton: usar un impulso con Transpose=0, Formant=0, Drive=0, Mix=100 %, sin otros efectos ni Warp; exportar una pista seca y otra procesada desde el mismo origen temporal. Con compensación activada sus picos deben coincidir; una captura sin compensación debe conservar los 1440/7200 samples. No confundir esto con la latencia física del monitoreo.


## Repositorio público

Este estudio conserva las mediciones históricas del motor. La versión pública del proyecto sigue siendo 0.1; el número 0.2 en algunos logs antiguos corresponde a una compilación de desarrollo. Las tablas CSV seleccionadas están en [measurements](measurements/README.md). Los WAV, logs y respaldos completos son archivos locales bajo `Working/`; se pueden regenerar con las herramientas de pruebas. Los instaladores públicos todavía no se han implementado.
