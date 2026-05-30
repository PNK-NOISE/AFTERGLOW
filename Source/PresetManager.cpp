#include "PresetManager.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : valueTreeState(apvts), currentPresetName("Init Preset")
{
    // Ensure the preset directory exists
    auto presetDir = getPresetDirectory();
    if (!presetDir.exists())
    {
        presetDir.createDirectory();
    }
    
    refreshPresetList();
    
    // If empty, save the default state as "Init Preset"
    if (currentPresetList.isEmpty())
    {
        savePreset("Init Preset");
    }
}

juce::File PresetManager::getPresetDirectory() const
{
    auto osDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    
#if JUCE_MAC
    // On Mac: ~/Library/Audio/Presets/PNK NOISE/MICROLOOP
    osDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile("Library").getChildFile("Audio").getChildFile("Presets");
#endif
    
    return osDir.getChildFile(companyName).getChildFile(pluginName);
}

void PresetManager::refreshPresetList()
{
    currentPresetList.clear();
    auto presetDir = getPresetDirectory();
    
    auto files = presetDir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false, "*" + extension);
    
    for (const auto& file : files)
    {
        currentPresetList.add(file.getFileNameWithoutExtension());
    }
    currentPresetList.sort(true);
}

juce::StringArray PresetManager::getAllPresets() const
{
    return currentPresetList;
}

int PresetManager::getCurrentPresetIndex() const
{
    return currentPresetList.indexOf(currentPresetName);
}

juce::String PresetManager::getCurrentPresetName() const
{
    return currentPresetName;
}

void PresetManager::savePreset(const juce::String& presetName)
{
    if (presetName.isEmpty())
        return;

    auto presetDir = getPresetDirectory();
    if (!presetDir.exists())
        presetDir.createDirectory();

    auto file = presetDir.getChildFile(presetName + extension);
    
    if (auto xml = valueTreeState.copyState().createXml())
    {
        xml->writeTo(file);
        currentPresetName = presetName;
        refreshPresetList();
    }
}

void PresetManager::loadPreset(const juce::String& presetName)
{
    if (presetName.isEmpty())
        return;

    auto presetDir = getPresetDirectory();
    auto file = presetDir.getChildFile(presetName + extension);

    if (file.existsAsFile())
    {
        if (auto xml = juce::parseXML(file))
        {
            valueTreeState.replaceState(juce::ValueTree::fromXml(*xml));
            currentPresetName = presetName;
        }
    }
}

void PresetManager::loadPreset(int index)
{
    if (index >= 0 && index < currentPresetList.size())
    {
        loadPreset(currentPresetList[index]);
    }
}
