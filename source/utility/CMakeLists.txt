add_executable (asset_packer
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core>
  asset_packer.cpp)
target_include_directories (asset_packer PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )
target_link_libraries (asset_packer PRIVATE star_base_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)
set_target_properties (asset_packer PROPERTIES CXX_SCAN_FOR_MODULES ON)

add_executable (asset_unpacker
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core>
  asset_unpacker.cpp)
target_include_directories (asset_unpacker PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )
target_link_libraries (asset_unpacker PRIVATE star_base_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)
set_target_properties (asset_unpacker PROPERTIES CXX_SCAN_FOR_MODULES ON)

add_executable (btree_repacker
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core>
  btree_repacker.cpp)
target_include_directories (btree_repacker PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )
target_link_libraries (btree_repacker PRIVATE star_base_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)
set_target_properties (btree_repacker PROPERTIES CXX_SCAN_FOR_MODULES ON)

add_executable (dump_versioned_json
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_game>
  dump_versioned_json.cpp)
target_include_directories (dump_versioned_json PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )
target_link_libraries (dump_versioned_json PRIVATE star_base_modules star_game_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)

add_executable (make_versioned_json
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_game>
  make_versioned_json.cpp)
target_include_directories (make_versioned_json PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )
target_link_libraries (make_versioned_json PRIVATE star_base_modules star_game_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)
set_target_properties (dump_versioned_json make_versioned_json PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (asset_packer asset_unpacker btree_repacker dump_versioned_json make_versioned_json
      PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()

# Maintenance utilities that are not part of a release.  Built when
# STAR_BUILD_DEV_TOOLS is on, so that they do not rot unnoticed.
if(STAR_BUILD_DEV_TOOLS)

add_executable (update_tilesets
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_game>
  update_tilesets.cpp tileset_updater.cpp)
target_include_directories (update_tilesets PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )
target_link_libraries (update_tilesets PRIVATE star_base_modules star_game_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)
set_target_properties (update_tilesets PROPERTIES CXX_SCAN_FOR_MODULES ON)

add_executable (fix_embedded_tilesets
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_game>
  fix_embedded_tilesets.cpp)
target_include_directories (fix_embedded_tilesets PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )
target_link_libraries (fix_embedded_tilesets PRIVATE star_base_modules star_game_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)
set_target_properties (fix_embedded_tilesets PROPERTIES CXX_SCAN_FOR_MODULES ON)

add_executable (map_grep
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_game>
  map_grep.cpp)
target_include_directories (map_grep PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
  )
target_link_libraries (map_grep PRIVATE star_base_modules star_game_modules star_build_config
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>)
set_target_properties (map_grep PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (update_tilesets fix_embedded_tilesets map_grep
      PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()

endif()
