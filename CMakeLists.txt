cmake_minimum_required(VERSION 3.24)
project(BigAltedChico VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(WIN32)
    # The installer should not depend on a separately installed VC++ runtime.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
set(JUCE_DIR "$ENV{JUCE_DIR}" CACHE PATH "JUCE source directory (tested with 9.0.4)")
if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Set -DJUCE_DIR=/path/to/JUCE (tested with 9.0.4). See docs/BUILDING.md.")
endif()
add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)

# AU is a macOS format; Windows and Linux build VST3 and Standalone only.
set(BIGCHICO_FORMATS VST3 Standalone)
set(BIGCHICO_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Icons/BAC-windows.png")
if(APPLE)
    list(APPEND BIGCHICO_FORMATS AU)
    set(BIGCHICO_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Icons/BAC-mac.png")
endif()
set(BIGCHICO_ICON_ARGS ICON_BIG "${BIGCHICO_ICON}" ICON_SMALL "${BIGCHICO_ICON}")
if(WIN32)
    # Embed the icon directly in the Standalone PE resource below. JUCE's
    # shared plugin icon also invokes Windows-only attrib for VST3 folders,
    # which cannot run on a macOS cross-build host.
    set(BIGCHICO_ICON_ARGS)
endif()

# Keep the generated archive out of Git; a clean clone builds it from source.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
file(GLOB SKIN_TEMPLATE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/SkinTemplate/*")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${SKIN_TEMPLATE_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/Tools/package_skin_template.py")
set(SKIN_TEMPLATE_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/generated/SkinTemplate.zip")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/Tools/package_skin_template.py"
    --output "${SKIN_TEMPLATE_ARCHIVE}" COMMAND_ERROR_IS_FATAL ANY)


# The manifest helper must run on the target OS. A cross-built VST3 can be
# scanned normally without the optional moduleinfo.json; native builds keep it.
set(BIGCHICO_AUTO_MANIFEST TRUE)
if(CMAKE_CROSSCOMPILING AND WIN32)
    set(BIGCHICO_AUTO_MANIFEST FALSE)
endif()

juce_add_plugin(BigAltedChico
    VST3_AUTO_MANIFEST ${BIGCHICO_AUTO_MANIFEST}
    ${BIGCHICO_ICON_ARGS}
    COMPANY_NAME "Notficial LABS"
    BUNDLE_ID "com.bysanmateo.bigaltechico"
    PLUGIN_MANUFACTURER_CODE Bysm
    PLUGIN_CODE Bchc
    FORMATS ${BIGCHICO_FORMATS}
    PRODUCT_NAME "BigAltedChico"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "BigAltedChico uses audio input to transform your voice."
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
    VST3_CATEGORIES Fx Pitch\ Shift
    AU_MAIN_TYPE kAudioUnitType_Effect)

juce_add_binary_data(BigChicoAssets SOURCES Assets/Roboto.ttf Assets/NotficialLabs.png
    Assets/NotficialLabs-Logo.svg Assets/NotficialLabs-Wordmark.svg Assets/ThirdPartyNotices.txt
    Assets/Skins/Ondas/OndasBackground.svg Assets/Skins/Salem/SalemBackground.svg "${SKIN_TEMPLATE_ARCHIVE}")

target_sources(BigAltedChico PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/SettingsPanel.cpp Source/SkinDefinition.cpp Source/VoiceEngine.cpp)
target_include_directories(BigAltedChico PUBLIC Source)
target_include_directories(BigAltedChico SYSTEM PUBLIC ThirdParty)
target_compile_definitions(BigAltedChico PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(BigAltedChico PRIVATE BigChicoAssets juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Fresh Clean audio on each standalone launch; the DAW state path is unchanged.
target_sources(BigAltedChico_Standalone PRIVATE Source/StandaloneApp.cpp)
target_compile_definitions(BigAltedChico_Standalone PRIVATE JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1)
if(WIN32)
    target_sources(BigAltedChico_Standalone PRIVATE Packaging/Windows/StandaloneIcon.rc)
    target_include_directories(BigAltedChico_Standalone PRIVATE Assets/Icons)
endif()

option(BIGCHICO_BUILD_CHECKS "Build offline audio checks and GUI snapshot tool" OFF)
if(BIGCHICO_BUILD_CHECKS)
    add_executable(BigAltedChicoChecks)
    set_target_properties(BigAltedChicoChecks PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/BigAltedChicoChecks_artefacts/${CMAKE_BUILD_TYPE}")
    target_sources(BigAltedChicoChecks PRIVATE Tools/Checks.cpp)
    target_link_libraries(BigAltedChicoChecks PRIVATE BigAltedChico juce::juce_audio_utils)
endif()

if(BIGCHICO_BUILD_CHECKS)
    juce_add_console_app(BigAltedChicoHostChecks PRODUCT_NAME "BigAltedChico Host Checks")
    target_sources(BigAltedChicoHostChecks PRIVATE Tools/HostChecks.cpp)
    target_compile_definitions(BigAltedChicoHostChecks PRIVATE
        JUCE_PLUGINHOST_VST3=1 JUCE_MODAL_LOOPS_PERMITTED=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(BigAltedChicoHostChecks PRIVATE juce::juce_audio_utils
        juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endif()

# JUCE generates moduleinfo.json after linking VST3. Seal the completed bundle,
# including that file, so Ableton's scanner accepts the local development build.
if(APPLE)
    foreach(format IN LISTS BIGCHICO_FORMATS)
        add_custom_command(TARGET BigAltedChico_${format} POST_BUILD
            COMMAND /usr/bin/codesign --force --deep --sign - "$<TARGET_FILE_DIR:BigAltedChico_${format}>/../.."
            VERBATIM)
    endforeach()
endif()
