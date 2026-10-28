// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <stdexcept>
#include <vector>

static void require(bool b, const char* why) { if (!b) throw std::runtime_error(why); }
int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    try
    {
        require(argc > 1, "Pass the absolute VST3 bundle path");
        juce::VST3PluginFormat format;
        juce::OwnedArray<juce::PluginDescription> types;
        format.findAllTypesForFile(types, argv[1]);
        require(types.size() == 1, "Expected one VST3 effect");
        juce::String error;
        auto plugin = format.createInstanceFromDescription(*types[0], 48000, 128, error);
        require(plugin != nullptr, error.toRawUTF8());
        std::cout << "Loaded VST3: " << types[0]->manufacturerName << " / " << types[0]->version << '\n';
        juce::AudioProcessorParameter* processing = nullptr;
        for (auto* p : plugin->getParameters())
            if (p->getName(100) == "Processing") processing = p;
        require(processing != nullptr, "Processing parameter missing from VST3");
        plugin->setPlayConfigDetails(2, 2, 48000, 128);
        plugin->prepareToPlay(48000, 128);
        auto findParameter = [&](const char* name) -> juce::AudioProcessorParameter* {
            for (auto* p : plugin->getParameters()) if (p->getName(100) == name) return p;
            throw std::runtime_error("Missing linked VST3 parameter");
        };
        auto flush = [&] {
            juce::AudioBuffer<float> buffer(2, 128); buffer.clear(); juce::MidiBuffer midi;
            plugin->processBlock(buffer, midi);
            juce::MessageManager::getInstance()->runDispatchLoopUntil(40);
        };
        require(findParameter("Mix")->getValue()==0,"VST3 must start dry in Clean");
        findParameter("Mix")->setValueNotifyingHost(1);
        auto* pitch = findParameter("Pitch"); auto* formant = findParameter("Formant"); auto* link = findParameter("Link");
        pitch->setValueNotifyingHost(16.0f / 24.0f); flush();
        link->setValueNotifyingHost(1); flush();
        require(std::abs(formant->getValue() - pitch->getValue()) < 0.001f, "VST3 LINK failed to copy Pitch");
        formant->setValueNotifyingHost(5.0f / 24.0f); flush();
        require(std::abs(formant->getValue() - pitch->getValue()) < 0.001f, "VST3 linked Pitch did not follow Formant");
        link->setValueNotifyingHost(0); flush();
        pitch->setValueNotifyingHost(0.5f); formant->setValueNotifyingHost(0.5f); flush();
        std::cout << "VST3 LINK bidirectional parameter notifications: PASS\n";
        for (int live : {0, 1, 0})
        {
            processing->setValueNotifyingHost(static_cast<float>(live));
            constexpr int impulse = 12000;
            std::vector<float> output;
            for (int offset = 0; offset < 36000; offset += 128)
            {
                juce::AudioBuffer<float> buffer(2, 128); buffer.clear();
                if (offset <= impulse && offset + 128 > impulse)
                    for (int ch = 0; ch < 2; ++ch) buffer.setSample(ch, impulse - offset, 0.5f);
                juce::MidiBuffer midi; plugin->processBlock(buffer, midi);
                output.insert(output.end(), buffer.getReadPointer(0), buffer.getReadPointer(0) + 128);
                juce::MessageManager::getInstance()->runDispatchLoopUntil(1);
            }
            const auto peak = std::max_element(output.begin(), output.end(), [](float a, float b) { return std::abs(a) < std::abs(b); });
            const int actual = static_cast<int>(peak - output.begin()) - impulse;
            std::cout << (live ? "Live" : "Quality") << ": VST3 host latency=" << plugin->getLatencySamples()
                << ", impulse latency=" << actual << " samples\n";
            require(actual == (live ? 1440 : 7200), "Unexpected impulse delay");
            require(plugin->getLatencySamples() == actual, "VST3 host report differs from real delay");
        }
        plugin->releaseResources();
        std::cout << "VST3 binary, dynamic latency notification and impulse alignment: PASS\n";
    }
    catch (const std::exception& e) { std::cerr << "FAILED: " << e.what() << '\n'; return 1; }
}
