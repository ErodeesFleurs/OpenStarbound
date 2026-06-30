add_library(star_base OBJECT)
target_sources(star_base PRIVATE
    StarAnimatedPartSet.hpp
    StarAssets.hpp
    StarAssetSource.hpp
    StarBlocksAlongLine.hpp
    StarCellularLightArray.hpp
    StarCellularLighting.hpp
    StarCellularLiquid.hpp
    StarConfiguration.hpp
    StarDirectoryAssetSource.hpp
    StarMemoryAssetSource.hpp
    StarMixer.hpp
    StarPackedAssetSource.hpp
    StarRootBase.hpp
    StarVersionOptionParser.hpp
    StarWorldGeometry.hpp
    scripting/StarImageLuaBindings.hpp
    StarAnimatedPartSet.cpp
    StarAssets.cpp
    StarCellularLightArray.cpp
    StarCellularLighting.cpp
    StarConfiguration.cpp
    StarDirectoryAssetSource.cpp
    StarMemoryAssetSource.cpp
    StarMixer.cpp
    StarPackedAssetSource.cpp
    StarRootBase.cpp
    StarVersionOptionParser.cpp
    StarWorldGeometry.cpp
    scripting/StarImageLuaBindings.cpp
  )

target_include_directories(star_base PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/scripting)
target_link_libraries(star_base PUBLIC star_core star_build_definitions)
target_link_libraries(star_base PRIVATE star_project_warnings)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(star_base REUSE_FROM star_core)
endif()
