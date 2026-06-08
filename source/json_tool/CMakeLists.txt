FIND_PACKAGE (Qt5Core)
FIND_PACKAGE (Qt5Widgets)

SET (CMAKE_INCLUDE_CURRENT_DIR ON)
SET (CMAKE_AUTOMOC ON)

ADD_EXECUTABLE (json_tool WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core>
  json_tool.cpp editor_gui.cpp)
TARGET_INCLUDE_DIRECTORIES (json_tool PRIVATE ${STAR_CORE_INCLUDES} ${STAR_EXTERN_INCLUDES})
target_link_libraries(json_tool PRIVATE star_build_definitions star_external_libraries Qt5::Core Qt5::Gui Qt5::Widgets)

SET (CMAKE_AUTOMOC OFF)
SET (CMAKE_INCLUDE_CURRENT_DIR OFF)
