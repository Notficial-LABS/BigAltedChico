// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#include "SkinDefinition.h"
#include "BinaryData.h"
#include <cmath>

juce::Rectangle<int> SkinDefinition::bounds(const juce::String& key) const
{
    const auto it = layout.find(key); return it == layout.end() ? juce::Rectangle<int>() : it->second;
}
juce::Colour SkinDefinition::colour(const juce::String& key) const
{
    const auto it = colours.find(key); return it == colours.end() ? juce::Colours::white : it->second;
}
SkinDefinition SkinDefinition::builtin(bool wave)
{
    SkinDefinition s; s.ondas = wave;
    s.layout = {{"previous",{44,6,34,34}}, {"next",{83,6,34,34}}, {"preset",{126,6,340,34}},
        {"save",{474,6,34,34}}, {"reset",{514,6,34,34}}, {"processing",{932,6,88,34}},
        {"settings",{1029,6,34,34}}, {"bypass",{1070,6,88,34}},
        {"pitch",{143,154,106,106}}, {"formant",{377,154,106,106}},
        {"drive",{777,154,106,106}}, {"mix",{957,154,106,106}},
        {"pitchValue",{148,264,96,27}}, {"formantValue",{382,264,96,27}},
        {"link",{297,190,30,30}}, {"transpose",{572,168,134,29}},
        {"quantize",{572,210,134,29}}, {"robot",{572,252,134,29}},
        {"pluginLogo",{100,316,250,32}}, {"brandLogo",{1000,313,102,30}},
        {"skinName",{550,365,100,16}}, {"modeLabel",{385,314,70,28}}};
    s.colours = {{"accent",juce::Colour(0xffe9b956)}, {"ring",juce::Colour(0xff78607f)},
        {"pointer",juce::Colour(0xfff1e4be)}, {"label",juce::Colour(0xff29131a)},
        {"text",juce::Colour(0xffeedcc4)}, {"muted",juce::Colour(0xffbb8393)},
        {"button",juce::Colour(0xff352030)}};
    if (wave)
    {
        s.id = "ondas"; s.name = "Ondas";
        s.layout["pitch"] = {168,111,152,152}; s.layout["formant"] = {522,111,152,152};
        s.layout["drive"] = {845,135,112,112}; s.layout["mix"] = {1025,135,112,112};
        s.layout["pitchValue"] = {200,273,88,28}; s.layout["formantValue"] = {554,273,88,28};
        s.layout["link"] = {395,144,58,58}; s.layout["transpose"] = {464,314,112,28};
        s.layout["quantize"] = {584,314,112,28}; s.layout["robot"] = {704,314,112,28};
        s.layout["pluginLogo"] = {40,342,245,32}; s.layout["brandLogo"] = {1050,340,115,34};
        s.background = juce::Drawable::createFromImageData(BinaryData::OndasBackground_svg, BinaryData::OndasBackground_svgSize);
        s.brandLogo = juce::Drawable::createFromImageData(BinaryData::NotficialLabsWordmark_svg, BinaryData::NotficialLabsWordmark_svgSize);
    }
    return s;
}
SkinDefinition SkinDefinition::makeSalem()
{
    auto s = builtin(true); s.salem = true; s.id = "salem"; s.name = "Salem";
    s.layout["previous"] = {34,12,32,32}; s.layout["next"] = {76,12,32,32};
    s.layout["preset"] = {122,12,350,32}; s.layout["save"] = {484,12,36,32}; s.layout["reset"] = {530,12,36,32};
    s.layout["processing"] = {878,12,100,32}; s.layout["settings"] = {992,12,40,32}; s.layout["bypass"] = {1045,12,120,32};
    s.layout["pitch"] = {110,119,156,156}; s.layout["formant"] = {484,119,156,156};
    s.layout["drive"] = {811,142,112,112}; s.layout["mix"] = {1009,142,112,112};
    s.layout["pitchValue"] = {139,278,98,23}; s.layout["formantValue"] = {513,278,98,23};
    s.layout["link"] = {354,170,56,42}; s.layout["transpose"] = {430,326,125,34};
    s.layout["quantize"] = {565,326,125,34}; s.layout["robot"] = {700,326,125,34};
    s.layout["pluginLogo"] = {43,330,235,28}; s.layout["brandLogo"] = {1041,327,114,32};
    s.layout["modeLabel"] = {344,329,62,28}; s.layout["skinName"] = {550,373,100,12};
    s.colours = {{"accent",juce::Colour(0xffe7a05a)}, {"ring",juce::Colour(0xff525b64)},
        {"pointer",juce::Colour(0xffe6e9ed)}, {"label",juce::Colour(0xffd4d9dd)},
        {"text",juce::Colour(0xffe7a05a)}, {"muted",juce::Colour(0xff939ca5)},
        {"button",juce::Colour(0xff20262c)}};
    s.background = juce::Drawable::createFromImageData(BinaryData::SalemBackground_svg,BinaryData::SalemBackground_svgSize);
    return s;
}

namespace
{
bool safeId(const juce::String& id)
{
    return id.isNotEmpty() && id.length() <= 64 && id.containsOnly("abcdefghijklmnopqrstuvwxyz0123456789-_");
}
bool selfContainedSVG(const juce::XmlElement& element)
{
    const juce::StringArray allowed { "svg","g","path","rect","circle","ellipse","line","polyline","polygon",
        "defs","linearGradient","radialGradient","stop","clipPath","title","desc" };
    if (!allowed.contains(element.getTagName())) return false;
    for (int i=0;i<element.getNumAttributes();++i)
    {
        const auto name = element.getAttributeName(i).toLowerCase();
        if (name.contains("href") || name.startsWith("on")) return false;
        auto value = element.getAttributeValue(i);
        while (value.containsIgnoreCase("url("))
        {
            value = value.substring(value.indexOfIgnoreCase("url(") + 4);
            if (!value.containsChar(')')) return false;
            const auto ref = value.upToFirstOccurrenceOf(")",false,false).trim().unquoted();
            if (!ref.startsWithChar('#') || !ref.containsOnly("#abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_:.") || ref.length()<2) return false;
            value = value.fromFirstOccurrenceOf(")",false,false);
        }
    }
    for (const auto* child : element.getChildIterator())
        if (!child->isTextElement() && !selfContainedSVG(*child)) return false;
    return true;
}
juce::Result asset(const juce::File& root, const juce::var& value, std::shared_ptr<juce::Drawable>& drawable)
{
    if (value.isVoid()) return juce::Result::ok();
    const auto name = value.toString();
    if (!value.isString() || name.isEmpty() || !name.containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.") || !name.endsWithIgnoreCase(".svg"))
        return juce::Result::fail("Artwork must be an SVG filename in the skin folder.");
    const auto file = root.getChildFile(name);
    if (!file.existsAsFile() || file.isSymbolicLink() || file.getSize() > 1024 * 1024)
        return juce::Result::fail("Missing or oversized SVG: " + name);
    const auto xml = file.loadFileAsString();
    // No external resources, embedded bitmaps, entities, scripts or CSS imports.
    for (const auto* denied : {"<!doctype", "<!entity", "<script", "<image", "<foreignobject", "@import", "data:", "file:", "http:", "https:"})
    {
        // Standard namespace declarations are not resource references.
        auto body = xml.replace("http://www.w3.org/2000/svg", "").replace("http://www.w3.org/1999/xlink", "");
        if (body.containsIgnoreCase(denied)) return juce::Result::fail("Unsupported SVG content: " + name);
    }
    if (xml.containsIgnoreCase("href=") || xml.containsIgnoreCase("href ="))
        return juce::Result::fail("SVG href references are not supported; expand symbols to paths.");
    auto parsed = juce::XmlDocument::parse(xml);
    if (!parsed || !parsed->hasTagName("svg") || !selfContainedSVG(*parsed)) return juce::Result::fail("Invalid or unsupported SVG: " + name);
    drawable = juce::Drawable::createFromImageData(xml.toRawUTF8(), static_cast<size_t>(xml.getNumBytesAsUTF8()));
    if (!drawable || drawable->getDrawableBounds().isEmpty()) return juce::Result::fail("Empty SVG: " + name);
    return juce::Result::ok();
}
}
juce::Result SkinDefinition::load(const juce::File& manifest, SkinDefinition& output)
{
    if (!manifest.existsAsFile() || manifest.isSymbolicLink() || manifest.getSize() > 65536)
        return juce::Result::fail("Missing or oversized skin.json (maximum 64 KB).");
    juce::var data;
    if (auto r = juce::JSON::parse(manifest.loadFileAsString(), data); r.failed()) return r;
    if (!data.isObject() || static_cast<int>(data["schemaVersion"]) != 1 || (data["extends"].toString() != "ondas" && data["extends"].toString() != "salem"))
        return juce::Result::fail("Expected schemaVersion 1 and extends: ondas or salem.");
    auto s = data["extends"].toString() == "salem" ? makeSalem() : builtin(true);
    s.id = data["id"].toString(); s.name = data["name"].toString();
    if (!safeId(s.id) || s.id == "juguete" || s.id == "ondas" || s.id == "salem" || s.name.isEmpty() || s.name.length() > 48)
        return juce::Result::fail("Choose a unique id (lowercase letters, digits, - or _) and a name up to 48 characters.");
    if (!data["controls"].isVoid())
    {
        auto* controls = data["controls"].getDynamicObject();
        if (!controls) return juce::Result::fail("controls must be an object.");
        for (const auto& entry : controls->getProperties())
        {
            auto key = entry.name.toString(); auto* a = entry.value.getArray();
            if (s.layout.count(key) == 0 || a == nullptr || a->size() != 4)
                return juce::Result::fail("Unknown control or invalid rectangle: " + key);
            for (const auto& number : *a)
                if (!(number.isInt() || number.isDouble()) || !std::isfinite(static_cast<double>(number)) || std::abs(static_cast<double>(number)) > 1200)
                    return juce::Result::fail("Invalid coordinate: " + key);
            const juce::Rectangle<int> r(static_cast<int>((*a)[0]),static_cast<int>((*a)[1]),static_cast<int>((*a)[2]),static_cast<int>((*a)[3]));
            if (r.getWidth() < 16 || r.getHeight() < 12 || !juce::Rectangle<int>(0,0,1200,390).contains(r))
                return juce::Result::fail("Control outside the 1200 x 390 canvas: " + key);
            s.layout[key] = r;
        }
    }
    if (!data["colours"].isVoid())
    {
        auto* colours = data["colours"].getDynamicObject();
        if (!colours) return juce::Result::fail("colours must be an object.");
        for (const auto& entry : colours->getProperties())
        {
            auto key = entry.name.toString(), value = entry.value.toString();
            if (s.colours.count(key) == 0 || value.length() != 7 || !value.startsWithChar('#') || !value.substring(1).containsOnly("0123456789abcdefABCDEF"))
                return juce::Result::fail("Expected a known colour and #RRGGBB: " + key);
            s.colours[key] = juce::Colour::fromString("ff" + value.substring(1));
        }
    }
    for (auto pair : {std::make_pair("background", &s.background), {"pluginLogo", &s.pluginLogo}, {"brandLogo", &s.brandLogo}, {"modalLogo", &s.modalLogo}, {"modalBackground", &s.modalBackground}})
        if (auto r = asset(manifest.getParentDirectory(), data[pair.first], *pair.second); r.failed()) return r;
    output = std::move(s); return juce::Result::ok();
}
juce::File SkinDefinition::folder()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Notficial LABS/BigAltedChico/Skins");
}
std::vector<std::pair<juce::String, juce::String>> SkinDefinition::installed()
{
    std::vector<std::pair<juce::String, juce::String>> result {{"juguete","Juguete"},{"ondas","Ondas"},{"salem","Salem"}};
    for (const auto& dir : folder().findChildFiles(juce::File::findDirectories, false))
    {
        if (dir.isSymbolicLink()) continue;
        // Read metadata only here; SVG parsing happens only on selection/import.
        const auto file = dir.getChildFile("skin.json");
        if (file.getSize() > 65536) continue;
        const auto json = juce::JSON::parse(file);
        const auto id = json["id"].toString(), name = json["name"].toString();
        if (safeId(id) && id == dir.getFileName() && id != "juguete" && id != "ondas" && id != "salem" && name.isNotEmpty()) result.emplace_back(id,name.substring(0,48));
    }
    return result;
}
juce::Result SkinDefinition::install(const juce::File& manifest, SkinDefinition& output)
{
    SkinDefinition candidate;
    if (auto r = load(manifest,candidate); r.failed()) return r;
    if (auto r = folder().createDirectory(); r.failed()) return r;
    const auto target = folder().getChildFile(candidate.id);
    if (target.exists()) return juce::Result::fail("That skin id is already installed. Edit its installed files and use Reload skins, or choose a new id.");
    const auto staging = folder().getNonexistentChildFile(".import-", "", false);
    if (auto r = staging.createDirectory(); r.failed()) return r;
    const auto json = juce::JSON::parse(manifest);
    bool ok = manifest.copyFileTo(staging.getChildFile("skin.json"));
    for (const auto* key : {"background","pluginLogo","brandLogo","modalLogo","modalBackground"})
        if (json[key].isString())
        { const auto file = manifest.getParentDirectory().getChildFile(json[key].toString());
          ok = file.copyFileTo(staging.getChildFile(file.getFileName())) && ok; }
    // Revalidate the copied package before publishing its directory.
    SkinDefinition check;
    if (!ok || load(staging.getChildFile("skin.json"),check).failed() || !staging.moveFileTo(target))
    { staging.deleteRecursively(); return juce::Result::fail("Could not install the skin package."); }
    output = std::move(candidate); return juce::Result::ok();
}
juce::Result SkinDefinition::exportTemplate(juce::File& destination)
{
    if (auto r = folder().getParentDirectory().createDirectory(); r.failed()) return r;
    destination = folder().getParentDirectory().getNonexistentChildFile("Skin Template", "", true);
    if (auto r = destination.createDirectory(); r.failed()) return r;
    juce::MemoryInputStream input(BinaryData::SkinTemplate_zip, BinaryData::SkinTemplate_zipSize, false);
    juce::ZipFile zip(input);
    return zip.uncompressTo(destination);
}
