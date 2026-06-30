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

target_include_directories(star_rendering PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(star_rendering PUBLIC star_application star_game star_build_definitions)
target_link_libraries(star_rendering PRIVATE star_project_warnings)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(star_rendering REUSE_FROM star_core)
endif()
