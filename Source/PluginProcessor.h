// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "VoiceEngine.h"

class BigAltedChicoProcessor final : public juce::AudioProcessor, private juce::AudioProcessorValueTreeState::Listener, private juce::Timer
{
public:
    BigAltedChicoProcessor();
    ~BigAltedChicoProcessor() override;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void reset() override { engine.reset(); liveEngine.reset(); }
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "BigAltedChico"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.25; }
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentPreset.load(); }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override {}
    juce::AudioProcessorParameter* getBypassParameter() const override;
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    bool savePreset(const juce::File&);
    bool loadPreset(const juce::File&);

    juce::AudioProcessorValueTreeState state;
    juce::String getSkinId() const;
    void setSkinId(const juce::String&);
    VoiceEngine engine, liveEngine;
    bool isLive() const { return activeLive.load(std::memory_order_relaxed); }
    float detectedFrequency() const { return (isLive() ? liveEngine : engine).detectedHz.load(); }
private:
    mutable juce::CriticalSection skinLock;
    juce::String selectedSkin { "juguete" };
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    void process(juce::AudioBuffer<float>&, bool bypass);
    void parameterChanged(const juce::String&, float) override;
    void syncLinkedControls();
    void timerCallback() override;
    std::atomic<unsigned> pendingLinkedNotifications { 0 };
    std::array<juce::RangedAudioParameter*, 2> linkedParameters {};
    std::atomic_flag linking = ATOMIC_FLAG_INIT;
    std::atomic<bool> restoringControls { false };
    std::array<std::atomic<float>*, 7> values {};
    std::atomic<float>* processingMode = nullptr;
    std::atomic<bool> activeLive { false };
    std::atomic<int> currentPreset { 0 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BigAltedChicoProcessor)
};
