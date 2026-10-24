find_package(benchmark 1.9.5 CONFIG REQUIRED GLOBAL)
get_target_property(benchmark_library_type benchmark::benchmark TYPE)
if(NOT benchmark_library_type STREQUAL "STATIC_LIBRARY")
  message(FATAL_ERROR "Benchmarks require a static Google Benchmark built with the pinned xclang/runtime")
endif()

add_executable(core_benchmarks core_benchmarks.cpp
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core>)
target_include_directories(core_benchmarks PRIVATE
  ${STAR_EXTERN_INCLUDES} ${STAR_CORE_INCLUDES} ${STAR_BASE_INCLUDES}
  ${STAR_PLATFORM_INCLUDES})
target_link_libraries(core_benchmarks PRIVATE
  benchmark::benchmark star_base_modules star_build_config star_vorbis_dependencies star_lua
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)
set_target_properties(core_benchmarks PROPERTIES CXX_SCAN_FOR_MODULES ON)

add_executable(game_benchmarks game_benchmarks.cpp
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_game>)
target_include_directories(game_benchmarks PRIVATE
  ${STAR_EXTERN_INCLUDES} ${STAR_CORE_INCLUDES} ${STAR_BASE_INCLUDES}
  ${STAR_PLATFORM_INCLUDES} ${STAR_GAME_INCLUDES})
target_link_libraries(game_benchmarks PRIVATE
  benchmark::benchmark star_base_modules star_game_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)
set_target_properties(game_benchmarks PROPERTIES CXX_SCAN_FOR_MODULES ON)
