SET(UTILITY_INCLUDES
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/base
    ${PROJECT_SOURCE_DIR}/base/scripting
    ${PROJECT_SOURCE_DIR}/game
    ${PROJECT_SOURCE_DIR}/game/interfaces
    ${PROJECT_SOURCE_DIR}/game/scripting
    ${PROJECT_SOURCE_DIR}/extern
    ${PROJECT_SOURCE_DIR}/extern/lua
)

macro(configure_star_utility NAME)
    TARGET_INCLUDE_DIRECTORIES(${NAME} PUBLIC ${UTILITY_INCLUDES})
    TARGET_LINK_LIBRARIES(${NAME} PUBLIC
    $<TARGET_OBJECTS:star_game>
    $<TARGET_OBJECTS:star_base>
    $<TARGET_OBJECTS:star_core>
    $<TARGET_OBJECTS:star_extern>
    ${STAR_EXT_LIBS}
    z dl
    )

    if(NOT MSVC)
        target_compile_options(${NAME} PRIVATE -Wno-nan-infinity-disabled)
    endif()
endmacro()

ADD_EXECUTABLE(asset_packer asset_packer.cpp ${UTILITY_INCLUDES})
configure_star_utility(asset_packer)

ADD_EXECUTABLE(asset_unpacker asset_unpacker.cpp ${UTILITY_INCLUDES})
configure_star_utility(asset_unpacker)

ADD_EXECUTABLE(btree_repacker btree_repacker.cpp ${UTILITY_INCLUDES})
configure_star_utility(btree_repacker)

ADD_EXECUTABLE(dump_versioned_json dump_versioned_json.cpp ${UTILITY_INCLUDES})
configure_star_utility(dump_versioned_json)

ADD_EXECUTABLE(make_versioned_json make_versioned_json.cpp ${UTILITY_INCLUDES})
configure_star_utility(make_versioned_json)

#ADD_EXECUTABLE (game_repl
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  game_repl.cpp)
#TARGET_LINK_LIBRARIES (game_repl ${STAR_EXT_LIBS})

#ADD_EXECUTABLE (planet_mapgen
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  planet_mapgen.cpp)
#TARGET_LINK_LIBRARIES (planet_mapgen ${STAR_EXT_LIBS})

#ADD_EXECUTABLE (render_terrain_selector
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  render_terrain_selector.cpp)
#TARGET_LINK_LIBRARIES (render_terrain_selector ${STAR_EXT_LIBS})

#ADD_EXECUTABLE (update_tilesets
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  update_tilesets.cpp tileset_updater.cpp)
#TARGET_LINK_LIBRARIES (update_tilesets ${STAR_EXT_LIBS})

#ADD_EXECUTABLE (fix_embedded_tilesets
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  fix_embedded_tilesets.cpp)
#TARGET_LINK_LIBRARIES (fix_embedded_tilesets ${STAR_EXT_LIBS})

#ADD_EXECUTABLE (world_benchmark
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  world_benchmark.cpp)
#TARGET_LINK_LIBRARIES (world_benchmark ${STAR_EXT_LIBS})

#ADD_EXECUTABLE (generation_benchmark
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  generation_benchmark.cpp)
#TARGET_LINK_LIBRARIES (generation_benchmark ${STAR_EXT_LIBS})

#ADD_EXECUTABLE (dungeon_generation_benchmark
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  dungeon_generation_benchmark.cpp)
#TARGET_LINK_LIBRARIES (dungeon_generation_benchmark ${STAR_EXT_LIBS})

#ADD_EXECUTABLE (map_grep
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  map_grep.cpp)
#TARGET_LINK_LIBRARIES (map_grep ${STAR_EXT_LIBS})

#ADD_EXECUTABLE (word_count
#  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
#  word_count.cpp)
#TARGET_LINK_LIBRARIES (word_count ${STAR_EXT_LIBS})
