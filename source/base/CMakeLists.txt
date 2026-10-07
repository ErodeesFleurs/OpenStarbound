
add_library (star_base_modules STATIC)
target_sources (star_base_modules PUBLIC FILE_SET CXX_MODULES FILES
    StarAssets.cppm
    StarCellularLiquid.cppm
    scripting/StarImageLuaBindings.cppm
    StarMemoryAssetSource.cppm
    StarPackedAssetSource.cppm
    StarConfiguration.cppm
    StarWorldGeometry.cppm
    StarAnimatedPartSet.cppm
    StarAssetSource.cppm
    StarDirectoryAssetSource.cppm
    StarVersionOptionParser.cppm
    StarRootBase.cppm
    StarMixer.cppm
    StarCellularLightArray.cppm
    StarCellularLighting.cppm
    StarBlocksAlongLine.cppm)
target_sources (star_base_modules PRIVATE
    StarAssets.cpp
    scripting/StarImageLuaBindings.cpp
    StarConfiguration.cpp
    StarWorldGeometry.cpp
    StarAnimatedPartSet.cpp
    StarDirectoryAssetSource.cpp
    StarVersionOptionParser.cpp
    StarRootBase.cpp
    StarMixer.cpp
    StarCellularLightArray.cpp
    StarCellularLighting.cpp)
set_target_properties (star_base_modules PROPERTIES CXX_SCAN_FOR_MODULES ON)
target_include_directories (star_base_modules PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
  )
target_link_libraries (star_base_modules PUBLIC star_modules PRIVATE star_build_config)


if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (star_base_modules PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()