SET (star_client_HEADERS
      StarClientApplication.hpp
      StarRenderingLuaBindings.hpp
    )

SET (star_client_SOURCES
      StarClientApplication.cpp
      StarRenderingLuaBindings.cpp
    )

IF (STAR_SYSTEM_WINDOWS)
    SET (star_client_RESOURCES starbound.rc)
ENDIF ()

ADD_EXECUTABLE (starbound WIN32
    ${star_client_HEADERS}
    ${star_client_SOURCES}
    ${star_client_RESOURCES}
    $<TARGET_OBJECTS:star_extern>
    $<TARGET_OBJECTS:star_core>
    $<TARGET_OBJECTS:star_base>
    $<TARGET_OBJECTS:star_game>
    $<TARGET_OBJECTS:star_application>
    $<TARGET_OBJECTS:star_rendering>
    $<TARGET_OBJECTS:star_windowing>
    $<TARGET_OBJECTS:star_frontend>
)

TARGET_INCLUDE_DIRECTORIES(starbound PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/base
    ${PROJECT_SOURCE_DIR}/platform
    ${PROJECT_SOURCE_DIR}/game
    ${PROJECT_SOURCE_DIR}/game/interfaces
    ${PROJECT_SOURCE_DIR}/game/scripting
    ${PROJECT_SOURCE_DIR}/frontend
    ${PROJECT_SOURCE_DIR}/rendering
    ${PROJECT_SOURCE_DIR}/windowing
    ${PROJECT_SOURCE_DIR}/application
    ${PROJECT_SOURCE_DIR}/extern
    ${PROJECT_SOURCE_DIR}/source/extern/fmt/include
    ${PROJECT_SOURCE_DIR}/extern/lua
)

IF (STAR_PRECOMPILED_HEADERS)
  TARGET_PRECOMPILE_HEADERS (starbound REUSE_FROM star_core)
ENDIF()

IF(STAR_SYSTEM_MACOS)
  SET_TARGET_PROPERTIES (starbound PROPERTIES LINK_FLAGS "-Wl,-rpath,'@executable_path'")
ELSEIF (UNIX)
  SET_TARGET_PROPERTIES (starbound PROPERTIES LINK_FLAGS "-Wl,--export-dynamic,-rpath,'$ORIGIN'")
ENDIF()

IF (STAR_SYSTEM_MACOS)
  SET_TARGET_PROPERTIES (starbound PROPERTIES XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
ENDIF()

TARGET_LINK_LIBRARIES (starbound
    PRIVATE
    ${STAR_EXT_LIBS}
    ${STAR_EXT_GUI_LIBS}
    z
    pthread
    dl
)
