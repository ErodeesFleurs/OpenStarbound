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
    StarMemoryAssetSource.hpp
    StarMixer.hpp
    StarPackedAssetSource.hpp
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
    StarMemoryAssetSource.cpp
    StarMixer.cpp
    StarPackedAssetSource.cpp
    StarRootBase.cpp
    StarVersionOptionParser.cpp
    StarWorldGeometry.cpp
    scripting/StarImageLuaBindings.cpp
  )

ADD_LIBRARY (star_base OBJECT ${star_base_SOURCES} ${star_base_HEADERS})
TARGET_COMPILE_FEATURES (star_base PUBLIC cxx_std_20)
TARGET_INCLUDE_DIRECTORIES (star_base PUBLIC ${STAR_BASE_INCLUDES} ${STAR_CORE_INCLUDES} ${STAR_EXTERN_INCLUDES})

IF(STAR_PRECOMPILED_HEADERS)
  TARGET_PRECOMPILE_HEADERS (star_base REUSE_FROM star_core)
ENDIF()
