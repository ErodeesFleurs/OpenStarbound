SET (star_windowing_HEADERS
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
    )

SET (star_windowing_SOURCES
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

ADD_LIBRARY (star_windowing OBJECT ${star_windowing_SOURCES} ${star_windowing_HEADERS})

TARGET_INCLUDE_DIRECTORIES(star_windowing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/application
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/base
    ${PROJECT_SOURCE_DIR}/base/scripting
    ${PROJECT_SOURCE_DIR}/game
    ${PROJECT_SOURCE_DIR}/game/interfaces
    ${PROJECT_SOURCE_DIR}/game/scripting
    ${PROJECT_SOURCE_DIR}/rendering
    ${PROJECT_SOURCE_DIR}/platform
    ${PROJECT_SOURCE_DIR}/extern
    ${PROJECT_SOURCE_DIR}/extern/lua
)

TARGET_LINK_LIBRARIES(star_windowing PUBLIC
    star_application
    star_game
    star_rendering
    star_base
    star_core
    star_extern
)
