add_library(star_application OBJECT)
target_sources(star_application PRIVATE
    StarApplication.hpp
    StarApplicationController.hpp
    StarMainApplication.hpp
    StarRenderer.hpp
    StarApplication.cpp
    StarRenderer.cpp
    StarP2PNetworkingService_pc.hpp
    StarPlatformServices_pc.hpp
    StarRenderer_opengl.hpp
    StarMainApplication_sdl.cpp
    StarP2PNetworkingService_pc.cpp
    StarPlatformServices_pc.cpp
    StarRenderer_opengl.cpp
  )

if(STAR_ENABLE_STEAM_INTEGRATION)
  target_sources(star_application PRIVATE
      StarDesktopService_pc_steam.hpp
      StarStatisticsService_pc_steam.hpp
      StarUserGeneratedContentService_pc_steam.hpp
      StarDesktopService_pc_steam.cpp
      StarStatisticsService_pc_steam.cpp
      StarUserGeneratedContentService_pc_steam.cpp
    )
endif()

if(STAR_ENABLE_DISCORD_INTEGRATION)
  target_sources(star_application PRIVATE
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
endif()

target_include_directories(star_application PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/platform)
target_link_libraries(star_application PUBLIC star_core star_build_definitions)
target_link_libraries(star_application PRIVATE star_project_warnings)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(star_application REUSE_FROM star_core)
endif()
