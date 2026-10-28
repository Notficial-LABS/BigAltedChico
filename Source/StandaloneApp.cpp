// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Leonardo Marcelo / Notficial LABS

#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

// Preserve the device configuration and last skin, but start the standalone's
// sound from constructor defaults (Clean). Explicit Load State remains available.
class BigAltedChicoApplication final : public juce::JUCEApplication
{
public:
    BigAltedChicoApplication()
    {
        juce::PropertiesFile::Options options;
        options.applicationName = JucePlugin_Name;
        options.filenameSuffix = ".settings";
        options.osxLibrarySubFolder = "Application Support";
        appProperties.setStorageParameters(options);
    }
    const juce::String getApplicationName() override { return JucePlugin_Name; }
    const juce::String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override { return false; }
    void anotherInstanceStarted(const juce::String&) override
    { if (window) window->toFront(true); }
    void initialise(const juce::String&) override
    {
        auto* settings = appProperties.getUserSettings();
        const auto previousState = settings->getValue("filterState");
        juce::String selectedSkin = "juguete";
        juce::MemoryBlock data;
        if (data.fromBase64Encoding(previousState))
            if (auto xml = juce::AudioProcessor::getXmlFromBinary(data.getData(),static_cast<int>(data.getSize())))
                selectedSkin = xml->getStringAttribute("uiSkin","juguete");
        settings->removeValue("filterState");
        auto holder = std::make_unique<juce::StandalonePluginHolder>(settings,false,juce::String{},nullptr,juce::Array<juce::StandalonePluginHolder::PluginInOuts>{},false);
        // Keep the prior saved sound available until the normal save-on-quit.
        settings->setValue("filterState",previousState);
        if (auto* processor = dynamic_cast<BigAltedChicoProcessor*>(holder->processor.get()))
            processor->setSkinId(selectedSkin);
        window = std::make_unique<juce::StandaloneFilterWindow>(getApplicationName(),juce::Colour(0xff171b20),std::move(holder));
        window->setVisible(true);
    }
    void shutdown() override
    {
        window.reset(); appProperties.saveIfNeeded();
    }
    void systemRequestedQuit() override
    {
        if (window) window->pluginHolder->savePluginState();
        if (juce::ModalComponentManager::getInstance()->cancelAllModalComponents())
            juce::Timer::callAfterDelay(100,[] { if (auto* app=juce::JUCEApplicationBase::getInstance()) app->systemRequestedQuit(); });
        else quit();
    }
private:
    juce::ApplicationProperties appProperties;
    std::unique_ptr<juce::StandaloneFilterWindow> window;
};
JUCE_CREATE_APPLICATION_DEFINE(BigAltedChicoApplication)
