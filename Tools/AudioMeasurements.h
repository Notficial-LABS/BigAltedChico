// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#pragma once
#include <fstream>
#include <iomanip>

// Actual processor renders, with warm-up and untrimmed tails. WAVs are reproducible
// synthetic fixtures, not recordings of a singer. Input and output share t=0.
static void writeWave(const juce::File& file, const std::vector<float>& samples, int rate)
{
    file.deleteFile();
    std::unique_ptr<juce::OutputStream> stream = file.createOutputStream();
    juce::WavAudioFormat format;
    auto writer = format.createWriterFor(stream, juce::AudioFormatWriterOptions()
        .withSampleRate(rate).withNumChannels(1).withBitsPerSample(24));
    require(writer != nullptr, "Cannot write WAV");
    const float* channel = samples.data();
    require(writer->writeFromFloatArrays(&channel, 1, static_cast<int>(samples.size())), "WAV write failed");
}

static std::vector<float> renderFixture(BigAltedChicoProcessor& p, const std::vector<float>& input,
                                int block, bool hostBypass = false, std::ostream* trace = nullptr)
{
    std::vector<float> output;
    output.reserve(input.size());
    for (int offset = 0; offset < static_cast<int>(input.size()); offset += block)
    {
        const int count = juce::jmin(block, static_cast<int>(input.size()) - offset);
        juce::AudioBuffer<float> buffer(p.getTotalNumInputChannels(), count);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.copyFrom(ch, 0, input.data() + offset, count);
        juce::MidiBuffer midi;
        if (hostBypass) p.processBlockBypassed(buffer, midi); else p.processBlock(buffer, midi);
        if (trace) *trace << offset + count << "," << p.detectedFrequency() << "\n";
        for (int i = 0; i < count; ++i)
        {
            const auto sample = buffer.getSample(0, i);
            require(std::isfinite(sample), "Nonfinite render");
            if (buffer.getNumChannels() > 1)
                require(std::abs(sample - buffer.getSample(1, i)) < 1e-5f, "Stereo coherence failed");
            output.push_back(sample);
        }
    }
    return output;
}

struct LatencyListener : juce::AudioProcessorListener
{
    int updates = 0;
    void audioProcessorParameterChanged(juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged(juce::AudioProcessor*, const ChangeDetails& details) override
    { if (details.latencyChanged) ++updates; }
};

static void checkLatencyAndSwitching(const juce::File& directory)
{
    std::ofstream csv(directory.getChildFile("latency.csv").getFullPathName().toStdString());
    csv << "processing,rate,block,channels,path,reported_samples,measured_peak_samples,peak_error_samples\n";
    for (int live : {0, 1}) for (int rate : {44100, 48000, 96000})
        for (int block : {64, 127, 257, 1024}) for (int channels : {1, 2})
            for (int path = 0; path < 4; ++path)
    {
        BigAltedChicoProcessor p;
        parameter(p, "processingMode", static_cast<float>(live));
        p.setPlayConfigDetails(channels, channels, rate, block); p.prepareToPlay(rate, block);
        parameter(p, "mix", path == 1 ? 0.0f : 1.0f);
        if (path == 2) parameter(p, "bypass", 1);
        const int impulse = rate / 4;
        std::vector<float> input(static_cast<size_t>(rate * 3 / 4), 0);
        input[static_cast<size_t>(impulse)] = 0.5f;
        const auto output = renderFixture(p, input, block, path == 3);
        const auto peak = std::max_element(output.begin(), output.end(), [](float a, float b) { return std::abs(a) < std::abs(b); });
        const int measured = static_cast<int>(peak - output.begin()) - impulse;
        csv << (live ? "Live" : "Quality") << ',' << rate << ',' << block << ',' << channels << ','
            << path << ',' << p.getLatencySamples() << ',' << measured << ',' << measured - p.getLatencySamples() << '\n';
        require(std::abs(*peak) > 0.1f, "Impulse disappeared");
        require(measured == p.getLatencySamples(), "Measured wet/dry/bypass delay differs from host report");
    }
    BigAltedChicoProcessor p;
    p.setPlayConfigDetails(2, 2, 48000, 127); p.prepareToPlay(48000, 127);
    LatencyListener listener; p.addListener(&listener);
    for (int live : {1, 0, 1, 0})
    {
        parameter(p, "processingMode", static_cast<float>(live));
        std::vector<float> silence(127, 0); renderFixture(p, silence, 127);
        require(p.getLatencySamples() == (live ? 1440 : 7200), "Mode switch latency failed");
        juce::MemoryBlock saved; p.getStateInformation(saved);
        BigAltedChicoProcessor restored;
        restored.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
        restored.setPlayConfigDetails(2, 2, 48000, 127); restored.prepareToPlay(48000, 127);
        require(restored.getLatencySamples() == p.getLatencySamples(), "Processing mode state roundtrip failed");
    }
    p.removeListener(&listener);
    require(listener.updates == 4, "Host not notified of latency changes");
    // Legacy state must return to Quality even when loaded over a Live instance.
    parameter(p, "processingMode", 1);
    auto legacy = p.state.copyState();
    legacy.removeChild(legacy.getChildWithProperty("id", "processingMode"), nullptr);
    const auto preset = directory.getChildFile("legacy-test.bigchico");
    legacy.createXml()->writeTo(preset);
    require(p.loadPreset(preset), "Legacy preset failed");
    require(p.state.getRawParameterValue("processingMode")->load() < 0.5f, "Legacy preset should select Quality");
    preset.deleteFile();
    std::cout << "192 wet/dry/plugin-bypass/host-bypass impulse measurements: exact host alignment; switching/state/legacy: OK\n";
}

static void renderAudioComparisons(const juce::File& directory)
{
    constexpr int rate = 48000, block = 128;
    const auto audioDir = directory.getChildFile("audio"); audioDir.createDirectory();
    std::ofstream csv(directory.getChildFile("renders.csv").getFullPathName().toStdString());
    csv << "file,processing,tuning,input_hz,pitch,expected_hz,reported_samples,detected_hz,cpu_percent\n";
    for (const auto& name : {"bass", "high", "bass-detuned", "high-detuned", "rapid", "transients", "voice"})
    {
        std::vector<float> input(rate * 3, 0);
        double phase = 0;
        for (int i = rate / 4; i < rate * 5 / 2; ++i)
        {
            const double t = static_cast<double>(i - rate / 4) / rate;
            double hz = juce::String(name) == "bass" ? 82.4069 : juce::String(name) == "bass-detuned" ? 85 : juce::String(name) == "high-detuned" ? 852 : 880;
            if (juce::String(name) == "rapid") hz = std::array<double, 4>{85, 230, 455, 852}[static_cast<size_t>(t / 0.08) % 4];
            if (juce::String(name) == "voice") hz = 175;
            phase += juce::MathConstants<double>::twoPi * hz / rate;
            double sample = 0.25 * std::sin(phase);
            if (juce::String(name) == "voice")
            {
                sample = 0;
                for (int h = 1; h <= 24; ++h)
                {
                    const double f = h * hz;
                    const double gain = 0.12 / h + std::exp(-std::pow((f - 900) / 300, 2)) + 0.6 * std::exp(-std::pow((f - 2400) / 450, 2));
                    sample += 0.06 * gain * std::sin(h * phase);
                }
            }
            if (juce::String(name) == "transients")
            {
                const int pos = (i - rate / 4) % (rate / 4);
                sample = pos == 0 ? 0.5 : (pos < 960 ? 0.25 * std::exp(-pos / 160.0) * std::sin(phase) : 0);
            }
            input[static_cast<size_t>(i)] = static_cast<float>(sample);
        }
        writeWave(audioDir.getChildFile(juce::String(name) + "-input.wav"), input, rate);
        for (int live : {0, 1}) for (int tuning = 0; tuning < 3; ++tuning)
        {
            BigAltedChicoProcessor p;
            parameter(p, "processingMode", static_cast<float>(live));
            parameter(p, "mix", 1);
            parameter(p, "mode", static_cast<float>(tuning));
            const float pitch = tuning == 0 ? 7.0f : 0;
            parameter(p, "pitch", pitch);
            p.setPlayConfigDetails(1, 1, rate, block); p.prepareToPlay(rate, block);
            const double start = juce::Time::getMillisecondCounterHiRes();
            std::ofstream trace(directory.getChildFile(juce::String(name) + (live ? "-Live-tracker.csv" : "-Quality-tracker.csv")).getFullPathName().toStdString());
            trace << "input_sample,detected_hz\n";
            const auto output = renderFixture(p, input, block, false, &trace);
            const double cpu = (juce::Time::getMillisecondCounterHiRes() - start) / 30.0;
            const auto file = juce::String(name) + (live ? "-Live-" : "-Quality-") + juce::StringArray{"Transpose", "Quantize", "Robot"}[tuning] + ".wav";
            writeWave(audioDir.getChildFile(file), output, rate);
            const double hz = juce::String(name) == "bass" ? 82.4069 : juce::String(name) == "high" ? 880 : juce::String(name) == "bass-detuned" ? 85 : juce::String(name) == "high-detuned" ? 852 : 0;
            const double expected = hz <= 0 ? 0 : tuning == 0 ? hz * std::pow(2.0, 7.0 / 12) : tuning == 1 ? 440 * std::pow(2.0, std::round(12 * std::log2(hz / 440)) / 12) : 523.25113;
            csv << file << ',' << (live ? "Live" : "Quality") << ',' << tuning << ',' << hz << ',' << pitch << ',' << expected << ',' << p.getLatencySamples() << ',' << p.detectedFrequency() << ',' << cpu << '\n';
        }
    }
    std::cout << "42 comparison WAVs + 7 inputs rendered (bass, high, 80 ms note changes, transients, harmonic voice; all tuning modes).\n";
}

static void checkLinkedParameters()
{
    BigAltedChicoProcessor p;
    parameter(p, "pitch", 4); parameter(p, "formant", -3);
    parameter(p, "link", 1);
    require(std::abs(p.state.getRawParameterValue("formant")->load() - 4) < 0.01f, "Link enable did not copy Pitch");
    parameter(p, "formant", -7);
    require(std::abs(p.state.getRawParameterValue("pitch")->load() + 7) < 0.01f, "Formant did not move linked Pitch");
    parameter(p, "pitch", 2);
    require(std::abs(p.state.getRawParameterValue("formant")->load() - 2) < 0.01f, "Pitch did not move linked Formant");
    juce::MemoryBlock data; p.getStateInformation(data);
    p.setCurrentProgram(0); p.setStateInformation(data.getData(), static_cast<int>(data.getSize()));
    require(p.state.getRawParameterValue("link")->load() > 0.5f, "Link state not restored");
    require(std::abs(p.state.getRawParameterValue("formant")->load() - 2) < 0.01f, "Linked values not restored");
    parameter(p, "link", 0); parameter(p, "formant", -4);
    require(std::abs(p.state.getRawParameterValue("pitch")->load() - 2) < 0.01f, "Unlink did not restore independence");
    p.setCurrentProgram(6);
    require(std::abs(p.state.getRawParameterValue("formant")->load() + 5) < 0.01f, "Linked factory preset inconsistent");
    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
    auto* pitch = dynamic_cast<juce::Slider*>(editor->findChildWithID("pitch"));
    auto* formant = dynamic_cast<juce::Slider*>(editor->findChildWithID("formant"));
    require(pitch && formant, "Linked sliders missing");
    pitch->setValue(6, juce::sendNotificationSync);
    require(std::abs(formant->getValue() - 6) < 0.01, "Linked Formant GUI did not follow Pitch");
    formant->setValue(-2, juce::sendNotificationSync);
    require(std::abs(pitch->getValue() + 2) < 0.01, "Linked Pitch GUI did not follow Formant");
    std::cout << "LINK enable, bidirectional host parameters/GUI, unlink, presets and state restore: OK\n";
}
