cmake_minimum_required(VERSION 3.28)
# Remember command-line configuration flags before project() gives cache entries
# their normal types. These values take precedence over project defaults.
foreach(language C CXX)
  foreach(config DEBUG RELWITHASSERTS RELWITHDEBINFO RELEASE)
    set(flag_variable CMAKE_${language}_FLAGS_${config})
    if(NOT DEFINED ${flag_variable})
      unset(STAR_EXPLICIT_${language}_FLAGS_${config} CACHE)
    endif()
    get_property(flag_type CACHE ${flag_variable} PROPERTY TYPE)
    get_property(flag_help CACHE ${flag_variable} PROPERTY HELPSTRING)
    if(flag_type STREQUAL "UNINITIALIZED" OR flag_help MATCHES "specified on the command line")
      set(STAR_EXPLICIT_${language}_FLAGS_${config} TRUE CACHE INTERNAL "Caller-selected configuration flags")
    endif()
  endforeach()
endforeach()

if(NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES Debug RelWithAsserts RelWithDebInfo Release CACHE STRING "Available build configurations")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compiler commands")
project(starbound)

# GCC module interfaces produce .gcm files; sccache cache hits restore only
# object files, not BMIs. Other explicitly selected launchers are preserved.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_LAUNCHER MATCHES "(^|[/;])sccache([.;]|$)")
  message(WARNING "Disabling the C++ sccache launcher: GCC module BMIs are not cached")
  set(CMAKE_CXX_COMPILER_LAUNCHER "")
endif()

if(NOT DEFINED CMAKE_C_COMPILER_LAUNCHER OR
    (NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
  find_program(SCCACHE_PATH sccache)
  if(SCCACHE_PATH)
    set(STAR_DEFAULT_COMPILER_LAUNCHER "${SCCACHE_PATH}")
  else()
    find_program(CCACHE_PATH ccache)
    if(CCACHE_PATH)
      set(STAR_DEFAULT_COMPILER_LAUNCHER "${CCACHE_PATH}")
    endif()
  endif()
  if(STAR_DEFAULT_COMPILER_LAUNCHER)
    if(NOT DEFINED CMAKE_C_COMPILER_LAUNCHER)
      set(CMAKE_C_COMPILER_LAUNCHER "${STAR_DEFAULT_COMPILER_LAUNCHER}")
    endif()
    if(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set(CMAKE_CXX_COMPILER_LAUNCHER "${STAR_DEFAULT_COMPILER_LAUNCHER}")
    endif()
  endif()
endif()

if(CMAKE_CXX_COMPILER_LAUNCHER MATCHES "(^|[/;])ccache([.;]|$)")
  set(STAR_CCACHE TRUE)
else()
  set(STAR_CCACHE FALSE)
endif()

# The default follows the ccache decision (a compiler cache and a precompiled
# header solve the same problem), but an explicit -DSTAR_PRECOMPILED_HEADERS=...
# wins, which is how the cost of the precompiled header is measured.
if(NOT DEFINED STAR_PRECOMPILED_HEADERS)
  if(STAR_CCACHE)
    set(STAR_PRECOMPILED_HEADERS FALSE)
  else()
    set(STAR_PRECOMPILED_HEADERS TRUE)
  endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake")

# One place for the language standard.  The per-platform flag blocks below used
# to pass -std=c++20 for GNU/Clang while MSVC, Xcode and the vcpkg presets were
# left on C++17, so the same tree was being built as two different languages.
# CMAKE_CXX_EXTENSIONS OFF keeps -std=c++20 rather than -std=gnu++20, which is
# what the explicit flags asked for.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Only the targets that import modules opt into scanning; module file sets are
# scanned regardless. Avoid scanning every unrelated C++20 translation unit.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)


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
  elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
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

option(STAR_BUILD_GUI "Build GUI utilities and Client" ON)

if(STAR_BUILD_GUI)
  option(STAR_BUILD_QT_TOOLS "Build GUI utilities that require Qt" OFF)
  option(STAR_ENABLE_STEAM_INTEGRATION "Use Steam platform services" OFF)
  option(STAR_ENABLE_DISCORD_INTEGRATION "Use Discord platform services" OFF)
endif()

option(STAR_BUILD_DEV_TOOLS "Build the maintenance utilities (map_grep, tileset tools)" OFF)

option(STAR_LUA_APICHECK "Use lua api checks" OFF)
option(STAR_USE_JEMALLOC "Use jemalloc allocators" OFF)
option(STAR_USE_MIMALLOC "Use mimalloc allocators" OFF)
option(STAR_USE_RPMALLOC "Use rpmalloc allocators" OFF)

# "default" preserves Clang's line-table-only RelWithDebInfo behavior; "full"
# requests full symbols and "minimal" is useful for sanitizer/assert packages.
set(STAR_DEBUG_SYMBOL_LEVEL "default" CACHE STRING "Debug symbols: default, minimal, or full")
set_property(CACHE STAR_DEBUG_SYMBOL_LEVEL PROPERTY STRINGS default minimal full)
if(NOT STAR_DEBUG_SYMBOL_LEVEL MATCHES "^(default|minimal|full)$")
  message(FATAL_ERROR "STAR_DEBUG_SYMBOL_LEVEL must be default, minimal, or full")
endif()

add_library(star_build_config INTERFACE)
add_library(star_gui_dependencies INTERFACE)

# Report all the discovered system / environment settings and all options.

message(STATUS "Source ID: ${STAR_SOURCE_IDENTIFIER}")
message(STATUS "Architecture: ${STAR_ARCHITECTURE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "System: ${STAR_SYSTEM}")
message(STATUS "System family: ${STAR_SYSTEM_FAMILY}")
message(STATUS "C/C++ compiler: ${STAR_COMPILER}")

if(DEFINED STAR_ENABLE_STATIC_LIBGCC_LIBSTDCXX)
  message(STATUS "Statically linking to libgcc / libstdc++: ${STAR_ENABLE_STATIC_LIBGCC_LIBSTDCXX}")
endif()

if(DEFINED STAR_ENABLE_STATIC_MSVC_RUNTIME)
  message(STATUS "Statically linking to CRT: ${STAR_ENABLE_STATIC_MSVC_RUNTIME}")
endif()

if(DEFINED STAR_ENABLE_GLIBCXX_DEBUG)
  message(STATUS "Enabling _GLIBCXX_DEBUG: ${STAR_ENABLE_GLIBCXX_DEBUG}")
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

# Set a cmake variable to true and define a corresponding C/C++ definition
function(SET_FLAG flagValue)
  set(${flagValue} TRUE PARENT_SCOPE)
  target_compile_definitions(star_build_config INTERFACE ${flagValue})
endfunction()

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
  target_compile_definitions(star_build_config INTERFACE LUA_USE_APICHECK)
endif()

if(STAR_SYSTEM_WINDOWS)
  # LUA_USE_WINDOWS is automatically defined in luaconf if _WIN32 is defined
elseif(STAR_SYSTEM_MACOS)
  target_compile_definitions(star_build_config INTERFACE LUA_USE_MACOSX)
elseif(STAR_SYSTEM_LINUX)
  target_compile_definitions(star_build_config INTERFACE LUA_USE_LINUX)
elseif(STAR_SYSTEM_FAMILY_UNIX)
  target_compile_definitions(star_build_config INTERFACE LUA_USE_POSIX)
endif()

if(STAR_ENABLE_STEAM_INTEGRATION)
  target_compile_definitions(star_build_config INTERFACE STAR_ENABLE_STEAM_INTEGRATION)
endif()

if(STAR_ENABLE_DISCORD_INTEGRATION)
  target_compile_definitions(star_build_config INTERFACE STAR_ENABLE_DISCORD_INTEGRATION)
endif()

if(STAR_USE_JEMALLOC)
  target_compile_definitions(star_build_config INTERFACE STAR_USE_JEMALLOC)
elseif(STAR_USE_MIMALLOC)
  target_compile_definitions(star_build_config INTERFACE STAR_USE_MIMALLOC)
elseif(STAR_USE_RPMALLOC)
  target_compile_definitions(star_build_config INTERFACE STAR_USE_RPMALLOC ENABLE_PRELOAD)
endif()

# Compiler/configuration defaults belong only to in-tree compiled targets.
include(StarBuildOptions)
if((STAR_COMPILER_GNU OR STAR_COMPILER_CLANG) AND NOT DEFINED CMAKE_SKIP_BUILD_RPATH)
  set(CMAKE_SKIP_BUILD_RPATH TRUE)
endif()

if(STAR_SYSTEM_WINDOWS)
  enable_language(RC)
  target_link_libraries(star_build_config INTERFACE ws2_32 iphlpapi shlwapi dbghelp)
elseif(STAR_SYSTEM_LINUX)
  target_link_libraries(star_build_config INTERFACE pthread ${CMAKE_DL_LIBS} rt)
elseif(STAR_SYSTEM_FREEBSD)
  target_link_libraries(star_build_config INTERFACE pthread rt)
elseif(STAR_SYSTEM_NETBSD)
  target_link_libraries(star_build_config INTERFACE pthread rt execinfo)
elseif(STAR_SYSTEM_HAIKU)
  target_link_libraries(star_build_config INTERFACE pthread execinfo network)
endif()

# Find all required external libraries, based on build settings...
find_package(PkgConfig)

if(STAR_USE_JEMALLOC)
  pkg_check_modules(JEMALLOC REQUIRED IMPORTED_TARGET GLOBAL jemalloc)

  set(STAR_EXT_LIBS PkgConfig::JEMALLOC)
  if (STAR_SYSTEM_WINDOWS OR STAR_SYSTEM_MACOS)
    set_flag(STAR_JEMALLOC_IS_PREFIXED)
  endif()
endif()

if (STAR_USE_MIMALLOC)
  find_package(mimalloc CONFIG REQUIRED)
  set(STAR_EXT_LIBS ${STAR_EXT_LIBS} $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)
endif()

find_package(zlib-ng CONFIG REQUIRED)
target_compile_definitions(star_build_config INTERFACE WITH_GZFILEOP)
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
target_include_directories(star_build_config SYSTEM INTERFACE
    ${FREETYPE_INCLUDE_DIRS}
    ${OGGVORBIS_INCLUDE_DIR}
)

set(STAR_EXT_LIBS ${STAR_EXT_LIBS}
    zlib-ng::zlib
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
  set(STAR_EXT_LIBS ${STAR_EXT_LIBS} cpptrace::cpptrace)
  message(STATUS "Using cpptrace")
endif()

if(STAR_BUILD_GUI)
  find_package(SDL3 CONFIG REQUIRED)

  set(STAR_EXT_GUI_LIBS
    $<TARGET_NAME_IF_EXISTS:SDL3::SDL3main>
    $<IF:$<TARGET_EXISTS:SDL3::SDL3>,SDL3::SDL3,SDL3::SDL3-static>
  )

  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)

  target_include_directories(star_gui_dependencies SYSTEM INTERFACE ${GLEW_INCLUDE_DIR})
  set(STAR_EXT_GUI_LIBS ${STAR_EXT_GUI_LIBS}
      ${OPENGL_LIBRARY}
      $<IF:$<TARGET_EXISTS:GLEW::glew_s>,GLEW::glew_s,GLEW>
  )

  if(STAR_SYSTEM_MACOS)
    set(STAR_EXT_GUI_LIBS ${STAR_EXT_GUI_LIBS} "-framework CoreAudio")
  endif()

  if(STAR_ENABLE_STEAM_INTEGRATION)
    find_package(SteamApi REQUIRED)
    target_include_directories(star_gui_dependencies SYSTEM INTERFACE ${STEAM_API_INCLUDE_DIR})
    set(STAR_EXT_GUI_LIBS ${STAR_EXT_GUI_LIBS} ${STEAM_API_LIBRARY})
  endif()

  if(STAR_ENABLE_DISCORD_INTEGRATION)
    find_package(DiscordApi REQUIRED)
    set(STAR_EXT_GUI_LIBS ${STAR_EXT_GUI_LIBS} ${DISCORD_API_LIBRARY})
  endif()
endif()

target_link_libraries(star_build_config INTERFACE ${STAR_EXT_LIBS})
if(STAR_BUILD_GUI)
  target_link_libraries(star_gui_dependencies INTERFACE ${STAR_EXT_GUI_LIBS})
endif()

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

