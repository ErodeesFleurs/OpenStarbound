INCLUDE_DIRECTORIES (
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_RENDERING_INCLUDES}
    ${STAR_WINDOWING_INCLUDES}
    ${STAR_FRONTEND_INCLUDES}
  )

SET (star_client_HEADERS
      StarClientApplication.hpp
    )

SET (star_client_SOURCES
      StarClientApplication.cpp
    )

IF (STAR_SYSTEM_WINDOWS)
  SET (star_client_RESOURCES
      starbound.rc
      starbound.exe.manifest
    )
ENDIF ()

ADD_EXECUTABLE (starbound WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  $<TARGET_OBJECTS:star_application> $<TARGET_OBJECTS:star_rendering> $<TARGET_OBJECTS:star_windowing> $<TARGET_OBJECTS:star_frontend>
  ${star_client_HEADERS} ${star_client_SOURCES} ${star_client_RESOURCES})
TARGET_SOURCES (starbound PRIVATE FILE_SET CXX_MODULES FILES
    StarRenderingLuaBindings.cppm
  )
SET_SOURCE_FILES_PROPERTIES (StarRenderingLuaBindings.cppm PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

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

TARGET_LINK_LIBRARIES (starbound star_modules star_base_modules star_frontend_modules star_game_modules ${STAR_EXT_LIBS} ${STAR_EXT_GUI_LIBS})
SET_TARGET_PROPERTIES (starbound PROPERTIES CXX_SCAN_FOR_MODULES ON)
