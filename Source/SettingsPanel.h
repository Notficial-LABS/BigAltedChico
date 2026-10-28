// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "SkinDefinition.h"

// Owned by the editor: no native window or blocking message loop.
// Coordinates are in the editor's 1200 x 390 design space.
class SettingsPanel final : public juce::Component
{
public:
    SettingsPanel(std::function<void(float)> resizeEditor, std::function<void()> restoreFocus, std::function<void(const juce::String&)> selectSkin);
    ~SettingsPanel() override;
    void show(float editorScale);
    void dismiss();
    void setSkins(const std::vector<std::pair<juce::String, juce::String>>&, const juce::String& selected, const juce::String& error = {});
    void setTheme(const SkinDefinition&);
    void updateScale(float editorScale);
    void paint(juce::Graphics&) override;
    bool keyPressed(const juce::KeyPress&) override;
    void mouseDown(const juce::MouseEvent&) override;
private:
    class Skin final : public juce::LookAndFeel_V4
    {
    public:
        Skin();
        juce::Colour accent { 0xffef3941 };
        juce::Font getComboBoxFont(juce::ComboBox&) override;
        void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
    } skin;
    void showNotices(bool);
    static juce::Image createArtwork(const SkinDefinition&);
    std::shared_ptr<juce::Drawable> pluginLogo;
    juce::ComboBox skinChoice;
    std::vector<juce::String> skinIds;
    std::function<void(const juce::String&)> selectSkinCallback;
    juce::Image artwork;
    juce::TextButton small { "75%" }, normal { "100%" }, large { "125%" };
    juce::TextButton close, company { "github.com/Notficial-LABS" };
    juce::TextButton developer { "github.com/byLeonardoMarcelo" };
    juce::TextButton licenses { "View license notices" }, back { "Back" };
    juce::TextEditor notices;
    bool viewingNotices = false;
    std::function<void(float)> resizeCallback;
    std::function<void()> restoreFocusCallback;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel)
};
