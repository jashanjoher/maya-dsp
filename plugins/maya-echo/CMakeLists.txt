cmake_minimum_required(VERSION 3.22)
project(MayaEcho VERSION 0.1.0 LANGUAGES C CXX)

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

include(FetchContent)

# Eigen 3.4.0 (header-only, required by NAM).
FetchContent_Declare(eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz)

# nlohmann/json 3.11.3 (header-only, required by NAM).
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)

# NeuralAmpModelerCore at the same pinned commit as plugins/chamber-reverb.
FetchContent_Declare(nam
    GIT_REPOSITORY https://github.com/sdatkinson/NeuralAmpModelerCore.git
    GIT_TAG 0b3d3c97b0859a3a8c92a8628c4dd89a25eb5842
    GIT_SHALLOW TRUE
    GIT_SUBMODULES "")

FetchContent_MakeAvailable(eigen nlohmann_json)

# Download NAM sources only (Populate does not run NAM's CMake tree).
FetchContent_GetProperties(nam)
if (NOT nam_POPULATED)
    FetchContent_Populate(nam)
endif()

# Embedded default model, generated at configure time (same as chamber-reverb).
set(NAM_GEN_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${NAM_GEN_DIR}")
set(NAM_MODEL_URL "https://raw.githubusercontent.com/sdatkinson/NeuralAmpModelerCore/0b3d3c97b0859a3a8c92a8628c4dd89a25eb5842/example_models/wavenet_a1_standard.nam")
set(NAM_MODEL_FILE "${NAM_GEN_DIR}/wavenet_a1_standard.nam")
set(NAM_MODEL_HEADER "${NAM_GEN_DIR}/EmbeddedModel.h")
set(NAM_MODEL_EXPECTED_BYTES 407452)

if (NOT EXISTS "${NAM_MODEL_FILE}")
    file(DOWNLOAD "${NAM_MODEL_URL}" "${NAM_MODEL_FILE}"
         TIMEOUT 120 INACTIVITY_TIMEOUT 30
         STATUS NAM_DL_STATUS TLS_VERIFY ON)
    list(GET NAM_DL_STATUS 0 NAM_DL_CODE)
    if (NOT NAM_DL_CODE EQUAL 0)
        file(REMOVE "${NAM_MODEL_FILE}")
        message(FATAL_ERROR "Failed to download wavenet_a1_standard.nam: ${NAM_DL_STATUS}")
    endif()
endif()

file(SIZE "${NAM_MODEL_FILE}" NAM_MODEL_BYTES)
if (NOT NAM_MODEL_BYTES EQUAL NAM_MODEL_EXPECTED_BYTES)
    file(REMOVE "${NAM_MODEL_FILE}")
    message(FATAL_ERROR "wavenet_a1_standard.nam has ${NAM_MODEL_BYTES} bytes, expected ${NAM_MODEL_EXPECTED_BYTES}")
endif()

file(READ "${NAM_MODEL_FILE}" NAM_MODEL_HEX HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," NAM_MODEL_ARRAY "${NAM_MODEL_HEX}")
file(WRITE "${NAM_MODEL_HEADER}"
"#pragma once\n\n#include <cstddef>\n\n// Generated at configure time from NeuralAmpModelerCore wavenet_a1_standard.nam.\ninline constexpr unsigned char kWavenetA1Model[] = {\n${NAM_MODEL_ARRAY}\n};\n\ninline constexpr std::size_t kWavenetA1ModelSize = ${NAM_MODEL_BYTES};\n")

set(NLOHMANN_INCLUDE_DIRS
    "${nlohmann_json_SOURCE_DIR}/include"
    "${nlohmann_json_SOURCE_DIR}/include/nlohmann")

file(GLOB NAM_CORE_SOURCES
    "${nam_SOURCE_DIR}/NAM/*.cpp"
    "${nam_SOURCE_DIR}/NAM/wavenet/*.cpp")

add_library(nam_core STATIC ${NAM_CORE_SOURCES})
target_compile_features(nam_core PUBLIC cxx_std_20)
target_compile_definitions(nam_core PUBLIC NAM_SAMPLE_FLOAT)
target_include_directories(nam_core PUBLIC
    "${nam_SOURCE_DIR}"
    "${eigen_SOURCE_DIR}"
    ${NLOHMANN_INCLUDE_DIRS})
set_target_properties(nam_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

juce_add_plugin(MayaEcho
  COMPANY_NAME "Maya DSP"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  PLUGIN_MANUFACTURER_CODE Mayd
  PLUGIN_CODE Mech
  FORMATS VST3 AU Standalone
  PRODUCT_NAME "Maya Echo ME-24")

# NeuralDrive and CuteLookAndFeel are reused from plugins/chamber-reverb until they move to shared/.
target_sources(MayaEcho PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
  ../chamber-reverb/Source/dsp/NeuralDrive.cpp)

target_include_directories(MayaEcho PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Source"
  "${CMAKE_CURRENT_SOURCE_DIR}/../chamber-reverb/Source"
  "${NAM_GEN_DIR}"
  ${NLOHMANN_INCLUDE_DIRS})

target_compile_definitions(MayaEcho PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MayaEcho PRIVATE
  juce::juce_audio_basics
  juce::juce_audio_devices
  juce::juce_audio_formats
  juce::juce_audio_plugin_client
  juce::juce_audio_processors
  juce::juce_audio_utils
  juce::juce_dsp
  juce::juce_gui_basics
  juce::juce_gui_extra
  juce::juce_recommended_config_flags
  nam_core)
