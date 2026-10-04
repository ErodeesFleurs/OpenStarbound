set (star_frontend_HEADERS
    StarActionBar.hpp
    StarAiInterface.hpp
    StarBaseScriptPane.hpp
    StarBindingsMenu.hpp
    StarBookmarkInterface.hpp
    StarChat.hpp
    StarCharCreation.hpp
    StarCharSelection.hpp
    StarChatBubbleSeparation.hpp
    StarChatBubbleManager.hpp
    StarCinematic.hpp
    StarClientCommandProcessor.hpp
    StarCodexInterface.hpp
    StarConfirmationDialog.hpp
    StarContainerInterface.hpp
    StarContainerInteractor.hpp
    StarCraftingInterface.hpp
    StarErrorScreen.hpp
    StarGraphicsMenu.hpp
    StarInventory.hpp
    StarInterfaceCursor.hpp
    StarJoinRequestDialog.hpp
    StarKeybindingsMenu.hpp
    StarMainInterface.hpp
    StarMainInterfaceTypes.hpp
    StarMainMixer.hpp
    StarMerchantInterface.hpp
    StarModsMenu.hpp
    StarNameplatePainter.hpp
    StarOptionsMenu.hpp
    StarPopupInterface.hpp
    StarQuestIndicatorPainter.hpp
    StarQuestInterface.hpp
    StarQuestTracker.hpp
    StarRadioMessagePopup.hpp
    StarTeamBar.hpp
    StarTitleScreen.hpp
    StarScriptPane.hpp
    StarSongbookInterface.hpp
    StarStatusPane.hpp
    StarShadersMenu.hpp
    StarTeleportDialog.hpp
    StarVoice.hpp
    StarVoiceSettingsMenu.hpp
    StarWireInterface.hpp
  )

set (star_frontend_SOURCES
    StarActionBar.cpp
    StarAiInterface.cpp
    StarBaseScriptPane.cpp
    StarBindingsMenu.cpp
    StarBookmarkInterface.cpp
    StarChat.cpp
    StarCharCreation.cpp
    StarCharSelection.cpp
    StarChatBubbleSeparation.cpp
    StarChatBubbleManager.cpp
    StarCinematic.cpp
    StarClientCommandProcessor.cpp
    StarCodexInterface.cpp
    StarConfirmationDialog.cpp
    StarContainerInterface.cpp
    StarContainerInteractor.cpp
    StarCraftingInterface.cpp
    StarErrorScreen.cpp
    StarGraphicsMenu.cpp
    StarInventory.cpp
    StarInterfaceCursor.cpp
    StarJoinRequestDialog.cpp
    StarKeybindingsMenu.cpp
    StarMainInterface.cpp
    StarMainInterfaceTypes.cpp
    StarMainMixer.cpp
    StarMerchantInterface.cpp
    StarModsMenu.cpp
    StarNameplatePainter.cpp
    StarOptionsMenu.cpp
    StarPopupInterface.cpp
    StarQuestIndicatorPainter.cpp
    StarQuestInterface.cpp
    StarQuestTracker.cpp
    StarRadioMessagePopup.cpp
    StarTeamBar.cpp
    StarTitleScreen.cpp
    StarScriptPane.cpp
    StarSongbookInterface.cpp
    StarStatusPane.cpp
    StarShadersMenu.cpp
    StarTeleportDialog.cpp
    StarVoice.cpp
    StarVoiceSettingsMenu.cpp
    StarWireInterface.cpp
  )

add_library (star_frontend_module_interfaces OBJECT)
target_sources (star_frontend_module_interfaces PUBLIC FILE_SET CXX_MODULES FILES
    StarSimpleTooltip.cppm
    StarHttpTrustDialog.cppm
    StarItemTooltip.cppm
    StarClipboardLuaBindings.cppm
    StarVoiceLuaBindings.cppm
    StarInterfaceLuaBindings.cppm
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
target_link_libraries (star_frontend_module_interfaces PUBLIC star_game_module_interfaces PRIVATE star_build_config)

add_library (star_frontend_modules STATIC
    StarInterfaceLuaBindings.cpp
    StarHttpTrustDialog.cpp
    StarVoiceLuaBindings.cpp
    StarClipboardLuaBindings.cpp
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
target_link_libraries (star_frontend_modules PUBLIC star_frontend_module_interfaces star_game_modules PRIVATE star_build_config)
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
