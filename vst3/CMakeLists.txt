cmake_minimum_required(VERSION 3.22)
project(Celeste VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
if(MINGW)
 add_link_options(-static-libgcc -static-libstdc++ -static)
endif()
set(JUCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build/JUCE7" CACHE PATH "JUCE 7.0.12 source directory")
add_subdirectory("${JUCE_SOURCE_DIR}" JUCE)
# Keep Windows version resources in source; no generated helper executable is needed.
if(WIN32)
 function(_juce_add_resources_rc source_target dest_target)
  target_sources(${dest_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Version.rc")
 endfunction()
endif()
set(CELESTE_FORMATS VST3 Standalone)
if(APPLE)
 list(APPEND CELESTE_FORMATS AU)
endif()
juce_add_plugin(Celeste COMPANY_NAME "Celeste Lab" BUNDLE_ID "audio.celeste.parallel"
 PLUGIN_MANUFACTURER_CODE Clst PLUGIN_CODE Cpf1 FORMATS ${CELESTE_FORMATS}
 PRODUCT_NAME "CELESTE Parallel" IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
 COPY_PLUGIN_AFTER_BUILD FALSE VST3_AUTO_MANIFEST FALSE VST3_CATEGORIES Fx Delay Modulation
 MICROPHONE_PERMISSION_ENABLED TRUE
 MICROPHONE_PERMISSION_TEXT "CELESTE processes audio from your selected input device.")
target_sources(Celeste PRIVATE Plugin.cpp)
target_compile_definitions(Celeste PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(Celeste PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags)
juce_add_console_app(CelesteCheck PRODUCT_NAME "Celeste DSP checks")
target_sources(CelesteCheck PRIVATE Check.cpp)
target_compile_definitions(CelesteCheck PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(CelesteCheck PRIVATE juce::juce_dsp juce::juce_audio_formats)
juce_add_console_app(CelestePreview PRODUCT_NAME "Celeste preview")
target_sources(CelestePreview PRIVATE Preview.cpp)
target_link_libraries(CelestePreview PRIVATE Celeste juce::juce_audio_utils)

juce_add_console_app(CelesteTangCheck PRODUCT_NAME "Celeste Tang checks")
target_sources(CelesteTangCheck PRIVATE TangCheck.cpp)
target_link_libraries(CelesteTangCheck PRIVATE juce::juce_audio_utils)
target_compile_definitions(CelesteTangCheck PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
