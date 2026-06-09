function(star_configure_utility_target target)
  target_compile_features(${target} PRIVATE cxx_std_20)
  target_link_libraries(${target} PRIVATE star_build_definitions)
endfunction()

add_executable(asset_packer
  asset_packer.cpp)
star_configure_utility_target(asset_packer)
target_link_libraries(asset_packer PRIVATE star_extern star_core star_base star_external_libraries)

add_executable(asset_unpacker
  asset_unpacker.cpp)
star_configure_utility_target(asset_unpacker)
target_link_libraries(asset_unpacker PRIVATE star_extern star_core star_base star_external_libraries)

add_executable(btree_repacker
  btree_repacker.cpp)
star_configure_utility_target(btree_repacker)
target_link_libraries(btree_repacker PRIVATE star_extern star_core star_base star_external_libraries)

add_executable(dump_versioned_json
  dump_versioned_json.cpp)
star_configure_utility_target(dump_versioned_json)
target_link_libraries(dump_versioned_json PRIVATE star_extern star_core star_base star_game star_external_libraries)

add_executable(make_versioned_json
  make_versioned_json.cpp)
star_configure_utility_target(make_versioned_json)
target_link_libraries(make_versioned_json PRIVATE star_extern star_core star_base star_game star_external_libraries)

add_executable(planet_mapgen
  planet_mapgen.cpp)
star_configure_utility_target(planet_mapgen)
target_link_libraries(planet_mapgen PRIVATE star_extern star_core star_base star_game star_external_libraries)

# map_grep has rotted API references -- disabled until updated
#add_executable(map_grep
#  map_grep.cpp)
#star_configure_utility_target(map_grep)
#target_link_libraries(map_grep PRIVATE star_extern star_core star_base star_game star_external_libraries)

# Benchmarks
add_executable(world_benchmark
  world_benchmark.cpp)
star_configure_utility_target(world_benchmark)
target_link_libraries(world_benchmark PRIVATE star_extern star_core star_base star_game star_external_libraries)

add_executable(generation_benchmark
  generation_benchmark.cpp)
star_configure_utility_target(generation_benchmark)
target_link_libraries(generation_benchmark PRIVATE star_extern star_core star_base star_game star_external_libraries)

add_executable(dungeon_generation_benchmark
  dungeon_generation_benchmark.cpp)
star_configure_utility_target(dungeon_generation_benchmark)
target_link_libraries(dungeon_generation_benchmark PRIVATE star_extern star_core star_base star_game star_external_libraries)
