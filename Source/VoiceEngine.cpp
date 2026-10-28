// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#include "VoiceEngine.h"
#include <cmath>

void VoiceEngine::PitchTracker::prepare(double rate, bool live)
{
    frameSize = live ? 512 : 1024;
    analysisHop = live ? 64 : 256;
    decimation = juce::jmax(1, static_cast<int>(std::round(rate / 12000.0)));
    analysisRate = rate / decimation;
    lowpassCoefficient = static_cast<float>(1.0 - std::exp(-2.0 * juce::MathConstants<double>::pi * 1600.0 / rate));
    reset();
}

void VoiceEngine::PitchTracker::reset()
{
    ring.fill(0);
    position = filled = phase = hop = 0;
    frequency = lowpass = 0;
}

void VoiceEngine::PitchTracker::push(float sample)
{
    lowpass += lowpassCoefficient * (sample - lowpass);
    if (++phase < decimation) return;
    phase = 0;
    ring[static_cast<size_t>(position)] = lowpass;
    position = (position + 1) % frameSize;
    filled = juce::jmin(frameSize, filled + 1);
    if (++hop >= analysisHop && filled == frameSize) { hop = 0; analyse(); }
}

void VoiceEngine::PitchTracker::analyse()
{
    float energy = 0;
    for (int i = 0; i < frameSize; ++i)
    {
        frame[static_cast<size_t>(i)] = ring[static_cast<size_t>((position + i) % frameSize)];
        energy += frame[static_cast<size_t>(i)] * frame[static_cast<size_t>(i)];
    }
    if (energy / static_cast<float>(frameSize) < 1.0e-7f) { frequency = 0; return; }
    const int minLag = juce::jmax(2, static_cast<int>(analysisRate / 1000.0));
    const int maxLag = juce::jmin(frameSize / 2 - 1, static_cast<int>(analysisRate / 65.0));
    float accumulated = 0;
    difference[0] = 1;
    for (int lag = 1; lag <= maxLag; ++lag)
    {
        float sum = 0;
        for (int i = 0; i < frameSize / 2; ++i)
        {
            const float delta = frame[static_cast<size_t>(i)] - frame[static_cast<size_t>(i + lag)];
            sum += delta * delta;
        }
        accumulated += sum;
        difference[static_cast<size_t>(lag)] = sum * static_cast<float>(lag) / (accumulated + 1.0e-20f);
    }
    int selected = 0;
    for (int lag = minLag; lag < maxLag; ++lag)
        if (difference[static_cast<size_t>(lag)] < 0.18f)
        {
            while (lag + 1 < maxLag && difference[static_cast<size_t>(lag + 1)] < difference[static_cast<size_t>(lag)]) ++lag;
            selected = lag;
            break;
        }
    if (selected == 0) { frequency = 0; return; }
    const float a = difference[static_cast<size_t>(selected - 1)];
    const float b = difference[static_cast<size_t>(selected)];
    const float c = difference[static_cast<size_t>(selected + 1)];
    const float denominator = a - 2 * b + c;
    const float correction = std::abs(denominator) > 1.0e-8f ? juce::jlimit(-0.5f, 0.5f, 0.5f * (a - c) / denominator) : 0;
    frequency = static_cast<float>(analysisRate) / (static_cast<float>(selected) + correction);
}

void VoiceEngine::prepare(double rate, int channels, bool live)
{
    sampleRate = rate;
    channelCount = juce::jlimit(1, 2, channels);
    // Library quality preset: 120 ms window, 30 ms step, split computation.
    if (live)
        stretch.configure(channelCount, static_cast<int>(std::round(rate * 0.03)),
                          static_cast<int>(std::round(rate * 0.005)), false);
    else
        stretch.presetDefault(channelCount, static_cast<float>(rate), true);
    latencySamples = stretch.inputLatency() + stretch.outputLatency();
    input.setSize(channelCount, chunkSize);
    wet.setSize(channelCount, chunkSize);
    dryDelay.setSize(channelCount, latencySamples + chunkSize + 1);
    tracker.prepare(rate, live);
    for (auto* smoother : { &pitchSmoother, &formantSmoother, &driveSmoother, &mixSmoother, &bypassSmoother })
        smoother->reset(rate, live ? 0.005 : 0.015);
    pitchSmoother.setCurrentAndTargetValue(0);
    formantSmoother.setCurrentAndTargetValue(0);
    driveSmoother.setCurrentAndTargetValue(0);
    mixSmoother.setCurrentAndTargetValue(1);
    bypassSmoother.setCurrentAndTargetValue(1);
    reset();
}

void VoiceEngine::reset()
{
    stretch.reset();
    tracker.reset();
    dryDelay.clear();
    delayPosition = 0;
    detectedHz.store(0);
    inputPeak.store(0);
    outputPeak.store(0);
}

void VoiceEngine::process(juce::AudioBuffer<float>& buffer, const Controls& p)
{
    if (dryDelay.getNumSamples() == 0) { buffer.clear(); return; }
    const int channels = juce::jmin(channelCount, buffer.getNumChannels());
    float inputMaximum = 0, outputMaximum = 0;
    driveSmoother.setTargetValue(p.drive);
    mixSmoother.setTargetValue(p.mix);
    bypassSmoother.setTargetValue(p.bypass ? 0.0f : 1.0f);
    formantSmoother.setTargetValue(p.formant);

    for (int offset = 0; offset < buffer.getNumSamples(); offset += chunkSize)
    {
        const int count = juce::jmin(chunkSize, buffer.getNumSamples() - offset);
        for (int channel = 0; channel < channels; ++channel)
            input.copyFrom(channel, 0, buffer, channel, offset, count);
        for (int i = 0; i < count; ++i)
        {
            float mono = 0;
            for (int ch = 0; ch < channels; ++ch) mono += input.getSample(ch, i);
            tracker.push(mono / static_cast<float>(channels));
        }
        const float frequency = tracker.frequency;
        detectedHz.store(frequency, std::memory_order_relaxed);
        float shift = p.pitch;
        if (p.mode != 0 && frequency > 0)
        {
            const float note = 69.0f + 12.0f * std::log2(frequency / 440.0f);
            shift += (p.mode == 1 ? std::round(note) : 72.0f) - note;
        }
        pitchSmoother.setTargetValue(juce::jlimit(-36.0f, 36.0f, shift));
        stretch.setTransposeSemitones(pitchSmoother.skip(count));
        // Formant is an absolute envelope shift. LINK synchronises its parameter
        // with Pitch in the processor; compensation avoids applying pitch twice.
        stretch.setFormantSemitones(formantSmoother.skip(count), true);
        stretch.setFormantBase(frequency > 0 ? frequency / static_cast<float>(sampleRate) : 0.0f);
        stretch.process(input.getArrayOfReadPointers(), count, wet.getArrayOfWritePointers(), count);

        for (int i = 0; i < count; ++i)
        {
            const float driveAmount = driveSmoother.getNextValue();
            const float gain = std::pow(10.0f, driveAmount * 1.2f);
            const float wetAmount = mixSmoother.getNextValue() * bypassSmoother.getNextValue();
            const int readPosition = (delayPosition - latencySamples + dryDelay.getNumSamples()) % dryDelay.getNumSamples();
            for (int ch = 0; ch < channels; ++ch)
            {
                const float x = input.getSample(ch, i);
                dryDelay.setSample(ch, delayPosition, x);
                const float dry = dryDelay.getSample(ch, readPosition);
                const float w = wet.getSample(ch, i);
                const float driven = w + driveAmount * (std::tanh(w * gain) / std::tanh(gain) - w);
                const float result = dry + wetAmount * (driven - dry);
                buffer.setSample(ch, offset + i, result);
                inputMaximum = juce::jmax(inputMaximum, std::abs(x));
                outputMaximum = juce::jmax(outputMaximum, std::abs(result));
            }
            delayPosition = (delayPosition + 1) % dryDelay.getNumSamples();
        }
    }
    inputPeak.store(inputMaximum, std::memory_order_relaxed);
    outputPeak.store(outputMaximum, std::memory_order_relaxed);
}
