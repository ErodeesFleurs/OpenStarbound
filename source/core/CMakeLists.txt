set (star_core_HEADERS
    StarAStar.hpp
    StarAlgorithm.hpp
    StarArray.hpp
    StarAssetPath.hpp
    StarAudio.hpp
    StarBTree.hpp
    StarBTreeDatabase.hpp
    StarBiMap.hpp
    StarBlockAllocator.hpp
    StarBuffer.hpp
    StarByteArray.hpp
    StarBytes.hpp
    StarCasting.hpp
    StarColor.hpp
    StarConfig.hpp
    StarDataStream.hpp
    StarDataStreamDevices.hpp
    StarDataStreamExtra.hpp
    StarDirectives.hpp
    StarEither.hpp
    StarEncode.hpp
    StarException.hpp
    StarFile.hpp
    StarFlatHashMap.hpp
    StarFlatHashSet.hpp
    StarFont.hpp
    StarFormat.hpp
    StarHash.hpp
    StarHostAddress.hpp
    StarIODevice.hpp
    StarIdMap.hpp
    StarImage.hpp
    StarImageProcessing.hpp
    StarInputEvent.hpp
    StarInterpolation.hpp
    StarRefPtr.hpp
    StarIterator.hpp
    StarJson.hpp
    StarJsonBuilder.hpp
    StarJsonExtra.hpp
    StarJsonParser.hpp
    StarJsonPath.hpp
    StarJsonRpc.hpp
    StarFormattedJson.hpp
    StarLexicalCast.hpp
    StarLine.hpp
    StarList.hpp
    StarListener.hpp
    StarLockFile.hpp
    StarLogging.hpp
    StarLruCache.hpp
    StarLua.hpp
    StarLuaConverters.hpp
    StarMap.hpp
    StarMathCommon.hpp
    StarMatrix3.hpp
    StarMaybe.hpp
    StarMemory.hpp
    StarMultiArray.hpp
    StarMultiArrayInterpolator.hpp
    StarMultiTable.hpp
    StarNetCompatibility.hpp
    StarNetElement.hpp
    StarNetElementBasicFields.hpp
    StarNetElementContainers.hpp
    StarNetElementDynamicGroup.hpp
    StarNetElementExt.hpp
    StarNetElementFloatFields.hpp
    StarNetElementGroup.hpp
    StarNetElementSignal.hpp
    StarNetElementSyncGroup.hpp
    StarNetElementSystem.hpp
    StarNetElementTop.hpp
    StarNetImpl.hpp
    StarObserverStream.hpp
    StarOptionParser.hpp
    StarOrderedMap.hpp
    StarOrderedSet.hpp
    StarOutputProxy.hpp
    StarParametricFunction.hpp
    StarPch.hpp
    StarPeriodic.hpp
    StarPeriodicFunction.hpp
    StarPerlin.hpp
    StarPoly.hpp
    StarPythonic.hpp
    StarRandom.hpp
    StarRandomPoint.hpp
    StarRect.hpp
    StarRpcPromise.hpp
    StarSectorArray2D.hpp
    StarSet.hpp
    StarShellParser.hpp
    StarSocket.hpp
    StarSpatialHash2D.hpp
    StarSpline.hpp
    StarStaticRandom.hpp
    StarStaticVector.hpp
    StarString.hpp
    StarStringView.hpp
    StarStrongTypedef.hpp
    StarTcp.hpp
    StarText.hpp
    StarThread.hpp
    StarTime.hpp
    StarTtlCache.hpp
    StarUnicode.hpp
    StarUuid.hpp
    StarVector.hpp
    StarVersion.hpp
    StarWeightedPool.hpp
    StarWorkerPool.hpp
    StarXXHash.hpp
    StarZSTDCompression.hpp
    scripting/StarUtilityLuaBindings.hpp
  )

set (star_core_SOURCES
    StarAudio.cpp
    StarAssetPath.cpp
    StarBTreeDatabase.cpp
    StarBuffer.cpp
    StarByteArray.cpp
    StarColor.cpp
    StarDataStream.cpp
    StarDataStreamDevices.cpp
    StarDirectives.cpp
    StarEncode.cpp
    StarFile.cpp
    StarFont.cpp
    StarHostAddress.cpp
    StarIODevice.cpp
    StarImage.cpp
    StarImageProcessing.cpp
    StarInputEvent.cpp
    StarJson.cpp
    StarJsonBuilder.cpp
    StarJsonExtra.cpp
    StarJsonPath.cpp
    StarJsonRpc.cpp
    StarFormattedJson.cpp
    StarLexicalCast.cpp
    StarListener.cpp
    StarLogging.cpp
    StarLua.cpp
    StarLuaConverters.cpp
    StarMemory.cpp
    StarNetCompatibility.cpp
    StarNetElement.cpp
    StarNetElementBasicFields.cpp
    StarNetElementGroup.cpp
    StarNetElementSyncGroup.cpp
    StarOptionParser.cpp
    StarPerlin.cpp
    StarRandom.cpp
    StarShellParser.cpp
    StarSocket.cpp
    StarString.cpp
    StarStringView.cpp
    StarTcp.cpp
    StarText.cpp
    StarThread.cpp
    StarTime.cpp
    StarUnicode.cpp
    StarUuid.cpp
    StarWorkerPool.cpp
    StarZSTDCompression.cpp
    scripting/StarUtilityLuaBindings.cpp
  )

if (STAR_SYSTEM_FAMILY_UNIX)
  set (star_core_SOURCES ${star_core_SOURCES}
      StarException_unix.cpp
      StarFile_unix.cpp
      StarLockFile_unix.cpp
      StarThread_unix.cpp
      StarTime_unix.cpp
    )
elseif (STAR_SYSTEM_FAMILY_WINDOWS)
  set (star_core_HEADERS ${star_core_HEADERS}
      StarString_windows.hpp
    )

  set (star_core_SOURCES ${star_core_SOURCES}
      StarFile_windows.cpp
      StarLockFile_windows.cpp
      StarString_windows.cpp
      StarThread_windows.cpp
      StarTime_windows.cpp
      StarException_windows.cpp
    )

endif ()

configure_file (StarVersion.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/StarVersion.cpp)
add_library (star_core OBJECT ${star_core_SOURCES} ${star_core_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/StarVersion.cpp)
target_include_directories (star_core PRIVATE ${STAR_EXTERN_INCLUDES} ${STAR_CORE_INCLUDES})

# Module sources have no PCH: GCC rejects a forced PCH before a module
# declaration. These core interfaces contain their implementations directly.
add_library (star_modules STATIC)
target_sources (star_modules PUBLIC FILE_SET CXX_MODULES
    FILES StarVlqEncoding.cppm StarCurve25519.cppm
          StarSha256.cppm StarImageScaling.cppm StarSecureRandom.cppm
          StarSignalHandler.cppm StarDynamicLib.cppm StarHttpClient.cppm StarUdp.cppm
          StarIODeviceCallbacks.cppm StarCompression.cppm StarJsonPatch.cppm
          StarTickRateMonitor.cppm)
target_include_directories (star_modules PRIVATE ${STAR_EXTERN_INCLUDES} ${STAR_CORE_INCLUDES})
target_link_libraries (star_modules PRIVATE star_build_config star_module_dependencies)
target_link_libraries (star_core PRIVATE star_modules star_build_config star_core_dependencies)
# Sources that import a module need the module flags and the module dependency
# scan too; CMake only turns those on for targets that declare module sources.
set_target_properties (star_core PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers (star_core PUBLIC StarPch.hpp)
endif()

if(STAR_COMPILER_CLANG AND STAR_SYSTEM_MACOS)
  set_source_files_properties(StarColor.cpp PROPERTIES COMPILE_OPTIONS "-fno-fast-math;-fassociative-math;-freciprocal-math")
endif()

if(STAR_USE_JEMALLOC AND JEMALLOC_IS_PREFIXED)
  set_source_files_properties(StarMemory.cpp PROPERTIES
          COMPILE_DEFINITIONS STAR_JEMALLOC_IS_PREFIXED
          )
endif()

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (star_core star_modules PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()
