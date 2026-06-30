add_executable(starbound_server
  StarServerQueryThread.hpp
  StarServerRconClient.hpp
  StarServerRconThread.hpp
  StarServerQueryThread.cpp
  StarServerRconClient.cpp
  StarServerRconThread.cpp
  main.cpp)
if(STAR_SYSTEM_WINDOWS)
  target_sources(starbound_server PRIVATE starbound_server.rc)
endif()
target_compile_features(starbound_server PRIVATE cxx_std_20)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(starbound_server REUSE_FROM star_core)
endif()

if(STAR_SYSTEM_MACOS)
  target_link_options(starbound_server PRIVATE "LINKER:-rpath,@executable_path")
elseif(UNIX)
  target_link_options(starbound_server PRIVATE "LINKER:--export-dynamic,-rpath,$ORIGIN")
endif()

target_link_libraries(starbound_server PRIVATE star_extern star_core star_base star_game star_build_definitions star_external_libraries)
