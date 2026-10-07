set (star_core_HEADERS
    StarAStar.hpp
    StarAlgorithm.hpp
    StarArray.hpp
    StarAudio.hpp
    StarBTree.hpp
    StarBiMap.hpp
    StarBlockAllocator.hpp
    StarByteArray.hpp
    StarBytes.hpp
    StarCasting.hpp
    StarColor.hpp
    StarConfig.hpp
    StarDataStream.hpp
    StarDataStreamDevices.hpp
    StarDataStreamExtra.hpp
    StarEither.hpp
    StarException.hpp
    StarFile.hpp
    StarFlatHashMap.hpp
    StarFlatHashSet.hpp
    StarFont.hpp
    StarFormat.hpp
    StarHash.hpp
    StarIODevice.hpp
    StarIdMap.hpp
    StarImage.hpp
    StarInputEvent.hpp
    StarInterpolation.hpp
    StarRefPtr.hpp
    StarIterator.hpp
    StarJson.hpp
    StarJsonBuilder.hpp
    StarJsonExtra.hpp
    StarJsonParser.hpp
    StarJsonRpc.hpp
    StarFormattedJson.hpp
    StarLexicalCast.hpp
    StarLine.hpp
    StarList.hpp
    StarLogging.hpp
    StarLua.hpp
    StarLuaConverters.hpp
    StarMap.hpp
    StarMathCommon.hpp
    StarMatrix3.hpp
    StarMaybe.hpp
    StarMemory.hpp
    StarMultiArray.hpp
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
    StarOrderedMap.hpp
    StarOrderedSet.hpp
    StarOutputProxy.hpp
    StarPch.hpp
    StarPoly.hpp
    StarPythonic.hpp
    StarRandom.hpp
    StarRandomPoint.hpp
    StarRect.hpp
    StarRpcPromise.hpp
    StarSet.hpp
    StarSpline.hpp
    StarStaticRandom.hpp
    StarStaticVector.hpp
    StarString.hpp
    StarStringView.hpp
    StarStrongTypedef.hpp
    StarThread.hpp
    StarVector.hpp
    StarXXHash.hpp
    scripting/StarUtilityLuaBindings.hpp
  )

set (star_core_SOURCES
    StarAudio.cpp
    StarByteArray.cpp
    StarColor.cpp
    StarDataStream.cpp
    StarDataStreamDevices.cpp
    StarFile.cpp
    StarFont.cpp
    StarIODevice.cpp
    StarImage.cpp
    StarInputEvent.cpp
    StarJson.cpp
    StarJsonBuilder.cpp
    StarJsonExtra.cpp
    StarJsonRpc.cpp
    StarFormattedJson.cpp
    StarLexicalCast.cpp
    StarLogging.cpp
    StarLua.cpp
    StarLuaConverters.cpp
    StarMemory.cpp
    StarNetCompatibility.cpp
    StarNetElement.cpp
    StarNetElementBasicFields.cpp
    StarNetElementGroup.cpp
    StarNetElementSyncGroup.cpp
    StarRandom.cpp
    StarString.cpp
    StarStringView.cpp
    StarThread.cpp
    scripting/StarUtilityLuaBindings.cpp
  )

if (STAR_SYSTEM_FAMILY_UNIX)
  set (star_core_SOURCES ${star_core_SOURCES}
      StarException_unix.cpp
      StarFile_unix.cpp
      StarThread_unix.cpp
    )
elseif (STAR_SYSTEM_FAMILY_WINDOWS)
  set (star_core_HEADERS ${star_core_HEADERS}
      StarString_windows.hpp
    )

  set (star_core_SOURCES ${star_core_SOURCES}
      StarFile_windows.cpp
      StarString_windows.cpp
      StarThread_windows.cpp
      StarException_windows.cpp
    )

endif ()

configure_file (StarVersion.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/StarVersion.cpp)
add_library (star_core OBJECT ${star_core_SOURCES} ${star_core_HEADERS})
target_include_directories (star_core PRIVATE ${STAR_EXTERN_INCLUDES} ${STAR_CORE_INCLUDES})

# Interfaces have no PCH: GCC rejects a forced PCH before a module
# declaration. Thin global-type interfaces retain ordinary companions below;
# naturally self-contained core interfaces remain single-file.
add_library (star_modules STATIC)
target_sources (star_modules PUBLIC FILE_SET CXX_MODULES
    FILES StarVlqEncoding.cppm StarCurve25519.cppm
          StarSha256.cppm StarImageScaling.cppm StarSecureRandom.cppm
          StarSignalHandler.cppm StarDynamicLib.cppm StarHttpClient.cppm StarUdp.cppm
          StarIODeviceCallbacks.cppm StarCompression.cppm StarJsonPatch.cppm
          StarTickRateMonitor.cppm
          StarUuid.cppm StarHostAddress.cppm StarSocket.cppm StarTcp.cppm
          StarWorkerPool.cppm StarOptionParser.cppm StarShellParser.cppm
          StarJsonPath.cppm
          StarAssetPath.cppm
          StarBTreeDatabase.cppm
          StarBuffer.cppm
          StarDirectives.cppm
          StarEncode.cppm
          StarImageProcessing.cppm
          StarLockFile.cppm
          StarPerlin.cppm
          StarText.cppm
          StarTime.cppm
          StarUnicode.cppm
          StarVersion.cppm
          StarZSTDCompression.cppm
          StarObserverStream.cppm
          StarPeriodic.cppm
          StarPeriodicFunction.cppm
          StarListener.cppm
          StarTtlCache.cppm
          StarLruCache.cppm
          StarWeightedPool.cppm
          StarParametricFunction.cppm
          StarMultiTable.cppm
          StarMultiArrayInterpolator.cppm
          StarSpatialHash2D.cppm
          StarSectorArray2D.cppm)
target_sources (star_modules PRIVATE
    StarUuid.cpp StarHostAddress.cpp StarSocket.cpp StarTcp.cpp
    StarWorkerPool.cpp StarOptionParser.cpp StarShellParser.cpp StarJsonPath.cpp
    StarAssetPath.cpp
    StarBTreeDatabase.cpp
    StarBuffer.cpp
    StarDirectives.cpp
    StarEncode.cpp
    StarImageProcessing.cpp
    StarPerlin.cpp
    StarText.cpp
    StarTime.cpp
    StarUnicode.cpp
    StarUnicode.inc
    StarZSTDCompression.cpp
    StarListener.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/StarVersion.cpp)
if (STAR_SYSTEM_FAMILY_UNIX)
  target_sources (star_modules PRIVATE StarLockFile_unix.cpp StarTime_unix.cpp)
elseif (STAR_SYSTEM_FAMILY_WINDOWS)
  target_sources (star_modules PRIVATE StarLockFile_windows.cpp StarTime_windows.cpp)
endif ()
set_target_properties (star_modules PROPERTIES CXX_SCAN_FOR_MODULES ON)
target_include_directories (star_modules PRIVATE ${STAR_EXTERN_INCLUDES} ${STAR_CORE_INCLUDES})
target_link_libraries (star_modules PRIVATE star_build_config star_module_dependencies star_core_dependencies)
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
