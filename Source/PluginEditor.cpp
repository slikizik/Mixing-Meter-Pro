#include "PluginProcessor.h"
#include "PluginEditor.h"

Mixing_Assistant_ProAudioProcessorEditor::Mixing_Assistant_ProAudioProcessorEditor (Mixing_Assistant_ProAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      webView (juce::WebBrowserComponent::Options()
                .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
                .withResourceProvider ([] (const juce::String& url) -> std::optional<juce::WebBrowserComponent::Resource>
                {
                    if (url == "/" || url == "/meter.html" || url == "meter.html")
                    {
                        const auto* dataBegin = reinterpret_cast<const std::byte*> (BinaryData::meter_html);
                        const auto* dataEnd = dataBegin + BinaryData::meter_htmlSize;

                        return juce::WebBrowserComponent::Resource {
                            std::vector<std::byte> { dataBegin, dataEnd },
                            "text/html"
                        };
                    }

                    return std::nullopt;
                }))
{
    addAndMakeVisible (webView);
    setSize (920, 670);

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
