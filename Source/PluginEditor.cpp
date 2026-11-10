#include "PluginProcessor.h"
#include "PluginEditor.h"

Mixing_Assistant_ProAudioProcessorEditor::Mixing_Assistant_ProAudioProcessorEditor (Mixing_Assistant_ProAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      webView (juce::WebBrowserComponent::Options()
               .withBackend (juce::WebBrowserComponent::Options::Backend::webview2))
{
    addAndMakeVisible (webView);
    setSize (920, 670);

    auto htmlData = juce::String::createStringFromData (
        BinaryData::meter_html,
        BinaryData::meter_htmlSize
    );
    webView.goToURL (juce::WebBrowserComponent::getResourceProviderRoot() + "index.html");

    startTimerHz (60);
}

Mixing_Assistant_ProAudioProcessorEditor::~Mixing_Assistant_ProAudioProcessorEditor()
{
    stopTimer();
}

void Mixing_Assistant_ProAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void Mixing_Assistant_ProAudioProcessorEditor::resized()
{
    webView.setBounds (getLocalBounds());
}

void Mixing_Assistant_ProAudioProcessorEditor::timerCallback()
{
    auto data = audioProcessor.getLatestTelemetry();

    juce::String js = juce::String::formatted (
        "window.postMessage({ "
        "  type: 'meter-update', "
        "  mainLufs: %f, "
        "  mainPeak: %f, "
        "  scLufs: %f, "
        "  scPeak: %f, "
        "  crest: %f, "
        "  stereoWidth: %f, "
        "  correlation: %f, "
        "  isSibilance: %s, "
        "  score: %d "
        "}, '*');",
        data.mainLufs,
        data.mainPeak,
        data.scLufs,
        data.scPeak,
        data.crestFactor,
        data.stereoWidth,
        data.correlation,
        (data.isSibilant ? "true" : "false"),
        data.score
    );

    webView.evaluateJavascript (js);
}
