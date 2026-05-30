#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"

class MICROLOOPAudioProcessor  : public juce::AudioProcessor, public juce::Timer
{
public:
    MICROLOOPAudioProcessor();
    ~MICROLOOPAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // APVTS for our Tape Distortion parameters
    juce::AudioProcessorValueTreeState apvts;
    
    bool isLicensed = false;
    void validateLicense();
    void timerCallback() override;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Live UI Meters
    std::atomic<float> currentLoopPos { 0.0f };
    std::atomic<float> currentMasterGR { 0.0f };
    std::atomic<float> delay1OutputLevel { 0.0f };
    std::atomic<float> delay2OutputLevel { 0.0f };
    
    // Scope data circular buffer
    static constexpr int scopeBufferSize = 512;
    float scopeBufferL[scopeBufferSize] = { 0.0f };
    float scopeBufferR[scopeBufferSize] = { 0.0f };
    std::atomic<int> scopeWriteIndex { 0 };
    
    // Preset Manager
    std::unique_ptr<PresetManager> presetManager;

private:
    std::atomic<float>* delayOnParam = nullptr;
    std::atomic<float>* delay1TempoParam = nullptr;
    std::atomic<float>* delay2TempoParam = nullptr;
    std::atomic<float>* delayOffsetParam = nullptr;
    std::atomic<float>* delayReverseParam = nullptr;
    std::atomic<float>* delayFeedbackParam = nullptr;
    std::atomic<float>* delayDryWetParam = nullptr;
    std::atomic<float>* delayDuckParam = nullptr;
    
    std::atomic<float>* chorusOnParam = nullptr;
    
    std::atomic<float>* wetWidthParam = nullptr;
    std::atomic<float>* wetLowCutParam = nullptr;
    std::atomic<float>* wetHighCutParam = nullptr;
    
    std::atomic<float>* tapeOnParam = nullptr;
    std::atomic<float>* tapeDestabilizeParam = nullptr;
    std::atomic<float>* tapeAgeParam = nullptr;
    std::atomic<float>* tapeSatParam = nullptr;
    
    std::atomic<float>* dryWetParam = nullptr;
    std::atomic<float>* masterOutputParam = nullptr;
    
    std::atomic<float>* glitchOnParam = nullptr;
    std::atomic<float>* glitchReverseParam = nullptr;
    std::atomic<float>* glitchAmountParam = nullptr;
    
    std::atomic<float>* granOnParam = nullptr;
    std::atomic<float>* granSizeParam = nullptr;
    std::atomic<float>* granPitchParam = nullptr;
    std::atomic<float>* granMixParam = nullptr;

    std::atomic<float>* optoOnParam = nullptr;
    std::atomic<float>* optoGainParam = nullptr;
    float optoLdrState = 0.0f;

    float glitchPlaybackSpeed = 1.0f;

    double currentSampleRate = 44100.0;
    float delay1Envelope = 0.0f;
    float delay2Envelope = 0.0f;
    
    // Delay 1 DSP
    juce::AudioBuffer<float> delay1Buffer;
    int delay1WritePosition = 0;
    double delay1ReversePhase = 0.0;
    double smoothedDelay1TimeSamples = -1.0;
    float delay1DuckEnv = 0.0f;
    
    // Delay 2 DSP
    juce::AudioBuffer<float> delay2Buffer;
    int delay2WritePosition = 0;
    double delay2ReversePhase = 0.0;
    double smoothedDelay2TimeSamples = -1.0;
    float delay2DuckEnv = 0.0f;
    
    // Glitch DSP
    juce::AudioBuffer<float> circularBuffer;
    int writePosition = 0;
    double glitchSampleCounter = 0.0;
    bool isGlitchLooping = false;
    int glitchStartPos = 0;
    int glitchLengthSamples = 0;
    float glitchPhase = 0.0f;
    int glitchEighthNotesRemaining = 0;
    float glitchEnvelope = 0.0f;
    float glitchDuckEnv = 0.0f;
    bool glitchReverse = false;
    
    struct TapeEffectState {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { 48000 };
        float wowPhase = 0.0f;
        bool isDipping = false;
        int dipState = 0;
        float dipCurrentOffset = 0.0f;
        float dipTargetDepth = 0.0f;
        float dipRate = 0.0f;
        float wowSpeedScale = 1.0f;
        
        void prepare(const juce::dsp::ProcessSpec& spec, float speedScale) {
            delayLine.prepare(spec);
            delayLine.setMaximumDelayInSamples(spec.sampleRate * 0.2); // 200ms max
            wowPhase = 0.0f;
            isDipping = false;
            dipState = 0;
            dipCurrentOffset = 0.0f;
            dipTargetDepth = 0.0f;
            dipRate = 0.0f;
            wowSpeedScale = speedScale;
        }
        
        void updateOffset(double sampleRate, float tapeWow, float tapeFlutter, juce::Random& random)
        {
            wowPhase += (1.0f * wowSpeedScale) / static_cast<float>(sampleRate);
            if (wowPhase >= 1.0f) wowPhase -= 1.0f;
            
            if (tapeFlutter > 0.0f) {
                if (!isDipping) {
                    float triggerChance = tapeFlutter * 0.0001f;
                    if (random.nextFloat() < triggerChance) {
                        isDipping = true;
                        dipState = 0;
                        dipTargetDepth = (random.nextFloat() * 15.0f + 5.0f) * tapeFlutter;
                        dipRate = 1.0f / (static_cast<float>(sampleRate) * (random.nextFloat() * 0.035f + 0.015f));
                    }
                }
                if (isDipping) {
                    if (dipState == 0) {
                        dipCurrentOffset += (dipTargetDepth - dipCurrentOffset) * dipRate;
                        if (dipCurrentOffset >= dipTargetDepth * 0.95f) {
                            dipState = 1;
                            dipRate = 1.0f / (static_cast<float>(sampleRate) * (random.nextFloat() * 0.15f + 0.05f));
                        }
                    } else {
                        dipCurrentOffset += (0.0f - dipCurrentOffset) * dipRate;
                        if (dipCurrentOffset <= 0.02f) {
                            dipCurrentOffset = 0.0f;
                            isDipping = false;
                        }
                    }
                }
            } else {
                dipCurrentOffset = 0.0f;
                isDipping = false;
            }
        }
        
        float getOffsetSamples(int channel, double sampleRate, float tapeWow, float tapeFlutter) const
        {
            float wowOffset = std::sin(wowPhase * juce::MathConstants<float>::twoPi) * tapeWow * 6.0f; // up to 6ms wow
            float totalOffsetMs = 0.0f;
            if (channel == 0) {
                totalOffsetMs = wowOffset + dipCurrentOffset;
            } else {
                totalOffsetMs = wowOffset * 0.9f + dipCurrentOffset * 1.1f;
            }
            return totalOffsetMs * static_cast<float>(sampleRate) / 1000.0f;
        }
    };
    
    TapeEffectState tapeDry;
    TapeEffectState tapeDelay1;
    TapeEffectState tapeDelay2;
    juce::dsp::IIR::Filter<float> tapeFilterL, tapeFilterR;
    
    // Master Wet Filters (identically structured L/R to avoid phase issues)
    juce::dsp::IIR::Filter<float> wetHpfL, wetHpfR;
    juce::dsp::IIR::Filter<float> wetLpfL, wetLpfR;
    juce::dsp::IIR::Filter<float> glitchHpfL, glitchHpfR;
    juce::dsp::IIR::Filter<float> glitchLpfL, glitchLpfR;
    
    struct ChorusVoice {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { 48000 };
        float lfoPhase = 0.0f;
        float lpfState = 0.0f;
        float hpfState = 0.0f;
        
        void prepare(const juce::dsp::ProcessSpec& spec) {
            delayLine.prepare(spec);
            delayLine.setMaximumDelayInSamples(spec.sampleRate * 0.15); // 150ms max
            lfoPhase = 0.0f;
            lpfState = 0.0f;
            hpfState = 0.0f;
        }
        
        float process(float input, float rateHz, float depthMs, float baseDelayMs, float sampleRate, float phaseOffset, float lpCutoffHz, float hpCutoffHz) {
            lfoPhase += rateHz / sampleRate;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
            
            float phase = lfoPhase + phaseOffset;
            if (phase >= 1.0f) phase -= 1.0f;
            if (phase < 0.0f) phase += 1.0f;
            
            float lfoOut = std::sin(phase * juce::MathConstants<float>::twoPi);
            float currentDelayMs = baseDelayMs + lfoOut * depthMs;
            float delaySamples = currentDelayMs * sampleRate / 1000.0f;
            
            delayLine.pushSample(0, input);
            float delayed = delayLine.popSample(0, delaySamples);
            
            // 1. One-pole HPF (highpass) to remove low-frequency mud
            float dt = 1.0f / sampleRate;
            float rc_hp = 1.0f / (juce::MathConstants<float>::twoPi * hpCutoffHz);
            float alpha_hp = dt / (rc_hp + dt);
            hpfState += alpha_hp * (delayed - hpfState);
            float hpfOut = delayed - hpfState;
            
            // 2. One-pole LPF (lowpass) to darken the wet chorus
            float rc_lp = 1.0f / (juce::MathConstants<float>::twoPi * lpCutoffHz);
            float alpha_lp = dt / (rc_lp + dt);
            lpfState += alpha_lp * (hpfOut - lpfState);
            
            return lpfState;
        }
    };
    
    ChorusVoice chorusL1, chorusL2;
    ChorusVoice chorusR1, chorusR2;
    
    // Granulizer DSP
    juce::AudioBuffer<float> granRingBuffer;
    int granWritePos = 0;
    int granSamplesSinceLastSpawn = 0;
    
    struct GranularGrain {
        double startPos = 0.0;
        int currentOffset = 0;
        int duration = 0;
        float speed = 1.0f;
        float panL = 0.707f;
        float panR = 0.707f;
        bool active = false;
    };
    std::array<GranularGrain, 12> granGrains;
    
    // Tape Compression Envelope
    float tapeCompEnv = 0.0f;
    
    float lastWetLowCut = -1.0f;
    float lastWetHighCut = -1.0f;
    double lastSampleRate = -1.0;
    
    void updateParameters();
    float noiseFilterL = 0.0f;
    float noiseFilterR = 0.0f;
    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MICROLOOPAudioProcessor)
};
