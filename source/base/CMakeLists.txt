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

target_compile_features(star_base PUBLIC cxx_std_20)
target_include_directories(star_base PUBLIC ${STAR_BASE_INCLUDES} ${STAR_CORE_INCLUDES} ${STAR_EXTERN_INCLUDES})
target_link_libraries(star_base PUBLIC star_build_definitions)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(star_base REUSE_FROM star_core)
endif()
