FIND_PACKAGE (Qt5Core)
FIND_PACKAGE (Qt5Widgets)

SET (CMAKE_INCLUDE_CURRENT_DIR ON)
SET (CMAKE_AUTOMOC ON)

SET (star_mod_uploader_HEADERS
    StarStringConversion.hpp
    StarModUploader.hpp
    StarSPlainTextEdit.hpp
  )

SET (star_mod_uploader_SOURCES
    StarModUploader.cpp
    StarSPlainTextEdit.cpp
    main.cpp
  )

ADD_EXECUTABLE (mod_uploader WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  ${star_mod_uploader_HEADERS} ${star_mod_uploader_SOURCES})
TARGET_INCLUDE_DIRECTORIES (mod_uploader PRIVATE ${STAR_BASE_INCLUDES} ${STAR_CORE_INCLUDES} ${STAR_EXTERN_INCLUDES})
TARGET_LINK_LIBRARIES (mod_uploader star_build_definitions star_external_libraries ${STEAM_API_LIBRARY} Qt5::Core Qt5::Gui Qt5::Widgets)

SET (CMAKE_AUTOMOC OFF)
SET (CMAKE_INCLUDE_CURRENT_DIR OFF)
