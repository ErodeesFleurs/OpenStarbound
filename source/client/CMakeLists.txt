add_executable(starbound WIN32
  StarClientApplication.hpp
  StarRenderingLuaBindings.hpp
  StarClientApplication.cpp
  StarRenderingLuaBindings.cpp)
if(STAR_SYSTEM_WINDOWS)
  target_sources(starbound PRIVATE starbound.rc)
endif()
target_compile_features(starbound PRIVATE cxx_std_20)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(starbound REUSE_FROM star_core)
endif()

if(STAR_SYSTEM_MACOS)
  target_link_options(starbound PRIVATE "LINKER:-rpath,@executable_path")
elseif(UNIX)
  target_link_options(starbound PRIVATE "LINKER:--export-dynamic,-rpath,$ORIGIN")
endif()

if(STAR_SYSTEM_MACOS)
  set_target_properties(starbound PROPERTIES XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
endif()

target_link_libraries(starbound PRIVATE
  star_extern
  star_core
  star_base
  star_game
  star_application
  star_rendering
  star_windowing
  star_frontend
  star_build_definitions
  star_external_libraries
  star_external_gui_libraries)
