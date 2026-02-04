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
    SET (star_server_RESOURCES starbound_server.rc)
ENDIF ()

ADD_EXECUTABLE (starbound_server
    ${star_server_SOURCES}
    ${star_server_HEADERS}
    ${star_server_RESOURCES}
    $<TARGET_OBJECTS:star_game>
    $<TARGET_OBJECTS:star_platform>
    $<TARGET_OBJECTS:star_base>
    $<TARGET_OBJECTS:star_core>
    $<TARGET_OBJECTS:star_extern>
)

TARGET_INCLUDE_DIRECTORIES(starbound_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/base
    ${PROJECT_SOURCE_DIR}/base/scripting
    ${PROJECT_SOURCE_DIR}/game
    ${PROJECT_SOURCE_DIR}/game/interfaces
    ${PROJECT_SOURCE_DIR}/game/scripting
    ${PROJECT_SOURCE_DIR}/platform
    ${PROJECT_SOURCE_DIR}/extern
    ${PROJECT_SOURCE_DIR}/extern/lua
)

IF(STAR_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS (starbound_server REUSE_FROM star_core)
ENDIF()

IF(STAR_SYSTEM_MACOS)
    SET_TARGET_PROPERTIES (starbound_server PROPERTIES LINK_FLAGS "-Wl,-rpath,'@executable_path'")
ELSEIF (UNIX)
    SET_TARGET_PROPERTIES(starbound_server PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
    SET_TARGET_PROPERTIES(starbound_server PROPERTIES
        LINK_FLAGS "-Wl,--export-dynamic"
    )
ENDIF()

TARGET_LINK_LIBRARIES (starbound_server ${STAR_EXT_LIBS} z pthread dl)
