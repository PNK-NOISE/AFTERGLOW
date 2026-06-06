#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>

MICROLOOPAudioProcessor::MICROLOOPAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), false)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    delayOnParam = apvts.getRawParameterValue("DELAY_ON");
    delay1TempoParam = apvts.getRawParameterValue("DELAY1_TEMPO");
    delay2TempoParam = apvts.getRawParameterValue("DELAY2_TEMPO");
    delayOffsetParam = apvts.getRawParameterValue("DELAY_OFFSET");
    delayReverseParam = apvts.getRawParameterValue("DELAY_REVERSE");
    delayFeedbackParam = apvts.getRawParameterValue("DELAY_FEEDBACK");
    delayDryWetParam = apvts.getRawParameterValue("DELAY_DRYWET");
    delayDuckParam = apvts.getRawParameterValue("DELAY_DUCK");
    
    chorusOnParam = apvts.getRawParameterValue("CHORUS_ON");
    
    wetWidthParam = apvts.getRawParameterValue("WET_WIDTH");
    wetLowCutParam = apvts.getRawParameterValue("WET_LOWCUT");
    wetHighCutParam = apvts.getRawParameterValue("WET_HIGHCUT");
    
    tapeOnParam = apvts.getRawParameterValue("TAPE_ON");
    tapeDestabilizeParam = apvts.getRawParameterValue("TAPE_DESTABILIZE");
    tapeAgeParam = apvts.getRawParameterValue("TAPE_AGE");
    tapeSatParam = apvts.getRawParameterValue("TAPE_SAT");
    
    dryWetParam = apvts.getRawParameterValue("DRY_WET");
    masterOutputParam = apvts.getRawParameterValue("MASTER_OUTPUT");
    
    glitchOnParam = apvts.getRawParameterValue("GLITCH_ON");
    glitchAmountParam = apvts.getRawParameterValue("GLITCH_AMOUNT");
    glitchReverseParam = apvts.getRawParameterValue("GLITCH_REVERSE");
    
    granOnParam = apvts.getRawParameterValue("GRAN_ON");
    granSizeParam = apvts.getRawParameterValue("GRAN_SIZE");
    granPitchParam = apvts.getRawParameterValue("GRAN_PITCH");
    granMixParam = apvts.getRawParameterValue("GRAN_MIX");
    
    optoOnParam = apvts.getRawParameterValue("OPTO_ON");
    optoGainParam = apvts.getRawParameterValue("OPTO_GAIN");
    
    presetManager = std::make_unique<PresetManager>(apvts);
    presetManager->loadPreset("Init Preset");
    
    validateLicense();
    startTimer(2000);
}

void MICROLOOPAudioProcessor::timerCallback()
{
    validateLicense();
}

void MICROLOOPAudioProcessor::validateLicense()
{
    bool newLicenseState = false;
    
    juce::Array<juce::File> filesToCheck;
    
#if JUCE_MAC
    // 1. User Presets (highly compatible with sandboxed DAWs on macOS)
    filesToCheck.add(juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                     .getChildFile("Library").getChildFile("Audio").getChildFile("Presets")
                     .getChildFile("PNK NOISE").getChildFile("license.dat"));
                     
    // 2. Shared folder (installer location)
    filesToCheck.add(juce::File("/Users/Shared/PNK-NOISE/license.dat"));
#endif

    // 3. User Application Data folder (standard fallback for both Mac & Windows)
    filesToCheck.add(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                     .getChildFile("PNK-NOISE").getChildFile("license.dat"));

    for (const auto& licenseFile : filesToCheck)
    {
        if (licenseFile.existsAsFile())
        {
            juce::String b64 = licenseFile.loadFileAsString().trim();
            if (!b64.isEmpty()) 
            {
                juce::MemoryOutputStream decoded;
                if (juce::Base64::convertFromBase64(decoded, b64)) 
                {
                    auto* data = static_cast<const char*>(decoded.getData());
                    int size = static_cast<int>(decoded.getDataSize());
                    
                    juce::String key = "PNK_NOISE_SECRET_KEY_2026";
                    juce::String result;
                    for (int i = 0; i < size; ++i) {
                        result += (char)(data[i] ^ key[i % key.length()]);
                    }
                    
                    juce::String upperResult = result.toUpperCase();
                    if (upperResult.contains("DISTWIDE") || upperResult.contains("MICROLOOP") || upperResult.contains("AFTERGLOW")) {
                        newLicenseState = true;
                        break; // Valid license found, stop checking
                    }
                }
            }
        }
    }
    
    isLicensed = newLicenseState;
}

MICROLOOPAudioProcessor::~MICROLOOPAudioProcessor()
{
}

const juce::String MICROLOOPAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MICROLOOPAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MICROLOOPAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MICROLOOPAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MICROLOOPAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MICROLOOPAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MICROLOOPAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MICROLOOPAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MICROLOOPAudioProcessor::getProgramName (int index)
{
    return {};
}

void MICROLOOPAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void MICROLOOPAudioProcessor::updateParameters()
{
    // Handled in processBlock now
}

void MICROLOOPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    tapeDry.prepare(spec, 1.0f);
    tapeDelay1.prepare(spec, 0.85f);
    tapeDelay2.prepare(spec, 1.15f);
    
    chorusL1.prepare(spec);
    chorusL2.prepare(spec);
    chorusR1.prepare(spec);
    chorusR2.prepare(spec);
    
    circularBuffer.setSize(getTotalNumOutputChannels(), static_cast<int>(sampleRate * 4.0));
    circularBuffer.clear();
    
    delay1Buffer.setSize(getTotalNumOutputChannels(), static_cast<int>(sampleRate * 8.0));
    delay1Buffer.clear();
    delay2Buffer.setSize(getTotalNumOutputChannels(), static_cast<int>(sampleRate * 8.0));
    delay2Buffer.clear();
    
    granRingBuffer.setSize(getTotalNumOutputChannels(), static_cast<int>(sampleRate * 2.0));
    granRingBuffer.clear();
    granWritePos = 0;
    granSamplesSinceLastSpawn = 0;
    for (auto& grain : granGrains) {
        grain.active = false;
    }
    optoLdrState = 0.0f;
    
    auto bpCoefs = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 1000.0f, 0.3f);
    tapeFilterL.coefficients = bpCoefs;
    tapeFilterR.coefficients = bpCoefs;
    tapeFilterL.prepare(spec);
    tapeFilterR.prepare(spec);
    
    // Prepare master wet filters
    wetHpfL.prepare(spec);
    wetHpfR.prepare(spec);
    wetLpfL.prepare(spec);
    wetLpfR.prepare(spec);
    
    glitchHpfL.prepare(spec);
    glitchHpfR.prepare(spec);
    glitchLpfL.prepare(spec);
    glitchLpfR.prepare(spec);
    
    auto flatCoefsHP = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
    auto flatCoefsLP = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 20000.0f);
    
    wetHpfL.coefficients = flatCoefsHP;
    wetHpfR.coefficients = flatCoefsHP;
    wetLpfL.coefficients = flatCoefsLP;
    wetLpfR.coefficients = flatCoefsLP;
    
    glitchHpfL.coefficients = flatCoefsHP;
    glitchHpfR.coefficients = flatCoefsHP;
    glitchLpfL.coefficients = flatCoefsLP;
    glitchLpfR.coefficients = flatCoefsLP;
    
    delay1DuckEnv = 0.0f;
    delay2DuckEnv = 0.0f;
    glitchDuckEnv = 0.0f;
    lastWetLowCut = -1.0f;
    lastWetHighCut = -1.0f;
    lastSampleRate = -1.0;
    
    delay1ReversePhase = 0.0;
    delay2ReversePhase = 0.0;
    smoothedDelay1TimeSamples = -1.0;
    smoothedDelay2TimeSamples = -1.0;
    
    tapeCompEnv = 0.0f;
    
    writePosition = 0;
    delay1WritePosition = 0;
    delay2WritePosition = 0;
    
    delay1Envelope = 0.0f;
    delay2Envelope = 0.0f;
    
    glitchSampleCounter = 0.0;
    isGlitchLooping = false;
    glitchStartPos = 0;
    glitchLengthSamples = 0;
    glitchPhase = 0.0f;
    glitchEighthNotesRemaining = 0;
    glitchEnvelope = 0.0f;
    glitchReverse = false;
}

void MICROLOOPAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MICROLOOPAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

static float getQuarterNotesForDelayDivision(int index) {
    int baseType = index / 3;  // 0 to 6
    int modType = index % 3;   // 0: straight, 1: dotted, 2: triplet
    
    float baseQuarterNotes = 1.0f;
    switch (baseType) {
        case 0: baseQuarterNotes = 4.0f;    // 1/1
                break;
        case 1: baseQuarterNotes = 2.0f;    // 1/2
                break;
        case 2: baseQuarterNotes = 1.0f;    // 1/4
                break;
        case 3: baseQuarterNotes = 0.5f;    // 1/8
                break;
        case 4: baseQuarterNotes = 0.25f;   // 1/16
                break;
        case 5: baseQuarterNotes = 0.125f;  // 1/32
                break;
        case 6: baseQuarterNotes = 0.0625f; // 1/64
                break;
        default: break;
    }
    
    if (modType == 1) { // Dotted: multiply by 1.5
        return baseQuarterNotes * 1.5f;
    } else if (modType == 2) { // Triplet: multiply by 2/3
        return baseQuarterNotes * (2.0f / 3.0f);
    }
    
    return baseQuarterNotes;
}

void MICROLOOPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    if (!isLicensed) {
        buffer.clear();
        return;
    }

    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateParameters();
    
    // 1. Get Params
    bool delayOn = delayOnParam->load() > 0.5f;
    int delay1TempoIdx = static_cast<int>(delay1TempoParam->load());
    int delay2TempoIdx = static_cast<int>(delay2TempoParam->load());
    float delayOffset = delayOffsetParam->load();
    float delayFeedback = delayFeedbackParam->load();
    bool delayReverse = delayReverseParam->load() > 0.5f;
    float delayDryWet = delayDryWetParam->load() / 100.0f;
    float delayDuck = delayDuckParam->load();
    
    bool chorusOn = chorusOnParam->load() > 0.5f;
    
    float wetWidth = wetWidthParam->load();
    
    bool glitchOn = glitchOnParam->load() > 0.5f;
    float glitchAmount = glitchAmountParam->load();
    float glitchMix = glitchAmount / 100.0f;
    
    bool granOn = granOnParam->load() > 0.5f;
    float granSizeMs = granSizeParam->load();
    float granPitch = granPitchParam->load();
    float granMix = granMixParam->load() / 100.0f;
    
    float wetLowCut = wetLowCutParam->load();
    float wetHighCut = wetHighCutParam->load();
    
    bool tapeOn = tapeOnParam->load() > 0.5f;
    float tapeDestabilize = tapeDestabilizeParam->load() / 100.0f;
    float tapeWow = tapeDestabilize;
    float tapeFlutter = tapeDestabilize;
    float tapeAge = tapeAgeParam->load() / 100.0f;
    float tapeSat = tapeSatParam->load() / 100.0f;
    
    bool optoOn = optoOnParam->load() > 0.5f;
    float optoGain = optoGainParam->load() / 100.0f;
    
    float dryWet = dryWetParam->load() / 100.0f;
    float outGain = juce::Decibels::decibelsToGain(masterOutputParam->load());
    
    int numSamples = buffer.getNumSamples();
    int glitchBufferSize = circularBuffer.getNumSamples();
    if (glitchBufferSize == 0) return;
    
    // Get host BPM
    float bpm = 120.0f;
    if (auto* playHead = getPlayHead()) {
        if (auto optPlayHeadInfo = playHead->getPosition()) {
            if (optPlayHeadInfo->getBpm().hasValue())
                bpm = static_cast<float>(*optPlayHeadInfo->getBpm());
        }
    }
    if (bpm < 10.0f) bpm = 120.0f;
    
    // Update master filters if parameters or sample rate changed
    if (wetLowCut != lastWetLowCut || currentSampleRate != lastSampleRate) {
        auto coefs = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, wetLowCut);
        wetHpfL.coefficients = coefs;
        wetHpfR.coefficients = coefs;
        glitchHpfL.coefficients = coefs;
        glitchHpfR.coefficients = coefs;
        lastWetLowCut = wetLowCut;
    }
    if (wetHighCut != lastWetHighCut || currentSampleRate != lastSampleRate) {
        auto coefs = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, wetHighCut);
        wetLpfL.coefficients = coefs;
        wetLpfR.coefficients = coefs;
        glitchLpfL.coefficients = coefs;
        glitchLpfR.coefficients = coefs;
        lastWetHighCut = wetHighCut;
    }
    lastSampleRate = currentSampleRate;
    
    // Calculate delay variables using individual tempo selections
    float delay1QuarterNotes = getQuarterNotesForDelayDivision(delay1TempoIdx);
    float delay2QuarterNotes = getQuarterNotesForDelayDivision(delay2TempoIdx);
    
    float delay1Secs = delay1QuarterNotes * (60.0f / bpm);
    float delay2Secs = delay2QuarterNotes * (60.0f / bpm);
    
    // Apply timing offset to both delays (expressed in %)
    float offsetMult = 1.0f + (delayOffset / 100.0f);
    delay1Secs *= offsetMult;
    delay2Secs *= offsetMult;
    
    // Delay 1 target time
    double targetD1 = delay1Secs * currentSampleRate;
    int d1BufferSize = delay1Buffer.getNumSamples();
    double maxD1 = delayReverse ? ((d1BufferSize - 2.0) / 2.0) : (d1BufferSize - 2.0);
    targetD1 = juce::jlimit(1.0, maxD1, targetD1);
    
    // Delay 2 target time: add Haas offset from wetWidth (0 to 25 ms)
    double targetD2 = delay2Secs * currentSampleRate;
    double haasDelaySecs = (wetWidth / 100.0f) * 0.025f; // Haas offset up to 25ms
    targetD2 += haasDelaySecs * currentSampleRate;
    
    int d2BufferSize = delay2Buffer.getNumSamples();
    double maxD2 = delayReverse ? ((d2BufferSize - 2.0) / 2.0) : (d2BufferSize - 2.0);
    targetD2 = juce::jlimit(1.0, maxD2, targetD2);
    
    if (smoothedDelay1TimeSamples < 0.0) smoothedDelay1TimeSamples = targetD1;
    if (smoothedDelay2TimeSamples < 0.0) smoothedDelay2TimeSamples = targetD2;
    
    // Map feedback directly (0.0 to 1.0)
    float feedbackScale = delayFeedback / 100.0f;
    
    // Pre-calculate coefficients
    float duckAttackCoef = 1.0f - std::exp(-1.0f / (0.010f * static_cast<float>(currentSampleRate))); // 10ms
    float duckReleaseCoef = 1.0f - std::exp(-1.0f / (0.100f * static_cast<float>(currentSampleRate))); // 100ms
    
    float tapeAttackCoef = 1.0f - std::exp(-1.0f / (0.010f * static_cast<float>(currentSampleRate))); // 10ms
    float tapeReleaseCoef = 1.0f - std::exp(-1.0f / (0.100f * static_cast<float>(currentSampleRate))); // 100ms
    float tapeDrive = 1.0f + (tapeSat * 3.0f);
    float tapeSatGain = 1.0f / std::sqrt(tapeDrive);
    
    // Helper lambda for reading fractional delay with linear interpolation
    auto readDelayBufferSample = [](const juce::AudioBuffer<float>& buf, int writePos, int channel, double delaySamples) -> float
    {
        int bufSize = buf.getNumSamples();
        double readPos = static_cast<double>(writePos) - delaySamples;
        while (readPos < 0.0) readPos += bufSize;
        while (readPos >= bufSize) readPos -= bufSize;
        
        int idx1 = static_cast<int>(std::floor(readPos));
        int idx2 = (idx1 + 1) % bufSize;
        float frac = static_cast<float>(readPos - idx1);
        
        return buf.getSample(channel, idx1) * (1.0f - frac) + buf.getSample(channel, idx2) * frac;
    };
    
    float d1PeakThisBlock = 0.0f;
    float d2PeakThisBlock = 0.0f;
    
    // Main processing loop
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float inputL = dryBuffer.getSample(0, sample);
        float inputR = totalNumInputChannels > 1 ? dryBuffer.getSample(1, sample) : inputL;
        
        // Update glitch ducking envelope
        float cleanInputLevel = std::max(std::abs(inputL), std::abs(inputR));
        float glitchDuckCoef = (cleanInputLevel > glitchDuckEnv) ? duckAttackCoef : duckReleaseCoef;
        glitchDuckEnv = glitchDuckEnv + glitchDuckCoef * (cleanInputLevel - glitchDuckEnv);
        
        // 1. Write dry input to circular buffer for Glitch
        circularBuffer.setSample(0, writePosition, inputL);
        if (circularBuffer.getNumChannels() > 1) {
            circularBuffer.setSample(1, writePosition, inputR);
        }
        writePosition++;
        if (writePosition >= glitchBufferSize) writePosition = 0;
        
        float glitchedL = inputL;
        float glitchedR = inputR;
        
        // 2. Glitch logic
        if (!glitchOn) {
            isGlitchLooping = false;
        }
        
        double samplesPerEighth = (60.0 / bpm) * 0.5 * currentSampleRate;
        if (samplesPerEighth < 1.0) samplesPerEighth = 1000.0;
        
        glitchSampleCounter += 1.0;
        if (glitchSampleCounter >= samplesPerEighth) {
            glitchSampleCounter -= samplesPerEighth;
            
            if (isGlitchLooping) {
                glitchEighthNotesRemaining--;
                if (glitchEighthNotesRemaining <= 0) {
                    isGlitchLooping = false;
                }
            }
            
            if (glitchOn && !isGlitchLooping) {
                if (random.nextFloat() < (glitchAmount / 100.0f)) {
                    float r = random.nextFloat();
                    if (r < 0.5f) {
                        glitchEighthNotesRemaining = 1;
                    } else if (r < 0.85f) {
                        glitchEighthNotesRemaining = 2;
                    } else {
                        glitchEighthNotesRemaining = 4;
                    }
                    
                    int divIdx = random.nextInt(4);
                    float qNotes = 0.25f;
                    if (divIdx == 0) qNotes = 0.0625f;
                    else if (divIdx == 1) qNotes = 0.125f;
                    else if (divIdx == 2) qNotes = 0.25f;
                    else qNotes = 0.5f;
                    
                    double glitchSecs = qNotes * (60.0 / bpm);
                    glitchLengthSamples = static_cast<int>(glitchSecs * currentSampleRate);
                    if (glitchLengthSamples > glitchBufferSize) glitchLengthSamples = glitchBufferSize;
                    if (glitchLengthSamples < 16) glitchLengthSamples = 16;
                    
                    glitchReverse = glitchReverseParam->load() > 0.5f;
                    glitchPlaybackSpeed = 1.0f;
                    isGlitchLooping = true;
                    glitchPhase = 0.0f;
                    glitchStartPos = writePosition - glitchLengthSamples;
                    if (glitchStartPos < 0) {
                        glitchStartPos += glitchBufferSize;
                    }
                }
            }
        }
        
        if (isGlitchLooping) {
            glitchEnvelope = std::min(1.0f, glitchEnvelope + (1.0f / 128.0f));
        } else {
            glitchEnvelope = std::max(0.0f, glitchEnvelope - (1.0f / 128.0f));
        }
        
        if (glitchEnvelope > 0.0f && glitchLengthSamples > 0) {
            float gReadPos = 0.0f;
            if (glitchReverse) {
                gReadPos = glitchStartPos + (glitchLengthSamples - 1) - glitchPhase;
            } else {
                gReadPos = glitchStartPos + glitchPhase;
            }
            while (gReadPos >= glitchBufferSize) gReadPos -= glitchBufferSize;
            while (gReadPos < 0.0f) gReadPos += glitchBufferSize;
            
            int gIdx1 = static_cast<int>(gReadPos);
            int gIdx2 = (gIdx1 + 1) % glitchBufferSize;
            float gFrac = gReadPos - gIdx1;
            
            float glitchOutL = circularBuffer.getSample(0, gIdx1) * (1.0f - gFrac) + circularBuffer.getSample(0, gIdx2) * gFrac;
            float glitchOutR = (circularBuffer.getNumChannels() > 1) ? 
                (circularBuffer.getSample(1, gIdx1) * (1.0f - gFrac) + circularBuffer.getSample(1, gIdx2) * gFrac) : glitchOutL;
                
            float gFade = std::min(128.0f, glitchLengthSamples * 0.1f);
            float gEnv = 1.0f;
            if (glitchPhase < gFade) {
                gEnv = glitchPhase / gFade;
            } else if (glitchPhase > glitchLengthSamples - gFade) {
                gEnv = (glitchLengthSamples - glitchPhase) / gFade;
            }
            gEnv = juce::jlimit(0.0f, 1.0f, gEnv);
            
            glitchOutL *= gEnv;
            glitchOutR *= gEnv;
            
            // Sidechain Ducking Glitch
            float glitchDuckGain = 1.0f - ((delayDuck / 100.0f) * juce::jlimit(0.0f, 1.0f, glitchDuckEnv * 4.0f));
            glitchOutL *= glitchDuckGain;
            glitchOutR *= glitchDuckGain;
            
            // Apply low/high cut filters to the glitch output
            float filteredGlitchL = glitchLpfL.processSample(glitchHpfL.processSample(glitchOutL));
            float filteredGlitchR = glitchLpfR.processSample(glitchHpfR.processSample(glitchOutR));
            
            float glitchMixVal = glitchMix * glitchEnvelope;
            glitchedL = glitchedL * (1.0f - glitchMixVal) + filteredGlitchL * glitchMixVal;
            glitchedR = glitchedR * (1.0f - glitchMixVal) + filteredGlitchR * glitchMixVal;
            
            glitchPhase += glitchPlaybackSpeed;
            if (glitchPhase >= glitchLengthSamples || glitchPhase < 0.0f) {
                glitchPhase = 0.0f;
            }
        }
        
        // 3. Dry wow & flutter (tapeDry)
        float dryL = glitchedL;
        float dryR = glitchedR;
        
        if (tapeOn) {
            tapeDry.updateOffset(currentSampleRate, tapeWow, tapeFlutter, random);
            
            tapeDry.delayLine.pushSample(0, glitchedL);
            if (totalNumInputChannels > 1) {
                tapeDry.delayLine.pushSample(1, glitchedR);
            } else {
                tapeDry.delayLine.pushSample(1, glitchedL);
            }
            
            float dryOffsetL = tapeDry.getOffsetSamples(0, currentSampleRate, tapeWow, tapeFlutter);
            float dryOffsetR = tapeDry.getOffsetSamples(1, currentSampleRate, tapeWow, tapeFlutter);
            
            float dryDelayL = 20.0f * static_cast<float>(currentSampleRate) / 1000.0f + dryOffsetL;
            float dryDelayR = 20.0f * static_cast<float>(currentSampleRate) / 1000.0f + dryOffsetR;
            
            dryDelayL = juce::jlimit(1.0f, 8000.0f, dryDelayL);
            dryDelayR = juce::jlimit(1.0f, 8000.0f, dryDelayR);
            
            dryL = tapeDry.delayLine.popSample(0, dryDelayL, true);
            dryR = tapeDry.delayLine.popSample(1, dryDelayR, true);
        }
        
        // 4. Update Delay bypass/activation envelopes
        if (delayOn) {
            delay1Envelope = std::min(1.0f, delay1Envelope + (1.0f / 256.0f));
            delay2Envelope = std::min(1.0f, delay2Envelope + (1.0f / 256.0f));
        } else {
            delay1Envelope = std::max(0.0f, delay1Envelope - (1.0f / 256.0f));
            delay2Envelope = std::max(0.0f, delay2Envelope - (1.0f / 256.0f));
        }
        
        // 5. Smooth delay times sample-by-sample
        smoothedDelay1TimeSamples += 0.001 * (targetD1 - smoothedDelay1TimeSamples);
        double D1 = smoothedDelay1TimeSamples;
        smoothedDelay2TimeSamples += 0.001 * (targetD2 - smoothedDelay2TimeSamples);
        double D2 = smoothedDelay2TimeSamples;
        
        // Update tape offset modulations
        if (tapeOn && (tapeWow > 0.0f || tapeFlutter > 0.0f)) {
            tapeDelay1.updateOffset(currentSampleRate, tapeWow, tapeFlutter, random);
            tapeDelay2.updateOffset(currentSampleRate, tapeWow, tapeFlutter, random);
        }
        
        float delay1OutL = 0.0f;
        float delay2OutR = 0.0f;
        
        // Read Delay 1 (Left channel only)
        if (d1BufferSize > 0) {
            float offset1L = tapeDelay1.getOffsetSamples(0, currentSampleRate, tapeWow, tapeFlutter);
            
            if (delayReverse) {
                delay1ReversePhase = std::fmod(delay1ReversePhase + 1.0, D1);
                double phaseA = delay1ReversePhase;
                double phaseB = std::fmod(delay1ReversePhase + D1 / 2.0, D1);
                
                double dTimeA_L = 2.0 * phaseA + offset1L;
                double dTimeB_L = 2.0 * phaseB + offset1L;
                
                float sAL = readDelayBufferSample(delay1Buffer, delay1WritePosition, 0, dTimeA_L);
                float sBL = readDelayBufferSample(delay1Buffer, delay1WritePosition, 0, dTimeB_L);
                
                double halfD1 = D1 / 2.0;
                float wA = 0.0f;
                if (phaseA < halfD1) wA = static_cast<float>(phaseA / halfD1);
                else wA = static_cast<float>((D1 - phaseA) / halfD1);
                float wB = 1.0f - wA;
                
                delay1OutL = sAL * wA + sBL * wB;
            } else {
                delay1OutL = readDelayBufferSample(delay1Buffer, delay1WritePosition, 0, D1 + offset1L);
            }
            
            // Sidechain Ducking Delay 1
            float dryLevel = std::abs(glitchedL);
            float duck1Coef = (dryLevel > delay1DuckEnv) ? duckAttackCoef : duckReleaseCoef;
            delay1DuckEnv = delay1DuckEnv + duck1Coef * (dryLevel - delay1DuckEnv);
            float duck1Gain = 1.0f - ((delayDuck / 100.0f) * juce::jlimit(0.0f, 1.0f, delay1DuckEnv * 4.0f));
            
            delay1OutL *= duck1Gain;
        }
        
        // Read Delay 2 (Right channel only)
        if (d2BufferSize > 0) {
            float offset2R = tapeDelay2.getOffsetSamples(1, currentSampleRate, tapeWow, tapeFlutter);
            
            if (delayReverse) {
                delay2ReversePhase = std::fmod(delay2ReversePhase + 1.0, D2);
                double phaseA = delay2ReversePhase;
                double phaseB = std::fmod(delay2ReversePhase + D2 / 2.0, D2);
                
                double dTimeA_R = 2.0 * phaseA + offset2R;
                double dTimeB_R = 2.0 * phaseB + offset2R;
                
                float sAR = readDelayBufferSample(delay2Buffer, delay2WritePosition, 1, dTimeA_R);
                float sBR = readDelayBufferSample(delay2Buffer, delay2WritePosition, 1, dTimeB_R);
                
                double halfD2 = D2 / 2.0;
                float wA = 0.0f;
                if (phaseA < halfD2) wA = static_cast<float>(phaseA / halfD2);
                else wA = static_cast<float>((D2 - phaseA) / halfD2);
                float wB = 1.0f - wA;
                
                delay2OutR = sAR * wA + sBR * wB;
            } else {
                delay2OutR = readDelayBufferSample(delay2Buffer, delay2WritePosition, 1, D2 + offset2R);
            }
            
            // Sidechain Ducking Delay 2
            float dryLevel = std::abs(glitchedR);
            float duck2Coef = (dryLevel > delay2DuckEnv) ? duckAttackCoef : duckReleaseCoef;
            delay2DuckEnv = delay2DuckEnv + duck2Coef * (dryLevel - delay2DuckEnv);
            float duck2Gain = 1.0f - ((delayDuck / 100.0f) * juce::jlimit(0.0f, 1.0f, delay2DuckEnv * 4.0f));
            
            delay2OutR *= duck2Gain;
        }
        
        // Track peak levels for visual tubes
        if (std::abs(delay1OutL) > d1PeakThisBlock) d1PeakThisBlock = std::abs(delay1OutL);
        if (std::abs(delay2OutR) > d2PeakThisBlock) d2PeakThisBlock = std::abs(delay2OutR);
        
        // Write to buffers (independent parallel delay with direct feedback)
        float d1InputL = glitchedL * delay1Envelope + delay1OutL * feedbackScale;
        float d2InputR = glitchedR * delay2Envelope + delay2OutR * feedbackScale;
        
        // Soft limit feedback to 1.0 to prevent runaway overload
        d1InputL = std::tanh(d1InputL);
        d2InputR = std::tanh(d2InputR);
        
        if (d1BufferSize > 0) {
            delay1Buffer.setSample(0, delay1WritePosition, d1InputL);
            delay1WritePosition++;
            if (delay1WritePosition >= d1BufferSize) delay1WritePosition = 0;
        }
        
        if (d2BufferSize > 0) {
            delay2Buffer.setSample(1, delay2WritePosition, d2InputR);
            delay2WritePosition++;
            if (delay2WritePosition >= d2BufferSize) delay2WritePosition = 0;
        }
        
        // Sum ONLY the actual wet delay outputs (Left is Delay 1, Right is Delay 2)
        float delayWetL = delay1OutL * delay1Envelope;
        float delayWetR = delay2OutR * delay2Envelope;
        
        // Additive mixing (level-style) so dry signal volume is untouched by the delay mix (scaled to 16.0f max gain)
        float combinedL = dryL + delayWetL * delayDryWet * 16.0f;
        float combinedR = dryR + delayWetR * delayDryWet * 16.0f;
        
        float dryScaleL = 1.0f;
        float dryScaleR = 1.0f;
        
        // Apply Tape Saturation & Glue Compressor (gentle compression, extreme saturation)
        if (tapeOn) {
            float combinedLevel = std::max(std::abs(combinedL), std::abs(combinedR));
            float compCoef = (combinedLevel > tapeCompEnv) ? tapeAttackCoef : tapeReleaseCoef;
            tapeCompEnv = tapeCompEnv + compCoef * (combinedLevel - tapeCompEnv);
            
            float threshold = 0.316f; // -10dB constant threshold
            float ratio = 1.0f + tapeSat * 3.0f; // 1:1 to 4:1 ratio
            float gr = 1.0f;
            if (tapeCompEnv > threshold) {
                float dbEnv = juce::Decibels::gainToDecibels(tapeCompEnv);
                float dbThreshold = juce::Decibels::gainToDecibels(threshold);
                float dbExcess = dbEnv - dbThreshold;
                float dbReduction = dbExcess * (1.0f - 1.0f / ratio);
                gr = juce::Decibels::decibelsToGain(-dbReduction);
            }
            gr = std::max(0.40f, gr); // clamp to -8dB max reduction
            
            combinedL *= gr;
            combinedR *= gr;
            
            dryScaleL *= gr;
            dryScaleR *= gr;
            
            // Store GR for meter
            currentMasterGR.store(1.0f - gr);
            
            // Saturation
            float satDrive = 1.0f + tapeSat * 15.0f; // up to 16x drive
            float satGain = 1.0f / std::sqrt(satDrive);
            
            float satGainL = 1.0f;
            if (std::abs(combinedL) > 0.0001f) {
                satGainL = std::tanh(combinedL * satDrive) * satGain / combinedL;
            } else {
                satGainL = satDrive * satGain;
            }
            combinedL = combinedL * satGainL;
            dryScaleL *= satGainL;

            float satGainR = 1.0f;
            if (std::abs(combinedR) > 0.0001f) {
                satGainR = std::tanh(combinedR * satDrive) * satGain / combinedR;
            } else {
                satGainR = satDrive * satGain;
            }
            combinedR = combinedR * satGainR;
            dryScaleR *= satGainR;
            
            // Age filter & tape noise injection
            if (tapeAge > 0.0f) {
                float filteredL = tapeFilterL.processSample(combinedL);
                float filteredR = tapeFilterR.processSample(combinedR);
                
                combinedL = combinedL * (1.0f - tapeAge) + filteredL * tapeAge;
                combinedR = combinedR * (1.0f - tapeAge) + filteredR * tapeAge;
                
                // Add warm low-pass filtered tape noise hiss
                float noiseAmt = tapeAge * 0.035f; // scale maximum noise amplitude
                float rawNoiseL = random.nextFloat() * 2.0f - 1.0f;
                float rawNoiseR = random.nextFloat() * 2.0f - 1.0f;
                
                // 1-pole lowpass filter for noise (cutoff around 3.5kHz)
                noiseFilterL += 0.15f * (rawNoiseL - noiseFilterL);
                noiseFilterR += 0.15f * (rawNoiseR - noiseFilterR);
                
                combinedL += noiseFilterL * noiseAmt;
                combinedR += noiseFilterR * noiseAmt;
            }
        } else {
            currentMasterGR.store(0.0f);
        }
        
        // Apply Optical Distortion with automatic volume compensation
        if (optoOn && optoGain > 0.0f) {
            float inputLevel = std::max(std::abs(combinedL), std::abs(combinedR));
            
            // LDR model attack (2ms) and fixed musical release (130ms)
            float attackCoef = 1.0f - std::exp(-1.0f / (0.002f * static_cast<float>(currentSampleRate)));
            float releaseCoef = 1.0f - std::exp(-1.0f / (0.130f * static_cast<float>(currentSampleRate)));
            
            float coef = (inputLevel > optoLdrState) ? attackCoef : releaseCoef;
            optoLdrState += coef * (inputLevel - optoLdrState);
            
            // Gain drives input saturation, with a dynamic boost on transients (increased for extra aggression)
            float satDrive = 1.0f + optoGain * 14.0f; // up to 15x drive (increased from 6.0f)
            float transientBoost = 10.0f * std::max(0.0f, inputLevel - optoLdrState) * optoGain; // increased from 6.0f
            float dynamicDrive = satDrive * (1.0f + transientBoost);
            
            // Asymmetric soft clipper with extra cubic grit term for high aggression and automatic volume compensation
            float offset = 0.15f * optoGain;
            float makeup = 1.25f / std::sqrt(dynamicDrive); // compensates energy reduction of clipping
            
            float xL = combinedL * dynamicDrive + offset;
            float xR = combinedR * dynamicDrive + offset;
            
            // Apply standard tanh clipper
            float clipL = std::tanh(xL);
            float clipR = std::tanh(xR);
            
            // Add a small amount of harsh cubic distortion for extra bite
            clipL += 0.25f * optoGain * (clipL * clipL * clipL);
            clipR += 0.25f * optoGain * (clipR * clipR * clipR);
            
            // Clamp to avoid digital clipping overflow
            clipL = juce::jlimit(-1.3f, 1.3f, clipL);
            clipR = juce::jlimit(-1.3f, 1.3f, clipR);
            
            float outputL = (clipL - std::tanh(offset)) * makeup;
            float outputR = (clipR - std::tanh(offset)) * makeup;
            
            float optoGainL = 1.0f;
            if (std::abs(combinedL) > 0.0001f) {
                optoGainL = outputL / combinedL;
            } else {
                optoGainL = dynamicDrive * makeup;
            }
            combinedL = outputL;
            dryScaleL *= optoGainL;
            
            float optoGainR = 1.0f;
            if (std::abs(combinedR) > 0.0001f) {
                optoGainR = outputR / combinedR;
            } else {
                optoGainR = dynamicDrive * makeup;
            }
            combinedR = outputR;
            dryScaleR *= optoGainR;
        }
        
        // Apply Chorus to the WHOLE signal at the end of the chain
        // Designed to be like a Juno chorus but deeper, darker, and lusher. Not muddy.
        if (chorusOn) {
            float sr = static_cast<float>(currentSampleRate);
            
            // Voice 1: deep, slow modulation, panned wide. High-cut at 1800Hz, Low-cut at 130Hz.
            float wetL1 = chorusL1.process(combinedL, 0.55f, 6.5f, 18.0f, sr, 0.0f, 1800.0f, 130.0f);
            float wetR1 = chorusR1.process(combinedR, 0.55f, 6.5f, 18.0f, sr, 0.5f, 1800.0f, 130.0f);
            
            // Voice 2: faster modulation for lush density, offset phase. High-cut at 1600Hz, Low-cut at 140Hz.
            float wetL2 = chorusL2.process(combinedL, 0.85f, 4.2f, 24.0f, sr, 0.25f, 1600.0f, 140.0f);
            float wetR2 = chorusR2.process(combinedR, 0.85f, 4.2f, 24.0f, sr, 0.75f, 1600.0f, 140.0f);
            
            // Sum voices
            float wetL = 0.5f * (wetL1 + wetL2);
            float wetR = 0.5f * (wetR1 + wetR2);
            
            // Cross-mix with phase inversion for extreme wide Juno-style spaciousness
            float wideL = wetL - wetR;
            float wideR = wetR - wetL;
            
            // Mix dry and wide wet signals with gain compensation to prevent level drops
            combinedL = combinedL * 0.85f + wideL * 0.5f;
            combinedR = combinedR * 0.85f + wideR * 0.5f;
            
            // Set dryScale to 0 because we do NOT want the master Dry/Wet knob to remove the dry component from the chorus
            dryScaleL = 0.0f;
            dryScaleR = 0.0f;
        }
        
        // Apply Granulizer to the WHOLE signal just before the output stage
        int granBufSize = granRingBuffer.getNumSamples();
        if (granBufSize > 0) {
            granRingBuffer.setSample(0, granWritePos, combinedL);
            if (granRingBuffer.getNumChannels() > 1) {
                granRingBuffer.setSample(1, granWritePos, combinedR);
            }
        }
        
        float granOutL = 0.0f;
        float granOutR = 0.0f;
        
        if (granOn && granBufSize > 0) {
            int granSizeSamples = static_cast<int>((granSizeMs / 1000.0f) * currentSampleRate);
            granSizeSamples = std::max(64, granSizeSamples);
            
            int spawnInterval = granSizeSamples / 4;
            if (spawnInterval < 16) spawnInterval = 16;
            
            granSamplesSinceLastSpawn++;
            if (granSamplesSinceLastSpawn >= spawnInterval) {
                granSamplesSinceLastSpawn = 0;
                
                // Spawn a new grain
                for (auto& grain : granGrains) {
                    if (!grain.active) {
                        grain.active = true;
                        grain.currentOffset = 0;
                        grain.duration = granSizeSamples;
                        grain.speed = std::pow(2.0f, granPitch / 12.0f);
                        
                        // Equal-power panning
                        float p = random.nextFloat();
                        grain.panL = std::cos(p * juce::MathConstants<float>::halfPi);
                        grain.panR = std::sin(p * juce::MathConstants<float>::halfPi);
                        
                        // Tight, speed-compensated offset to keep playback in sync with the dry signal
                        float grainDurationSecs = granSizeMs / 1000.0f;
                        float minOffsetSecs = 0.010f; // 10ms baseline to avoid write pointer collision
                        if (grain.speed > 1.0f) {
                            minOffsetSecs += grainDurationSecs * (grain.speed - 1.0f);
                        }
                        float maxOffsetSecs = minOffsetSecs + 0.020f; // tight 20ms random variation
                        
                        float offsetSecs = minOffsetSecs + random.nextFloat() * (maxOffsetSecs - minOffsetSecs);
                        int offsetSamples = static_cast<int>(offsetSecs * currentSampleRate);
                        
                        double start = granWritePos - offsetSamples;
                        while (start < 0) start += granBufSize;
                        grain.startPos = start;
                        break;
                    }
                }
            }
            
            // Synthesize active grains
            for (auto& grain : granGrains) {
                if (grain.active) {
                    float progress = static_cast<float>(grain.currentOffset) / grain.duration;
                    if (progress >= 1.0f) {
                        grain.active = false;
                        continue;
                    }
                    
                    // Raised-cosine envelope with 5ms attack and 5ms release to prevent clicking
                    float attackTimeMs = 5.0f;
                    float releaseTimeMs = 5.0f;
                    int attackSamples = static_cast<int>((attackTimeMs / 1000.0f) * currentSampleRate);
                    int releaseSamples = static_cast<int>((releaseTimeMs / 1000.0f) * currentSampleRate);
                    
                    if (attackSamples + releaseSamples > grain.duration) {
                        attackSamples = grain.duration / 2;
                        releaseSamples = grain.duration / 2;
                    }
                    
                    float envelope = 1.0f;
                    if (attackSamples > 0 && grain.currentOffset < attackSamples) {
                        float prog = static_cast<float>(grain.currentOffset) / static_cast<float>(attackSamples);
                        envelope = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::pi * prog);
                    } else if (releaseSamples > 0 && grain.currentOffset > grain.duration - releaseSamples) {
                        float prog = static_cast<float>(grain.duration - grain.currentOffset) / static_cast<float>(releaseSamples);
                        envelope = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::pi * prog);
                    }
                    
                    double readPos = grain.startPos + (grain.currentOffset * grain.speed);
                    while (readPos < 0.0) readPos += granBufSize;
                    while (readPos >= granBufSize) readPos -= granBufSize;
                    
                    int idx1 = static_cast<int>(std::floor(readPos));
                    int idx2 = (idx1 + 1) % granBufSize;
                    float frac = static_cast<float>(readPos - idx1);
                    
                    float sL = granRingBuffer.getSample(0, idx1) * (1.0f - frac) + granRingBuffer.getSample(0, idx2) * frac;
                    float sR = (granRingBuffer.getNumChannels() > 1) ? 
                        (granRingBuffer.getSample(1, idx1) * (1.0f - frac) + granRingBuffer.getSample(1, idx2) * frac) : sL;
                    
                    granOutL += sL * envelope * grain.panL;
                    granOutR += sR * envelope * grain.panR;
                    
                    grain.currentOffset++;
                }
            }
            
            // Scale output to keep gain consistent
            granOutL *= 1.5f;
            granOutR *= 1.5f;
            
            // Additive mixing (level-knob style) so dry signal volume is untouched
            combinedL = combinedL + granOutL * granMix;
            combinedR = combinedR + granOutR * granMix;
        }
        
        if (granBufSize > 0) {
            granWritePos++;
            if (granWritePos >= granBufSize) granWritePos = 0;
        }
        
        float finalL = 0.0f;
        float finalR = 0.0f;
        
        float finalDryL = dryL * dryScaleL;
        float finalDryR = dryR * dryScaleR;
        
        // Separate the pure wet effects from the combined signal
        float pureWetL = combinedL - finalDryL;
        float pureWetR = combinedR - finalDryR;
        
        // Apply the master wet filters to the entire wet chain
        float filteredWetL = wetLpfL.processSample(wetHpfL.processSample(pureWetL));
        float filteredWetR = wetLpfR.processSample(wetHpfR.processSample(pureWetR));
        
        if (dryWet <= 0.5f) {
            // First 50% of the knob: blend from pure clean dry to full processed signal (which contains dry and wet)
            float wetMix = dryWet * 2.0f;
            float outL = std::tanh(finalDryL + filteredWetL);
            float outR = std::tanh(finalDryR + filteredWetR);
            finalL = (inputL * (1.0f - wetMix) + outL * wetMix) * outGain;
            finalR = (inputR * (1.0f - wetMix) + outR * wetMix) * outGain;
        } else {
            // Last 50% of the knob: fade out the dry signal component to 0, leaving only the wet effects
            float dryFade = 2.0f * (1.0f - dryWet);
            float outL = std::tanh(filteredWetL + finalDryL * dryFade);
            float outR = std::tanh(filteredWetR + finalDryR * dryFade);
            finalL = outL * outGain;
            finalR = outR * outGain;
        }
        
        buffer.setSample(0, sample, finalL);
        if (totalNumOutputChannels > 1) {
            buffer.setSample(1, sample, finalR);
        }
        
        // Write to vectorscope circular buffer
        int sIdx = scopeWriteIndex.load();
        scopeBufferL[sIdx] = finalL;
        scopeBufferR[sIdx] = finalR;
        scopeWriteIndex.store((sIdx + 1) % scopeBufferSize);
    }
    
    // Smooth levels for visual meters
    float d1Lvl = delay1OutputLevel.load();
    if (d1PeakThisBlock > d1Lvl) d1Lvl = d1PeakThisBlock;
    else d1Lvl += 0.05f * (d1PeakThisBlock - d1Lvl);
    delay1OutputLevel.store(d1Lvl);
    
    float d2Lvl = delay2OutputLevel.load();
    if (d2PeakThisBlock > d2Lvl) d2Lvl = d2PeakThisBlock;
    else d2Lvl += 0.05f * (d2PeakThisBlock - d2Lvl);
    delay2OutputLevel.store(d2Lvl);

    // Update meters for UI
    float currentPosNorm = 0.0f;
    if (d1BufferSize > 0) {
        currentPosNorm = static_cast<float>(delay1WritePosition) / static_cast<float>(d1BufferSize);
    }
    currentLoopPos.store(currentPosNorm);
}

juce::AudioProcessorValueTreeState::ParameterLayout MICROLOOPAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // Delay parameters (consolidated)
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("DELAY_ON", 1), "Delay On", true));
    
    juce::StringArray delayDivisions {
        "1/1", "1/1D", "1/1T",
        "1/2", "1/2D", "1/2T",
        "1/4", "1/4D", "1/4T",
        "1/8", "1/8D", "1/8T",
        "1/16", "1/16D", "1/16T",
        "1/32", "1/32D", "1/32T",
        "1/64", "1/64D", "1/64T"
    };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("DELAY1_TEMPO", 1), "Delay 1 Sync", delayDivisions, 12)); // default 1/16 straight
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("DELAY2_TEMPO", 1), "Delay 2 Sync", delayDivisions, 12)); // default 1/16 straight
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("DELAY_REVERSE", 1), "Delay Reverse Mode", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("DELAY_DRYWET", 1), "Delay Gain (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("DELAY_DUCK", 1), "Delay Duck (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("DELAY_FEEDBACK", 1), "Delay Feedback (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("DELAY_OFFSET", 1), "Delay Offset (%)", juce::NormalisableRange<float>(-20.0f, 20.0f, 1.0f), 0.0f));
    
    // Chorus
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("CHORUS_ON", 1), "Chorus On", false));
    
    // Master Wet Width/Filters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("WET_WIDTH", 1), "Master Width (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("WET_LOWCUT", 1), "Wet Low Cut (Hz)", juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("WET_HIGHCUT", 1), "Wet High Cut (Hz)", juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f), 20000.0f));
    
    // Tape Simulation
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("TAPE_ON", 1), "Tape On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("TAPE_DESTABILIZE", 1), "Destabilize (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("TAPE_AGE", 1), "Tape Age (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("TAPE_SAT", 1), "Tape Saturation (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 25.0f));
    
    // Glitch
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("GLITCH_ON", 1), "Glitch On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GLITCH_AMOUNT", 1), "Glitch Amount (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 35.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("GLITCH_REVERSE", 1), "Glitch Reverse", false));
    
    // Granulizer
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("GRAN_ON", 1), "Granulizer On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GRAN_SIZE", 1), "Grain Size (ms)", juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GRAN_PITCH", 1), "Grain Pitch (st)", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("GRAN_MIX", 1), "Granulizer Mix (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f));
    
    // Global output controls
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("DRY_WET", 1), "Dry/Wet (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("MASTER_OUTPUT", 1), "Master Output (dB)", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    
    // Optical Distortion
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("OPTO_ON", 1), "Optical Distortion On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("OPTO_GAIN", 1), "Opto Gain (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f));
    
    return { params.begin(), params.end() };
}

bool MICROLOOPAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MICROLOOPAudioProcessor::createEditor()
{
    return new MICROLOOPAudioProcessorEditor (*this);
}

void MICROLOOPAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    if (auto xmlState = apvts.copyState().createXml())
        copyXmlToBinary (*xmlState, destData);
}

void MICROLOOPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MICROLOOPAudioProcessor();
}
