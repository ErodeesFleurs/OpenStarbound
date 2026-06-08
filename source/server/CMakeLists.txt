add_executable(starbound_server
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
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
target_include_directories(starbound_server PRIVATE
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_EXTERN_INCLUDES})

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(starbound_server REUSE_FROM star_core)
endif()

if(STAR_SYSTEM_MACOS)
  set_target_properties(starbound_server PROPERTIES LINK_FLAGS "-Wl,-rpath,'@executable_path'")
elseif(UNIX)
  set_target_properties(starbound_server PROPERTIES LINK_FLAGS "-Wl,--export-dynamic,-rpath,'$ORIGIN'")
endif()

target_link_libraries(starbound_server PRIVATE star_build_definitions star_external_libraries)
