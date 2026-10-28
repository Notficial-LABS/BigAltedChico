// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#include "SettingsPanel.h"
#include "BinaryData.h"
#include <cmath>

namespace
{
juce::Typeface::Ptr modalTypeface()
{
    static auto face = juce::Typeface::createSystemTypefaceFor(BinaryData::Roboto_ttf, BinaryData::Roboto_ttfSize);
    return face;
}
juce::Font modalFont(float height, bool bold = false)
{
    return juce::Font(juce::FontOptions(modalTypeface()).withHeight(height))
        .withStyle(bold ? juce::Font::bold : juce::Font::plain);
}
void text(juce::Graphics& g, float x, float baseline, const juce::String& value,
          float height = 13, juce::Colour colour = juce::Colour(0xffeceeed), bool bold = false)
{
    g.setColour(colour); g.setFont(modalFont(height, bold));
    g.drawSingleLineText(value, juce::roundToInt(x), juce::roundToInt(baseline));
}
const juce::Rectangle<float> panel { 270, 25, 660, 340 };
}

SettingsPanel::Skin::Skin()
{
    setDefaultSansSerifTypeface(modalTypeface());
    setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff202628));
    setColour(juce::PopupMenu::textColourId,juce::Colour(0xffeceeed));
    setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff5a2731));
    setColour(juce::ComboBox::textColourId,juce::Colour(0xffeceeed));
}
juce::Font SettingsPanel::Skin::getComboBoxFont(juce::ComboBox&) { return modalFont(13); }
void SettingsPanel::Skin::drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float>(0,0,static_cast<float>(width),static_cast<float>(height)).reduced(.5f);
    g.setColour(juce::Colour(0xff202628)); g.fillRoundedRectangle(r,4);
    g.setColour(juce::Colour(box.hasKeyboardFocus(true) ? 0xffa6b9c4 : 0xff596165)); g.drawRoundedRectangle(r,4,1);
    juce::Path arrow; const auto x = r.getRight()-14, y = r.getCentreY();
    arrow.startNewSubPath(x-3,y-2); arrow.lineTo(x,y+2); arrow.lineTo(x+3,y-2);
    g.strokePath(arrow,juce::PathStrokeType(1.4f));
}
void SettingsPanel::Skin::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                               const juce::Colour&, bool hover, bool down)
{
    const auto role = b.getProperties()["role"].toString();
    const auto r = b.getLocalBounds().toFloat().reduced(0.75f);
    if (role == "close" || role == "link")
    {
        if (hover || b.hasKeyboardFocus(true))
        { g.setColour(juce::Colours::white.withAlpha(0.07f)); g.fillRoundedRectangle(r, 3); }
        return;
    }
    const bool selected = b.getToggleState();
    g.setGradientFill(juce::ColourGradient((selected ? accent.darker(.7f) : juce::Colour(0xff202628)),
        r.getTopLeft(), (selected ? accent.darker(1.2f) : juce::Colour(0xff202628)), r.getBottomRight(), false));
    g.fillRoundedRectangle(r, 4);
    g.setColour((selected ? accent : juce::Colour((hover || down || b.hasKeyboardFocus(true)) ? 0xffa4b0b5 : 0xff596165)));
    g.drawRoundedRectangle(r, 4, selected ? 1.3f : 0.8f);
}
void SettingsPanel::Skin::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool hover, bool)
{
    const auto role = b.getProperties()["role"].toString();
    if (role == "close")
    {
        const auto c = b.getLocalBounds().toFloat().getCentre();
        g.setColour(juce::Colour(0xffbfc4c6));
        g.drawLine(c.x - 5, c.y - 5, c.x + 5, c.y + 5, 1.5f);
        g.drawLine(c.x + 5, c.y - 5, c.x - 5, c.y + 5, 1.5f);
        return;
    }
    const bool link = role == "link";
    g.setFont(modalFont(link ? 11.0f : 13.0f, !link));
    g.setColour(juce::Colour(link ? (hover ? 0xffd7e6ee : 0xffa6b9c4) : 0xfff5f5f1));
    g.drawText(b.getButtonText(), b.getLocalBounds(),
        link && b.getComponentID() != "companyLink" ? juce::Justification::centredLeft : juce::Justification::centred);
}

juce::Image SettingsPanel::createArtwork(const SkinDefinition& theme)
{
    // Cache at 2x; paint and resize never trace the SVG or rebuild its mask.
    juce::Image result(juce::Image::ARGB, 1320, 680, true);
    juce::Graphics g(result);
    g.addTransform(juce::AffineTransform::scale(2.0f).translated(-540, -50));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff292e30), 270, 25,
                                         juce::Colour(0xff181c1e), 732, 365, false));
    g.fillRoundedRectangle(panel, 10);
    juce::Path clip; clip.addRoundedRectangle(panel, 10); g.reduceClipRegion(clip);
    if (theme.modalBackground) theme.modalBackground->drawWithin(g,panel,juce::RectanglePlacement::stretchToFit,1);
    g.setColour(juce::Colours::white.withAlpha(0.025f));
    for (int y = 25; y < 365; y += 6)
        for (int x = 270; x < 930; x += 120)
        { g.drawLine(static_cast<float>(x), static_cast<float>(y), static_cast<float>(x + 80), static_cast<float>(y), 0.5f);
          g.drawLine(static_cast<float>(x + 35), static_cast<float>(y + 3), static_cast<float>(x + 120), static_cast<float>(y + 3), 0.5f); }

    auto logo = theme.modalLogo;
    if (!logo) logo = juce::Drawable::createFromImageData(BinaryData::NotficialLabsLogo_svg, BinaryData::NotficialLabsLogo_svgSize);
    jassert(logo != nullptr);
    if (logo == nullptr) return result;
    juce::Image pattern(juce::Image::ARGB, 1320, 680, true), mask(juce::Image::ARGB, 1320, 680, true);
    {
        juce::Graphics pg(pattern);
        pg.addTransform(juce::AffineTransform::scale(2.0f).translated(-540, -50));
        for (int y = 0; y < 390; y += 90)
            for (int x = 208; x < 1000; x += 104)
            {
                logo->drawWithin(pg, { static_cast<float>(x + 8), static_cast<float>(y + 8), 32, 31 }, juce::RectanglePlacement::centred, 1);
                logo->drawWithin(pg, { static_cast<float>(x + 60), static_cast<float>(y + 53), 32, 31 }, juce::RectanglePlacement::centred, 1);
            }
    }
    const juce::Rectangle<float> largeLogo { 480, 115, 240, 233 };
    {
        juce::Graphics mg(mask);
        mg.addTransform(juce::AffineTransform::scale(2.0f).translated(-540, -50));
        auto outline = logo->getOutlineAsPath();
        outline.applyTransform(juce::RectanglePlacement(juce::RectanglePlacement::centred).getTransformToFit(logo->getDrawableBounds(), largeLogo));
        mg.setColour(juce::Colours::white);
        mg.fillPath(outline);
        mg.strokePath(outline, juce::PathStrokeType(20, juce::PathStrokeType::curved));
        // Also exclude the X-shaped opening, so no repetitions show through it.
        mg.fillEllipse(532, 163.5f, 136, 136);
    }
    {
        juce::Image::BitmapData ink(pattern, juce::Image::BitmapData::readWrite);
        juce::Image::BitmapData exclusion(mask, juce::Image::BitmapData::readOnly);
        for (int y = 0; y < ink.height; ++y)
            for (int x = 0; x < ink.width; ++x)
                ink.setPixelColour(x, y, ink.getPixelColour(x, y).withMultipliedAlpha(
                    1.0f - exclusion.getPixelColour(x, y).getFloatAlpha()));
    }
    g.setOpacity(0.02f); g.drawImage(pattern, panel); g.setOpacity(1);
    logo->drawWithin(g, largeLogo, juce::RectanglePlacement::centred, 0.10f);
    return result;
}

SettingsPanel::SettingsPanel(std::function<void(float)> resizeEditor, std::function<void()> restoreFocus, std::function<void(const juce::String&)> chooseSkin)
    : selectSkinCallback(std::move(chooseSkin)), artwork(createArtwork(SkinDefinition::builtin(false))),
      resizeCallback(std::move(resizeEditor)), restoreFocusCallback(std::move(restoreFocus))
{
    setComponentID("settingsModal"); setTitle("BigAltedChico settings");
    setLookAndFeel(&skin); setWantsKeyboardFocus(true);
    setFocusContainerType(FocusContainerType::keyboardFocusContainer);
    setSize(1200, 390);
    auto setup = [this](juce::TextButton& b, const char* id, juce::Rectangle<int> bounds, const char* role = "") {
        b.setComponentID(id); b.getProperties().set("role", role); b.setBounds(bounds);
        b.setWantsKeyboardFocus(true); addAndMakeVisible(b);
    };
    setup(small, "size75", {300,143,90,31}); setup(normal, "size100", {402,143,90,31}); setup(large, "size125", {504,143,90,31});
    small.onClick = [this] { resizeCallback(0.75f); };
    normal.onClick = [this] { resizeCallback(1.0f); };
    large.onClick = [this] { resizeCallback(1.25f); };
    setup(close, "closeSettings", {893,31,28,28}, "close");
    close.setTitle("Close settings"); close.setTooltip("Close settings (Escape)"); close.onClick = [this] { dismiss(); };
    setup(company, "companyLink", {480,81,240,20}, "link");
    company.onClick = [] { juce::URL("https://github.com/Notficial-LABS").launchInDefaultBrowser(); };
    setup(developer, "developerLink", {300,251,305,20}, "link");
    developer.onClick = [] { juce::URL("https://github.com/byLeonardoMarcelo").launchInDefaultBrowser(); };
    setup(licenses, "licenseNotices", {655,280,240,22}, "link");
    licenses.onClick = [this] { showNotices(true); };
    setup(back, "backToSettings", {300,298,70,24}); back.onClick = [this] { showNotices(false); };
    back.setVisible(false);
    notices.setComponentID("licenseText"); notices.setTitle("Third-party license notices");
    notices.setMultiLine(true, true); notices.setReadOnly(true); notices.setScrollbarsShown(true);
    notices.onEscapeKey = [this] { showNotices(false); };
    notices.setFont(modalFont(12)); notices.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff171c1e));
    notices.setColour(juce::TextEditor::textColourId, juce::Colour(0xffe0e4e5));
    notices.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff596165));
    notices.setText(juce::String::fromUTF8(BinaryData::ThirdPartyNotices_txt, BinaryData::ThirdPartyNotices_txtSize));
    notices.setBounds(300,143,600,146); addChildComponent(notices);
    skinChoice.setComponentID("skinChoice"); skinChoice.setTitle("Skin");
    skinChoice.setBounds(350,182,244,26); addAndMakeVisible(skinChoice);
    skinChoice.onChange = [this] {
        const auto index = skinChoice.getSelectedId()-1;
        if (index >= 0 && index < static_cast<int>(skinIds.size())) selectSkinCallback(skinIds[static_cast<size_t>(index)]);
    };
}

SettingsPanel::~SettingsPanel() { exitModalState(0); setLookAndFeel(nullptr); }
void SettingsPanel::show(float editorScale)
{
    showNotices(false); updateScale(editorScale); setVisible(true); toFront(false);
    if (!isCurrentlyModal()) enterModalState(false, nullptr, false);
    if (isShowing()) normal.grabKeyboardFocus();
}
void SettingsPanel::dismiss()
{
    exitModalState(0); setVisible(false);
    if (restoreFocusCallback) restoreFocusCallback();
}
void SettingsPanel::updateScale(float editorScale)
{
    small.setToggleState(std::abs(editorScale - .75f) < .002f, juce::dontSendNotification);
    normal.setToggleState(std::abs(editorScale - 1.f) < .002f, juce::dontSendNotification);
    large.setToggleState(std::abs(editorScale - 1.25f) < .002f, juce::dontSendNotification);
}
void SettingsPanel::showNotices(bool show)
{
    viewingNotices = show;
    for (auto* b : { &small, &normal, &large, &developer, &licenses }) b->setVisible(!show);
    notices.setVisible(show); back.setVisible(show); skinChoice.setVisible(!show);
    if (isShowing()) { if (show) notices.grabKeyboardFocus(); else normal.grabKeyboardFocus(); }
    repaint();
}
bool SettingsPanel::keyPressed(const juce::KeyPress& key)
{
    if (key != juce::KeyPress::escapeKey) return false;
    if (viewingNotices) showNotices(false); else dismiss();
    return true;
}
void SettingsPanel::mouseDown(const juce::MouseEvent& e)
{
    if (!panel.contains(e.position)) dismiss();
}
void SettingsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.6f));
    juce::Path shape; shape.addRoundedRectangle(panel, 10);
    juce::DropShadow(juce::Colours::black.withAlpha(0.7f), 20, {0,7}).drawForPath(g, shape);
    g.setOpacity(1.0f);
    g.drawImage(artwork, panel);
    g.setColour(juce::Colour(0xff596063)); g.drawRoundedRectangle(panel.reduced(.5f), 10, 1);
    if (pluginLogo) pluginLogo->drawWithin(g,{400,33,400,30},juce::RectanglePlacement::centred,1);
    else
    {
    juce::GlyphArrangement title;
    title.addLineOfText(modalFont(27, true), "BigAltedChico", 0, 0);
    const auto bounds = title.getBoundingBox(0, -1, true);
    title.moveRangeOfGlyphs(0, -1, 600 - bounds.getCentreX(), 59);
    for (int i = 0; i < title.getNumGlyphs(); ++i)
    { g.setColour(juce::Colour(i < 3 ? 0xffef2e36 : 0xfff4f4f1)); title.getGlyph(i).draw(g); }
    }
    g.setFont(modalFont(13, true)); g.setColour(juce::Colour(0xffd6dadd));
    g.drawText("Notficial LABS", 400, 64, 400, 18, juce::Justification::centred);
    g.setColour(juce::Colour(0xff555d61).withAlpha(.65f));
    g.drawLine(294,109,906,109,.6f); g.drawLine(294,327,906,327,.6f);
    if (viewingNotices) text(g,300,133,"Licenses & Credits",13,juce::Colour(0xfff0f1ee),true);
    else
    {
        g.drawLine(633,129,633,314,.6f);
        const auto heading = juce::Colour(0xfff0f1ee);
        text(g,300,133,"UI Size",13,heading,true);
        text(g,300,199,"Skin",13,heading,true);
        text(g,300,232,"Developer",13,heading,true); text(g,300,249,"Leonardo Marcelo",14);
        text(g,300,291,"Contributors",12,heading,true); text(g,300,309,"No contributors yet.",11,juce::Colour(0xffa1a9ad));
        text(g,463,291,"Sponsors",12,heading,true); text(g,463,309,"No sponsors yet.",11,juce::Colour(0xffa1a9ad));
        text(g,655,133,"Licenses & Credits",13,heading,true);
        text(g,655,157,"Signalsmith Stretch & Linear",12); text(g,655,175,"Signalsmith Audio - MIT License",11,juce::Colour(0xffaeb7bc));
        text(g,655,202,"YIN pitch detection",12); text(g,655,227,"JUCE - Audio framework",12);
        text(g,655,252,"Roboto - SIL Open Font License 1.1",11);
    }
    auto version = juce::String(JucePlugin_VersionString);
    if (version.endsWith(".0")) version = version.dropLastCharacters(2);
    g.setColour(juce::Colour(0xffaab2b6)); g.setFont(modalFont(11));
    g.drawText("Version " + version, 400,335,400,20,juce::Justification::centred);
}

void SettingsPanel::setTheme(const SkinDefinition& theme)
{
    skin.accent = theme.salem ? theme.colour("accent") : juce::Colour(0xffef3941);
    skin.setColour(juce::PopupMenu::highlightedBackgroundColourId,skin.accent.darker(.9f));
    pluginLogo = theme.pluginLogo; artwork = createArtwork(theme); repaint();
}
void SettingsPanel::setSkins(const std::vector<std::pair<juce::String, juce::String>>& entries, const juce::String& selected, const juce::String& error)
{
    skinChoice.clear(juce::dontSendNotification); skinIds.clear(); int selection = 0;
    for (const auto& entry : entries)
    {
        skinIds.push_back(entry.first); const int id = static_cast<int>(skinIds.size());
        skinChoice.addItem(entry.second,id); if (entry.first == selected) selection = id;
    }
    skinChoice.addSeparator();
    for (auto entry : {std::make_pair("__import","Import skin..."), {"__reload","Reload skins"}, {"__folder","Open skins folder"}, {"__template","Get skin template..."}})
    { skinIds.push_back(entry.first); skinChoice.addItem(entry.second,static_cast<int>(skinIds.size())); }
    if (selection != 0 && error.isEmpty()) skinChoice.setSelectedId(selection,juce::dontSendNotification);
    else skinChoice.setText("Unavailable (Juguete)",juce::dontSendNotification);
    skinChoice.setTooltip(error.isEmpty() ? "Choose a skin or import a skin.json package." : error);
}
