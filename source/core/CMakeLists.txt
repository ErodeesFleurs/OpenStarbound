set (star_core_HEADERS
    # StarAStar.hpp
    # StarAlgorithm.hpp
    # StarAssetPath.hpp
    # StarAtomicSharedPtr.hpp
    # StarAudio.hpp
    # StarIODeviceCallbacks.hpp
    # StarBTree.hpp
    # StarBTreeDatabase.hpp
    # StarBlockAllocator.hpp
    # StarBuffer.hpp
    # StarCasting.hpp
    # StarColor.hpp
    # StarCompression.hpp
    # StarConfig.hpp
    # StarCurve25519.hpp
    # StarDataStreamDevices.hpp
    # StarDirectives.hpp
    # StarDynamicLib.hpp
    # StarEither.hpp
    # StarFlatHashMap.hpp
    # StarFlatHashSet.hpp
    # StarFont.hpp
    # StarHostAddress.hpp
    # StarHttpClient.hpp
    # StarIdMap.hpp
    # StarImageProcessing.hpp
    # StarImageScaling.hpp
    # StarInputEvent.hpp
    # StarInterpolation.hpp
    # StarRefPtr.hpp
    # StarIterator.hpp
    # StarJsonExtra.hpp
    # StarJsonParser.hpp
    # StarJsonPath.hpp
    # StarJsonPatch.hpp
    # StarJsonRpc.hpp
    # StarFormattedJson.hpp
    # StarLexicalCast.hpp
    # StarList.hpp
    # StarListener.hpp
    # StarLockFile.hpp
    # StarLruCache.hpp
    # StarLua.hpp
    # StarLuaConverters.hpp
    # StarMap.hpp
    # StarMultiArray.hpp
    # StarMultiArrayInterpolator.hpp
    # StarMultiTable.hpp
    # StarNetCompatibility.hpp
    # StarNetElement.hpp
    # StarNetElementBasicFields.hpp
    # StarNetElementContainers.hpp
    # StarNetElementDynamicGroup.hpp
    # StarNetElementExt.hpp
    # StarNetElementFloatFields.hpp
    # StarNetElementGroup.hpp
    # StarNetElementSignal.hpp
    # StarNetElementSyncGroup.hpp
    # StarNetElementSystem.hpp
    # StarNetElementTop.hpp
    # StarNetImpl.hpp
    # StarObserverStream.hpp
    # StarOptionParser.hpp
    # StarOrderedMap.hpp
    # StarOrderedSet.hpp
    # StarOutputProxy.hpp
    # StarParametricFunction.hpp
    # StarPeriodic.hpp
    # StarPeriodicFunction.hpp
    # StarPerlin.hpp
    # StarPythonic.hpp
    # StarRandomPoint.hpp
    # StarRpcPromise.hpp
    # StarRpcThreadPromise.hpp
    # StarSectorArray2D.hpp
    # StarSecureRandom.hpp
    # StarSet.hpp
    # StarSha256.hpp
    # StarShellParser.hpp
    # StarSignalHandler.hpp
    # StarSocket.hpp
    # StarSpatialHash2D.hpp
    # StarSpline.hpp
    # StarStaticVector.hpp
    # StarStrongTypedef.hpp
    # StarTcp.hpp
    # StarText.hpp
    # StarThread.hpp
    # StarTickRateMonitor.hpp
    # StarTtlCache.hpp
    # StarUdp.hpp
    # StarUnicode.hpp
    # StarUuid.hpp
    # StarVersion.hpp
    # StarWeightedPool.hpp
    # StarWorkerPool.hpp
    # StarXXHash.hpp
    # StarZSTDCompression.hpp
    # scripting/StarUtilityLuaBindings.hpp
  )

set (star_core_SOURCES
    # StarAudio.cpp
    # StarAssetPath.cpp
    # StarBTreeDatabase.cpp
    # StarBuffer.cpp
    # StarColor.cpp
    # StarIODeviceCallbacks.cpp
    # StarCompression.cpp
    # StarCurve25519.cpp
    # StarDataStreamDevices.cpp
    # StarDirectives.cpp
    # StarFont.cpp
    # StarHostAddress.cpp
    # StarHttpClient.cpp
    # StarImageProcessing.cpp
    # StarImageScaling.cpp
    # StarInputEvent.cpp
    # StarJsonExtra.cpp
    # StarJsonPath.cpp
    # StarJsonPatch.cpp
    # StarJsonRpc.cpp
    # StarFormattedJson.cpp
    # StarLexicalCast.cpp
    # StarListener.cpp
    # StarLockFile.cpp
    # StarLua.cpp
    # StarLuaConverters.cpp
    # StarNetCompatibility.cpp
    # StarNetElement.cpp
    # StarNetElementBasicFields.cpp
    # StarNetElementGroup.cpp
    # StarNetElementSyncGroup.cpp
    # StarOptionParser.cpp
    # StarPerlin.cpp
    # StarSecureRandom.cpp
    # StarSha256.cpp
    # StarShellParser.cpp
    # StarSignalHandler.cpp
    # StarSocket.cpp
    # StarTcp.cpp
    # StarText.cpp
    # StarThread.cpp
    # StarTickRateMonitor.cpp
    # StarUdp.cpp
    # StarUnicode.cpp
    # StarUuid.cpp
    # StarWorkerPool.cpp
    # StarZSTDCompression.cpp
    # scripting/StarUtilityLuaBindings.cpp
  )

set (star_core_MODULES
    StarString.cppm
    StarFormat.cppm
    StarStaticRandom.cppm
    StarFuncional.cppm
    StarBiMap.cppm
    StarBytes.cppm
    StarVersion.cppm
    StarTime.cppm
    StarHash.cppm
    StarEncode.cppm
    StarException.cppm
    StarMathCommon.cppm
    StarArray.cppm
    StarVector.cppm
    StarMatrix3.cppm
    StarLine.cppm
    StarRect.cppm
    StarPoly.cppm
    StarVlqEncoding.cppm
    StarByteArray.cppm
    StarIODevice.cppm
    StarFile.cppm
    StarUnicode.cppm
    StarColor.cppm
    StarNetCompatibility.cppm
    StarDataStream.cppm
    StarBuffer.cppm
    StarRandom.cppm
    StarLogging.cppm
    StarImage.cppm
    StarDataStreamExtra.cppm
    StarLexicalCast.cppm
    StarJsonParser.cppm
    StarJson.cppm
    StarJsonBuilder.cppm
    StarImageProcessing.cppm
  )

set (star_core_SOURCES
    StarTime.cpp
    StarColor.cpp
    StarDataStream.cpp
    StarIODevice.cpp
    StarRandom.cpp
    StarUnicode.cpp
    StarFile.cpp
    StarBuffer.cpp
    StarLogging.cpp
    StarImage.cpp
    StarLexicalCast.cpp
    StarJsonBuilder.cpp
    StarImageProcessing.cpp
)

#configure_file (StarVersion.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/StarVersion.cpp)

add_library (star_core OBJECT
    # ${star_core_SOURCES}
    # ${star_core_HEADERS}
    # ${CMAKE_CURRENT_BINARY_DIR}/StarVersion.cpp
)

target_sources(star_core
    PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${star_core_MODULES}
    PRIVATE
    ${star_core_SOURCES}
)

set_target_properties(star_core PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

target_include_directories(star_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/extern
    ${PROJECT_SOURCE_DIR}/extern/lua
)

TARGET_LINK_LIBRARIES(star_core PUBLIC
    star_extern
    ${STAR_EXT_LIBS}
)
