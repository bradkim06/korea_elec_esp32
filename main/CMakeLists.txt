file(GLOB_RECURSE app_sources "*.c")
file(GLOB_RECURSE filter_sources "filter/*.c")

idf_component_register(
  SRCS
  ${app_sources}
  ${filter_sources}
  INCLUDE_DIRS
  "include"
  REQUIRES
  "esp-dsp")
