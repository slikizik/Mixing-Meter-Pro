cmake_minimum_required(VERSION 3.22)
project(Mixing_Assistant_Pro VERSION 2.0.0)

# Pin JUCE to the 8.0.4 commit containing WebBrowserComponent resource-provider support.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        51d11a2be6d5c97ccf12b4e5e827006e19f0555a
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Mixing_Assistant_Pro
    COMPANY_NAME "Slikizik"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    NEEDS_WEBVIEW2 TRUE
    PLUGIN_MANUFACTURER_CODE Slik
    PLUGIN_CODE Map1
    FORMATS VST3
    PRODUCT_NAME "Mixing_Assistant_Pro"
)

# Enable WebView2-specific APIs, including the resource provider, in JUCE headers.
target_compile_definitions(Mixing_Assistant_Pro PRIVATE
    $<$<PLATFORM_ID:Windows>:JUCE_USE_WIN_WEBVIEW2=1>
)

# Ensure the generated JUCE header exists for all source files.
# This is required for <JuceHeader.h> includes in the plugin sources.
juce_generate_juce_header(Mixing_Assistant_Pro)

target_include_directories(Mixing_Assistant_Pro PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(Mixing_Assistant_Pro PRIVATE
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
)

# Embed meter.html as binary data
juce_add_binary_data(Mixing_Assistant_Pro_Data SOURCES
    Resources/meter.html
)

target_link_libraries(Mixing_Assistant_Pro PRIVATE
    Mixing_Assistant_Pro_Data
    juce::juce_dsp
    juce::juce_audio_utils
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
)
