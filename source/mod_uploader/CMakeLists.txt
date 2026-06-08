find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(mod_uploader WIN32
  StarStringConversion.hpp
  StarModUploader.hpp
  StarSPlainTextEdit.hpp
  StarModUploader.cpp
  StarSPlainTextEdit.cpp
  main.cpp)

set_target_properties(mod_uploader PROPERTIES AUTOMOC ON)
target_include_directories(mod_uploader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(mod_uploader PRIVATE
  star_extern
  star_core
  star_base
  star_build_definitions
  star_external_libraries
  SteamApi::SteamApi
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets)
