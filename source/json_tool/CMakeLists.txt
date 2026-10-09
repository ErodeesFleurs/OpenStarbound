add_executable (json_tool WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core>
  json_tool.cpp editor_gui.cpp)
target_include_directories (json_tool PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )
target_link_libraries (json_tool PRIVATE star_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>
  Qt5::Core Qt5::Gui Qt5::Widgets)
star_import_qt_plugins(json_tool)

set_target_properties (json_tool PROPERTIES AUTOMOC ON CXX_SCAN_FOR_MODULES ON)

