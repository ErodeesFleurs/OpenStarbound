add_library(star_windowing OBJECT)
target_sources(star_windowing PRIVATE
        StarButtonGroup.hpp
        StarButtonWidget.hpp
        StarCanvasWidget.hpp
        StarFlowLayout.hpp
        StarFuelWidget.hpp
        StarGuiContext.hpp
        StarGuiReader.hpp
        StarGuiTypes.hpp
        StarImageWidget.hpp
        StarImageStretchWidget.hpp
        StarItemGridWidget.hpp
        StarItemSlotWidget.hpp
        StarKeyBindings.hpp
        StarLabelWidget.hpp
        StarLargeCharPlateWidget.hpp
        StarLayout.hpp
        StarListWidget.hpp
        StarPane.hpp
        StarPaneManager.hpp
        StarPortraitWidget.hpp
        StarProgressWidget.hpp
        StarScrollArea.hpp
        StarSliderBar.hpp
        StarStackWidget.hpp
        StarTabSet.hpp
        StarTextBoxWidget.hpp
        StarVerticalLayout.hpp
        StarWidget.hpp
        StarWidgetLuaBindings.hpp
        StarWidgetParsing.hpp
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
        StarLayout.cpp
        StarLabelWidget.cpp
        StarLargeCharPlateWidget.cpp
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

target_include_directories(star_windowing PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(star_windowing PUBLIC star_rendering star_application star_game star_build_definitions)
target_link_libraries(star_windowing PRIVATE star_project_warnings)
