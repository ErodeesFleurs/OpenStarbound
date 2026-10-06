set (star_frontend_HEADERS
    StarCinematic.hpp
    StarClientCommandProcessor.hpp
    StarMainInterface.hpp
    StarTitleScreen.hpp
    StarVoice.hpp
  )

set (star_frontend_SOURCES
    StarCinematic.cpp
    StarClientCommandProcessor.cpp
    StarMainInterface.cpp
    StarTitleScreen.cpp
    StarVoice.cpp
  )

add_library (star_frontend_module_interfaces OBJECT)
target_sources (star_frontend_module_interfaces PUBLIC FILE_SET CXX_MODULES FILES
    StarSimpleTooltip.cppm
    StarHttpTrustDialog.cppm
    StarItemTooltip.cppm
    StarClipboardLuaBindings.cppm
    StarVoiceLuaBindings.cppm
    StarInterfaceLuaBindings.cppm
    StarBindingsMenu.cppm
    StarBookmarkInterface.cppm
    StarConfirmationDialog.cppm
    StarErrorScreen.cppm
    StarGraphicsMenu.cppm
    StarJoinRequestDialog.cppm
    StarKeybindingsMenu.cppm
    StarModsMenu.cppm
    StarOptionsMenu.cppm
    StarPopupInterface.cppm
    StarRadioMessagePopup.cppm
    StarShadersMenu.cppm
    StarTeleportDialog.cppm
    StarVoiceSettingsMenu.cppm
    StarCharCreation.cppm
    StarCharSelection.cppm
    StarCodexInterface.cppm
    StarQuestInterface.cppm
    StarSongbookInterface.cppm
    StarAiInterface.cppm
    StarActionBar.cppm
    StarBaseScriptPane.cppm
    StarChat.cppm
    StarChatBubbleSeparation.cppm
    StarChatBubbleManager.cppm
    StarContainerInterface.cppm
    StarContainerInteractor.cppm
    StarCraftingInterface.cppm
    StarInventory.cppm
    StarInterfaceCursor.cppm
    StarMainInterfaceTypes.cppm
    StarMainMixer.cppm
    StarMerchantInterface.cppm
    StarNameplatePainter.cppm
    StarQuestIndicatorPainter.cppm
    StarQuestTracker.cppm
    StarScriptPane.cppm
    StarStatusPane.cppm
    StarTeamBar.cppm
    StarWireInterface.cppm
  )
target_include_directories (star_frontend_module_interfaces PRIVATE
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
target_link_libraries (star_frontend_module_interfaces PUBLIC star_game_module_interfaces star_rendering_module_interfaces star_windowing_module_interfaces PRIVATE star_build_config)

add_library (star_frontend_modules STATIC
    StarInterfaceLuaBindings.cpp
    StarHttpTrustDialog.cpp
    StarVoiceLuaBindings.cpp
    StarClipboardLuaBindings.cpp
    StarBindingsMenu.cpp
    StarBookmarkInterface.cpp
    StarConfirmationDialog.cpp
    StarErrorScreen.cpp
    StarGraphicsMenu.cpp
    StarJoinRequestDialog.cpp
    StarKeybindingsMenu.cpp
    StarModsMenu.cpp
    StarOptionsMenu.cpp
    StarPopupInterface.cpp
    StarRadioMessagePopup.cpp
    StarShadersMenu.cpp
    StarTeleportDialog.cpp
    StarVoiceSettingsMenu.cpp
    StarCharCreation.cpp
    StarCharSelection.cpp
    StarCodexInterface.cpp
    StarQuestInterface.cpp
    StarSongbookInterface.cpp
    StarAiInterface.cpp
    StarActionBar.cpp
    StarBaseScriptPane.cpp
    StarChat.cpp
    StarChatBubbleSeparation.cpp
    StarChatBubbleManager.cpp
    StarContainerInterface.cpp
    StarContainerInteractor.cpp
    StarCraftingInterface.cpp
    StarInventory.cpp
    StarInterfaceCursor.cpp
    StarMainInterfaceTypes.cpp
    StarMainMixer.cpp
    StarMerchantInterface.cpp
    StarNameplatePainter.cpp
    StarQuestIndicatorPainter.cpp
    StarQuestTracker.cpp
    StarScriptPane.cpp
    StarStatusPane.cpp
    StarTeamBar.cpp
    StarWireInterface.cpp
  )
target_include_directories (star_frontend_modules PRIVATE
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
target_link_libraries (star_frontend_modules PUBLIC star_frontend_module_interfaces star_game_modules star_rendering_modules star_windowing_modules PRIVATE star_build_config star_voice_dependencies)
set_target_properties (star_frontend_modules PROPERTIES CXX_SCAN_FOR_MODULES ON)

add_library (star_frontend OBJECT ${star_frontend_SOURCES} ${star_frontend_HEADERS})
target_include_directories (star_frontend PRIVATE
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
target_link_libraries (star_frontend PRIVATE star_frontend_modules star_build_config star_voice_dependencies)
set_target_properties (star_frontend PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers (star_frontend REUSE_FROM star_core)
endif()

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (star_frontend_module_interfaces star_frontend_modules star_frontend PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()
