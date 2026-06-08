SET (star_server_HEADERS
    StarServerQueryThread.hpp
    StarServerRconClient.hpp
    StarServerRconThread.hpp
  )

SET (star_server_SOURCES
    StarServerQueryThread.cpp
    StarServerRconClient.cpp
    StarServerRconThread.cpp
    main.cpp
  )

IF (STAR_SYSTEM_WINDOWS)
  SET (star_server_RESOURCES
      starbound_server.rc
    )
ENDIF ()

add_executable(starbound_server
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  ${star_server_HEADERS} ${star_server_SOURCES} ${star_server_RESOURCES})
target_compile_features(starbound_server PRIVATE cxx_std_20)
target_include_directories(starbound_server PRIVATE
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_EXTERN_INCLUDES})

IF(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(starbound_server REUSE_FROM star_core)
ENDIF()

IF(STAR_SYSTEM_MACOS)
  set_target_properties(starbound_server PROPERTIES LINK_FLAGS "-Wl,-rpath,'@executable_path'")
ELSEIF (UNIX)
  set_target_properties(starbound_server PROPERTIES LINK_FLAGS "-Wl,--export-dynamic,-rpath,'$ORIGIN'")
ENDIF()

target_link_libraries(starbound_server PRIVATE star_build_definitions star_external_libraries)
