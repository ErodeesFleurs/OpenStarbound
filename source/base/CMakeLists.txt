set(star_base_HEADERS
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
  )

set(star_base_SOURCES
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

add_library(star_base OBJECT ${star_base_SOURCES} ${star_base_HEADERS})
target_compile_features(star_base PUBLIC cxx_std_20)
target_include_directories(star_base PUBLIC ${STAR_BASE_INCLUDES} ${STAR_CORE_INCLUDES} ${STAR_EXTERN_INCLUDES})
target_link_libraries(star_base PUBLIC star_build_definitions)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(star_base REUSE_FROM star_core)
endif()
