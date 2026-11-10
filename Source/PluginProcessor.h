#pragma once
#include <JuceHeader.h>

class Mixing_Assistant_ProAudioProcessor : public juce::AudioProcessor
{
public:
    Mixing_Assistant_ProAudioProcessor();
    ~Mixing_Assistant_ProAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
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

    struct MeterTelemetry {
        float mainPeak = -60.0f;
        float mainRms  = -60.0f;
        float mainLufs = -60.0f;
        float scPeak   = -60.0f;
        float scLufs   = -60.0f;
        float crestFactor = 12.0f;
        float stereoWidth = 60.0f;
        float correlation = 0.9f;
        float spectrum[7] = {-12, -10, -14, -16, -18, -22, -26};
        float scPills[5]  = {50.0f, 35.0f, 45.0f, 52.0f, 60.0f};
        bool  isSibilant  = false;
        int   score       = 78;
    };

    MeterTelemetry getLatestTelemetry() const { return telemetry; }

private:
    MeterTelemetry telemetry;
    juce::dsp::FFT forwardFFT { 10 };
    std::array<float, 2048> fftData;
    int fftIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mixing_Assistant_ProAudioProcessor)
};
