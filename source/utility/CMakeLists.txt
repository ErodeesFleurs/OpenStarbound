function(star_configure_utility_target target)
  target_compile_features(${target} PRIVATE cxx_std_20)
  target_include_directories(${target} PRIVATE
      ${STAR_GAME_INCLUDES}
      ${STAR_PLATFORM_INCLUDES}
      ${STAR_BASE_INCLUDES}
      ${STAR_CORE_INCLUDES}
      ${STAR_EXTERN_INCLUDES})
  target_link_libraries(${target} PRIVATE star_build_definitions)
endfunction()

add_executable(asset_packer
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  asset_packer.cpp)
star_configure_utility_target(asset_packer)
target_link_libraries(asset_packer PRIVATE star_external_libraries)

add_executable(asset_unpacker
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  asset_unpacker.cpp)
star_configure_utility_target(asset_unpacker)
target_link_libraries(asset_unpacker PRIVATE star_external_libraries)

add_executable(btree_repacker
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base>
  btree_repacker.cpp)
star_configure_utility_target(btree_repacker)
target_link_libraries(btree_repacker PRIVATE star_external_libraries)

add_executable(dump_versioned_json
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  dump_versioned_json.cpp)
star_configure_utility_target(dump_versioned_json)
target_link_libraries(dump_versioned_json PRIVATE star_external_libraries)

#add_executable(game_repl
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  game_repl.cpp)
#target_link_libraries(game_repl PRIVATE star_external_libraries)

add_executable(make_versioned_json
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  make_versioned_json.cpp)
star_configure_utility_target(make_versioned_json)
target_link_libraries(make_versioned_json PRIVATE star_external_libraries)

#add_executable(planet_mapgen
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  planet_mapgen.cpp)
#target_link_libraries(planet_mapgen PRIVATE star_external_libraries)

#add_executable(render_terrain_selector
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  render_terrain_selector.cpp)
#target_link_libraries(render_terrain_selector PRIVATE star_external_libraries)

#add_executable(update_tilesets
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  update_tilesets.cpp tileset_updater.cpp)
#target_link_libraries(update_tilesets PRIVATE star_external_libraries)

#add_executable(fix_embedded_tilesets
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  fix_embedded_tilesets.cpp)
#target_link_libraries(fix_embedded_tilesets PRIVATE star_external_libraries)

#add_executable(world_benchmark
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  world_benchmark.cpp)
#target_link_libraries(world_benchmark PRIVATE star_external_libraries)

#add_executable(generation_benchmark
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  generation_benchmark.cpp)
#target_link_libraries(generation_benchmark PRIVATE star_external_libraries)

#add_executable(dungeon_generation_benchmark
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  dungeon_generation_benchmark.cpp)
#target_link_libraries(dungeon_generation_benchmark PRIVATE star_external_libraries)

#add_executable(map_grep
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  map_grep.cpp)
#target_link_libraries(map_grep PRIVATE star_external_libraries)

#add_executable(word_count
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  word_count.cpp)
#target_link_libraries(word_count PRIVATE star_external_libraries)
