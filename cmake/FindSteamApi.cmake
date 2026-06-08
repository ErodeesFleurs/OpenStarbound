# Variables defined by this module:
#
#  STEAM_API_LIBRARY           The steam api library
#  STEAM_API_INCLUDE_DIR       The location of steam api headers
#  SteamApi::SteamApi          The steam api imported target

include(FindPackageHandleStandardArgs)

find_library(STEAM_API_LIBRARY
  NAMES libsteam_api steam_api steam_api64
)
find_path(STEAM_API_INCLUDE_DIR
  steam/steam_api.h
)

find_package_handle_standard_args(SteamApi
  REQUIRED_VARS STEAM_API_LIBRARY STEAM_API_INCLUDE_DIR
)

if(SteamApi_FOUND AND NOT TARGET SteamApi::SteamApi)
  add_library(SteamApi::SteamApi UNKNOWN IMPORTED)
  set_target_properties(SteamApi::SteamApi PROPERTIES
    IMPORTED_LOCATION "${STEAM_API_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${STEAM_API_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  STEAM_API_LIBRARY
  STEAM_API_INCLUDE_DIR
)
