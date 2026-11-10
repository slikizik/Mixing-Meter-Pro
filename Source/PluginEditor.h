#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class Mixing_Assistant_ProAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    Mixing_Assistant_ProAudioProcessorEditor (Mixing_Assistant_ProAudioProcessor&);
    ~Mixing_Assistant_ProAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    Mixing_Assistant_ProAudioProcessor& audioProcessor;
    juce::WebBrowserComponent webView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mixing_Assistant_ProAudioProcessorEditor)
};
