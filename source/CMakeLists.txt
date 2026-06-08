cmake_minimum_required(VERSION 3.23)

option(STAR_USE_COMPILER_CACHE "Use sccache or ccache when available" ON)

project(starbound)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake)

set(CMAKE_CONFIGURATION_TYPES Debug RelWithAsserts RelWithDebInfo Release)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHASSERTS "" CACHE STRING "" FORCE)

if(STAR_USE_COMPILER_CACHE)
  find_program(SCCACHE_PATH sccache)
  if(SCCACHE_PATH)
    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_PATH}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PATH}")
    set(STAR_COMPILER_CACHE "sccache")
    set(STAR_PRECOMPILED_HEADERS FALSE)
  else()
    find_program(CCACHE_PATH ccache)
    if(CCACHE_PATH)
      set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PATH}")
      set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PATH}")
      set(STAR_COMPILER_CACHE "ccache")
      set(STAR_PRECOMPILED_HEADERS FALSE)
    endif()
  endif()
endif()

if(NOT DEFINED STAR_COMPILER_CACHE)
  set(STAR_COMPILER_CACHE "disabled")
  set(STAR_PRECOMPILED_HEADERS TRUE)
endif()

option(STAR_ENABLE_LTO "Enable CMake interprocedural optimization / LTO" OFF)
if(STAR_ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT STAR_LTO_SUPPORTED OUTPUT STAR_LTO_OUTPUT)
  if(STAR_LTO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(FATAL_ERROR "STAR_ENABLE_LTO requested but IPO/LTO is not supported: ${STAR_LTO_OUTPUT}")
  endif()
endif()

option(STAR_ENABLE_FRAME_POINTERS "Keep frame pointers for profiling" OFF)
option(STAR_ENABLE_UNITY_BUILD "Use CMake unity builds for faster clean builds" OFF)
set(CMAKE_UNITY_BUILD ${STAR_ENABLE_UNITY_BUILD})

# Update the docstring on CMAKE_BUILD_TYPE to show what options we actually
# allow
# SET (CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Choose the type of build, options are: Debug RelWithAsserts RelWithDebInfo Release" FORCE)

# Discover all the relevant environment / system information and place the
# result in STAR_* cmake variables.

# STAR_SOURCE_IDENTIFIER may be set to any string value
if(NOT DEFINED STAR_SOURCE_IDENTIFIER)
  include(GetGitRevisionDescription)
  get_git_head_revision(STAR_GIT_REFSPEC STAR_GIT_HASHVAR)
  set(STAR_SOURCE_IDENTIFIER "${STAR_GIT_HASHVAR}")
endif()

# Architecture identifier, like i386, x86_64 or ppc
if(NOT DEFINED STAR_ARCHITECTURE)
  include(TargetArch)
  target_architecture(STAR_ARCHITECTURE)
endif()

# Either TRUE or FALSE
if(NOT DEFINED STAR_LITTLE_ENDIAN)
  include(TestBigEndian)
  test_big_endian(BIGENDIAN)
  if(NOT BIGENDIAN)
    set(STAR_LITTLE_ENDIAN TRUE)
  else()
    set(STAR_LITTLE_ENDIAN FALSE)
  endif()
endif()

# System name, like windows, macos, linux, freebsd, or (generic) unix
if(NOT DEFINED STAR_SYSTEM)
  if(WIN32)
    set(STAR_SYSTEM "windows")
  elseif(APPLE AND ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(STAR_SYSTEM "macos")
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(STAR_SYSTEM "linux")
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    set(STAR_SYSTEM "freebsd")
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD")
    set(STAR_SYSTEM "netbsd")
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Haiku")
    set(STAR_SYSTEM "haiku")
  elseif(UNIX)
    set(STAR_SYSTEM "unix")
  else()
    set(STAR_SYSTEM "unknown")
  endif()
endif()

if(NOT DEFINED STAR_SYSTEM_FAMILY)
  if(WIN32)
    set(STAR_SYSTEM_FAMILY "windows")
  elseif(UNIX)
    set(STAR_SYSTEM_FAMILY "unix")
  else()
    set(STAR_SYSTEM_FAMILY "unknown")
  endif()
endif()

# C/C++ compiler ID, like clang, gnu, or msvc
if(NOT DEFINED STAR_COMPILER)
  if(NOT CMAKE_C_COMPILER_ID STREQUAL CMAKE_CXX_COMPILER_ID)
    message(FATAL_ERROR "C and C++ compiler id do not match, unsupported build configuration")
  endif()

  if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
    set(STAR_COMPILER "clang")
  elseif(CMAKE_COMPILER_IS_GNUC)
    set(STAR_COMPILER "gnu")
  elseif(MSVC)
    set(STAR_COMPILER "msvc")
  else()
    string(TOLOWER "${CMAKE_C_COMPILER_ID}" STAR_COMPILER)
  endif()
endif()

# Enable OPTIONs based on the discovered system / environment...

if(STAR_COMPILER STREQUAL "gnu")
  option(STAR_ENABLE_STATIC_LIBGCC_LIBSTDCXX "Statically link libgcc and libstdc++" OFF)
  option(STAR_ENABLE_GCC_PROFILING "Enable gcc/g++ profiling via the -pg flag" OFF)
  option(STAR_ENABLE_GLIBCXX_DEBUG "Enable _GLIBCXX_DEBUG for g++" OFF)
endif()

if(STAR_COMPILER STREQUAL "msvc")
  option(STAR_ENABLE_STATIC_MSVC_RUNTIME "Statically link with the CRT" OFF)
endif()

if((STAR_COMPILER STREQUAL "clang" OR STAR_COMPILER STREQUAL "gnu") AND NOT STAR_SYSTEM_MACOS)
  option(STAR_USE_LLD "Link with LLVM lld when supported by the compiler" ON)
endif()

option(STAR_BUILD_GUI "Build GUI utilities and Client" ON)

if(STAR_BUILD_GUI)
  option(STAR_BUILD_QT_TOOLS "Build GUI utilities that require Qt" OFF)
  option(STAR_ENABLE_STEAM_INTEGRATION "Use Steam platform services" OFF)
  option(STAR_ENABLE_DISCORD_INTEGRATION "Use Discord platform services" OFF)
endif()

option(STAR_LUA_APICHECK "Use lua api checks" OFF)
option(STAR_USE_JEMALLOC "Use jemalloc allocators" OFF)
option(STAR_USE_MIMALLOC "Use mimalloc allocators" OFF)
option(STAR_USE_RPMALLOC "Use rpmalloc allocators" OFF)

# Report all the discovered system / environment settings and all options.

message(STATUS "Source ID: ${STAR_SOURCE_IDENTIFIER}")
message(STATUS "Architecture: ${STAR_ARCHITECTURE}")
message(STATUS "Little Endian: ${STAR_LITTLE_ENDIAN}")
message(STATUS "System: ${STAR_SYSTEM}")
message(STATUS "System family: ${STAR_SYSTEM_FAMILY}")
message(STATUS "C/C++ compiler: ${STAR_COMPILER}")
message(STATUS "C++ standard: C++20")
message(STATUS "Compiler cache: ${STAR_COMPILER_CACHE}")
message(STATUS "Precompiled headers: ${STAR_PRECOMPILED_HEADERS}")
message(STATUS "LTO / IPO: ${STAR_ENABLE_LTO}")
message(STATUS "Frame pointers: ${STAR_ENABLE_FRAME_POINTERS}")
message(STATUS "Unity build: ${STAR_ENABLE_UNITY_BUILD}")

if(DEFINED STAR_ENABLE_STATIC_LIBGCC_LIBSTDCXX)
  message(STATUS "Statically linking to libgcc / libstdc++: ${STAR_ENABLE_STATIC_LIBGCC_LIBSTDCXX}")
endif()

if(DEFINED STAR_ENABLE_STATIC_MSVC_RUNTIME)
  message(STATUS "Statically linking to CRT: ${STAR_ENABLE_STATIC_MSVC_RUNTIME}")
endif()

if(DEFINED STAR_ENABLE_GLIBCXX_DEBUG)
  message(STATUS "Enabling _GLIBCXX_DEBUG: ${STAR_ENABLE_GLIBCXX_DEBUG}")
endif()

if(DEFINED STAR_USE_LLD)
  message(STATUS "Using lld linker: ${STAR_USE_LLD}")
endif()

message(STATUS "Building GUI: ${STAR_BUILD_GUI}")

if(DEFINED STAR_BUILD_QT_TOOLS)
  message(STATUS "Building Qt tools: ${STAR_BUILD_QT_TOOLS}")
endif()

if(DEFINED STAR_ENABLE_STEAM_INTEGRATION)
  message(STATUS "Using Steam platform services: ${STAR_ENABLE_STEAM_INTEGRATION}")
endif()

if(DEFINED STAR_ENABLE_DISCORD_INTEGRATION)
  message(STATUS "Using Discord platform services: ${STAR_ENABLE_DISCORD_INTEGRATION}")
endif()

message(STATUS "Using Lua API checks: ${STAR_LUA_APICHECK}")
message(STATUS "Using jemalloc: ${STAR_USE_JEMALLOC}")
message(STATUS "Using mimalloc: ${STAR_USE_MIMALLOC}")
message(STATUS "Using rpmalloc: ${STAR_USE_RPMALLOC}")

# Set C defines and cmake variables based on the build settings we have now
# determined...

add_library(star_build_definitions INTERFACE)

# Set a cmake variable to true and define a corresponding C/C++ definition
function(SET_FLAG flagValue)
  set(${flagValue} TRUE PARENT_SCOPE)
  target_compile_definitions(star_build_definitions INTERFACE ${flagValue})
endfunction()

if(STAR_LITTLE_ENDIAN)
  set_flag(STAR_LITTLE_ENDIAN)
else()
  set_flag(STAR_BIG_ENDIAN)
endif()

if(STAR_ARCHITECTURE STREQUAL "i386")
  set_flag(STAR_ARCHITECTURE_I386)
elseif(STAR_ARCHITECTURE STREQUAL "x86_64")
  set_flag(STAR_ARCHITECTURE_X86_64)
endif()

if(STAR_SYSTEM STREQUAL "windows")
  set_flag(STAR_SYSTEM_WINDOWS)
elseif(STAR_SYSTEM STREQUAL "macos")
  set_flag(STAR_SYSTEM_MACOS)
elseif(STAR_SYSTEM STREQUAL "linux")
  set_flag(STAR_SYSTEM_LINUX)
elseif(STAR_SYSTEM STREQUAL "freebsd")
  set_flag(STAR_SYSTEM_FREEBSD)
elseif(STAR_SYSTEM STREQUAL "netbsd")
  set_flag(STAR_SYSTEM_NETBSD)
elseif(STAR_SYSTEM STREQUAL "haiku")
  set_flag(STAR_SYSTEM_HAIKU)
endif()

if(STAR_SYSTEM_FAMILY STREQUAL "windows")
  set_flag(STAR_SYSTEM_FAMILY_WINDOWS)
elseif(STAR_SYSTEM_FAMILY STREQUAL "unix")
  set_flag(STAR_SYSTEM_FAMILY_UNIX)
endif()

if(STAR_COMPILER STREQUAL "gnu")
  set_flag(STAR_COMPILER_GNU)
elseif(STAR_COMPILER STREQUAL "clang")
  set_flag(STAR_COMPILER_CLANG)
elseif(STAR_COMPILER STREQUAL "msvc")
  set_flag(STAR_COMPILER_MSVC)
endif()

if(STAR_LUA_APICHECK)
  target_compile_definitions(star_build_definitions INTERFACE LUA_USE_APICHECK)
endif()

if(STAR_SYSTEM_WINDOWS)
  # LUA_USE_WINDOWS is automatically defined in luaconf if _WIN32 is defined
elseif(STAR_SYSTEM_MACOS)
  target_compile_definitions(star_build_definitions INTERFACE LUA_USE_MACOSX)
elseif(STAR_SYSTEM_LINUX)
  target_compile_definitions(star_build_definitions INTERFACE LUA_USE_LINUX)
elseif(STAR_SYSTEM_FAMILY_UNIX)
  target_compile_definitions(star_build_definitions INTERFACE LUA_USE_POSIX)
endif()

if(STAR_ENABLE_STEAM_INTEGRATION)
  target_compile_definitions(star_build_definitions INTERFACE STAR_ENABLE_STEAM_INTEGRATION)
endif()

if(STAR_ENABLE_DISCORD_INTEGRATION)
  target_compile_definitions(star_build_definitions INTERFACE STAR_ENABLE_DISCORD_INTEGRATION)
endif()

if(STAR_USE_JEMALLOC)
  target_compile_definitions(star_build_definitions INTERFACE STAR_USE_JEMALLOC)
elseif(STAR_USE_MIMALLOC)
  target_compile_definitions(star_build_definitions INTERFACE STAR_USE_MIMALLOC)
elseif(STAR_USE_RPMALLOC)
  target_compile_definitions(star_build_definitions INTERFACE STAR_USE_RPMALLOC ENABLE_PRELOAD)
endif()

# Set C/C++ compiler flags based on build environment...

add_library(star_build_options INTERFACE)
target_compile_features(star_build_options INTERFACE cxx_std_20)

set(STAR_C_COMPILE_OPTIONS)
set(STAR_CXX_COMPILE_OPTIONS)

function(star_clear_cmake_config_flags)
  foreach(config Debug RelWithAsserts RelWithDebInfo Release)
    string(TOUPPER "${config}" config_upper)
    set(CMAKE_C_FLAGS_${config_upper} "" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_${config_upper} "" PARENT_SCOPE)
  endforeach()
endfunction()

function(star_append_config_compile_options config)
  foreach(option IN LISTS ARGN)
    list(APPEND STAR_C_COMPILE_OPTIONS "$<$<CONFIG:${config}>:${option}>")
    list(APPEND STAR_CXX_COMPILE_OPTIONS "$<$<CONFIG:${config}>:${option}>")
  endforeach()

  set(STAR_C_COMPILE_OPTIONS "${STAR_C_COMPILE_OPTIONS}" PARENT_SCOPE)
  set(STAR_CXX_COMPILE_OPTIONS "${STAR_CXX_COMPILE_OPTIONS}" PARENT_SCOPE)
endfunction()

function(star_add_c_compile_options)
  target_compile_options(star_build_options INTERFACE "$<$<COMPILE_LANGUAGE:C>:${ARGN}>")
endfunction()

function(star_add_cxx_compile_options)
  target_compile_options(star_build_options INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:${ARGN}>")
endfunction()

function(star_add_config_compile_options config)
  target_compile_options(star_build_options INTERFACE
      "$<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:${config}>>:${ARGN}>"
      "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:${config}>>:${ARGN}>")
endfunction()

if(STAR_COMPILER_GNU)
  list(APPEND STAR_C_COMPILE_OPTIONS -Wall -Wextra -Wno-unused -Wno-implicit-fallthrough)
  list(APPEND STAR_CXX_COMPILE_OPTIONS -Wall -Wextra -Wno-unused -Wno-implicit-fallthrough -Wno-array-bounds -Wno-dangling-reference -Wno-maybe-uninitialized -Wno-redundant-move -Wno-tautological-compare -Wno-terminate)
  target_link_options(star_build_options INTERFACE -rdynamic -no-pie)
  star_clear_cmake_config_flags()

  if(STAR_SYSTEM_FAMILY_WINDOWS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthreads")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthreads")
  else()
    target_compile_definitions(star_build_definitions INTERFACE _REENTRANT)
    if(STAR_USE_LLD)
      target_link_options(star_build_options INTERFACE -fuse-ld=lld)
    endif()
  endif()

  if(STAR_ENABLE_FRAME_POINTERS)
    list(APPEND STAR_C_COMPILE_OPTIONS -fno-omit-frame-pointer)
    list(APPEND STAR_CXX_COMPILE_OPTIONS -fno-omit-frame-pointer)
  endif()

  if(STAR_ENABLE_STATIC_LIBGCC_LIBSTDCXX)
    target_link_options(star_build_options INTERFACE -static-libgcc $<$<LINK_LANGUAGE:CXX>:-static-libstdc++>)
  endif()

  if(STAR_ENABLE_GCC_PROFILING)
    list(APPEND STAR_C_COMPILE_OPTIONS -pg)
    list(APPEND STAR_CXX_COMPILE_OPTIONS -pg)
    target_link_options(star_build_options INTERFACE -pg)
  endif()

  star_append_config_compile_options(Debug -g -Og)
  star_append_config_compile_options(RelWithAsserts -g -O3 -ffast-math)
  star_append_config_compile_options(RelWithDebInfo -g -DNDEBUG -O3 -ffast-math)
  star_append_config_compile_options(Release -DNDEBUG -O3 -ffast-math)

  set(CMAKE_SKIP_BUILD_RPATH TRUE)

elseif(STAR_COMPILER_CLANG)
  star_add_c_compile_options(-Wall -Wextra -Wuninitialized -Wno-parentheses-equality -Wno-deprecated-declarations -Wno-nan-infinity-disabled)
  star_add_cxx_compile_options(-Wall -Wextra -Wuninitialized -Wno-parentheses-equality -Wno-deprecated-declarations -Wno-nan-infinity-disabled)
  star_clear_cmake_config_flags()

  if(STAR_SYSTEM_MACOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    target_link_options(star_build_options INTERFACE -Wl,-export_dynamic)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  else()
    target_compile_definitions(star_build_definitions INTERFACE _REENTRANT)
    target_link_options(star_build_options INTERFACE -Wl,--export-dynamic)
    if(STAR_USE_LLD)
      target_link_options(star_build_options INTERFACE -fuse-ld=lld)
    endif()
  endif()

  if(STAR_ENABLE_FRAME_POINTERS)
    star_add_c_compile_options(-fno-omit-frame-pointer)
    star_add_cxx_compile_options(-fno-omit-frame-pointer)
  endif()

  star_add_config_compile_options(Debug -g)
  star_add_config_compile_options(RelWithAsserts -g -O3 -ffast-math)
  star_add_config_compile_options(RelWithDebInfo -gline-tables-only -gz=zlib -DNDEBUG -O3 -ffast-math)
  star_add_config_compile_options(Release -DNDEBUG -O3 -ffast-math)

  set(CMAKE_SKIP_BUILD_RPATH TRUE)

elseif(STAR_COMPILER_MSVC)
  # /MP      - Multi-processor building
  # /EHsc    - Enable normal C++ exception handling
  # /bigobj  - More sections in .obj files (Cannot build in Debug without it)
  # /MT      - Use multi-threaded statically linked C runtime library
  # /GA      - Optimize for windows application
  # /Ox      - Full optimization
  # /fp:fast - Equivalent to -ffast-math
  # /GS-     - Disable buffers security check
  # /Zi      - Generates debugging information without Edit and Continue
  # /Z7      - Like the above, but debugging information is stored per-object
  # /Gy      - Use function-level linking
  # /wd4996  - Disable warnings about unsafe C functions
  # /wd4351  - Disable warnings about new behavior of default initialization of
  #            arrays (which is the correct behavior anyway)
  # /wd4800  - Disable warnings about using non-bool as true or false (useless
  #            performance warning)
  # /wd4244  - Disable warnings about type conversion loss of data, it's a nice
  #            warning, but it triggers on lots and lots of harmless things that no
  #            other compiler warns about, like passing an int as a float parameter
  # /wd4305  - Disable warnings about truncation from double to float
  # /wd4267  - Disable warnings about 64 - 32 bit truncation
  # /wd4456  - Disable warnings about hiding previous local declaration
  # /wd4503  - Silence warnings about MSVC generating a name so long it has to
  #            truncate it
  # /wd4250  - Silence "XX inherits YY via dominance"
  # /wd4624  - Silence implicitly deleted destructor warnings that show up when
  #            using unions in interesting ways.

  list(APPEND STAR_C_COMPILE_OPTIONS /nologo /MP /EHsc /bigobj /wd4996 /wd4351 /wd4800 /wd4244 /wd4305 /wd4267 /wd4456 /wd4503 /wd4250 /wd4624)
  list(APPEND STAR_CXX_COMPILE_OPTIONS /nologo /MP /EHsc /bigobj /wd4996 /wd4351 /wd4800 /wd4244 /wd4305 /wd4267 /wd4456 /wd4503 /wd4250 /wd4624)

  if(STAR_ENABLE_STATIC_MSVC_RUNTIME)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
  endif()

  set(CMAKE_C_FLAGS_DEBUG "/Z7 /Od")
  set(CMAKE_CXX_FLAGS_DEBUG "/Z7 /Od")

  set(CMAKE_C_FLAGS_RELWITHASSERTS "/Ox /fp:fast /GA /GS- /Z7 /Gy")
  set(CMAKE_CXX_FLAGS_RELWITHASSERTS "/Ox /fp:fast /GA /GS- /Z7 /Gy")

  set(CMAKE_C_FLAGS_RELWITHDEBINFO "/Ox /fp:fast /GA /GS- /Z7 /Gy /DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Ox /fp:fast /GA /GS- /Z7 /Gy /DNDEBUG")

  set(CMAKE_C_FLAGS_RELEASE "/Ox /fp:fast /GA /GS- /Gy /DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "/Ox /fp:fast /GA /GS- /Gy /DNDEBUG")

  if(STAR_ARCHITECTURE_I386)
    # Assume all 32 bit target cpus support MMX, SSE, and SSE2

    set(CMAKE_C_FLAGS_RELWITHASSERTS "${CMAKE_C_FLAGS_RELWITHASSERTS} /arch:SSE2")
    set(CMAKE_CXX_FLAGS_RELWITHASSERTS "${CMAKE_CXX_FLAGS_RELWITHASSERTS} /arch:SSE2")

    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /arch:SSE2")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /arch:SSE2")

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /arch:SSE2")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE2")
  endif()

  target_compile_definitions(star_build_definitions INTERFACE UNICODE _UNICODE NOMINMAX)

else()
  list(APPEND STAR_C_COMPILE_OPTIONS -Wall)
  list(APPEND STAR_CXX_COMPILE_OPTIONS -Wall)
  target_compile_definitions(star_build_definitions INTERFACE _REENTRANT)
  star_clear_cmake_config_flags()

  star_append_config_compile_options(Debug -g)
  star_append_config_compile_options(RelWithAsserts -g -O2)
  star_append_config_compile_options(RelWithDebInfo -DNDEBUG -g -O2)
  star_append_config_compile_options(Release -DNDEBUG -O2)

endif()

# Set other global build settings based on environment...

if(STAR_SYSTEM_MACOS)
  set(CMAKE_MODULE_LINKER_FLAGS "-flat_namespace -undefined suppress")
elseif(STAR_SYSTEM_WINDOWS)
  set(CMAKE_RC_COMPILER_INIT windres)

  enable_language(RC)
  if(STAR_COMPILER STREQUAL "msvc")
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> /fo <OBJECT> <SOURCE>")
  else()
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif()
endif()

if(STAR_COMPILER STREQUAL "msvc")
  # /largeaddressaware - Make 32 bit build able to use 3GB addresses
  # /OPT:REF           - Eliminates functions and data that are never referenced
  # /OPT:ICF           - Performs identical COMDAT folding
  # /PDBCompress       - Hint to windows that it should compress the resulting PDB files
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /largeaddressaware /OPT:REF /OPT:ICF /PDBCompress")

  # Make sure RelWithAsserts has debugging enabled
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHASSERTS "${CMAKE_EXE_LINKER_FLAGS_RELWITHASSERTS} /DEBUG")
endif()

add_library(star_system_libraries INTERFACE)
if(NOT STAR_SYSTEM_WINDOWS)
  find_package(Threads REQUIRED)
  target_link_libraries(star_system_libraries INTERFACE Threads::Threads)
endif()

if(STAR_SYSTEM_WINDOWS)
  target_link_libraries(star_system_libraries INTERFACE ws2_32.lib iphlpapi.lib shlwapi.lib dbghelp.lib)

elseif(STAR_SYSTEM_LINUX)
  target_link_libraries(star_system_libraries INTERFACE dl rt)

elseif(STAR_SYSTEM_FREEBSD)
  target_link_libraries(star_system_libraries INTERFACE rt)

elseif(STAR_SYSTEM_NETBSD)
  target_link_libraries(star_system_libraries INTERFACE rt execinfo)

elseif(STAR_SYSTEM_HAIKU)
  target_link_libraries(star_system_libraries INTERFACE execinfo network)

endif()

# Find all required external libraries, based on build settings...
add_library(star_system_includes INTERFACE)
add_library(star_external_libraries INTERFACE)
find_package(PkgConfig)

if(STAR_USE_JEMALLOC)
  pkg_check_modules(JEMALLOC REQUIRED IMPORTED_TARGET GLOBAL jemalloc)

  target_link_libraries(star_external_libraries INTERFACE PkgConfig::JEMALLOC)
  if (STAR_SYSTEM_WINDOWS OR STAR_SYSTEM_MACOS)
    set_flag(STAR_JEMALLOC_IS_PREFIXED)
  endif()
endif()

if (STAR_USE_MIMALLOC)
  find_package(mimalloc CONFIG REQUIRED)
  target_link_libraries(star_external_libraries INTERFACE $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)
endif()

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Freetype REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(OggVorbis REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
if (NOT STAR_SYSTEM_HAIKU)
	find_package(Opus CONFIG REQUIRED)
	find_package(re2 CONFIG REQUIRED)
endif()
target_include_directories(star_system_includes SYSTEM INTERFACE
    ${FREETYPE_INCLUDE_DIRS}
    ${OGGVORBIS_INCLUDE_DIR}
)

target_link_libraries(star_external_libraries INTERFACE
    ZLIB::ZLIB
    PNG::PNG
    cpr::cpr
    $<IF:$<TARGET_EXISTS:Freetype::Freetype>,Freetype::Freetype,freetype>
    $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>
    $<IF:$<TARGET_EXISTS:Opus::opus>,Opus::opus,opus>
    $<IF:$<TARGET_EXISTS:re2::re2>,re2::re2,re2>
    imgui::imgui
    ${VORBISFILE_LIBRARY}
    ${VORBIS_LIBRARY}
    ${OGG_LIBRARY}
)

pkg_check_modules(PKG_WAYLAND "wayland-client>=1.18" wayland-egl wayland-cursor egl "xkbcommon>=0.5.0")
if (PKG_WAYLAND_FOUND)
  message(STATUS "Wayland was found")
else()
  message(STATUS "Wayland was not found")
endif()

if(STAR_SYSTEM_FAMILY STREQUAL "unix" AND NOT STAR_SYSTEM_HAIKU)
  set_flag(STAR_USE_CPPTRACE)
  find_package(cpptrace CONFIG REQUIRED)
  target_link_libraries(star_external_libraries INTERFACE cpptrace::cpptrace)
  message(STATUS "Using cpptrace")
endif()

add_library(star_external_gui_libraries INTERFACE)
if(STAR_BUILD_GUI)
  find_package(SDL3 CONFIG REQUIRED)

  target_link_libraries(star_external_gui_libraries INTERFACE
    $<TARGET_NAME_IF_EXISTS:SDL3::SDL3main>
    $<IF:$<TARGET_EXISTS:SDL3::SDL3>,SDL3::SDL3,SDL3::SDL3-static>
  )

  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)

  target_include_directories(star_system_includes SYSTEM INTERFACE ${GLEW_INCLUDE_DIR})
  target_link_libraries(star_external_gui_libraries INTERFACE
      ${OPENGL_LIBRARY}
      $<IF:$<TARGET_EXISTS:GLEW::glew_s>,GLEW::glew_s,GLEW>
  )

  if(STAR_SYSTEM_MACOS)
    target_link_libraries(star_external_gui_libraries INTERFACE "-framework CoreAudio")
  endif()

  if(STAR_ENABLE_STEAM_INTEGRATION)
    find_package(SteamApi REQUIRED)
    target_include_directories(star_system_includes SYSTEM INTERFACE ${STEAM_API_INCLUDE_DIR})
    target_link_libraries(star_external_gui_libraries INTERFACE ${STEAM_API_LIBRARY})
  endif()

  if(STAR_ENABLE_DISCORD_INTEGRATION)
    find_package(DiscordApi REQUIRED)
    target_link_libraries(star_external_gui_libraries INTERFACE ${DISCORD_API_LIBRARY})
  endif()
endif()

if(STAR_C_COMPILE_OPTIONS)
  target_compile_options(star_build_options INTERFACE "$<$<COMPILE_LANGUAGE:C>:${STAR_C_COMPILE_OPTIONS}>")
endif()
if(STAR_CXX_COMPILE_OPTIONS)
  target_compile_options(star_build_options INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:${STAR_CXX_COMPILE_OPTIONS}>")
endif()

target_link_libraries(star_build_definitions INTERFACE star_system_includes star_system_libraries star_build_options)

# Set basic build flags, include all the relevant source directories, based on
# build settings...

set(BUILD_SHARED_LIBS false)

# First set output dir for the generic no-config case (e.g. macos / linux)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../dist)

# Second, set output dir for multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/../dist)
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

# External code included with starbound source, which core depends on
set(STAR_EXTERN_INCLUDES ${PROJECT_SOURCE_DIR}/extern)
add_subdirectory(extern)

# Core support code, not specific to starbound.
set(STAR_CORE_INCLUDES
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/core/scripting
)
add_subdirectory(core)

# Less general purpose code than core that is available to both the game and
# application modules.
set(STAR_BASE_INCLUDES
    ${PROJECT_SOURCE_DIR}/base
    ${PROJECT_SOURCE_DIR}/base/scripting
)
add_subdirectory(base)

# Platform APIs that are implemented by the application module
set(STAR_PLATFORM_INCLUDES ${PROJECT_SOURCE_DIR}/platform)
add_subdirectory(platform)

# Core game logic used by both server and client.
set(STAR_GAME_INCLUDES
    ${PROJECT_SOURCE_DIR}/game
    ${PROJECT_SOURCE_DIR}/game/interfaces
    ${PROJECT_SOURCE_DIR}/game/items
    ${PROJECT_SOURCE_DIR}/game/objects
    ${PROJECT_SOURCE_DIR}/game/scripting
    ${PROJECT_SOURCE_DIR}/game/terrain
  )
add_subdirectory(game)

# Googletest based tests
option(BUILD_TESTING "Build test projects" OFF)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

# Starbound stand-alone server.
add_subdirectory(server)

# cmdline utilities
add_subdirectory(utility)

if(STAR_BUILD_GUI)
  # Handles creating windows, keyboard / mouse / joystick input, and the 2d
  # rendering model.
  set(STAR_APPLICATION_INCLUDES ${PROJECT_SOURCE_DIR}/application)
  add_subdirectory(application)

  # Rendering code not dependent on widget system
  set(STAR_RENDERING_INCLUDES ${PROJECT_SOURCE_DIR}/rendering)
  add_subdirectory(rendering)

  # Panes and Widgets
  set(STAR_WINDOWING_INCLUDES ${PROJECT_SOURCE_DIR}/windowing)
  add_subdirectory(windowing)

  # Client interface code
  set(STAR_FRONTEND_INCLUDES ${PROJECT_SOURCE_DIR}/frontend)
  add_subdirectory(frontend)

  # Starbound game / client
  add_subdirectory(client)

  # Qt GUI tools
  if(STAR_BUILD_QT_TOOLS)
    add_subdirectory(json_tool)

    if(STAR_ENABLE_STEAM_INTEGRATION)
      add_subdirectory(mod_uploader)
    endif()
  endif()
endif()
