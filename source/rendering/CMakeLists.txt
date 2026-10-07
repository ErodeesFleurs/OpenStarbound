add_library (star_rendering_module_interfaces OBJECT)
target_sources (star_rendering_module_interfaces PUBLIC FILE_SET CXX_MODULES FILES
    StarAnchorTypes.cppm
    StarAssetTextureGroup.cppm
    StarFontTextureGroup.cppm
    StarDrawablePainter.cppm
    StarTextPainter.cppm
    StarEnvironmentPainter.cppm
    StarTilePainter.cppm
    StarWorldPainter.cppm
  )
target_include_directories (star_rendering_module_interfaces PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_RENDERING_INCLUDES}
  )
target_link_libraries (star_rendering_module_interfaces PUBLIC star_game_module_interfaces PRIVATE star_build_config)

add_library (star_rendering_modules STATIC
    StarAnchorTypes.cpp
    StarAssetTextureGroup.cpp
    StarFontTextureGroup.cpp
    StarDrawablePainter.cpp
    StarTextPainter.cpp
    StarEnvironmentPainter.cpp
    StarTilePainter.cpp
    StarWorldPainter.cpp
  )
target_include_directories (star_rendering_modules PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_RENDERING_INCLUDES}
  )
target_link_libraries (star_rendering_modules PUBLIC star_rendering_module_interfaces star_game_modules PRIVATE star_build_config)
set_target_properties (star_rendering_modules PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (star_rendering_module_interfaces star_rendering_modules PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()