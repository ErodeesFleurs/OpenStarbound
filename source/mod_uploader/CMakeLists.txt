find_package (Qt5Core)
find_package (Qt5Widgets)

set (star_mod_uploader_HEADERS
    StarStringConversion.hpp
    StarModUploader.hpp
    StarSPlainTextEdit.hpp
  )

set (star_mod_uploader_SOURCES
    StarModUploader.cpp
    StarSPlainTextEdit.cpp
    main.cpp
  )

add_executable (mod_uploader WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  ${star_mod_uploader_HEADERS} ${star_mod_uploader_SOURCES})
target_include_directories (mod_uploader PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )
target_link_libraries (mod_uploader PRIVATE star_base_modules star_build_config star_steam_dependencies
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>
  Qt5::Core Qt5::Gui Qt5::Widgets)
set_target_properties (mod_uploader PROPERTIES AUTOMOC ON CXX_SCAN_FOR_MODULES ON)

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (mod_uploader PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()
