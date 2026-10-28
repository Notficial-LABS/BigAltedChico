// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
struct Preset { const char* name; float pitch, formant, drive, mix; int mode; bool link; };
constexpr Preset presets[] {
    { "00 Clean Start",    0,  0, 0.00f, 0.00f, 0, false },
    { "01 Deep Character", 0, -4, 0.15f, 1.00f, 0, false },
    { "02 High Roller",    7,  1, 0.10f, 1.00f, 0, false },
    { "03 Modern Double",  0, -3, 0.10f, 0.50f, 1, false },
    { "04 Robot Voice",   -5, -2, 0.25f, 1.00f, 2, false },
    { "05 Low Octave",   -12, -3, 0.12f, 0.75f, 0, false },
    { "06 Tape Creature", -5,  0, 0.28f, 1.00f, 0, true  }
};
constexpr const char* ids[] { "pitch", "formant", "drive", "mix", "mode", "link", "bypass" };
}

juce::AudioProcessorValueTreeState::ParameterLayout BigAltedChicoProcessor::makeParameters()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout p;
    const auto semitoneAttributes = AudioParameterFloatAttributes().withLabel("st").withMeta(true);
    p.add(std::make_unique<AudioParameterFloat>(ParameterID { "pitch", 1 }, "Pitch", NormalisableRange<float>(-12, 12, 0.1f), 0, semitoneAttributes));
    p.add(std::make_unique<AudioParameterFloat>(ParameterID { "formant", 1 }, "Formant", NormalisableRange<float>(-12, 12, 0.1f), 0, semitoneAttributes));
    p.add(std::make_unique<AudioParameterFloat>(ParameterID { "drive", 1 }, "Drive", NormalisableRange<float>(0, 1, 0.001f), 0));
    p.add(std::make_unique<AudioParameterFloat>(ParameterID { "mix", 1 }, "Mix", NormalisableRange<float>(0, 1, 0.001f), 0));
    p.add(std::make_unique<AudioParameterChoice>(ParameterID { "mode", 1 }, "Mode", StringArray { "Transpose", "Quantize", "Robot" }, 0));
    p.add(std::make_unique<AudioParameterBool>(ParameterID { "link", 1 }, "Link", false, AudioParameterBoolAttributes().withMeta(true)));
    p.add(std::make_unique<AudioParameterBool>(ParameterID { "bypass", 1 }, "Bypass", false));
    // Latency changes are a setup choice, not sample-accurate musical automation.
    p.add(std::make_unique<AudioParameterChoice>(ParameterID { "processingMode", 2 }, "Processing",
        StringArray { "Quality", "Live" }, 0, AudioParameterChoiceAttributes().withAutomatable(false)));
    return p;
}

BigAltedChicoProcessor::BigAltedChicoProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "BigAltedChicoState", makeParameters())
{
    processingMode = state.getRawParameterValue("processingMode");
    for (size_t i = 0; i < values.size(); ++i) values[i] = state.getRawParameterValue(ids[i]);
    linkedParameters = { state.getParameter("pitch"), state.getParameter("formant") };
    for (const auto* id : { "pitch", "formant", "link" }) state.addParameterListener(id, this);
    startTimerHz(60);
}

BigAltedChicoProcessor::~BigAltedChicoProcessor()
{
    stopTimer();
    for (const auto* id : { "pitch", "formant", "link" }) state.removeParameterListener(id, this);
}

void BigAltedChicoProcessor::parameterChanged(const juce::String& id, float value)
{
    if (restoringControls.load() || values[5]->load() < 0.5f) return;
    if (linking.test_and_set()) return; // Peer notification must not recurse.
    const size_t peer = id == "formant" ? 0 : 1;
    const float linked = id == "link" ? values[0]->load() : value;
    auto* parameter = linkedParameters[peer];
    if (std::abs(parameter->convertFrom0to1(parameter->getValue()) - linked) > 0.001f)
    {
        parameter->setValueNotifyingHost(parameter->convertTo0to1(linked));
        // VST3 suppresses nested notifications while applying a host edit.
        // Replay the peer's current value on the message thread. Polling avoids
        // posting/allocating an OS message from an audio-thread parameter callback.
        pendingLinkedNotifications.fetch_or(1u << peer);
    }
    linking.clear();
}

void BigAltedChicoProcessor::timerCallback()
{
    const auto pending = pendingLinkedNotifications.exchange(0);
    for (size_t i = 0; i < linkedParameters.size(); ++i)
        if ((pending & (1u << i)) != 0)
            linkedParameters[i]->sendValueChangedMessageToListeners(linkedParameters[i]->getValue());
}

void BigAltedChicoProcessor::syncLinkedControls()
{
    parameterChanged("link", values[5]->load());
}

void BigAltedChicoProcessor::prepareToPlay(double rate, int)
{
    engine.prepare(rate, getTotalNumInputChannels());
    liveEngine.prepare(rate, getTotalNumInputChannels(), true);
    activeLive.store(processingMode->load() >= 0.5f);
    setLatencySamples(isLive() ? liveEngine.latency() : engine.latency());
}

bool BigAltedChicoProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo())
           && output == layouts.getMainInputChannelSet();
}

void BigAltedChicoProcessor::process(juce::AudioBuffer<float>& buffer, bool bypass)
{
    juce::ScopedNoDenormals noDenormals;
    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());
    VoiceEngine::Controls p;
    p.pitch = values[0]->load(); p.formant = values[1]->load();
    p.drive = values[2]->load(); p.mix = values[3]->load();
    p.mode = juce::roundToInt(values[4]->load());
    if (values[5]->load() >= 0.5f) p.formant = p.pitch;
    p.bypass = bypass || values[6]->load() >= 0.5f;
    const bool live = processingMode->load(std::memory_order_relaxed) >= 0.5f;
    auto& selected = live ? liveEngine : engine;
    if (live != isLive())
    {
        // Both configurations were allocated in prepareToPlay. Reset only clears
        // existing storage. Changing latency flushes history; switch while stopped.
        selected.reset();
        activeLive.store(live, std::memory_order_relaxed);
        setLatencySamples(selected.latency()); // JUCE sends latencyChanged to VST3/AU.
    }
    selected.process(buffer, p);
}

void BigAltedChicoProcessor::processBlock(juce::AudioBuffer<float>& b, juce::MidiBuffer&) { process(b, false); }
void BigAltedChicoProcessor::processBlockBypassed(juce::AudioBuffer<float>& b, juce::MidiBuffer&) { process(b, true); }
juce::AudioProcessorParameter* BigAltedChicoProcessor::getBypassParameter() const { return state.getParameter("bypass"); }
juce::AudioProcessorEditor* BigAltedChicoProcessor::createEditor() { return new BigAltedChicoEditor(*this); }
int BigAltedChicoProcessor::getNumPrograms() { return static_cast<int>(std::size(presets)); }
const juce::String BigAltedChicoProcessor::getProgramName(int i) { return presets[juce::jlimit(0, getNumPrograms() - 1, i)].name; }

void BigAltedChicoProcessor::setCurrentProgram(int i)
{
    i = juce::jlimit(0, getNumPrograms() - 1, i);
    const auto& p = presets[i];
    restoringControls.store(true);
    const float settings[] { p.pitch, p.formant, p.drive, p.mix, static_cast<float>(p.mode), p.link ? 1.0f : 0.0f, 0 };
    for (size_t k = 0; k < std::size(ids); ++k)
    {
        auto* parameter = state.getParameter(ids[k]);
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(settings[k]));
        parameter->endChangeGesture();
    }
    restoringControls.store(false);
    syncLinkedControls();
    currentPreset.store(i);
}

juce::String BigAltedChicoProcessor::getSkinId() const
{
    const juce::ScopedLock lock(skinLock); return selectedSkin;
}
void BigAltedChicoProcessor::setSkinId(const juce::String& id)
{
    { const juce::ScopedLock lock(skinLock); if (selectedSkin == id) return; selectedSkin = id; }
    updateHostDisplay(juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged(true));
}

void BigAltedChicoProcessor::getStateInformation(juce::MemoryBlock& data)
{
    auto tree = state.copyState();
    tree.setProperty("uiSkin", getSkinId(), nullptr);
    tree.setProperty("preset", currentPreset.load(), nullptr);
    if (auto xml = tree.createXml()) copyXmlToBinary(*xml, data);
}

void BigAltedChicoProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(state.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml(*xml);
            { const juce::ScopedLock lock(skinLock); selectedSkin = tree.getProperty("uiSkin", "juguete").toString(); }
            currentPreset.store(juce::jlimit(0, getNumPrograms() - 1, static_cast<int>(tree.getProperty("preset", 0))));
            // Legacy 0.1 sessions/presets have no Processing parameter.
            if (!tree.getChildWithProperty("id", "processingMode").isValid())
            {
                juce::ValueTree mode("PARAM");
                mode.setProperty("id", "processingMode", nullptr);
                mode.setProperty("value", 0.0f, nullptr);
                tree.appendChild(mode, nullptr);
            }
            restoringControls.store(true);
            state.replaceState(tree);
            restoringControls.store(false);
            syncLinkedControls();
        }
}

bool BigAltedChicoProcessor::savePreset(const juce::File& file)
{
    auto tree = state.copyState();
    tree.removeProperty("uiSkin", nullptr); // Sound presets do not change appearance.
    tree.setProperty("preset", currentPreset.load(), nullptr);
    if (auto xml = tree.createXml()) return xml->writeTo(file);
    return false;
}

bool BigAltedChicoProcessor::loadPreset(const juce::File& file)
{
    if (auto xml = juce::XmlDocument::parse(file))
        if (xml->hasTagName(state.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml(*xml);
            currentPreset.store(juce::jlimit(0, getNumPrograms() - 1, static_cast<int>(tree.getProperty("preset", 0))));
            // Legacy 0.1 sessions/presets have no Processing parameter.
            if (!tree.getChildWithProperty("id", "processingMode").isValid())
            {
                juce::ValueTree mode("PARAM");
                mode.setProperty("id", "processingMode", nullptr);
                mode.setProperty("value", 0.0f, nullptr);
                tree.appendChild(mode, nullptr);
            }
            restoringControls.store(true);
            state.replaceState(tree);
            restoringControls.store(false);
            syncLinkedControls();
            return true;
        }
    return false;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BigAltedChicoProcessor(); }
