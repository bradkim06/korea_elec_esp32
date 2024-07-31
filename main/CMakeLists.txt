file(GLOB_RECURSE app_sources "*.c")

idf_component_register(
  SRCS
  ${app_sources}
  "filter/fir_filter.c"
  "filter/iir_filter.c"
  INCLUDE_DIRS
  "include"
  REQUIRES
  "esp-dsp")
