set (star_rendering_HEADERS
      StarAnchorTypes.hpp
      StarAssetTextureGroup.hpp
      StarDrawablePainter.hpp
      StarEnvironmentPainter.hpp
      StarFontTextureGroup.hpp
      StarTextPainter.hpp
      StarTilePainter.hpp
      StarWorldPainter.hpp
    )

set (star_rendering_SOURCES
      StarAnchorTypes.cpp
      StarAssetTextureGroup.cpp
      StarDrawablePainter.cpp
      StarEnvironmentPainter.cpp
      StarFontTextureGroup.cpp
      StarTextPainter.cpp
      StarTilePainter.cpp
      StarWorldPainter.cpp
    )

add_library (star_rendering OBJECT ${star_rendering_SOURCES} ${star_rendering_HEADERS})
target_include_directories (star_rendering PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_RENDERING_INCLUDES}
  )
target_link_libraries (star_rendering PRIVATE star_build_config star_gui_dependencies)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers (star_rendering REUSE_FROM star_core)
endif()

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (star_rendering PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()