#include "PluginProcessor.h"
#include "PluginEditor.h"

MICROLOOPAudioProcessorEditor::MICROLOOPAudioProcessorEditor (MICROLOOPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      delayOnAttachment(*p.apvts.getParameter("DELAY_ON"), delayOnRelay, p.apvts.undoManager),
      delay1TempoAttachment(*p.apvts.getParameter("DELAY1_TEMPO"), delay1TempoRelay, p.apvts.undoManager),
      delay2TempoAttachment(*p.apvts.getParameter("DELAY2_TEMPO"), delay2TempoRelay, p.apvts.undoManager),
      delayOffsetAttachment(*p.apvts.getParameter("DELAY_OFFSET"), delayOffsetRelay, p.apvts.undoManager),
      delayReverseAttachment(*p.apvts.getParameter("DELAY_REVERSE"), delayReverseRelay, p.apvts.undoManager),
      delayFeedbackAttachment(*p.apvts.getParameter("DELAY_FEEDBACK"), delayFeedbackRelay, p.apvts.undoManager),
      delayDryWetAttachment(*p.apvts.getParameter("DELAY_DRYWET"), delayDryWetRelay, p.apvts.undoManager),
      delayDuckAttachment(*p.apvts.getParameter("DELAY_DUCK"), delayDuckRelay, p.apvts.undoManager),
      chorusOnAttachment(*p.apvts.getParameter("CHORUS_ON"), chorusOnRelay, p.apvts.undoManager),
      wetWidthAttachment(*p.apvts.getParameter("WET_WIDTH"), wetWidthRelay, p.apvts.undoManager),
      wetLowCutAttachment(*p.apvts.getParameter("WET_LOWCUT"), wetLowCutRelay, p.apvts.undoManager),
      wetHighCutAttachment(*p.apvts.getParameter("WET_HIGHCUT"), wetHighCutRelay, p.apvts.undoManager),
      tapeOnAttachment(*p.apvts.getParameter("TAPE_ON"), tapeOnRelay, p.apvts.undoManager),
      tapeDestabilizeAttachment(*p.apvts.getParameter("TAPE_DESTABILIZE"), tapeDestabilizeRelay, p.apvts.undoManager),
      tapeAgeAttachment(*p.apvts.getParameter("TAPE_AGE"), tapeAgeRelay, p.apvts.undoManager),
      tapeSatAttachment(*p.apvts.getParameter("TAPE_SAT"), tapeSatRelay, p.apvts.undoManager),
      optoOnAttachment(*p.apvts.getParameter("OPTO_ON"), optoOnRelay, p.apvts.undoManager),
      optoGainAttachment(*p.apvts.getParameter("OPTO_GAIN"), optoGainRelay, p.apvts.undoManager),
      dryWetAttachment(*p.apvts.getParameter("DRY_WET"), dryWetRelay, p.apvts.undoManager),
      masterOutputAttachment(*p.apvts.getParameter("MASTER_OUTPUT"), masterOutputRelay, p.apvts.undoManager),
      glitchOnAttachment(*p.apvts.getParameter("GLITCH_ON"), glitchOnRelay, p.apvts.undoManager),
      glitchAmountAttachment(*p.apvts.getParameter("GLITCH_AMOUNT"), glitchAmountRelay, p.apvts.undoManager),
      glitchReverseAttachment(*p.apvts.getParameter("GLITCH_REVERSE"), glitchReverseRelay, p.apvts.undoManager),
      granOnAttachment(*p.apvts.getParameter("GRAN_ON"), granOnRelay, p.apvts.undoManager),
      granSizeAttachment(*p.apvts.getParameter("GRAN_SIZE"), granSizeRelay, p.apvts.undoManager),
      granPitchAttachment(*p.apvts.getParameter("GRAN_PITCH"), granPitchRelay, p.apvts.undoManager),
      granMixAttachment(*p.apvts.getParameter("GRAN_MIX"), granMixRelay, p.apvts.undoManager),
      webBrowser(juce::WebBrowserComponent::Options{}
                 .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                 .withNativeIntegrationEnabled()
                 .withOptionsFrom(delayOnRelay)
                 .withOptionsFrom(delay1TempoRelay)
                 .withOptionsFrom(delay2TempoRelay)
                 .withOptionsFrom(delayOffsetRelay)
                 .withOptionsFrom(delayReverseRelay)
                 .withOptionsFrom(delayFeedbackRelay)
                 .withOptionsFrom(delayDryWetRelay)
                 .withOptionsFrom(delayDuckRelay)
                 .withOptionsFrom(chorusOnRelay)
                 .withOptionsFrom(wetWidthRelay)
                 .withOptionsFrom(wetLowCutRelay)
                 .withOptionsFrom(wetHighCutRelay)
                 .withOptionsFrom(tapeOnRelay)
                 .withOptionsFrom(tapeDestabilizeRelay)
                 .withOptionsFrom(tapeAgeRelay)
                 .withOptionsFrom(tapeSatRelay)
                 .withOptionsFrom(optoOnRelay)
                 .withOptionsFrom(optoGainRelay)
                 .withOptionsFrom(dryWetRelay)
                 .withOptionsFrom(masterOutputRelay)
                 .withOptionsFrom(glitchOnRelay)
                 .withOptionsFrom(glitchAmountRelay)
                 .withOptionsFrom(glitchReverseRelay)
                 .withOptionsFrom(granOnRelay)
                 .withOptionsFrom(granSizeRelay)
                 .withOptionsFrom(granPitchRelay)
                 .withOptionsFrom(granMixRelay)
                 .withKeepPageLoadedWhenBrowserIsHidden()
                 .withNativeFunction("savePreset", [this](const juce::Array<juce::var>& args, auto complete) {
                     if (args.size() > 0) {
                         audioProcessor.presetManager->savePreset(args[0].toString());
                     }
                     complete(juce::var());
                 })
                 .withNativeFunction("loadPreset", [this](const juce::Array<juce::var>& args, auto complete) {
                     if (args.size() > 0) {
                         audioProcessor.presetManager->loadPreset(args[0].toString());
                     }
                     complete(juce::var());
                 })
                 .withNativeFunction("setKeyboardFocus", [this](const juce::Array<juce::var>& args, auto complete) {
                     if (args.size() > 0) {
                         webBrowser.setWantsKeyboardFocus(args[0]);
                     }
                     complete(juce::var());
                 })
                 .withNativeFunction("getPresets", [this](const juce::Array<juce::var>& args, auto complete) {
                     juce::Array<juce::var> presets;
                     for (const auto& preset : audioProcessor.presetManager->getAllPresets()) {
                         presets.add(preset);
                     }
                     complete(juce::var(presets));
                 })
                 .withNativeFunction("getCurrentPreset", [this](const juce::Array<juce::var>& args, auto complete) {
                     complete(juce::var(audioProcessor.presetManager->getCurrentPresetName()));
                 })
                 .withResourceProvider([this](const auto& url) { return getResource(url); },
                                       juce::WebBrowserComponent::getResourceProviderRoot()))
{
    setSize(1200, 400);
    setResizable(true, true);
    if (auto* constrainer = getConstrainer())
        constrainer->setFixedAspectRatio(1200.0 / 400.0);
    setResizeLimits(600, 200, 2400, 800);
    webBrowser.setWantsKeyboardFocus(false);
    addAndMakeVisible(webBrowser);
    webBrowser.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    
    if (!audioProcessor.isLicensed) {
        webBrowser.setVisible(false);
    }
    
    addAndMakeVisible(openCenterButton);
    openCenterButton.setButtonText("OPEN PNK NOISE CENTER");
    openCenterButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe60000)); // Red
    openCenterButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    openCenterButton.onClick = [this] {
        juce::File appFile("/Applications/PNK NOISE CENTER.app");
        if (appFile.existsAsFile() || appFile.exists()) {
            appFile.startAsProcess();
        } else {
            juce::URL("https://pnk-noise.com").launchInDefaultBrowser();
        }
    };
    openCenterButton.setVisible(!audioProcessor.isLicensed);
    
    startTimerHz(30);
}

MICROLOOPAudioProcessorEditor::~MICROLOOPAudioProcessorEditor()
{
    stopTimer();
}

void MICROLOOPAudioProcessorEditor::timerCallback()
{
    float loopPos = audioProcessor.currentLoopPos.load();
    float masterGr = audioProcessor.currentMasterGR.load();
    float delay1Level = audioProcessor.delay1OutputLevel.load();
    float delay2Level = audioProcessor.delay2OutputLevel.load();
    
    int writeIdx = audioProcessor.scopeWriteIndex.load();
    juce::Array<juce::var> leftSamples;
    juce::Array<juce::var> rightSamples;
    int numSamplesToSend = 256;
    for (int i = 0; i < numSamplesToSend; ++i)
    {
        int readIdx = (writeIdx - numSamplesToSend + i + audioProcessor.scopeBufferSize) % audioProcessor.scopeBufferSize;
        leftSamples.add(audioProcessor.scopeBufferL[readIdx]);
        rightSamples.add(audioProcessor.scopeBufferR[readIdx]);
    }
    
    juce::DynamicObject::Ptr obj(new juce::DynamicObject());
    obj->setProperty("loopPos", loopPos);
    obj->setProperty("master", masterGr);
    obj->setProperty("delay1Level", delay1Level);
    obj->setProperty("delay2Level", delay2Level);
    obj->setProperty("leftSamples", leftSamples);
    obj->setProperty("rightSamples", rightSamples);
    webBrowser.emitEventIfBrowserIsVisible("meterData", obj.get());
    
    static bool wasLicensed = audioProcessor.isLicensed;
    if (wasLicensed != audioProcessor.isLicensed) {
        wasLicensed = audioProcessor.isLicensed;
        webBrowser.setVisible(wasLicensed);
        openCenterButton.setVisible(!wasLicensed);
        repaint();
    }
}

std::optional<juce::WebBrowserComponent::Resource> MICROLOOPAudioProcessorEditor::getResource(const juce::String& url)
{
    const auto urlToRetrieve = url == "/" ? juce::String("index.html") : url.fromFirstOccurrenceOf("/", false, false);

    if (urlToRetrieve == "index.html") {
        return juce::WebBrowserComponent::Resource { 
            std::vector<std::byte>(reinterpret_cast<const std::byte*>(WebAssets::index_html), 
                                   reinterpret_cast<const std::byte*>(WebAssets::index_html) + WebAssets::index_htmlSize), 
            "text/html" 
        };
    }
    
    // Create a macro or lambda to avoid repeating the boilerplate
    auto makeImgRes = [](const char* data, int size) {
        return juce::WebBrowserComponent::Resource {
            std::vector<std::byte>(reinterpret_cast<const std::byte*>(data), reinterpret_cast<const std::byte*>(data) + size),
            "image/png"
        };
    };

    if (urlToRetrieve == "bg_inspired.png") return makeImgRes(WebAssets::bg_inspired_png, WebAssets::bg_inspired_pngSize);
    if (urlToRetrieve == "AFTERGLOW.png") return makeImgRes(WebAssets::AFTERGLOW_png, WebAssets::AFTERGLOW_pngSize);
    if (urlToRetrieve == "button_matte.png") return makeImgRes(WebAssets::button_matte_png, WebAssets::button_matte_pngSize);
    if (urlToRetrieve == "KNOB/Long%20shadow.png" || urlToRetrieve == "KNOB/Long shadow.png") return makeImgRes(WebAssets::Long_shadow_png, WebAssets::Long_shadow_pngSize);
    if (urlToRetrieve == "KNOB/KNOB%20MOVE.png" || urlToRetrieve == "KNOB/KNOB MOVE.png") return makeImgRes(WebAssets::KNOB_MOVE_png, WebAssets::KNOB_MOVE_pngSize);
    if (urlToRetrieve == "KNOB/KNOB%20STILLA.png" || urlToRetrieve == "KNOB/KNOB STILLA.png") return makeImgRes(WebAssets::KNOB_STILLA_png, WebAssets::KNOB_STILLA_pngSize);
    if (urlToRetrieve == "KNOB/KNOB%20PEKARE.png" || urlToRetrieve == "KNOB/KNOB PEKARE.png") return makeImgRes(WebAssets::KNOB_PEKARE_png, WebAssets::KNOB_PEKARE_pngSize);

    return std::nullopt;
}

void MICROLOOPAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    
    if (!audioProcessor.isLicensed) {
        g.setColour(juce::Colours::red);
        g.setFont(24.0f);
        g.drawFittedText("UNREGISTERED\nPlease open PNK NOISE CENTER to activate.", 
            getLocalBounds().reduced(20), juce::Justification::centred, 3);
    }
}

void MICROLOOPAudioProcessorEditor::resized()
{
    webBrowser.setBounds(getLocalBounds());
    
    auto bounds = getLocalBounds();
    openCenterButton.setBounds(bounds.getCentreX() - 150, bounds.getCentreY() + 40, 300, 40);
}
