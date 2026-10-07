add_library (star_application_module_interfaces OBJECT)
target_sources (star_application_module_interfaces PUBLIC FILE_SET CXX_MODULES FILES
    StarApplication.cppm
    StarApplicationController.cppm
    StarMainApplication.cppm
    StarRenderer.cppm
    StarTextureAtlas.cppm
    StarRenderer_opengl.cppm
    StarP2PNetworkingService_pc.cppm
    StarPlatformServices_pc.cppm
  )

add_library (star_application_modules STATIC
    StarApplication.cpp
    StarRenderer.cpp
    StarMainApplication_sdl.cpp
    StarP2PNetworkingService_pc.cpp
    StarPlatformServices_pc.cpp
    StarRenderer_opengl.cpp
  )

if (STAR_ENABLE_STEAM_INTEGRATION)
  target_sources (star_application_module_interfaces PUBLIC FILE_SET CXX_MODULES FILES
      StarDesktopService_pc_steam.cppm
      StarStatisticsService_pc_steam.cppm
      StarUserGeneratedContentService_pc_steam.cppm
    )
  target_sources (star_application_modules PRIVATE
      StarDesktopService_pc_steam.cpp
      StarStatisticsService_pc_steam.cpp
      StarUserGeneratedContentService_pc_steam.cpp
    )
endif ()

if (STAR_ENABLE_DISCORD_INTEGRATION)
  set (star_application_DISCORD_SOURCES
      discord/core.cpp
      discord/image_manager.cpp
      discord/lobby_manager.cpp
      discord/network_manager.cpp
      discord/overlay_manager.cpp
      discord/relationship_manager.cpp
      discord/storage_manager.cpp
      discord/store_manager.cpp
      discord/types.cpp
      discord/user_manager.cpp
      discord/voice_manager.cpp
      discord/achievement_manager.cpp
      discord/activity_manager.cpp
      discord/application_manager.cpp
    )
endif ()

foreach (target star_application_module_interfaces star_application_modules)
  target_include_directories (${target} PRIVATE
      ${STAR_EXTERN_INCLUDES}
      ${STAR_CORE_INCLUDES}
      ${STAR_PLATFORM_INCLUDES}
      ${STAR_APPLICATION_INCLUDES}
    )
  target_link_libraries (${target} PRIVATE star_build_config star_application_dependencies)
  set_target_properties (${target} PROPERTIES CXX_SCAN_FOR_MODULES ON)
  if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
    set_target_properties (${target} PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
  endif()
endforeach ()
target_link_libraries (star_application_module_interfaces PUBLIC star_modules)
target_link_libraries (star_application_modules PUBLIC star_application_module_interfaces)

if (STAR_ENABLE_DISCORD_INTEGRATION)
  add_library (star_application OBJECT ${star_application_DISCORD_SOURCES})
  target_include_directories (star_application PRIVATE
      ${STAR_EXTERN_INCLUDES}
      ${STAR_CORE_INCLUDES}
      ${STAR_PLATFORM_INCLUDES}
      ${STAR_APPLICATION_INCLUDES}
    )
  target_link_libraries (star_application PRIVATE star_modules star_build_config star_application_dependencies)
  set_target_properties (star_application PROPERTIES CXX_SCAN_FOR_MODULES ON)
  if(STAR_PRECOMPILED_HEADERS)
    target_precompile_headers (star_application REUSE_FROM star_core)
  endif()
  if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
    set_target_properties (star_application PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
  endif()
endif ()