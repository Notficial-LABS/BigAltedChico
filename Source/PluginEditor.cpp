// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#include "PluginEditor.h"
#include "SettingsPanel.h"
#include <cmath>
#include "BinaryData.h"

namespace
{
juce::Typeface::Ptr roboto()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::Roboto_ttf, BinaryData::Roboto_ttfSize);
    return typeface;
}
juce::Font font(float height, bool bold = true)
{
    return juce::Font(juce::FontOptions(roboto()).withHeight(height)).withStyle(bold ? juce::Font::bold : juce::Font::plain);
}
void label(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> bounds, float size, juce::Justification align = juce::Justification::centred)
{
    g.setFont(font(size));
    g.setColour(juce::Colour(0xffe4e2d6));
    g.drawText(text, bounds, align);
}
void screw(juce::Graphics& g, float x, float y)
{
    g.setColour(juce::Colours::black.withAlpha(0.85f));
    g.fillEllipse(x - 10, y - 8, 21, 21);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffb5aa87), x - 6, y - 7, juce::Colour(0xff28231a), x + 6, y + 8, false));
    g.fillEllipse(x - 8, y - 8, 16, 16);
    g.setColour(juce::Colour(0xff171813));
    g.drawLine(x - 5, y - 4, x + 5, y + 4, 3);
    g.setColour(juce::Colour(0xffc8c2a0).withAlpha(0.4f));
    g.drawLine(x - 5, y - 2, x + 4, y + 5, 0.7f);
}
}

HardwareLookAndFeel::HardwareLookAndFeel() { setDefaultSansSerifTypeface(roboto()); }

void HardwareLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float position, float start, float end, juce::Slider&)
{
    if (theme.salem)
    {
        const float size = static_cast<float>(juce::jmin(width,height));
        const juce::Point<float> c(static_cast<float>(x)+static_cast<float>(width)/2,static_cast<float>(y)+static_cast<float>(height)/2);
        const float radius = size * .405f;
        const auto r = juce::Rectangle<float>(radius*2,radius*2).withCentre(c);
        for (int i=0;i<=40;++i)
        {
            const float a = start + (end-start)*static_cast<float>(i)/40;
            const juce::Point<float> d(std::sin(a),-std::cos(a));
            g.setColour(theme.colour("muted").withAlpha(i%5==0 ? .95f : .6f));
            g.drawLine(juce::Line<float>(c+d*size*.45f,c+d*size*(i%5==0 ? .495f : .478f)),i%5==0 ? 1.1f : .65f);
        }
        juce::Path disc; disc.addEllipse(r);
        juce::DropShadow(juce::Colours::black.withAlpha(.8f),10,{0,5}).drawForPath(g,disc);
        g.setColour(juce::Colour(0xff080c10)); g.fillEllipse(r.expanded(1));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffa4adb5),r.getTopLeft(),juce::Colour(0xff141a20),r.getBottomRight(),false));
        g.fillEllipse(r); g.setColour(juce::Colour(0xff0a1015)); g.fillEllipse(r.reduced(1.5f));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3d444c),r.getTopLeft(),juce::Colour(0xff11171d),r.getBottomRight(),false));
        g.fillEllipse(r.reduced(3));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff2a3036),c.x-radius*.4f,c.y-radius*.8f,juce::Colour(0xff111619),c.x+radius*.5f,c.y+radius*.8f,false));
        g.fillEllipse(r.reduced(5));
        g.setColour(juce::Colour(0xff9fa9b1).withAlpha(.20f)); g.drawEllipse(r.reduced(5),.6f);
        const float a=start+position*(end-start); const juce::Point<float> d(std::sin(a),-std::cos(a));
        g.setColour(theme.colour("pointer")); g.drawLine(juce::Line<float>(c+d*radius*.27f,c+d*radius*.87f),juce::jmax(1.8f,size*.015f));
        g.setColour(theme.colour("accent")); g.drawLine(juce::Line<float>(c+d*size*.424f,c+d*size*.444f),1.8f);
        return;
    }
    if (theme.ondas)
    {
        const float diameter = static_cast<float>(juce::jmin(width, height));
        const juce::Point<float> c(static_cast<float>(x) + static_cast<float>(width) / 2, static_cast<float>(y) + static_cast<float>(height) / 2);
        const auto r = juce::Rectangle<float>(diameter * .83f, diameter * .83f).withCentre(c);
        const float radius = r.getWidth() * .5f;
        for (int i = 0; i <= 20; ++i)
        {
            const float angle = start + (end - start) * static_cast<float>(i) / 20;
            const auto point = c + juce::Point<float>(std::sin(angle), -std::cos(angle)) * (diameter * .48f);
            g.setColour(theme.colour("label")); g.fillEllipse(point.x - 1.4f, point.y - 1.4f, 2.8f, 2.8f);
        }
        juce::Path disc; disc.addEllipse(r);
        juce::DropShadow(juce::Colours::black.withAlpha(.6f), 7, {0,4}).drawForPath(g,disc);
        const auto ring = theme.colour("ring");
        g.setGradientFill(juce::ColourGradient(ring.brighter(.35f), r.getTopLeft(), ring.darker(.6f), r.getBottomRight(), false));
        g.fillEllipse(r);
        g.setColour(ring.brighter(.45f)); g.drawEllipse(r.reduced(1), 1);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff2c292b),r.getTopLeft(),juce::Colour(0xff0d0e11),r.getBottomRight(),false));
        g.fillEllipse(r.reduced(radius * .19f));
        g.setColour(juce::Colour(0xff110d14)); g.drawEllipse(r.reduced(radius * .17f),2);
        g.setColour(ring.brighter(.4f).withAlpha(.4f)); g.drawEllipse(r.reduced(radius * .23f),.6f);
        const float angle = start + position * (end - start);
        const juce::Point<float> direction(std::sin(angle),-std::cos(angle));
        g.setColour(theme.colour("pointer"));
        g.drawLine(juce::Line<float>(c + direction * radius * .23f,c + direction * radius * .75f),juce::jmax(2.f,diameter * .023f));
        return;
    }
    const float diameter = static_cast<float>(juce::jmin(width, height)) * 0.83f;
    const float radius = diameter / 2;
    const juce::Point<float> centre(static_cast<float>(x) + static_cast<float>(width) / 2,
                                    static_cast<float>(y) + static_cast<float>(height) / 2);
    const auto bounds = juce::Rectangle<float>(diameter, diameter).withCentre(centre);
    juce::Path body;
    for (int i = 0; i < 64; ++i)
    {
        const float a = static_cast<float>(i) * juce::MathConstants<float>::twoPi / 64;
        const float r = radius * (i % 8 == 0 ? 1.025f : 1.0f);
        const auto point = centre + juce::Point<float>(std::sin(a) * r, -std::cos(a) * r);
        if (i == 0) body.startNewSubPath(point); else body.lineTo(point);
    }
    body.closeSubPath();
    juce::DropShadow(juce::Colours::black.withAlpha(0.65f), 8, { 2, 5 }).drawForPath(g, body);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff6c6c68), bounds.getTopLeft(), juce::Colour(0xff0b0d0e), bounds.getBottomRight(), false));
    g.fillPath(body);
    g.setColour(juce::Colour(0xff111314));
    g.strokePath(body, juce::PathStrokeType(2));
    g.setColour(juce::Colour(0xff858783).withAlpha(0.7f));
    g.drawEllipse(bounds.reduced(2), 1.2f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3e4142), bounds.getTopLeft(), juce::Colour(0xff101213), bounds.getBottomRight(), false));
    g.fillEllipse(bounds.reduced(5));
    for (int i = 0; i < 12; ++i)
    {
        const float a = static_cast<float>(i) * juce::MathConstants<float>::twoPi / 12;
        juce::Path facet;
        facet.startNewSubPath(centre);
        facet.lineTo(centre + juce::Point<float>(std::sin(a), -std::cos(a)) * (radius - 6));
        facet.lineTo(centre + juce::Point<float>(std::sin(a + 0.45f), -std::cos(a + 0.45f)) * (radius - 6));
        facet.closeSubPath();
        g.setColour((i % 3 == 0 ? juce::Colours::white : juce::Colours::black).withAlpha(i % 3 == 0 ? 0.07f : 0.17f));
        g.fillPath(facet);
    }
    const float a = start + position * (end - start);
    const juce::Point<float> direction(std::sin(a), -std::cos(a));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.8f));
    const juce::Point<float> shadowOffset(1, 1);
    g.drawLine(juce::Line<float>(centre + direction * (radius * 0.10f) + shadowOffset, centre + direction * (radius * 0.91f) + shadowOffset), 3);
    g.setColour(juce::Colour(0xffcecec1));
    g.drawLine(juce::Line<float>(centre + direction * (radius * 0.12f), centre + direction * (radius * 0.90f)), 2.1f);
}

void HardwareLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool hover, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced(1);
    const auto role = b.getProperties()["role"].toString();
    if (theme.salem)
    {
        const bool active = b.getToggleState();
        const auto corner = role == "preset" ? 3.f : 5.f;
        g.setColour(juce::Colour(0xff070b0f)); g.fillRoundedRectangle(r,corner);
        auto face=r.reduced(2);
        g.setGradientFill(juce::ColourGradient(juce::Colour(role=="preset" ? 0xff0c1115 : down ? 0xff171d23 : 0xff363e46),face.getTopLeft(),juce::Colour(0xff141a20),face.getBottomRight(),false));
        g.fillRoundedRectangle(face,corner);
        g.setColour(active ? theme.colour("accent") : theme.colour("ring").brighter(hover ? .45f : .1f));
        g.drawRoundedRectangle(face,corner,active ? 1.25f : .8f);
        if (!active) { g.setColour(juce::Colours::white.withAlpha(.18f)); g.drawHorizontalLine(juce::roundToInt(face.getY()+1),face.getX()+5,face.getRight()-5); }
        return;
    }
    if (role == "preset")
    {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff180503), 0, 0, juce::Colour(0xff350604), 0, r.getHeight(), false));
        g.fillRoundedRectangle(r, 2);
        g.setColour(hover ? juce::Colour(0xff9e4d36) : juce::Colour(0xff53544f));
        g.drawRoundedRectangle(r.reduced(0.5f), 2, 1.2f);
        return;
    }
    if (theme.ondas && (role == "mode" || role == "chain"))
    {
        const bool active = b.getToggleState();
        const auto top = active ? theme.colour("accent") : theme.colour("button").brighter(.15f);
        const auto bottom = active ? top.darker(.3f) : theme.colour("button").darker(.45f);
        g.setColour(juce::Colour(0xff130d14));
        const float corner = role == "chain" ? r.getWidth() * .5f : r.getHeight() * .5f;
        g.fillRoundedRectangle(r, corner);
        g.setGradientFill(juce::ColourGradient(down ? bottom : top, r.getTopLeft(), bottom, r.getBottomRight(), false));
        g.fillRoundedRectangle(r.reduced(2), corner);
        g.setColour((active ? theme.colour("accent").brighter(.5f) : theme.colour("ring")).withAlpha(hover ? 1.f : .7f));
        g.drawRoundedRectangle(r.reduced(2),corner,1);
        return;
    }
    const bool led = role == "led";
    if (led) r = r.removeFromLeft(r.getHeight());
    g.setColour(juce::Colour(0xff050606));
    g.fillRoundedRectangle(r, led ? 1.0f : 5.0f);
    auto inner = r.reduced(led ? 3.0f : 4.0f);
    const bool active = b.getToggleState();
    const auto top = active ? juce::Colour(0xfffc4937) : juce::Colour(hover ? 0xffc2c5be : 0xffa2a8a2);
    const auto bottom = active ? juce::Colour(0xff9d110b) : juce::Colour(0xff444c4c);
    g.setGradientFill(juce::ColourGradient(down ? bottom : top, inner.getTopLeft(), bottom, inner.getBottomRight(), false));
    g.fillRoundedRectangle(inner, led ? 0.5f : 2.5f);
    g.setColour(juce::Colours::white.withAlpha(0.22f));
    g.drawRoundedRectangle(inner.reduced(1), 1.5f, 0.8f);
    if (led)
    {
        g.setColour(active ? juce::Colour(0xffff3321).withAlpha(0.68f) : juce::Colour(0xff818d89).withAlpha(0.25f));
        g.fillRoundedRectangle(inner.reduced(3), 2);
    }
}

void HardwareLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const auto role = b.getProperties()["role"].toString();
    if (theme.ondas && role == "chain")
    {
        const auto c = r.getCentre(); const float u = r.getWidth() / 58;
        g.setColour(b.getToggleState() ? theme.colour(theme.salem ? "accent" : "label") : theme.colour("muted"));
        g.drawRoundedRectangle(c.x - 13*u,c.y - 5*u,17*u,10*u,5*u,2*u);
        g.drawRoundedRectangle(c.x - 3*u,c.y - 5*u,17*u,10*u,5*u,2*u);
        return;
    }
    if (theme.ondas && role == "mode")
    {
        g.setColour(b.getToggleState() ? theme.colour(theme.salem ? "accent" : "label") : theme.colour("muted"));
        g.setFont(font(r.getHeight() * .48f));
        g.drawText(b.getButtonText(),r,juce::Justification::centred); return;
    }
    if (theme.salem && b.getComponentID() == "processing")
    {
        const auto c = r.getCentre();
        g.setColour(b.getToggleState() ? theme.colour("accent") : theme.colour("ring"));
        g.fillEllipse(r.getX()+12,c.y-3,6,6);
        g.setColour(theme.colour("label")); g.setFont(font(r.getHeight()*.43f));
        g.drawText("LIVE",r.withTrimmedLeft(16),juce::Justification::centred); return;
    }
    if (role == "led")
    {
        label(g, b.getButtonText(), r.withTrimmedLeft(r.getHeight() + 8), r.getHeight() * 0.40f, juce::Justification::centredLeft);
        return;
    }
    if (role == "preset")
    {
        g.setColour(theme.salem ? theme.colour("accent") : juce::Colour(0xffff3323));
        g.setFont(font(r.getHeight() * (theme.salem ? .5f : .58f)));
        g.drawText(b.getButtonText(), r.reduced(14, 1), juce::Justification::centredLeft);
        return;
    }
    const auto icon = b.getProperties()["icon"].toString();
    g.setColour(theme.salem ? theme.colour(b.getToggleState() ? "accent" : "label") : juce::Colour(0xff141a1b));
    if (icon.isEmpty())
    {
        g.setFont(font(r.getHeight() * 0.45f));
        g.drawText(b.getButtonText(), r.reduced(6, 1), juce::Justification::centred);
        return;
    }
    const auto c = r.getCentre();
    const float s = r.getHeight() * 0.26f;
    juce::Path p;
    if (icon == "left" || icon == "right")
    {
        const float sign = icon == "left" ? 1.0f : -1.0f;
        p.startNewSubPath(c.x + sign * s * 0.4f, c.y - s);
        p.lineTo(c.x - sign * s * 0.6f, c.y);
        p.lineTo(c.x + sign * s * 0.4f, c.y + s);
    }
    else if (icon == "save")
    {
        p.addRectangle(c.x - s, c.y - s, 2 * s, 2 * s);
        p.addRectangle(c.x - s * 0.4f, c.y - s, s * 0.7f, s * 0.8f);
        p.addRectangle(c.x - s * 0.6f, c.y + s * 0.3f, s * 1.2f, s * 0.7f);
    }
    else if (icon == "reset")
    {
        p.addCentredArc(c.x, c.y + 1, s, s, 0, -0.2f, 5.0f, true);
        p.startNewSubPath(c.x - s * 0.2f, c.y - s * 1.35f);
        p.lineTo(c.x - s * 0.7f, c.y - s * 0.6f);
        p.lineTo(c.x + s * 0.1f, c.y - s * 0.4f);
    }
    else
    {
        // Eight teeth centred on the cardinal and diagonal directions (+ and x).
        // Flat outer tips, recessed roots, and a circular centre hole.
        for (int tooth = 0; tooth < 8; ++tooth)
        {
            const float centreAngle = static_cast<float>(tooth) * juce::MathConstants<float>::pi / 4;
            constexpr float offsets[] { -22.5f, -11.0f, 11.0f, 22.5f };
            for (int corner = 0; corner < 4; ++corner)
            {
                const float a = centreAngle + juce::degreesToRadians(offsets[corner]);
                const float radius = s * ((corner == 0 || corner == 3) ? 0.76f : 1.15f);
                const auto point = c + juce::Point<float>(std::sin(a), -std::cos(a)) * radius;
                if (tooth == 0 && corner == 0) p.startNewSubPath(point); else p.lineTo(point);
            }
        }
        p.closeSubPath();
        p.addEllipse(c.x - s * 0.40f, c.y - s * 0.40f, s * 0.80f, s * 0.80f);
        p.setUsingNonZeroWinding(false);
        g.fillPath(p);
        return;
    }
    g.strokePath(p, juce::PathStrokeType(r.getHeight() * 0.085f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

DigitalReadout::DigitalReadout()
{
    setEditable(false, true, false);
    setJustificationType(juce::Justification::centredRight);
    setColour(juce::Label::textWhenEditingColourId, juce::Colour(0xffff332b));
    setColour(juce::Label::backgroundWhenEditingColourId, juce::Colour(0xff200401));
    setTooltip("Double-click to enter a value in semitones.");
}

void DigitalReadout::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    if (ondas && !salem)
    {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0d0806),0,0,juce::Colour(0xff27150a),0,r.getHeight(),false));
        g.fillRoundedRectangle(r.reduced(.6f),5);
        g.setColour(juce::Colour(0xff72552f)); g.drawRoundedRectangle(r.reduced(.6f),5,1);
        if (!isBeingEdited()) { g.setColour(ink); g.setFont(::font(r.getHeight() * .82f,false)); g.drawText(getText(),r,juce::Justification::centred); }
        return;
    }
    g.setColour(juce::Colour(salem ? 0xff0a1014 : 0xff080101)); g.fillRect(r);
    g.setColour(juce::Colour(salem ? 0xff49525b : 0xff553f26)); g.drawRect(r, 1.2f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff000000), 0, 0, juce::Colour(salem ? 0xff10161b : 0xff300301), 0, r.getHeight(), false));
    g.fillRect(r.reduced(2));
    if (isBeingEdited()) return;
    const float h = salem ? r.getHeight() * .68f : r.getHeight() - 9;
    const float w = h * 0.53f, spacing = w * 0.32f;
    float total = 0;
    for (auto c : getText()) total += c == '.' ? spacing * 1.5f : w + spacing;
    float x = salem ? (r.getWidth()-total)*.5f : r.getRight() - total - 7;
    const float y = salem ? r.getHeight() * .16f : 4.5f;
    constexpr int digitMasks[] { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
    for (auto c : getText())
    {
        if (c == '.') { g.setColour((salem ? ink : juce::Colour(0xffff3027))); g.fillEllipse(x, y + h - 2, 2.5f, 2.5f); x += spacing * 1.5f; continue; }
        const int mask = c >= '0' && c <= '9' ? digitMasks[c - '0'] : c == '-' ? 0x40 : 0;
        const std::array<juce::Line<float>, 7> segments {
            juce::Line<float>(x + 1, y, x + w - 1, y), { x + w, y + 1, x + w, y + h / 2 - 1 },
            { x + w, y + h / 2 + 1, x + w, y + h - 1 }, { x + 1, y + h, x + w - 1, y + h },
            { x, y + h / 2 + 1, x, y + h - 1 }, { x, y + 1, x, y + h / 2 - 1 }, { x + 1, y + h / 2, x + w - 1, y + h / 2 }
        };
        for (int i = 0; i < 7; ++i)
        {
            g.setColour((mask & (1 << i)) != 0 ? (salem ? ink : juce::Colour(0xffff3027)) : juce::Colour(salem ? 0xff302319 : 0xff3b0604));
            g.drawLine(segments[static_cast<size_t>(i)], juce::jmax(1.3f, h * 0.11f));
        }
        x += w + spacing;
    }
}

juce::Image BigAltedChicoEditor::createBackdrop()
{
    juce::Image image(juce::Image::RGB, 1200, 390, true);
    juce::Graphics g(image);
    g.fillAll(juce::Colour(0xff653b21));
    juce::Random random(82147);
    for (int i = 0; i < 1300; ++i)
    {
        const float x = random.nextFloat() * 1200, y = 46 + random.nextFloat() * 344;
        g.setColour((i % 2 ? juce::Colour(0xffd58f50) : juce::Colour(0xff1c110b)).withAlpha(0.18f));
        g.drawLine(x, y, x + random.nextFloat() * 180, y + random.nextFloat() * 2, 0.3f + random.nextFloat());
    }
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3b4143), 42, 70, juce::Colour(0xff181e20), 900, 360, false));
    g.fillRoundedRectangle(42, 70, 1116, 290, 5);
    g.setColour(juce::Colour(0xff030708)); g.drawRoundedRectangle(45, 73, 1110, 284, 5, 5);
    for (int i = 0; i < 700; ++i)
    {
        const float x = 48 + random.nextFloat() * 1104, y = 77 + random.nextFloat() * 274;
        g.setColour(juce::Colours::white.withAlpha(random.nextFloat() * 0.035f));
        g.drawLine(x, y, x + random.nextFloat() * 5, y - random.nextFloat() * 4, 0.6f);
    }
    // Fine horizontal machining marks across the metal plate, deterministic at every size.
    for (int i = 0; i < 5200; ++i)
    {
        const float x = 51 + random.nextFloat() * 1092, y = 80 + random.nextFloat() * 268;
        const float length = juce::jmin(1148.0f - x, 12 + random.nextFloat() * 80);
        g.setColour((i % 3 == 0 ? juce::Colour(0xffc2ced0) : juce::Colour(0xff020506))
            .withAlpha(0.025f + random.nextFloat() * 0.055f));
        g.drawLine(x, y, x + length, y, 0.35f);
    }
    const std::array<juce::Rectangle<float>, 3> panels { juce::Rectangle<float>(96, 108, 428, 197), { 540, 108, 176, 197 }, { 732, 108, 372, 197 } };
    const std::array<juce::Colour, 3> colours { juce::Colour(0xffb18a12), juce::Colour(0xff125063), juce::Colour(0xffb31716) };
    for (size_t i = 0; i < panels.size(); ++i)
    {
        g.setGradientFill(juce::ColourGradient(colours[i].brighter(0.06f), panels[i].getTopLeft(), colours[i].darker(0.16f), panels[i].getBottomRight(), false));
        g.fillRoundedRectangle(panels[i], 2);
        g.setColour(juce::Colours::white.withAlpha(0.1f)); g.drawRoundedRectangle(panels[i].reduced(0.5f), 2, 1);
    }
    g.setColour(juce::Colour(0xff0c1011)); g.fillRect(0, 0, 1200, 46);
    g.setColour(juce::Colour(0xff3a3d3d)); g.drawLine(0, 1, 1200, 1, 1);
    g.setColour(juce::Colour(0xff080a0a)); g.drawLine(0, 46, 1200, 46, 2);
    screw(g, 68, 94); screw(g, 1132, 94); screw(g, 68, 337); screw(g, 1132, 337);
    return image;
}

BigAltedChicoEditor::BigAltedChicoEditor(BigAltedChicoProcessor& p)
    : AudioProcessorEditor(p), audioProcessor(p), backdrop(createBackdrop())
{
    setLookAndFeel(&skin);
    logo = juce::ImageCache::getFromMemory(BinaryData::NotficialLabs_png, BinaryData::NotficialLabs_pngSize);
    // Crop transparent padding without resampling the source artwork.
    auto ink = juce::Rectangle<int>();
    for (int y = 0; y < logo.getHeight(); ++y)
        for (int x = 0; x < logo.getWidth(); ++x)
            if (logo.getPixelAt(x, y).getAlpha() > 127)
                ink = ink.getUnion({ x, y, 1, 1 });
    if (!ink.isEmpty()) logo = logo.getClippedImage(ink);
    configureKnob(pitch, "Pitch: transpose in semitones. Double-click to reset.", 0);
    configureKnob(formant, "Formant: shift the vocal envelope in semitones.", 0);
    configureKnob(drive, "Drive: soft saturation of the processed signal.", 0);
    configureKnob(mix, "Mix: balance dry and processed audio. Clean starts dry; raise Mix to hear the effect.", 0);
    pitch.setComponentID("pitch"); formant.setComponentID("formant");
    drive.setComponentID("drive"); mix.setComponentID("mix");
    link.setComponentID("link"); bypass.setComponentID("bypass");
    previous.setComponentID("previous"); next.setComponentID("next"); preset.setComponentID("preset");
    save.setComponentID("save"); resetButton.setComponentID("reset");
    pitchValue.setComponentID("pitchValue"); formantValue.setComponentID("formantValue");
    transpose.setComponentID("transpose"); quantize.setComponentID("quantize"); robot.setComponentID("robot");
    pitchValue.onTextChange = [this] { pitch.setValue(pitchValue.getText().getDoubleValue(), juce::sendNotificationSync); };
    formantValue.onTextChange = [this] { formant.setValue(formantValue.getText().getDoubleValue(), juce::sendNotificationSync); };
    for (auto* button : { &link, &transpose, &quantize, &robot }) button->getProperties().set("role", "led");
    link.setClickingTogglesState(true);
    bypass.setClickingTogglesState(true);
    preset.getProperties().set("role", "preset");
    const std::array<std::pair<juce::TextButton*, const char*>, 5> icons {{ { &previous, "left" }, { &next, "right" }, { &save, "save" }, { &resetButton, "reset" }, { &settings, "settings" } }};
    for (auto [button, icon] : icons) button->getProperties().set("icon", icon);
    for (auto* component : std::array<juce::Component*, 18> { &pitch, &formant, &drive, &mix, &pitchValue, &formantValue, &link, &transpose, &quantize, &robot, &previous, &next, &save, &resetButton, &settings, &processing, &bypass, &preset })
        addAndMakeVisible(component);

    pitchAttachment = std::make_unique<SliderAttachment>(p.state, "pitch", pitch);
    formantAttachment = std::make_unique<SliderAttachment>(p.state, "formant", formant);
    driveAttachment = std::make_unique<SliderAttachment>(p.state, "drive", drive);
    mixAttachment = std::make_unique<SliderAttachment>(p.state, "mix", mix);
    linkAttachment = std::make_unique<ButtonAttachment>(p.state, "link", link);
    bypassAttachment = std::make_unique<ButtonAttachment>(p.state, "bypass", bypass);
    modeAttachment = std::make_unique<juce::ParameterAttachment>(*p.state.getParameter("mode"), [this](float mode) {
        transpose.setToggleState(mode < 0.5f, juce::dontSendNotification);
        quantize.setToggleState(mode >= 0.5f && mode < 1.5f, juce::dontSendNotification);
        robot.setToggleState(mode >= 1.5f, juce::dontSendNotification);
    });
    modeAttachment->sendInitialUpdate();
    transpose.onClick = [this] { modeAttachment->setValueAsCompleteGesture(0); };
    quantize.onClick = [this] { modeAttachment->setValueAsCompleteGesture(1); };
    robot.onClick = [this] { modeAttachment->setValueAsCompleteGesture(2); };
    previous.onClick = [this] { audioProcessor.setCurrentProgram((audioProcessor.getCurrentProgram() + audioProcessor.getNumPrograms() - 1) % audioProcessor.getNumPrograms()); timerCallback(); };
    next.onClick = [this] { audioProcessor.setCurrentProgram((audioProcessor.getCurrentProgram() + 1) % audioProcessor.getNumPrograms()); timerCallback(); };
    resetButton.onClick = [this] { audioProcessor.setCurrentProgram(0); timerCallback(); };
    save.onClick = [this] { choosePresetFile(true); };
    preset.onClick = [this] { presetMenu(); };
    settings.setComponentID("settings");
    settings.setTitle("Settings");
    settings.onClick = [this] { settingsMenu(); };
    processing.setComponentID("processing");
    processing.setButtonText("LIVE");
    processing.setClickingTogglesState(true);
    processingAttachment = std::make_unique<juce::ParameterAttachment>(*p.state.getParameter("processingMode"), [this](float value) {
        processing.setToggleState(value >= 0.5f, juce::dontSendNotification);
    });
    processingAttachment->sendInitialUpdate();
    processing.onClick = [this] {
        processingAttachment->setValueAsCompleteGesture(audioProcessor.state.getRawParameterValue("processingMode")->load() < 0.5f ? 1.0f : 0.0f);
    };
    processing.setTooltip("Quality / Live: switch while stopped. Changing latency clears audio history.");
    previous.setTooltip("Previous preset"); next.setTooltip("Next preset"); save.setTooltip("Save preset");
    resetButton.setTooltip("Reset sound controls"); settings.setTooltip("Settings, UI size and credits");
    bypass.setTooltip("Hear the latency-aligned original signal");
    link.setTooltip("Link Pitch and Formant. Enabling copies Pitch to Formant; either control then moves both.");
    transpose.setTooltip("Free transposition"); quantize.setTooltip("Tune to the nearest chromatic note"); robot.setTooltip("Tune the voice to MIDI 72 (523.25 Hz) + Pitch");
    setResizable(true, true);
    setResizeLimits(900, 293, 1800, 585);
    getConstrainer()->setFixedAspectRatio(1200.0 / 390.0);
    setSize(1200, 390);
    selectSkin(audioProcessor.getSkinId(), false);
    timerCallback();
    startTimerHz(24);
}

BigAltedChicoEditor::~BigAltedChicoEditor() { stopTimer(); settingsPanel.reset(); setLookAndFeel(nullptr); }

void BigAltedChicoEditor::configureKnob(juce::Slider& knob, const juce::String& tooltip, double resetValue)
{
    knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    knob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    knob.setDoubleClickReturnValue(true, resetValue);
    knob.setMouseDragSensitivity(220);
    knob.setTooltip(tooltip);
}

void BigAltedChicoEditor::paint(juce::Graphics& g)
{
    g.addTransform(juce::AffineTransform::scale(static_cast<float>(getWidth()) / 1200.0f, static_cast<float>(getHeight()) / 390.0f));
    if (definition.ondas) { paintOndas(g); return; }
    g.drawImageAt(backdrop, 0, 0);
    label(g, "PITCH", { 147, 126, 100, 24 }, 20);
    label(g, "FORMANT", { 370, 126, 130, 24 }, 20);
    label(g, "MODE", { 572, 126, 112, 24 }, 20);
    label(g, "DRIVE", { 780, 126, 100, 24 }, 20);
    label(g, "MIX", { 960, 126, 100, 24 }, 20);
    label(g, "LINK", { 290, 231, 44, 18 }, 13);
    g.setColour(juce::Colour(0xffe6d88d));
    g.drawLine(246, 205, 291, 205, 2); g.drawLine(334, 205, 379, 205, 2);
    label(g, "MIN", { 764, 245, 48, 20 }, 15);
    label(g, "MAX", { 850, 245, 48, 20 }, 15);
    label(g, "DRY", { 944, 245, 48, 20 }, 15);
    label(g, "WET", { 1030, 245, 48, 20 }, 15);
    juce::GlyphArrangement wordmark;
    wordmark.addLineOfText(font(32), "BigAltedChico", 100, 342);
    for (int i = 0; i < wordmark.getNumGlyphs(); ++i)
    {
        g.setColour(i < 3 ? juce::Colour(0xffef2e36) : juce::Colour(0xffeeece2));
        wordmark.getGlyph(i).draw(g);
    }
    // V11 white artwork on transparent alpha, preserving its wide aspect ratio.
    const juce::Rectangle<float> badge(1000, 313, 102, 30);
    g.setColour(juce::Colours::white);
    g.drawImage(logo, badge, juce::RectanglePlacement::centred, true);

}

void BigAltedChicoEditor::resized()
{
    const float sx = static_cast<float>(getWidth()) / 1200.0f, sy = static_cast<float>(getHeight()) / 390.0f;
    for (auto* c : std::array<juce::Component*,18> { &previous,&next,&preset,&save,&resetButton,&processing,&settings,&bypass,
        &pitch,&formant,&drive,&mix,&pitchValue,&formantValue,&link,&transpose,&quantize,&robot })
    {
        const auto r = definition.bounds(c->getComponentID()).toFloat();
        c->setBounds(juce::roundToInt(r.getX()*sx),juce::roundToInt(r.getY()*sy),juce::roundToInt(r.getWidth()*sx),juce::roundToInt(r.getHeight()*sy));
    }
    if (settingsPanel)
    {
        settingsPanel->setTransform(juce::AffineTransform::scale(sx, sy));
        settingsPanel->updateScale(sx);
    }
}

void BigAltedChicoEditor::timerCallback()
{
    if (audioProcessor.getSkinId() != requestedSkin) selectSkin(audioProcessor.getSkinId(), false);
    if (!pitchValue.isBeingEdited()) pitchValue.setText(juce::String(pitch.getValue(), 1), juce::dontSendNotification);
    if (!formantValue.isBeingEdited()) formantValue.setText(juce::String(formant.getValue(), 1), juce::dontSendNotification);
    const auto name = audioProcessor.getProgramName(audioProcessor.getCurrentProgram());
    if (preset.getButtonText() != name) preset.setButtonText(name);

}

void BigAltedChicoEditor::presetMenu()
{
    juce::PopupMenu m;
    for (int i = 0; i < audioProcessor.getNumPrograms(); ++i) m.addItem(i + 1, audioProcessor.getProgramName(i), true, i == audioProcessor.getCurrentProgram());
    m.addSeparator(); m.addItem(100, "Load preset..."); m.addItem(101, "Save preset...");
    m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&preset), [safe = juce::Component::SafePointer<BigAltedChicoEditor>(this)](int result) {
        if (safe == nullptr || result == 0) return;
        if (result == 100 || result == 101) safe->choosePresetFile(result == 101);
        else safe->audioProcessor.setCurrentProgram(result - 1);
        safe->timerCallback();
    });
}

void BigAltedChicoEditor::settingsMenu()
{
    if (!settingsPanel)
    {
        settingsPanel = std::make_unique<SettingsPanel>([this](float scale) {
            setSize(juce::roundToInt(1200 * scale), juce::roundToInt(390 * scale));
        }, [this] { if (isShowing()) settings.grabKeyboardFocus(); },
        [this](const juce::String& id) { skinAction(id); });
        addChildComponent(*settingsPanel);
    }
    refreshSkins();
    settingsPanel->setTheme(definition);
    resized();
    settingsPanel->show(static_cast<float>(getWidth()) / 1200.0f);
}

void BigAltedChicoEditor::paintOndas(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff21151e));
    if (definition.background)
    {
        juce::Graphics::ScopedSaveState saved(g); juce::Path edge; edge.addRoundedRectangle(0,0,1200,390,15);
        g.reduceClipRegion(edge); definition.background->drawWithin(g,{0,0,1200,390},juce::RectanglePlacement::stretchToFit,1);
    }
    const auto caption = [&](const juce::String& t, juce::Rectangle<float> r, float h, juce::Colour colour) {
        g.setColour(colour); g.setFont(font(h)); g.drawText(t,r,juce::Justification::centred);
    };
    for (const auto* id : {"pitch","formant","drive","mix"})
    {
        const auto r = definition.bounds(id).toFloat();
        caption(juce::String(id).toUpperCase(),{r.getX()-10,r.getY()-30,r.getWidth()+20,26},juce::String(id) == "pitch" || juce::String(id) == "formant" ? 23.f : 19.f,definition.colour("label"));
        const bool pitchControl = juce::String(id) == "pitch" || juce::String(id) == "formant";
        const bool mixControl = juce::String(id) == "mix";
        caption(pitchControl ? "-12" : mixControl ? "DRY" : "MIN",{r.getX()-9,r.getBottom()-5,43,20},13,definition.colour("label"));
        caption(pitchControl ? "+12" : mixControl ? "WET" : "MAX",{r.getRight()-34,r.getBottom()-5,43,20},13,definition.colour("label"));
    }
    const auto l = definition.bounds("link").toFloat();
    caption("LINK",{l.getX()-5,l.getBottom()+6,l.getWidth()+10,20},15,definition.colour("text"));
    caption("MODE",definition.bounds("modeLabel").toFloat(),15,definition.colour("muted"));
    caption(definition.name.toUpperCase(),definition.bounds("skinName").toFloat(),11,definition.colour("muted"));
    const auto logoBox = definition.bounds("pluginLogo").toFloat();
    if (definition.pluginLogo) definition.pluginLogo->drawWithin(g,logoBox,juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid,1);
    else
    {
        juce::GlyphArrangement wordmark; wordmark.addLineOfText(font(logoBox.getHeight()),"BigAltedChico",logoBox.getX(),logoBox.getBottom()-4);
        for (int i=0;i<wordmark.getNumGlyphs();++i) { g.setColour(juce::Colour(i<3 ? 0xffef2e36 : 0xffeeece2)); wordmark.getGlyph(i).draw(g); }
    }
    if (definition.brandLogo) definition.brandLogo->drawWithin(g,definition.bounds("brandLogo").toFloat(),juce::RectanglePlacement::centred,1);
}
void BigAltedChicoEditor::refreshSkins()
{
    if (settingsPanel) settingsPanel->setSkins(SkinDefinition::installed(),requestedSkin,skinError);
}
void BigAltedChicoEditor::selectSkin(const juce::String& id, bool persist)
{
    auto nextTheme = id == "salem" ? SkinDefinition::makeSalem() : SkinDefinition::builtin(id == "ondas");
    skinError.clear();
    if (id != "ondas" && id != "juguete" && id != "salem")
    {
        if (!id.containsOnly("abcdefghijklmnopqrstuvwxyz0123456789-_") || id.isEmpty()) skinError = "Invalid skin id; using Juguete.";
        else if (auto result = SkinDefinition::load(SkinDefinition::folder().getChildFile(id).getChildFile("skin.json"),nextTheme); result.failed())
            skinError = "Skin unavailable; using Juguete. " + result.getErrorMessage();
        if (skinError.isNotEmpty()) nextTheme = SkinDefinition::builtin(false);
    }
    requestedSkin = id; definition = std::move(nextTheme); skin.setTheme(definition);
    for (auto* readout : {&pitchValue,&formantValue})
    {
        readout->ondas = definition.ondas; readout->salem = definition.salem; readout->ink = definition.colour("text");
        readout->setColour(juce::Label::textWhenEditingColourId,definition.ondas ? definition.colour("text") : juce::Colour(0xffff332b));
        readout->setColour(juce::Label::backgroundWhenEditingColourId,juce::Colour(definition.salem ? 0xff10161b : 0xff200401));
    }
    for (auto* b : {&transpose,&quantize,&robot}) b->getProperties().set("role",definition.ondas ? "mode" : "led");
    link.getProperties().set("role",definition.ondas ? "chain" : "led");
    if (persist) audioProcessor.setSkinId(id);
    if (settingsPanel) settingsPanel->setTheme(definition);
    resized(); refreshSkins(); repaint();
}
void BigAltedChicoEditor::skinAction(const juce::String& id)
{
    if (id == "__folder") { SkinDefinition::folder().createDirectory(); SkinDefinition::folder().revealToUser(); refreshSkins(); return; }
    if (id == "__reload") { selectSkin(requestedSkin,false); return; }
    if (id == "__template")
    {
        juce::File destination; const auto result = SkinDefinition::exportTemplate(destination);
        if (result.wasOk()) destination.revealToUser();
        else juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Skin template",result.getErrorMessage());
        refreshSkins(); return;
    }
    if (id != "__import") { selectSkin(id); return; }
    refreshSkins();
    skinChooser = std::make_unique<juce::FileChooser>("Import skin.json",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),"*.json");
    skinChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safe = juce::Component::SafePointer<BigAltedChicoEditor>(this)](const juce::FileChooser& chooserResult) {
            if (!safe || chooserResult.getResult() == juce::File()) return;
            SkinDefinition loaded; const auto result = SkinDefinition::install(chooserResult.getResult(),loaded);
            if (result.wasOk()) safe->selectSkin(loaded.id);
            else juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Could not import skin",result.getErrorMessage());
        });
}

void BigAltedChicoEditor::choosePresetFile(bool shouldSave)
{
    chooser = std::make_unique<juce::FileChooser>(shouldSave ? "Save preset BigAltedChico" : "Load preset BigAltedChico", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("BigAltedChico.bigchico"), "*.bigchico");
    const int flags = juce::FileBrowserComponent::canSelectFiles | (shouldSave ? juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting : juce::FileBrowserComponent::openMode);
    chooser->launchAsync(flags, [safe = juce::Component::SafePointer<BigAltedChicoEditor>(this), shouldSave](const juce::FileChooser& dialog) {
        if (safe == nullptr || dialog.getResult() == juce::File()) return;
        auto file = dialog.getResult();
        if (shouldSave) file = file.withFileExtension("bigchico");
        const bool ok = shouldSave ? safe->audioProcessor.savePreset(file) : safe->audioProcessor.loadPreset(file);
        if (!ok) juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Preset", "Could not read or save the preset.");
        safe->timerCallback();
    });
}
