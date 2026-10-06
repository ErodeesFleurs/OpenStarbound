set (star_base_HEADERS
    StarAssets.hpp
    StarCellularLiquid.hpp
    scripting/StarImageLuaBindings.hpp
  )

set (star_base_SOURCES
    StarAssets.cpp
    scripting/StarImageLuaBindings.cpp
  )

add_library (star_base_modules STATIC)
target_sources (star_base_modules PUBLIC FILE_SET CXX_MODULES FILES
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

add_library (star_base OBJECT ${star_base_SOURCES} ${star_base_HEADERS})
target_include_directories (star_base PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
  )

# star_base imports core and base C++20 modules; the links bring in their
# module flags, and scanning tracks imports in its sources.
target_link_libraries (star_base PRIVATE star_base_modules star_build_config)
set_target_properties (star_base PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers (star_base REUSE_FROM star_core)
endif()

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (star_base_modules star_base PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()