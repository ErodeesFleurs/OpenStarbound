# - Try to find the OggVorbis libraries
# Once done this will define
#
#  OGGVORBIS_FOUND - system has OggVorbis
#  OGGVORBIS_VERSION - set either to 1 or 2
#  OGGVORBIS_INCLUDE_DIRS - the OggVorbis include directories
#  OGGVORBIS_LIBRARIES - The libraries needed to use OggVorbis
#  OGG_LIBRARY         - The Ogg library
#  VORBIS_LIBRARY      - The Vorbis library
#  VORBISFILE_LIBRARY  - The VorbisFile library

# Copyright (c) 2006, Richard Laerkaeng, <richard@goteborg.utfors.se>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

find_path(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h)
find_path(OGG_INCLUDE_DIR ogg/ogg.h)

find_library(OGG_LIBRARY NAMES ogg)
find_library(VORBIS_LIBRARY NAMES vorbis)
find_library(VORBISFILE_LIBRARY NAMES vorbisfile)

mark_as_advanced(VORBIS_INCLUDE_DIR OGG_INCLUDE_DIR
                 OGG_LIBRARY VORBIS_LIBRARY VORBISFILE_LIBRARY)

set(OGGVORBIS_INCLUDE_DIRS ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
set(OGGVORBIS_INCLUDE_DIR ${VORBIS_INCLUDE_DIR})
set(OGGVORBIS_LIBRARIES ${OGG_LIBRARY} ${VORBIS_LIBRARY} ${VORBISFILE_LIBRARY})

find_package_handle_standard_args(OggVorbis
  REQUIRED_VARS OGG_INCLUDE_DIR VORBIS_INCLUDE_DIR OGG_LIBRARY VORBIS_LIBRARY VORBISFILE_LIBRARY
)

if(OGGVORBIS_FOUND AND NOT TARGET OggVorbis::OggVorbis)
  add_library(OggVorbis::OggVorbis INTERFACE IMPORTED)
  set_target_properties(OggVorbis::OggVorbis PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${OGGVORBIS_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${OGGVORBIS_LIBRARIES}"
  )
endif()
