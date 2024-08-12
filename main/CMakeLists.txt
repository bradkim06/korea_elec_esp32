set(eeg_srcs
    "Bpmpulse.cpp"
    "Fulldowndetect.cpp"
    "GetRawData.cpp"
    "LeadOff.cpp"
    "main.cpp"
    "Uart.cpp"
    "EyeBlinkingChecker.cpp")
file(GLOB_RECURSE app_sources "*.c")
file(GLOB_RECURSE filter_sources "filter/*.c")

idf_component_register(
  SRCS
  ${eeg_srcs}
  ${app_sources}
  ${filter_sources}
  INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/include"REQUIRES
  "esp-dsp"
  driver)
