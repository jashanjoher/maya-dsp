cmake_minimum_required(VERSION 3.22)
project(MayaChamber VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE 8 is fetched via FetchContent unless JUCE_DIR points to a local checkout.
if(NOT JUCE_DIR)
  include(FetchContent)
  FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
else()
  add_subdirectory(${JUCE_DIR} JUCE)
endif()

juce_add_plugin(MayaChamber
  COMPANY_NAME "Maya DSP"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  PLUGIN_MANUFACTURER_CODE Mayd
  PLUGIN_CODE Mchm
  FORMATS VST3 AU Standalone
  PRODUCT_NAME "Maya Chamber")

target_sources(MayaChamber PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
  Source/ui/MayaIndustrialLookAndFeel.h
  Source/dsp/EarlyReflections.cpp
  Source/dsp/LateFDN.cpp
  Source/dsp/ResonatorBank.cpp
  Source/dsp/Environments.cpp)

target_compile_definitions(MayaChamber PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MayaChamber PRIVATE
  juce::juce_audio_basics
  juce::juce_audio_devices
  juce::juce_audio_formats
  juce::juce_audio_plugin_client
  juce::juce_audio_processors
  juce::juce_audio_utils
  juce::juce_dsp
  juce::juce_gui_basics
  juce::juce_gui_extra
  juce::juce_recommended_config_flags)
