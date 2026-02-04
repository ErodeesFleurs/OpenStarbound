SET (star_rendering_HEADERS
      StarAnchorTypes.hpp
      StarAssetTextureGroup.hpp
      StarDrawablePainter.hpp
      StarEnvironmentPainter.hpp
      StarFontTextureGroup.hpp
      StarTextPainter.hpp
      StarTilePainter.hpp
      StarWorldPainter.hpp
    )

SET (star_rendering_SOURCES
      StarAnchorTypes.cpp
      StarAssetTextureGroup.cpp
      StarDrawablePainter.cpp
      StarEnvironmentPainter.cpp
      StarFontTextureGroup.cpp
      StarTextPainter.cpp
      StarTilePainter.cpp
      StarWorldPainter.cpp
    )

ADD_LIBRARY (star_rendering OBJECT ${star_rendering_SOURCES} ${star_rendering_HEADERS})

TARGET_INCLUDE_DIRECTORIES(star_rendering PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/base
    ${PROJECT_SOURCE_DIR}/game
    ${PROJECT_SOURCE_DIR}/game/interfaces
    ${PROJECT_SOURCE_DIR}/game/objects
    ${PROJECT_SOURCE_DIR}/application
    ${PROJECT_SOURCE_DIR}/extern
    ${PROJECT_SOURCE_DIR}/extern/lua
)

TARGET_INCLUDE_DIRECTORIES(star_rendering SYSTEM PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES(star_rendering PUBLIC
    star_game
    star_base
    star_core
    star_extern
)

IF(STAR_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS (star_rendering REUSE_FROM star_core)
ENDIF()
