SET (star_core_tests_SOURCES
      gtest/gtest-all.cc

      core_tests_main.cpp

      algorithm_test.cpp
      block_allocator_test.cpp
      blocks_along_line_test.cpp
      btree_database_test.cpp
      btree_test.cpp
      byte_array_test.cpp
      clock_test.cpp
      color_test.cpp
      container_test.cpp
      encode_test.cpp
      file_test.cpp
      hash_test.cpp
      host_address_test.cpp
      ref_ptr_test.cpp
      json_test.cpp
      flat_hash_test.cpp
      formatted_json_test.cpp
      line_test.cpp
      lua_test.cpp
      lua_json_test.cpp
	      math_test.cpp
	      move_traits_test.cpp
	      multi_table_test.cpp
      net_states_test.cpp
      ordered_map_test.cpp
      ordered_set_test.cpp
      periodic_test.cpp
      poly_test.cpp
      random_test.cpp
      rect_test.cpp
      serialization_test.cpp
      static_vector_test.cpp
      small_vector_test.cpp
      sha_test.cpp
      shell_parse.cpp
      string_test.cpp
      strong_typedef_test.cpp
      thread_test.cpp
      worker_pool_test.cpp
      variant_test.cpp
      vlq_test.cpp
    )
ADD_EXECUTABLE (core_tests
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core>
  ${star_core_tests_SOURCES})
SET_TARGET_PROPERTIES (core_tests PROPERTIES UNITY_BUILD OFF)
TARGET_COMPILE_FEATURES (core_tests PRIVATE cxx_std_20)
TARGET_INCLUDE_DIRECTORIES (core_tests PRIVATE . ${STAR_BASE_INCLUDES} ${STAR_CORE_INCLUDES} ${STAR_EXTERN_INCLUDES})
TARGET_COMPILE_OPTIONS (core_tests PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wno-character-conversion>)
TARGET_LINK_LIBRARIES (core_tests star_build_definitions ${STAR_EXT_LIBS})
ADD_TEST (NAME core_tests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/core_tests)
SET_TESTS_PROPERTIES(core_tests PROPERTIES
                     LABELS "NoAssets"
                     TIMEOUT 120)

SET (star_game_tests_SOURCES
      gtest/gtest-all.cc

      game_tests_main.cpp

      StarTestUniverse.cpp
      assets_test.cpp
      function_test.cpp
      item_test.cpp
      root_test.cpp
      server_test.cpp
      spawn_test.cpp
      stat_test.cpp
      tile_array_test.cpp
      world_geometry_test.cpp
      universe_connection_test.cpp
    )
ADD_EXECUTABLE (game_tests
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  ${star_game_tests_SOURCES})
SET_TARGET_PROPERTIES (game_tests PROPERTIES UNITY_BUILD OFF)
TARGET_COMPILE_FEATURES (game_tests PRIVATE cxx_std_20)
TARGET_INCLUDE_DIRECTORIES (game_tests PRIVATE
    .
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_EXTERN_INCLUDES})
TARGET_COMPILE_OPTIONS (game_tests PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wno-character-conversion>)
TARGET_LINK_LIBRARIES (game_tests star_build_definitions ${STAR_EXT_LIBS})

SET (STAR_BASE_ASSET_PACKAGE ${PROJECT_SOURCE_DIR}/../assets/packed.pak)
IF (EXISTS ${STAR_BASE_ASSET_PACKAGE})
  SET (STAR_GAME_TEST_RUNTIME_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/game-test-runtime)
  FILE (MAKE_DIRECTORY ${STAR_GAME_TEST_RUNTIME_DIRECTORY}/storage ${STAR_GAME_TEST_RUNTIME_DIRECTORY}/logs)
  FILE (WRITE ${STAR_GAME_TEST_RUNTIME_DIRECTORY}/sbinit.config
  "{
    \"assetDirectories\" : [
      \"${PROJECT_SOURCE_DIR}/../assets/\",
      \"${PROJECT_SOURCE_DIR}/../mods/\"
    ],

    \"storageDirectory\" : \"${STAR_GAME_TEST_RUNTIME_DIRECTORY}/storage/\",
    \"logDirectory\" : \"${STAR_GAME_TEST_RUNTIME_DIRECTORY}/logs/\",

    \"assetsSettings\" : {
      \"pathIgnore\" : [],
      \"digestIgnore\" : [
        \".*\"
      ]
    },

    \"defaultConfiguration\" : {
      \"allowAdminCommandsFromAnyone\" : true,
      \"anonymousConnectionsAreAdmin\" : true
    }
  }
  ")
  ADD_TEST (NAME game_tests WORKING_DIRECTORY ${STAR_GAME_TEST_RUNTIME_DIRECTORY} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/game_tests)
  SET_TESTS_PROPERTIES(game_tests PROPERTIES
                       LABELS "Assets"
                       TIMEOUT 300
                       RESOURCE_LOCK game_assets)
ELSE ()
  MESSAGE (STATUS "Skipping game_tests registration because ${STAR_BASE_ASSET_PACKAGE} was not found")
ENDIF ()
