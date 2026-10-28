// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <signalsmith-stretch.h>
#include <array>
#include <atomic>

// Our audio engine: YIN pitch tracking -> spectral pitch/formant shift -> drive -> mix.
// All buffers are allocated in prepare(), never in the processing loop.
class VoiceEngine
{
public:
    struct Controls
    {
        float pitch = 0, formant = 0, drive = 0, mix = 1;
        int mode = 0;
        bool bypass = false;
    };
    void prepare(double rate, int channels, bool live = false);
    void reset();
    void process(juce::AudioBuffer<float>&, const Controls&);
    int latency() const { return latencySamples; }
    std::atomic<float> detectedHz { 0 }, inputPeak { 0 }, outputPeak { 0 };

private:
    class PitchTracker
    {
    public:
        void prepare(double rate, bool live);
        void push(float sample);
        void reset();
        float frequency = 0;
    private:
        void analyse();
        static constexpr int capacity = 1024;
        int frameSize = capacity, analysisHop = 256;
        std::array<float, capacity> ring {}, frame {}, difference {};
        int position = 0, filled = 0, decimation = 4, phase = 0, hop = 0;
        double analysisRate = 12000;
        float lowpass = 0, lowpassCoefficient = 0.5f;
    } tracker;

    static constexpr int chunkSize = 256;
    signalsmith::stretch::SignalsmithStretch<float> stretch { 31051994 };
    juce::AudioBuffer<float> input, wet, dryDelay;
    juce::SmoothedValue<float> pitchSmoother, formantSmoother, driveSmoother, mixSmoother, bypassSmoother;
    double sampleRate = 48000;
    int channelCount = 2, latencySamples = 0, delayPosition = 0;
};
