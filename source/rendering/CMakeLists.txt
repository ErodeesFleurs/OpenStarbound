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
TARGET_COMPILE_FEATURES (star_rendering PUBLIC cxx_std_20)
TARGET_INCLUDE_DIRECTORIES (star_rendering PUBLIC
    ${STAR_RENDERING_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_EXTERN_INCLUDES})
TARGET_LINK_LIBRARIES (star_rendering PUBLIC star_build_definitions)

IF(STAR_PRECOMPILED_HEADERS)
  TARGET_PRECOMPILE_HEADERS (star_rendering REUSE_FROM star_core)
ENDIF()
