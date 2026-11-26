cmake_minimum_required(VERSION 3.22)
project(MixAura VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(MixAura
    COMPANY_NAME "MixAura"
    PLUGIN_MANUFACTURER_CODE Mxar
    PLUGIN_CODE Mxa1
    FORMATS VST3 Standalone
    PRODUCT_NAME "MIX AURA ALL-IN-ONE"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(MixAura PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/AvatarComponent.cpp)
target_compile_definitions(MixAura PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(MixAura PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
