add_library(star_rendering OBJECT)
target_sources(star_rendering PRIVATE
      StarAnchorTypes.hpp
      StarAssetTextureGroup.hpp
      StarDrawablePainter.hpp
      StarEnvironmentPainter.hpp
      StarFontTextureGroup.hpp
      StarTextPainter.hpp
      StarTilePainter.hpp
      StarWorldPainter.hpp
      StarAnchorTypes.cpp
      StarAssetTextureGroup.cpp
      StarDrawablePainter.cpp
      StarEnvironmentPainter.cpp
      StarFontTextureGroup.cpp
      StarTextPainter.cpp
      StarTilePainter.cpp
      StarWorldPainter.cpp
    )

target_compile_features(star_rendering PUBLIC cxx_std_20)
target_include_directories(star_rendering PUBLIC
    ${STAR_RENDERING_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_EXTERN_INCLUDES})
target_link_libraries(star_rendering PUBLIC star_build_definitions)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(star_rendering REUSE_FROM star_core)
endif()
