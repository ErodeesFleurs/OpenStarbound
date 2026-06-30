set(star_application_HEADERS
    StarApplication.hpp
    StarApplicationController.hpp
    StarMainApplication.hpp
    StarRenderer.hpp
  )

set(star_application_SOURCES
    StarApplication.cpp
    StarRenderer.cpp
  )

set(star_application_HEADERS ${star_application_HEADERS}
    StarP2PNetworkingService_pc.hpp
    StarPlatformServices_pc.hpp
    StarRenderer_opengl.hpp
  )

set(star_application_SOURCES ${star_application_SOURCES}
    StarMainApplication_sdl.cpp
    StarP2PNetworkingService_pc.cpp
    StarPlatformServices_pc.cpp
    StarRenderer_opengl.cpp
  )

if(STAR_ENABLE_STEAM_INTEGRATION)
  set(star_application_HEADERS ${star_application_HEADERS}
      StarDesktopService_pc_steam.hpp
      StarStatisticsService_pc_steam.hpp
      StarUserGeneratedContentService_pc_steam.hpp
    )
  set(star_application_SOURCES ${star_application_SOURCES}
      StarDesktopService_pc_steam.cpp
      StarStatisticsService_pc_steam.cpp
      StarUserGeneratedContentService_pc_steam.cpp
    )
endif()

if(STAR_ENABLE_DISCORD_INTEGRATION)
  set(star_application_SOURCES ${star_application_SOURCES}
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

add_library(star_application OBJECT ${star_application_SOURCES} ${star_application_HEADERS})
target_compile_features(star_application PUBLIC cxx_std_20)
target_include_directories(star_application PUBLIC
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_EXTERN_INCLUDES})
target_link_libraries(star_application PUBLIC star_build_definitions)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(star_application REUSE_FROM star_core)
endif()
