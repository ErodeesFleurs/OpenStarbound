INCLUDE_DIRECTORIES (
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
  )

SET (star_base_HEADERS
    StarAnimatedPartSet.hpp
    StarAssets.hpp
    StarAssetSource.hpp
    StarBlocksAlongLine.hpp
    StarCellularLightArray.hpp
    StarCellularLighting.hpp
    StarCellularLiquid.hpp
    StarConfiguration.hpp
    StarDirectoryAssetSource.hpp
    StarMixer.hpp
    StarRootBase.hpp
    StarVersionOptionParser.hpp
    StarWorldGeometry.hpp
    scripting/StarImageLuaBindings.hpp
  )

SET (star_base_SOURCES
    StarAnimatedPartSet.cpp
    StarAssets.cpp
    StarCellularLightArray.cpp
    StarCellularLighting.cpp
    StarConfiguration.cpp
    StarDirectoryAssetSource.cpp
    StarMixer.cpp
    StarRootBase.cpp
    StarVersionOptionParser.cpp
    StarWorldGeometry.cpp
    scripting/StarImageLuaBindings.cpp
  )

ADD_LIBRARY (star_base_modules STATIC)
TARGET_SOURCES (star_base_modules PUBLIC FILE_SET CXX_MODULES FILES
    StarMemoryAssetSource.cppm
    StarPackedAssetSource.cppm)
TARGET_LINK_LIBRARIES (star_base_modules PUBLIC star_modules)

ADD_LIBRARY (star_base OBJECT ${star_base_SOURCES} ${star_base_HEADERS})

# star_base imports core and base C++20 modules; the links bring in their
# module flags, and scanning tracks imports in its sources.
TARGET_LINK_LIBRARIES (star_base star_modules star_base_modules)
SET_TARGET_PROPERTIES (star_base PROPERTIES CXX_SCAN_FOR_MODULES ON)

IF(STAR_PRECOMPILED_HEADERS)
  TARGET_PRECOMPILE_HEADERS (star_base REUSE_FROM star_core)
ENDIF()