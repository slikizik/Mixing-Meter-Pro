#include "PluginProcessor.h"
#include "PluginEditor.h"

Mixing_Assistant_ProAudioProcessorEditor::Mixing_Assistant_ProAudioProcessorEditor (Mixing_Assistant_ProAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      webView (juce::WebBrowserComponent::Options()
               .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
               .withResourceProvider ([] (const juce::String& url) -> std::optional<juce::WebBrowserComponent::Resource>
               {
                   // Intercept requests for the root or meter.html and serve the embedded binary data
                   if (url == "/" || url == "/meter.html" || url == "meter.html")
                   {
                       return juce::WebBrowserComponent::Resource {
                           (const std::byte*) BinaryData::meter_html,
                           (size_t) BinaryData::meter_htmlSize,
                           "text/html"
                       };
                   }
                   return std::nullopt;
               }, juce::URL("https://juce.backend")))
{
    addAndMakeVisible (webView);
    setSize (920, 670);

    // Request the correct embedded file name
    webView.goToURL (juce::WebBrowserComponent::getResourceProviderRoot() + "meter.html");

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
