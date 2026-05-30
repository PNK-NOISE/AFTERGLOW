#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WebAssets.h"

class MICROLOOPAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    MICROLOOPAudioProcessorEditor (MICROLOOPAudioProcessor&);
    ~MICROLOOPAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);
    void timerCallback() override;

private:
    MICROLOOPAudioProcessor& audioProcessor;
    
    // Web Relays
    juce::WebToggleButtonRelay delayOnRelay { "delayOn" };
    juce::WebSliderRelay delay1TempoRelay { "delay1Tempo" };
    juce::WebSliderRelay delay2TempoRelay { "delay2Tempo" };
    juce::WebSliderRelay delayOffsetRelay { "delayOffset" };
    juce::WebToggleButtonRelay delayReverseRelay { "delayReverse" };
    
    juce::WebSliderRelay delayFeedbackRelay { "delayFeedback" };
    juce::WebSliderRelay delayDryWetRelay { "delayDryWet" };
    juce::WebSliderRelay delayDuckRelay { "delayDuck" };
    juce::WebToggleButtonRelay chorusOnRelay { "chorusOn" };
    
    juce::WebToggleButtonRelay glitchOnRelay { "glitchOn" };
    juce::WebSliderRelay glitchAmountRelay { "glitchAmount" };
    juce::WebToggleButtonRelay glitchReverseRelay { "glitchReverse" };
    
    juce::WebToggleButtonRelay granOnRelay { "granOn" };
    juce::WebSliderRelay granSizeRelay { "granSize" };
    juce::WebSliderRelay granPitchRelay { "granPitch" };
    juce::WebSliderRelay granMixRelay { "granMix" };
    
    juce::WebSliderRelay wetWidthRelay { "wetWidth" };
    juce::WebSliderRelay wetLowCutRelay { "wetLowCut" };
    juce::WebSliderRelay wetHighCutRelay { "wetHighCut" };
    
    juce::WebToggleButtonRelay tapeOnRelay { "tapeOn" };
    juce::WebSliderRelay tapeDestabilizeRelay { "tapeDestabilize" };
    juce::WebSliderRelay tapeAgeRelay { "tapeAge" };
    juce::WebSliderRelay tapeSatRelay { "tapeSat" };
    
    juce::WebToggleButtonRelay optoOnRelay { "optoOn" };
    juce::WebSliderRelay optoGainRelay { "optoGain" };
    
    juce::WebSliderRelay dryWetRelay { "dryWet" };
    juce::WebSliderRelay masterOutputRelay { "masterOutput" };
    
    // APVTS Attachments
    juce::WebToggleButtonParameterAttachment delayOnAttachment;
    juce::WebSliderParameterAttachment delay1TempoAttachment;
    juce::WebSliderParameterAttachment delay2TempoAttachment;
    juce::WebSliderParameterAttachment delayOffsetAttachment;
    juce::WebToggleButtonParameterAttachment delayReverseAttachment;
    
    juce::WebSliderParameterAttachment delayFeedbackAttachment;
    juce::WebSliderParameterAttachment delayDryWetAttachment;
    juce::WebSliderParameterAttachment delayDuckAttachment;
    juce::WebToggleButtonParameterAttachment chorusOnAttachment;
    
    juce::WebSliderParameterAttachment wetWidthAttachment;
    juce::WebSliderParameterAttachment wetLowCutAttachment;
    juce::WebSliderParameterAttachment wetHighCutAttachment;
    
    juce::WebToggleButtonParameterAttachment tapeOnAttachment;
    juce::WebSliderParameterAttachment tapeDestabilizeAttachment;
    juce::WebSliderParameterAttachment tapeAgeAttachment;
    juce::WebSliderParameterAttachment tapeSatAttachment;
    
    juce::WebToggleButtonParameterAttachment optoOnAttachment;
    juce::WebSliderParameterAttachment optoGainAttachment;
    
    juce::WebSliderParameterAttachment dryWetAttachment;
    juce::WebSliderParameterAttachment masterOutputAttachment;
    
    juce::WebToggleButtonParameterAttachment glitchOnAttachment;
    juce::WebSliderParameterAttachment glitchAmountAttachment;
    juce::WebToggleButtonParameterAttachment glitchReverseAttachment;
    
    juce::WebToggleButtonParameterAttachment granOnAttachment;
    juce::WebSliderParameterAttachment granSizeAttachment;
    juce::WebSliderParameterAttachment granPitchAttachment;
    juce::WebSliderParameterAttachment granMixAttachment;
    
    // The Web Browser
    juce::WebBrowserComponent webBrowser;
    juce::TextButton openCenterButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MICROLOOPAudioProcessorEditor)
};
