cmake_minimum_required(VERSION 3.24)

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS" FORCE)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
project(AFTERGLOW VERSION 1.0.0)

# Build Universal Binary for macOS (Apple Silicon + Intel)
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS" FORCE)
endif()

# We use FetchContent to automatically download JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4   # Use JUCE 8 for macOS 15 compatibility
)
FetchContent_MakeAvailable(juce)

# Create the plugin target
juce_add_plugin(AFTERGLOW
    PLUGIN_NAME "AFTERGLOW"
    COMPANY_NAME "PNK NOISE"
    BUNDLE_ID "com.pnknoise.afterglow"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE PNKN
    PLUGIN_CODE Aftg
    NEEDS_WEBVIEW2 TRUE
    VST3_AUTO_MANIFEST FALSE
    FORMATS Standalone VST3 AU
)

juce_generate_juce_header(AFTERGLOW)

target_sources(AFTERGLOW
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/PresetManager.cpp
)

juce_add_binary_data(WebAssets
    HEADER_NAME "WebAssets.h"
    NAMESPACE WebAssets
    SOURCES
        Design/index.html
        Design/AFTERGLOW.png
        Design/bg_inspired.png
        Design/button_matte.png
        "Design/KNOB/Long shadow.png"
        "Design/KNOB/KNOB MOVE.png"
        "Design/KNOB/KNOB STILLA.png"
        "Design/KNOB/KNOB PEKARE.png"
)

target_compile_definitions(AFTERGLOW
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_WIN_WEBVIEW2=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link everything together, and enable the dsp module
target_link_libraries(AFTERGLOW
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
        WebAssets
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
