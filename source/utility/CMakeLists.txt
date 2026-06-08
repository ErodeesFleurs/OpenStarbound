FUNCTION (STAR_CONFIGURE_UTILITY_TARGET target)
  TARGET_COMPILE_FEATURES (${target} PRIVATE cxx_std_20)
  TARGET_INCLUDE_DIRECTORIES (${target} PRIVATE
      ${STAR_GAME_INCLUDES}
      ${STAR_PLATFORM_INCLUDES}
      ${STAR_BASE_INCLUDES}
      ${STAR_CORE_INCLUDES}
      ${STAR_EXTERN_INCLUDES})
  TARGET_LINK_LIBRARIES (${target} star_build_definitions)
ENDFUNCTION ()

ADD_EXECUTABLE (asset_packer
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  asset_packer.cpp)
STAR_CONFIGURE_UTILITY_TARGET (asset_packer)
TARGET_LINK_LIBRARIES (asset_packer star_external_libraries)

ADD_EXECUTABLE (asset_unpacker
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  asset_unpacker.cpp)
STAR_CONFIGURE_UTILITY_TARGET (asset_unpacker)
TARGET_LINK_LIBRARIES (asset_unpacker star_external_libraries)

ADD_EXECUTABLE (btree_repacker
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  btree_repacker.cpp)
STAR_CONFIGURE_UTILITY_TARGET (btree_repacker)
TARGET_LINK_LIBRARIES (btree_repacker star_external_libraries)

ADD_EXECUTABLE (dump_versioned_json
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  dump_versioned_json.cpp)
STAR_CONFIGURE_UTILITY_TARGET (dump_versioned_json)
TARGET_LINK_LIBRARIES (dump_versioned_json star_external_libraries)

#ADD_EXECUTABLE (game_repl
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  game_repl.cpp)
#TARGET_LINK_LIBRARIES (game_repl star_external_libraries)

ADD_EXECUTABLE (make_versioned_json
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  make_versioned_json.cpp)
STAR_CONFIGURE_UTILITY_TARGET (make_versioned_json)
TARGET_LINK_LIBRARIES (make_versioned_json star_external_libraries)

#ADD_EXECUTABLE (planet_mapgen
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  planet_mapgen.cpp)
#TARGET_LINK_LIBRARIES (planet_mapgen star_external_libraries)

#ADD_EXECUTABLE (render_terrain_selector
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  render_terrain_selector.cpp)
#TARGET_LINK_LIBRARIES (render_terrain_selector star_external_libraries)

#ADD_EXECUTABLE (update_tilesets
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  update_tilesets.cpp tileset_updater.cpp)
#TARGET_LINK_LIBRARIES (update_tilesets star_external_libraries)

#ADD_EXECUTABLE (fix_embedded_tilesets
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  fix_embedded_tilesets.cpp)
#TARGET_LINK_LIBRARIES (fix_embedded_tilesets star_external_libraries)

#ADD_EXECUTABLE (world_benchmark
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  world_benchmark.cpp)
#TARGET_LINK_LIBRARIES (world_benchmark star_external_libraries)

#ADD_EXECUTABLE (generation_benchmark
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  generation_benchmark.cpp)
#TARGET_LINK_LIBRARIES (generation_benchmark star_external_libraries)

#ADD_EXECUTABLE (dungeon_generation_benchmark
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  dungeon_generation_benchmark.cpp)
#TARGET_LINK_LIBRARIES (dungeon_generation_benchmark star_external_libraries)

#ADD_EXECUTABLE (map_grep
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  map_grep.cpp)
#TARGET_LINK_LIBRARIES (map_grep star_external_libraries)

#ADD_EXECUTABLE (word_count
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  word_count.cpp)
#TARGET_LINK_LIBRARIES (word_count star_external_libraries)
