# Variables defined by this module:
#
#  DISCORD_API_LIBRARY           The discord api library
#  DiscordApi::DiscordApi        The discord api imported target

include(FindPackageHandleStandardArgs)

find_library(DISCORD_API_LIBRARY
  NAMES discord_game_sdk libdiscord_game_sdk
)

find_package_handle_standard_args(DiscordApi
  REQUIRED_VARS DISCORD_API_LIBRARY
)

if(DiscordApi_FOUND AND NOT TARGET DiscordApi::DiscordApi)
  add_library(DiscordApi::DiscordApi UNKNOWN IMPORTED)
  set_target_properties(DiscordApi::DiscordApi PROPERTIES
    IMPORTED_LOCATION "${DISCORD_API_LIBRARY}"
  )
endif()

mark_as_advanced(
  DISCORD_API_LIBRARY
)
