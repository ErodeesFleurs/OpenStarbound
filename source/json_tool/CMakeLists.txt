find_package(Qt5Core)
find_package(Qt5Widgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_executable(json_tool WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core>
  json_tool.cpp editor_gui.cpp)
target_include_directories(json_tool PRIVATE ${STAR_CORE_INCLUDES} ${STAR_EXTERN_INCLUDES})
target_link_libraries(json_tool PRIVATE star_build_definitions star_external_libraries Qt5::Core Qt5::Gui Qt5::Widgets)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_INCLUDE_CURRENT_DIR OFF)
