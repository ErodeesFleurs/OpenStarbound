cmake_minimum_required(VERSION 4.1.2)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

project(openstarbound LANGUAGES C CXX)

# cache and global build options
find_program(LAUNCHER_PATH NAMES sccache ccache)
if(LAUNCHER_PATH)
    set(CMAKE_C_COMPILER_LAUNCHER ${LAUNCHER_PATH})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${LAUNCHER_PATH})
    message(STATUS "Using Launcher: ${LAUNCHER_PATH}")
    set(STAR_CCACHE TRUE)
    set(STAR_PRECOMPILED_HEADERS FALSE)
else()
    set(STAR_CCACHE FALSE)
    set(STAR_PRECOMPILED_HEADERS TRUE)
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake)

# Define build types
set(CMAKE_CONFIGURATION_TYPES
    Debug RelWithAsserts RelWithDebInfo Release CACHE STRING "" FORCE)

# Set output directory for all build types to ../dist
set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/../dist")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
foreach(cfg IN ITEMS DEBUG RELWITHASSERTS RELWITHDEBINFO RELEASE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} ${OUTPUT_DIR})
endforeach()

# Determine system type and endianness

if(APPLE)
    set(STAR_SYSTEM "macos")
elseif(WIN32)
    set(STAR_SYSTEM "windows")
elseif(UNIX)
    string(TOLOWER ${CMAKE_SYSTEM_NAME} STAR_SYSTEM)
endif()

include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    add_compile_definitions(STAR_BIG_ENDIAN)
else()
    add_compile_definitions(STAR_LITTLE_ENDIAN)
endif()

# Common compile options

if(MSVC)
    add_compile_options(/nologo /MP /EHsc /bigobj /DNOMINMAX
            /wd4996 /wd4244 /wd4267 /wd4351 /wd4503 /wd4624
    )
    add_definitions(UNICODE _UNICODE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /largeaddressaware
        /OPT:REF /OPT:ICF /PDBCompress")

    set(CMAKE_CXX_FLAGS_RELEASE "/Ox /fp:fast /GA /GS- /Gy /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHASSERTS "${CMAKE_CXX_FLAGS_RELEASE} /Z7")
else()
    add_compile_options(-stdlib=libstdc++)
    add_compile_options(-Wall -Wextra -Wno-unused -Wno-implicit-fallthrough
        -Wunused-command-line-argument)
    add_link_options(-stdlib=libstdc++)
    add_link_options(-rdynamic)
    if(NOT APPLE)
        add_compile_options(-pthread)
        add_link_options(-pthread)
    endif()

    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -ffast-math")
    set(CMAKE_CXX_FLAGS_RELWITHASSERTS "-g -O3 -ffast-math")
endif()

# External libraries
find_package(PkgConfig REQUIRED)

macro(smart_find_package NAME PKG)
    find_package(${NAME} QUIET)
    if(NOT ${NAME}_FOUND)
        pkg_check_modules(${NAME} REQUIRED IMPORTED_TARGET GLOBAL ${PKG})
        if(NOT TARGET ${NAME}::${NAME})
            add_library(${NAME}::${NAME} INTERFACE IMPORTED)
            target_link_libraries(${NAME}::${NAME} INTERFACE PkgConfig::${NAME})
        endif()
    endif()
endmacro()

set(STAR_EXT_LIBS "")
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Freetype REQUIRED)
find_package(cpr REQUIRED)
find_package(OggVorbis REQUIRED)
find_package(zstd REQUIRED)
find_package(re2 REQUIRED)
find_package(imgui REQUIRED)

list(APPEND STAR_EXT_LIBS
    ZLIB::ZLIB PNG::PNG cpr::cpr re2::re2
    imgui::imgui ${VORBIS_LIBRARIES} ${OGG_LIBRARIES}
    zstd::libzstd_shared
    Freetype::Freetype
)

smart_find_package(Opus opus)
smart_find_package(Vorbis vorbis)
smart_find_package(VorbisFile vorbisfile)

list(APPEND STAR_EXT_LIBS Opus::Opus Vorbis::Vorbis VorbisFile::VorbisFile)

option(STAR_USE_JEMALLOC "Use jemalloc" OFF)
option(STAR_USE_MIMALLOC "Use mimalloc" OFF)
if(STAR_USE_JEMALLOC)
    smart_find_package(JEMALLOC jemalloc)
    list(APPEND STAR_EXT_LIBS JEMALLOC::JEMALLOC)
    add_compile_definitions(STAR_USE_JEMALLOC)
elseif(STAR_USE_MIMALLOC)
    find_package(mimalloc CONFIG REQUIRED)
    list(APPEND STAR_EXT_LIBS mimalloc)
    add_compile_definitions(STAR_USE_MIMALLOC)
endif()

# GUI build options

option(STAR_BUILD_GUI "Build GUI utilities and Client" ON)
if(STAR_BUILD_GUI)
    find_package(SDL3 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    list(APPEND STAR_EXT_GUI_LIBS
        SDL3::SDL3 OpenGL::GL GLEW
        $<TARGET_NAME_IF_EXISTS:SDL3::SDL3main>
    )

    if(APPLE)
        list(APPEND STAR_EXT_GUI_LIBS "-framework CoreAudio")
    endif()

    option(STAR_ENABLE_STEAM_INTEGRATION "Steam Support" OFF)
    if(STAR_ENABLE_STEAM_INTEGRATION)
        find_package(SteamApi REQUIRED)
        list(APPEND STAR_EXT_GUI_LIBS ${STEAM_API_LIBRARY})
        add_compile_definitions(STAR_ENABLE_STEAM_INTEGRATION)
    endif()
endif()

# Include all the relevant source directories, based on build settings...
set(BUILD_SHARED_LIBS OFF)

set(STAR_CORE_INCLUDES ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/core/scripting
)
set(STAR_BASE_INCLUDES
    ${PROJECT_SOURCE_DIR}/base
    ${PROJECT_SOURCE_DIR}/base/scripting
)
set(STAR_GAME_INCLUDES
    ${PROJECT_SOURCE_DIR}/game
    ${PROJECT_SOURCE_DIR}/game/interfaces
    ${PROJECT_SOURCE_DIR}/game/items
)

add_subdirectory(extern)
add_subdirectory(core)
add_subdirectory(base)
add_subdirectory(platform)
add_subdirectory(game)
add_subdirectory(server)
add_subdirectory(utility)

if(STAR_BUILD_GUI)
    add_subdirectory(application)
    add_subdirectory(rendering)
    add_subdirectory(windowing)
    add_subdirectory(frontend)
    add_subdirectory(client)
endif()

option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
