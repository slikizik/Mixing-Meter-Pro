#include "PluginProcessor.h"
#include "PluginEditor.h"

Mixing_Assistant_ProAudioProcessor::Mixing_Assistant_ProAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
        .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true))
{
}

Mixing_Assistant_ProAudioProcessor::~Mixing_Assistant_ProAudioProcessor() {}

const juce::String Mixing_Assistant_ProAudioProcessor::getName() const { return JucePlugin_Name; }
bool Mixing_Assistant_ProAudioProcessor::acceptsMidi() const { return false; }
bool Mixing_Assistant_ProAudioProcessor::producesMidi() const { return false; }
bool Mixing_Assistant_ProAudioProcessor::isMidiEffect() const { return false; }
double Mixing_Assistant_ProAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int Mixing_Assistant_ProAudioProcessor::getNumPrograms() { return 1; }
int Mixing_Assistant_ProAudioProcessor::getCurrentProgram() { return 0; }
void Mixing_Assistant_ProAudioProcessor::setCurrentProgram (int index) {}
const juce::String Mixing_Assistant_ProAudioProcessor::getProgramName (int index) { return {}; }
void Mixing_Assistant_ProAudioProcessor::changeProgramName (int index, const juce::String& newName) {}
void Mixing_Assistant_ProAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {}
void Mixing_Assistant_ProAudioProcessor::releaseResources() {}

bool Mixing_Assistant_ProAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (auto* sc = layouts.getChannelSet (true, 1))
        if (! sc->isDisabled() && *sc != juce::AudioChannelSet::stereo())
            return false;

    return true;
}

void Mixing_Assistant_ProAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numSamples = buffer.getNumSamples();

    // 1. Process Main Track / Mix Bus (Bus 0: Channels 0 & 1)
    auto mainBus = getBusBuffer (buffer, true, 0);
    if (mainBus.getNumChannels() >= 2)
    {
        float leftPeak = mainBus.getMagnitude (0, 0, numSamples);
        float rightPeak = mainBus.getMagnitude (1, 0, numSamples);
        float peak = juce::jmax (leftPeak, rightPeak);
        float rms = mainBus.getRMSLevel (0, 0, numSamples);

        telemetry.mainPeak = juce::Decibels::gainToDecibels (peak, -60.0f);
        telemetry.mainRms  = juce::Decibels::gainToDecibels (rms, -60.0f);
        telemetry.mainLufs = telemetry.mainRms - 3.0f;
        telemetry.crestFactor = juce::jmax (0.0f, telemetry.mainPeak - telemetry.mainRms);

        float sumMid = 0.0f, sumSide = 0.0f, sumDot = 0.0f, sumL2 = 0.0f, sumR2 = 0.0f;
        auto* l = mainBus.getReadPointer(0);
        auto* r = mainBus.getReadPointer(1);

        for (int i = 0; i < numSamples; ++i)
        {
            float mid = 0.5f * (l[i] + r[i]);
            float side = 0.5f * (l[i] - r[i]);
            sumMid += mid * mid;
            sumSide += side * side;
            sumDot += l[i] * r[i];
            sumL2 += l[i] * l[i];
            sumR2 += r[i] * r[i];
        }

        float denom = std::sqrt (sumL2 * sumR2);
        telemetry.correlation = denom > 1e-6f ? (sumDot / denom) : 1.0f;
        telemetry.stereoWidth = sumMid > 1e-6f ? juce::jlimit (0.0f, 150.0f, (std::sqrt (sumSide / sumMid)) * 100.0f) : 0.0f;
    }

    // 2. Process Sidechain Input (Bus 1: Channels 2 & 3)
    if (getBusCount (true) > 1)
    {
        auto scBus = getBusBuffer (buffer, true, 1);
        if (scBus.getNumChannels() >= 2)
        {
            float scPeak = scBus.getMagnitude (0, 0, numSamples);
            float scRms  = scBus.getRMSLevel (0, 0, numSamples);
            telemetry.scPeak = juce::Decibels::gainToDecibels (scPeak, -60.0f);
            telemetry.scLufs = juce::Decibels::gainToDecibels (scRms, -60.0f) - 3.0f;
        }
    }
}

bool Mixing_Assistant_ProAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* Mixing_Assistant_ProAudioProcessor::createEditor()
{
    return new Mixing_Assistant_ProAudioProcessorEditor (*this);
}
void Mixing_Assistant_ProAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {}
void Mixing_Assistant_ProAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Mixing_Assistant_ProAudioProcessor();
}
