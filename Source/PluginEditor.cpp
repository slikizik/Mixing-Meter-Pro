#include "PluginProcessor.h"
#include "PluginEditor.h"

Mixing_Assistant_ProAudioProcessorEditor::Mixing_Assistant_ProAudioProcessorEditor (Mixing_Assistant_ProAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      webView (juce::WebBrowserComponent::Options()
               .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
               .withResourceProvider ([] (const juce::String& url) -> std::optional<juce::WebBrowserComponent::Resource>
               {
                   // Serve the embedded HTML UI when requested
                   if (url == "/" || url == "/meter.html" || url == "meter.html")
                   {
                       const auto* data = reinterpret_cast<const std::byte*> (BinaryData::meter_html);
                       return juce::WebBrowserComponent::Resource {
                           std::vector<std::byte> (data, data + BinaryData::meter_htmlSize),
                           juce::String { "text/html" }
                       };
                   }
                   return std::nullopt;
               }))
{
    addAndMakeVisible (webView);
    setSize (920, 670);

    // Navigate to the file bundled in BinaryData
    webView.goToURL (juce::WebBrowserComponent::getResourceProviderRoot() + "meter.html");

    // UI Refresh rate (60 FPS)
    startTimerHz (60);
}

Mixing_Assistant_ProAudioProcessorEditor::~Mixing_Assistant_ProAudioProcessorEditor()
{
    stopTimer();
}

void Mixing_Assistant_ProAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fallback background color if web view fails to load or is transparent
    g.fillAll (juce::Colours::black);
}

void Mixing_Assistant_ProAudioProcessorEditor::resized()
{
    // CRITICAL: Ensure the web view expands to fill the entire plugin window
    webView.setBounds (getLocalBounds());
}

void Mixing_Assistant_ProAudioProcessorEditor::timerCallback()
{
    // Fetch DSP data from processor
    auto data = audioProcessor.getLatestTelemetry();

    // Format JSON payload to send to meter.html
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

    // Send data to frontend
    webView.evaluateJavascript (js);
}
