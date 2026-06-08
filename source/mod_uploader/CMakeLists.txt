find_package(Qt5Core)
find_package(Qt5Widgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(star_mod_uploader_HEADERS
    StarStringConversion.hpp
    StarModUploader.hpp
    StarSPlainTextEdit.hpp
  )

set(star_mod_uploader_SOURCES
    StarModUploader.cpp
    StarSPlainTextEdit.cpp
    main.cpp
  )

add_executable(mod_uploader WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  ${star_mod_uploader_HEADERS} ${star_mod_uploader_SOURCES})
target_include_directories(mod_uploader PRIVATE ${STAR_BASE_INCLUDES} ${STAR_CORE_INCLUDES} ${STAR_EXTERN_INCLUDES})
target_link_libraries(mod_uploader PRIVATE star_build_definitions star_external_libraries ${STEAM_API_LIBRARY} Qt5::Core Qt5::Gui Qt5::Widgets)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_INCLUDE_CURRENT_DIR OFF)
