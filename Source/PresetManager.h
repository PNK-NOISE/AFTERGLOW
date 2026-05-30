#pragma once
#include <JuceHeader.h>

class PresetManager
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts);

    void savePreset(const juce::String& presetName);
    void loadPreset(const juce::String& presetName);
    void loadPreset(int index);
    
    juce::StringArray getAllPresets() const;
    int getCurrentPresetIndex() const;
    juce::String getCurrentPresetName() const;

private:
    juce::File getPresetDirectory() const;
    void refreshPresetList();
    
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::String currentPresetName;
    juce::StringArray currentPresetList;
    
    const juce::String extension = ".preset";
    const juce::String companyName = "PNK NOISE";
    const juce::String pluginName = "AFTERGLOW";
};
