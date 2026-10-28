// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#pragma once
#include "PluginProcessor.h"
#include "SkinDefinition.h"
#include <juce_gui_extra/juce_gui_extra.h>
class SettingsPanel;

class HardwareLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    HardwareLookAndFeel();
    void setTheme(const SkinDefinition& value) { theme = value; }
    SkinDefinition theme = SkinDefinition::builtin(false);
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
};

class DigitalReadout final : public juce::Label
{
public:
    DigitalReadout();
    bool ondas = false;
    bool salem = false;
    juce::Colour ink { 0xffeedcc4 };
    void paint(juce::Graphics&) override;
};

class BigAltedChicoEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit BigAltedChicoEditor(BigAltedChicoProcessor&);
    ~BigAltedChicoEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void presetMenu();
    void settingsMenu();
    void selectSkin(const juce::String&, bool persist = true);
    void refreshSkins();
    void skinAction(const juce::String&);
    void paintOndas(juce::Graphics&);
    void choosePresetFile(bool save);
    void configureKnob(juce::Slider&, const juce::String&, double resetValue);
    static juce::Image createBackdrop();
    BigAltedChicoProcessor& audioProcessor;
    HardwareLookAndFeel skin;
    SkinDefinition definition = SkinDefinition::builtin(false);
    juce::String requestedSkin { "juguete" };
    juce::String skinError;
    juce::Image backdrop, logo;
    juce::Slider pitch, formant, drive, mix;
    DigitalReadout pitchValue, formantValue;
    juce::TextButton link, transpose { "TRANSPOSE" }, quantize { "QUANTIZE" }, robot { "ROBOT" };
    juce::TextButton previous, next, save, resetButton, settings, processing, bypass { "BYPASS" }, preset;
    juce::TooltipWindow tooltips { this, 450 };
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> pitchAttachment, formantAttachment, driveAttachment, mixAttachment;
    std::unique_ptr<ButtonAttachment> linkAttachment, bypassAttachment;
    std::unique_ptr<juce::ParameterAttachment> modeAttachment, processingAttachment;
    std::unique_ptr<juce::FileChooser> chooser, skinChooser;
    std::unique_ptr<SettingsPanel> settingsPanel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BigAltedChicoEditor)
};
