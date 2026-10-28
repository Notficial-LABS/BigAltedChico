// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <map>

// Version 1 skin packs contain data only; never scripts or audio parameters.
struct SkinDefinition
{
    juce::String id { "juguete" }, name { "Juguete" };
    bool ondas = false;
    bool salem = false;
    std::map<juce::String, juce::Rectangle<int>> layout;
    std::map<juce::String, juce::Colour> colours;
    std::shared_ptr<juce::Drawable> background, pluginLogo, brandLogo, modalLogo, modalBackground;
    juce::Rectangle<int> bounds(const juce::String& key) const;
    juce::Colour colour(const juce::String& key) const;
    static SkinDefinition builtin(bool ondas);
    static SkinDefinition makeSalem();
    static juce::Result load(const juce::File& manifest, SkinDefinition& result);
    static juce::File folder();
    static std::vector<std::pair<juce::String, juce::String>> installed();
    static juce::Result install(const juce::File& manifest, SkinDefinition& result);
    static juce::Result exportTemplate(juce::File& destination);
};
