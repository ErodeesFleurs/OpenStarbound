find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(json_tool WIN32
  json_tool.cpp editor_gui.cpp)

set_target_properties(json_tool PROPERTIES AUTOMOC ON)
target_include_directories(json_tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(json_tool PRIVATE
  star_extern
  star_core
  star_build_definitions
  star_external_libraries
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets)
