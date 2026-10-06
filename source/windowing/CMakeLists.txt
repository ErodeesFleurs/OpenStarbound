add_library (star_windowing_module_interfaces OBJECT)
target_sources (star_windowing_module_interfaces PUBLIC FILE_SET CXX_MODULES FILES
    StarButtonGroup.cppm
    StarButtonWidget.cppm
    StarCanvasWidget.cppm
    StarFlowLayout.cppm
    StarFuelWidget.cppm
    StarGuiContext.cppm
    StarGuiReader.cppm
    StarGuiTypes.cppm
    StarImageWidget.cppm
    StarImageStretchWidget.cppm
    StarItemGridWidget.cppm
    StarItemSlotWidget.cppm
    StarKeyBindings.cppm
    StarLabelWidget.cppm
    StarLargeCharPlateWidget.cppm
    StarLayout.cppm
    StarListWidget.cppm
    StarPane.cppm
    StarPaneManager.cppm
    StarPortraitWidget.cppm
    StarProgressWidget.cppm
    StarRegisteredPaneManager.cppm
    StarScrollArea.cppm
    StarSliderBar.cppm
    StarStackWidget.cppm
    StarTabSet.cppm
    StarTextBoxWidget.cppm
    StarVerticalLayout.cppm
    StarWidget.cppm
    StarWidgetLuaBindings.cppm
    StarWidgetParsing.cppm
  )
target_include_directories (star_windowing_module_interfaces PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_RENDERING_INCLUDES}
    ${STAR_WINDOWING_INCLUDES}
  )
target_link_libraries (star_windowing_module_interfaces PUBLIC star_rendering_module_interfaces PRIVATE star_build_config)

add_library (star_windowing_modules STATIC
    StarButtonGroup.cpp
    StarButtonWidget.cpp
    StarCanvasWidget.cpp
    StarFlowLayout.cpp
    StarFuelWidget.cpp
    StarGuiContext.cpp
    StarGuiReader.cpp
    StarGuiTypes.cpp
    StarImageWidget.cpp
    StarImageStretchWidget.cpp
    StarItemGridWidget.cpp
    StarItemSlotWidget.cpp
    StarKeyBindings.cpp
    StarLabelWidget.cpp
    StarLargeCharPlateWidget.cpp
    StarLayout.cpp
    StarListWidget.cpp
    StarPane.cpp
    StarPaneManager.cpp
    StarPortraitWidget.cpp
    StarProgressWidget.cpp
    StarScrollArea.cpp
    StarSliderBar.cpp
    StarStackWidget.cpp
    StarTabSet.cpp
    StarTextBoxWidget.cpp
    StarVerticalLayout.cpp
    StarWidget.cpp
    StarWidgetLuaBindings.cpp
    StarWidgetParsing.cpp
  )
target_include_directories (star_windowing_modules PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_RENDERING_INCLUDES}
    ${STAR_WINDOWING_INCLUDES}
  )
target_link_libraries (star_windowing_modules PUBLIC star_windowing_module_interfaces star_game_modules star_rendering_modules PRIVATE star_build_config)
set_target_properties (star_windowing_modules PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (star_windowing_module_interfaces star_windowing_modules PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()
