INCLUDE_DIRECTORIES (
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )

SET (star_server_HEADERS
    StarServerRconClient.hpp
    StarServerRconThread.hpp
  )

SET (star_server_SOURCES
    StarServerRconClient.cpp
    StarServerRconThread.cpp
    main.cpp
  )

IF (STAR_SYSTEM_WINDOWS)
  SET (star_server_RESOURCES
      starbound_server.rc
      starbound_server.exe.manifest
    )
ENDIF ()

ADD_LIBRARY (star_server_modules STATIC)
TARGET_SOURCES (star_server_modules PUBLIC FILE_SET CXX_MODULES FILES StarServerQueryThread.cppm)
TARGET_LINK_LIBRARIES (star_server_modules PUBLIC star_modules)

ADD_EXECUTABLE (starbound_server
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  ${star_server_HEADERS} ${star_server_SOURCES} ${star_server_RESOURCES})
SET_TARGET_PROPERTIES (starbound_server PROPERTIES CXX_SCAN_FOR_MODULES ON)

IF(STAR_PRECOMPILED_HEADERS)
  TARGET_PRECOMPILE_HEADERS (starbound_server REUSE_FROM star_core)
ENDIF()

IF(STAR_SYSTEM_MACOS)
  SET_TARGET_PROPERTIES (starbound_server PROPERTIES LINK_FLAGS "-Wl,-rpath,'@executable_path'")
ELSEIF (UNIX)
  SET_TARGET_PROPERTIES (starbound_server PROPERTIES LINK_FLAGS "-Wl,--export-dynamic,-rpath,'$ORIGIN'")
ENDIF()

TARGET_LINK_LIBRARIES (starbound_server ${STAR_EXT_LIBS} star_modules star_server_modules star_game_modules)