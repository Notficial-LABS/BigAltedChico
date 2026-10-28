// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SettingsPanel.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
juce::File previewDirectory()
{
    const auto dir = juce::File::getCurrentWorkingDirectory().getChildFile("Working/Previews");
    require(dir.createDirectory().wasOk(), "Cannot create Working/Previews");
    return dir;
}
void parameter(BigAltedChicoProcessor& p, const char* id, float value)
{
    auto* v = p.state.getParameter(id);
    v->setValueNotifyingHost(v->convertTo0to1(value));
}
float measureFrequency(const std::vector<float>& audio, double rate)
{
    std::vector<double> crossings;
    for (size_t i = audio.size() / 2 + 1; i < audio.size(); ++i)
        if (audio[i - 1] <= 0 && audio[i] > 0)
            crossings.push_back(static_cast<double>(i - 1) - audio[i - 1] / (audio[i] - audio[i - 1]));
    require(crossings.size() > 5, "Output is silent or has no measurable fundamental");
    const double coarse = static_cast<double>(crossings.size() - 1) * rate / (crossings.back() - crossings.front());
    double strongest = -1, best = coarse;
    const size_t begin = audio.size() / 2;
    for (int bin = -100; bin <= 100; ++bin)
    {
        const double frequency = coarse + static_cast<double>(bin) * 0.1;
        const double coefficient = 2 * std::cos(juce::MathConstants<double>::twoPi * frequency / rate);
        double a = 0, b = 0;
        for (size_t i = begin; i < audio.size(); ++i)
        {
            const double window = 0.5 - 0.5 * std::cos(juce::MathConstants<double>::twoPi * static_cast<double>(i - begin) / static_cast<double>(audio.size() - begin - 1));
            const double value = audio[i] * window + coefficient * a - b;
            b = a; a = value;
        }
        const double power = a * a + b * b - coefficient * a * b;
        if (power > strongest) { strongest = power; best = frequency; }
    }
    return static_cast<float>(best);
}
void checkAudio(double rate, int blockSize, int channels, float inputHz, float pitch, int mode, float expectedHz)
{
    BigAltedChicoProcessor p;
    p.setPlayConfigDetails(channels, channels, rate, blockSize);
    p.prepareToPlay(rate, blockSize);
    parameter(p, "mix", 1);
    parameter(p, "pitch", pitch); parameter(p, "mode", static_cast<float>(mode));
    std::vector<float> output;
    const int samples = static_cast<int>(rate * 1.4);
    for (int offset = 0; offset < samples; offset += blockSize)
    {
        const int count = juce::jmin(blockSize, samples - offset);
        juce::AudioBuffer<float> b(channels, count);
        for (int ch = 0; ch < channels; ++ch)
            for (int i = 0; i < count; ++i)
                b.setSample(ch, i, 0.25f * static_cast<float>(std::sin(juce::MathConstants<double>::twoPi * inputHz * (offset + i) / rate)));
        juce::MidiBuffer midi;
        p.processBlock(b, midi);
        for (int i = 0; i < count; ++i)
        {
            for (int ch = 0; ch < channels; ++ch) require(std::isfinite(b.getSample(ch, i)), "Nonfinite audio sample");
            output.push_back(b.getSample(0, i));
        }
    }
    const float measured = measureFrequency(output, rate);
    std::cout << "audio " << rate << " Hz / " << blockSize << " frames / " << channels << " ch: " << measured << " Hz (expected " << expectedHz << "), latency " << p.getLatencySamples() << '\n';
    require(std::abs(measured - expectedHz) < 2.0f, "Pitch result outside tolerance");
}
void checkDryAndBypass(bool bypass)
{
    constexpr int block = 257, rate = 48000;
    BigAltedChicoProcessor p;
    p.setPlayConfigDetails(2, 2, rate, block); p.prepareToPlay(rate, block);
    parameter(p, "mix", 1);
    parameter(p, "pitch", 12);
    parameter(p, bypass ? "bypass" : "mix", bypass ? 1.0f : 0.0f);
    const int latency = p.getLatencySamples();
    for (int offset = 0; offset < rate / 2; offset += block)
    {
        juce::AudioBuffer<float> b(2, block);
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < block; ++i)
                b.setSample(ch, i, static_cast<float>(std::sin(0.031 * (offset + i) + ch)) * 0.2f);
        juce::MidiBuffer midi; p.processBlock(b, midi);
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < block; ++i)
                if (offset + i > latency + 2000)
                    require(std::abs(b.getSample(ch, i) - static_cast<float>(std::sin(0.031 * (offset + i - latency) + ch)) * 0.2f) < 1e-6f, "Dry/bypass delay mismatch");
    }
    std::cout << (bypass ? "bypass" : "dry") << " delay alignment: OK\n";
}
std::vector<float> renderVoice(float formant, bool link, float drive, bool live = false)
{
    constexpr int rate = 48000, block = 256;
    VoiceEngine engine;
    engine.prepare(rate, 1, live);
    VoiceEngine::Controls controls;
    controls.pitch = 4; controls.formant = link ? controls.pitch : formant; controls.drive = drive;
    std::vector<float> output;
    for (int offset = 0; offset < 48000; offset += block)
    {
        juce::AudioBuffer<float> buffer(1, block);
        for (int i = 0; i < block; ++i)
        {
            double sample = 0;
            for (int harmonic = 1; harmonic <= 24; ++harmonic)
            {
                const double hz = harmonic * 175.0;
                const double weight = 0.12 / harmonic + std::exp(-std::pow((hz - 900) / 300, 2)) + 0.6 * std::exp(-std::pow((hz - 2400) / 450, 2));
                sample += weight * std::sin(juce::MathConstants<double>::twoPi * hz * (offset + i) / rate);
            }
            buffer.setSample(0, i, static_cast<float>(sample * 0.06));
        }
        engine.process(buffer, controls);
        for (int i = 0; i < block; ++i)
        {
            require(std::isfinite(buffer.getSample(0, i)), "Formant/drive generated nonfinite output");
            if (offset + i > 24000) output.push_back(buffer.getSample(0, i));
        }
    }
    return output;
}
void checkFormantLinkAndDrive(bool live = false)
{
    const auto baseline = renderVoice(0, false, 0, live);
    for (const auto& changed : { renderVoice(-6, false, 0, live), renderVoice(0, true, 0, live), renderVoice(0, false, 0.8f, live) })
    {
        double difference = 0;
        for (size_t i = 0; i < baseline.size(); ++i) difference += std::pow(changed[i] - baseline[i], 2);
        require(std::sqrt(difference / static_cast<double>(baseline.size())) > 0.001, "Formant, Link or Drive has no audible signal effect");
    }
    VoiceEngine silenceEngine; silenceEngine.prepare(48000, 2, live);
    juce::AudioBuffer<float> silence(2, 1024); silence.clear();
    for (int i = 0; i < 16; ++i) { silenceEngine.process(silence, {}); require(silence.getMagnitude(0, 1024) < 1e-7f, "Silence produced output"); }
    std::cout << "formant, link, drive and silence: OK\n";
}
void checkSkinPackages()
{
    for (const auto* id : {"juguete", "ondas", "salem"})
    {
        BigAltedChicoProcessor fresh; fresh.setSkinId(id);
        std::unique_ptr<juce::AudioProcessorEditor> editor(fresh.createEditor());
        require(fresh.getCurrentProgram()==0,"New instance is not Clean");
        for (const auto* control : {"pitch","formant","drive","mix"})
        {
            require(std::abs(fresh.state.getRawParameterValue(control)->load())<0.001f,"Clean parameter is not zero");
            auto* knob=dynamic_cast<juce::Slider*>(editor->findChildWithID(control));
            require(knob && std::abs(knob->getValue())<0.001,"Clean knob is not zero");
        }
        const auto shot=previewDirectory().getChildFile("preview-clean-"+juce::String(id)+".png");
        shot.deleteFile(); auto stream=shot.createOutputStream(); require(stream!=nullptr,"Clean snapshot unavailable");
        juce::PNGImageFormat png; require(png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds(),true,1.5f),*stream),"Clean snapshot failed");
        parameter(fresh,"mix",1); fresh.setCurrentProgram(0);
        require(fresh.state.getRawParameterValue("mix")->load()==0,"Clean preset did not reset Mix");
    }
    SkinDefinition salemTemplate;
    require(SkinDefinition::load(juce::File::getCurrentWorkingDirectory().getChildFile("SkinTemplate/salem-example.json"),salemTemplate).wasOk() && salemTemplate.salem,"Salem template cannot load");
    std::cout << "Clean defaults and reset in all three skins: OK\n";
    SkinDefinition imported;
    const auto original = juce::File::getCurrentWorkingDirectory().getChildFile("SkinTemplate/skin.json");
    auto result = SkinDefinition::load(original,imported);
    require(result.wasOk(),result.getErrorMessage().toRawUTF8());
    require(imported.ondas && imported.pluginLogo && imported.modalLogo && imported.brandLogo, "Template branding missing");
    juce::File exported;
    require(SkinDefinition::exportTemplate(exported).wasOk(), "Template export failed");
    require(SkinDefinition::load(exported.getChildFile("skin.json"),imported).wasOk(), "Exported template cannot be loaded");
    const auto manifest = exported.getChildFile("skin.json");
    const auto base = manifest.loadFileAsString();
    auto json = juce::JSON::parse(base);
    const auto unique = "checks-" + juce::Uuid().toString().removeCharacters("-");
    json.getDynamicObject()->setProperty("id",unique);
    manifest.replaceWithText(juce::JSON::toString(json));
    require(SkinDefinition::install(manifest,imported).wasOk(), "Valid custom skin import failed");
    const auto installed = SkinDefinition::folder().getChildFile(unique);
    const auto installedManifest = installed.getChildFile("skin.json").loadFileAsString();
    require(SkinDefinition::install(manifest,imported).failed(), "Import silently overwrites an installed skin");
    require(installed.getChildFile("skin.json").loadFileAsString() == installedManifest, "Duplicate import modified the installed pack");
    auto entries = SkinDefinition::installed(); bool found = false;
    for (const auto& entry : entries) found = found || entry.first == unique;
    require(found,"Imported skin not listed");
    {
        BigAltedChicoProcessor custom; custom.setSkinId(unique);
        std::unique_ptr<juce::AudioProcessorEditor> editor(custom.createEditor());
        require(editor->findChildWithID("pitch")->getX()==168,"Imported custom skin did not use its layout");
        const auto save = [&](const char* name) {
            const auto path = previewDirectory().getChildFile(name); path.deleteFile();
            auto stream = path.createOutputStream(); require(stream!=nullptr,"Custom snapshot path unavailable");
            juce::PNGImageFormat png; require(png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds(),true,1.5f),*stream),"Custom snapshot failed");
        };
        save("preview-custom-template.png");
        dynamic_cast<juce::TextButton*>(editor->findChildWithID("settings"))->onClick();
        save("preview-custom-template-settings.png");
        editor.reset();
        juce::MemoryBlock state; custom.getStateInformation(state);
        BigAltedChicoProcessor reopened; reopened.setStateInformation(state.getData(),static_cast<int>(state.getSize()));
        require(reopened.getSkinId()==unique,"Custom skin ID was not saved");
    }

    json.getDynamicObject()->setProperty("background","../outside.svg"); manifest.replaceWithText(juce::JSON::toString(json));
    require(SkinDefinition::load(manifest,imported).failed(),"Parent-directory artwork reference accepted");
    json = juce::JSON::parse(base); manifest.replaceWithText(juce::JSON::toString(json));
    const auto artwork = exported.getChildFile("background.svg"); const auto originalArtwork = artwork.loadFileAsString();
    artwork.replaceWithText("<svg xmlns=\"http://www.w3.org/2000/svg\"><rect width=\"1200\" height=\"390\" fill=\"url(other.svg#paint)\"/></svg>");
    require(SkinDefinition::load(manifest,imported).failed(),"External SVG paint resource accepted");
    artwork.replaceWithText(originalArtwork);
    json = juce::JSON::parse(base); json.getDynamicObject()->setProperty("schemaVersion",2); manifest.replaceWithText(juce::JSON::toString(json));
    require(SkinDefinition::load(manifest,imported).failed(),"Unsupported skin schema accepted");
    json = juce::JSON::parse(base); json["colours"].getDynamicObject()->setProperty("ring","invalid"); manifest.replaceWithText(juce::JSON::toString(json));
    require(SkinDefinition::load(manifest,imported).failed(),"Invalid colour accepted");
    json = juce::JSON::parse(base); json["controls"].getDynamicObject()->setProperty("settings",juce::var(juce::Array<juce::var>{1190,0,50,50})); manifest.replaceWithText(juce::JSON::toString(json));
    require(SkinDefinition::load(manifest,imported).failed(),"Off-canvas settings button accepted");
    require(installed.deleteRecursively() && exported.deleteRecursively(),"Cannot clean up generated skin test fixtures");
    {
        BigAltedChicoProcessor missing; missing.setSkinId(unique);
        std::unique_ptr<juce::AudioProcessorEditor> editor(missing.createEditor());
        require(editor->findChildWithID("pitch")->getX()==143 && missing.getSkinId()==unique,"Missing skin fallback changed saved ID or failed to use Juguete");
        dynamic_cast<juce::TextButton*>(editor->findChildWithID("settings"))->onClick();
        auto* modal = editor->findChildWithID("settingsModal");
        auto* combo = dynamic_cast<juce::ComboBox*>(modal->findChildWithID("skinChoice"));
        require(combo->getText().contains("Unavailable"),"Missing skin warning absent");
    }
    std::cout << "skin template export/import, duplicate protection and malformed packs: OK\n";
}

void checkStateAndUI(const juce::File& screenshot)
{
    BigAltedChicoProcessor p;
    p.setPlayConfigDetails(2, 2, 48000, 512); p.prepareToPlay(48000, 512);
    p.setCurrentProgram(3);
    juce::MemoryBlock saved; p.getStateInformation(saved);
    p.setCurrentProgram(0);
    p.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    require(std::abs(p.state.getRawParameterValue("formant")->load() + 3) < 0.001f, "Host state restore failed");
    require(p.getCurrentProgram() == 3, "Program restore failed");
    const auto presetFile = juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("bigchico-roundtrip", ".bigchico");
    require(p.savePreset(presetFile), "Preset save failed"); p.setCurrentProgram(0);
    require(p.loadPreset(presetFile), "Preset load failed"); presetFile.deleteFile();
    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
    auto* slider = dynamic_cast<juce::Slider*>(editor->findChildWithID("pitch"));
    require(slider != nullptr, "Pitch slider missing");
    slider->setValue(5, juce::sendNotificationSync);
    require(std::abs(p.state.getRawParameterValue("pitch")->load() - 5) < 0.001f, "Slider automation attachment failed");
    auto* robot = dynamic_cast<juce::TextButton*>(editor->findChildWithID("robot"));
    require(robot != nullptr, "Robot button missing"); robot->onClick();
    require(std::abs(p.state.getRawParameterValue("mode")->load() - 2) < 0.01f, "Mode attachment failed");
    p.setCurrentProgram(3);
    const auto image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.5f);
    screenshot.getParentDirectory().createDirectory();
    screenshot.deleteFile();
    if (auto stream = screenshot.createOutputStream())
    {
        juce::PNGImageFormat png;
        require(png.writeImageToStream(image, *stream), "Snapshot failed");
    }
    else require(false, "Cannot create screenshot file");
    auto* liveButton = dynamic_cast<juce::TextButton*>(editor->findChildWithID("processing"));
    require(liveButton != nullptr, "LIVE button missing");
    liveButton->onClick();
    require(liveButton->getToggleState(), "LIVE illumination not attached to state");
    require(p.state.getRawParameterValue("processingMode")->load() > 0.5f, "LIVE parameter attachment failed");
    const auto liveImage = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.5f);
    const auto liveShot = screenshot.getSiblingFile("preview-live.png"); liveShot.deleteFile();
    if (auto stream = liveShot.createOutputStream())
    { juce::PNGImageFormat png; require(png.writeImageToStream(liveImage, *stream), "Live snapshot failed"); }
    auto* settings = dynamic_cast<juce::TextButton*>(editor->findChildWithID("settings"));
    require(settings != nullptr, "Settings button missing");
    settings->onClick();
    auto* modal = dynamic_cast<SettingsPanel*>(editor->findChildWithID("settingsModal"));
    require(modal != nullptr && modal->isVisible() && modal->isCurrentlyModal(), "Settings did not open modally");
    require(slider->isCurrentlyBlockedByAnotherModalComponent(), "Modal allows underlying controls to receive input");
    const auto processingBefore = p.state.getRawParameterValue("processingMode")->load();
    for (auto percent : {75, 100, 125})
    {
        auto* size = dynamic_cast<juce::TextButton*>(modal->findChildWithID("size" + juce::String(percent)));
        require(size != nullptr, "UI size button missing"); size->onClick();
        require(editor->getWidth() == 12 * percent && editor->getHeight() == juce::roundToInt(3.9 * percent), "UI size did not resize editor");
        require(size->getToggleState() && modal->isVisible(), "Selected UI size or modal lost after resize");
        const auto shot = screenshot.getSiblingFile("preview-settings-" + juce::String(percent) + ".png");
        shot.deleteFile();
        auto stream = shot.createOutputStream(); require(stream != nullptr, "Cannot create modal screenshot");
        juce::PNGImageFormat png;
        require(png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.5f), *stream), "Modal snapshot failed");
    }
    auto* skinChoice = dynamic_cast<juce::ComboBox*>(modal->findChildWithID("skinChoice"));
    require(skinChoice != nullptr, "Skin dropdown missing");
    const auto pitchBeforeSkin = p.state.getRawParameterValue("pitch")->load();
    const auto latencyBeforeSkin = p.getLatencySamples();
    skinChoice->setSelectedId(2,juce::sendNotificationSync);
    require(p.getSkinId() == "ondas", "Skin dropdown did not select Ondas");
    require(slider->getX() == 210 && slider->getWidth() == 190, "Ondas control layout did not apply at 125%");
    require(std::abs(p.state.getRawParameterValue("pitch")->load()-pitchBeforeSkin)<0.001f && p.getLatencySamples()==latencyBeforeSkin,"Skin changed sound or latency");
    juce::MemoryBlock skinState; p.getStateInformation(skinState);
    BigAltedChicoProcessor restoredSkin; restoredSkin.setStateInformation(skinState.getData(),static_cast<int>(skinState.getSize()));
    require(restoredSkin.getSkinId()=="ondas", "Skin not restored with host session");
    { std::unique_ptr<juce::AudioProcessorEditor> restoredEditor(restoredSkin.createEditor());
      require(restoredEditor->findChildWithID("pitch")->getX()==168,"Recreated editor did not restore Ondas"); }
    const auto soundPreset = juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("skin-sound", ".bigchico");
    require(p.savePreset(soundPreset), "Cannot save sound preset"); p.setSkinId("juguete");
    require(p.loadPreset(soundPreset) && p.getSkinId()=="juguete", "Sound preset overwrote skin"); soundPreset.deleteFile();
    p.setSkinId("ondas");
    auto oldTree = p.state.copyState(); oldTree.removeProperty("uiSkin",nullptr); juce::MemoryBlock legacy;
    juce::AudioProcessor::copyXmlToBinary(*oldTree.createXml(),legacy);
    restoredSkin.setStateInformation(legacy.getData(),static_cast<int>(legacy.getSize()));
    require(restoredSkin.getSkinId()=="juguete","Legacy session did not default to Juguete");
    const auto snapshotSkin = [&](const juce::String& name) {
        const auto shot = screenshot.getSiblingFile(name); shot.deleteFile(); auto stream=shot.createOutputStream();
        require(stream!=nullptr,"Cannot write skin snapshot"); juce::PNGImageFormat png;
        require(png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds(),true,1.5f),*stream),"Skin snapshot failed");
    };
    for (auto percent : {75,100,125})
    {
        dynamic_cast<juce::TextButton*>(modal->findChildWithID("size"+juce::String(percent)))->onClick();
        snapshotSkin("preview-ondas-settings-"+juce::String(percent)+".png");
        modal->dismiss(); snapshotSkin("preview-ondas-"+juce::String(percent)+".png"); settings->onClick();
    }
    dynamic_cast<juce::TextButton*>(modal->findChildWithID("size100"))->onClick();
    skinChoice->setSelectedId(3,juce::sendNotificationSync);
    require(p.getSkinId()=="salem" && slider->getX()==110,"Salem layout did not apply");
    require(p.state.getRawParameterValue("pitch")->load()==pitchBeforeSkin && p.getLatencySamples()==latencyBeforeSkin,"Salem changed sound or latency");
    p.getStateInformation(skinState); restoredSkin.setStateInformation(skinState.getData(),static_cast<int>(skinState.getSize()));
    require(restoredSkin.getSkinId()=="salem" && restoredSkin.state.getRawParameterValue("pitch")->load()==pitchBeforeSkin,"Salem session restore failed");
    for (auto percent : {75,100,125})
    {
        dynamic_cast<juce::TextButton*>(modal->findChildWithID("size"+juce::String(percent)))->onClick();
        snapshotSkin("preview-salem-settings-"+juce::String(percent)+".png");
        modal->dismiss(); snapshotSkin("preview-salem-"+juce::String(percent)+".png"); settings->onClick();
    }
    dynamic_cast<juce::TextButton*>(modal->findChildWithID("size100"))->onClick();
    skinChoice->setSelectedId(1,juce::sendNotificationSync);
    require(p.getSkinId()=="juguete" && slider->getX()==143,"Cannot return to Juguete");
    snapshotSkin("preview-juguete-settings.png");
    std::cout << "Juguete/Ondas/Salem switching, parameter preservation, editor/session restore and legacy default: OK\n";
    auto* notices = dynamic_cast<juce::TextButton*>(modal->findChildWithID("licenseNotices"));
    require(notices != nullptr, "License link missing"); notices->onClick();
    auto* licenseText = dynamic_cast<juce::TextEditor*>(modal->findChildWithID("licenseText"));
    require(licenseText != nullptr && licenseText->isVisible() && licenseText->isReadOnly(), "License reader not available");
    require(licenseText->getText().contains("Permission is hereby granted") && licenseText->getText().contains("SIL OPEN FONT LICENSE"), "License notices incomplete");
    licenseText->onEscapeKey();
    require(!licenseText->isVisible() && modal->isVisible(), "Escape did not return from notices");
    modal->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
    require(!modal->isVisible() && !modal->isCurrentlyModal(), "Escape did not close settings");
    require(!slider->isCurrentlyBlockedByAnotherModalComponent(), "Modal left editor blocked");
    settings->onClick();
    auto* close = dynamic_cast<juce::TextButton*>(modal->findChildWithID("closeSettings"));
    require(close != nullptr, "Close button missing"); close->onClick();
    require(!modal->isVisible(), "Close button did not close settings");
    require(std::abs(p.state.getRawParameterValue("processingMode")->load() - processingBefore) < 0.001f, "Settings changed processing mode");
    settings->onClick(); // Exercise editor destruction while the modal is open.
    editor.reset();
    require(juce::Component::getNumCurrentlyModalComponents() == 0, "Destroyed editor left a modal component registered");
    std::cout << "settings modal, scale 75/100/125, licenses, dismissal and teardown: OK\n";
    std::cout << "state, presets, UI attachments: OK\npreview: " << screenshot.getFullPathName() << '\n';
}
}

#include "AudioMeasurements.h"

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI initialise;
    try
    {
        if (argc > 2 && juce::String(argv[2]) == "--preview-only")
        {
            checkSkinPackages();
            checkStateAndUI(juce::File(argv[1]));
            return 0;
        }
        const auto results = juce::File::getCurrentWorkingDirectory().getChildFile("Working/Tests");
        results.createDirectory();
        checkSkinPackages();
        checkLinkedParameters();
        checkLatencyAndSwitching(results);
        renderAudioComparisons(results);
        checkAudio(48000, 127, 1, 220, 12, 0, 440);
        checkAudio(44100, 1024, 2, 220, -12, 0, 110);
        checkAudio(96000, 257, 2, 220, 0, 0, 220);
        checkAudio(48000, 512, 1, 230, 0, 1, 233.082f);
        checkAudio(48000, 64, 1, 220, 0, 2, 523.251f);
        checkDryAndBypass(false); checkDryAndBypass(true);
        checkFormantLinkAndDrive(); checkFormantLinkAndDrive(true);
        checkStateAndUI(argc > 1 ? juce::File(argv[1]) : previewDirectory().getChildFile("preview.png"));
        std::cout << "All checks passed\n";
        return 0;
    }
    catch (const std::exception& e) { std::cerr << "FAILED: " << e.what() << '\n'; return 1; }
}
