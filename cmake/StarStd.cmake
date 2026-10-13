# Use the pinned toolchain's own module sources, without depending on CMake's
# version-specific experimental import-std API. BMIs belong to this build tree.
if(NOT STAR_STD_MODULE_MANIFEST)
  if(DEFINED ENV{XCLANG_ROOT})
    if(XCLANG_TARGET)
      set(std_target "${XCLANG_TARGET}")
    elseif(CMAKE_CXX_COMPILER_TARGET)
      set(std_target "${CMAKE_CXX_COMPILER_TARGET}")
    else()
      execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -dumpmachine
        OUTPUT_VARIABLE std_target OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY)
    endif()
    find_file(STAR_STD_MODULE_MANIFEST NAMES libc++.modules.json
      PATHS "$ENV{XCLANG_ROOT}/${std_target}/usr/lib" "$ENV{XCLANG_ROOT}/${std_target}/lib"
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH REQUIRED)
  else()
    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libc++.modules.json
      OUTPUT_VARIABLE STAR_STD_MODULE_MANIFEST OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY)
  endif()
endif()
if(NOT EXISTS "${STAR_STD_MODULE_MANIFEST}")
  message(FATAL_ERROR "Missing xclang libc++ module manifest: ${STAR_STD_MODULE_MANIFEST}. Use nix develop or set XCLANG_ROOT to the pinned toolchain.")
endif()
file(READ "${STAR_STD_MODULE_MANIFEST}" std_manifest)
string(JSON std_manifest_version GET "${std_manifest}" version)
if(NOT std_manifest_version EQUAL 1)
  message(FATAL_ERROR "Unsupported libc++ module manifest version: ${std_manifest_version}")
endif()
get_filename_component(std_manifest_directory "${STAR_STD_MODULE_MANIFEST}" DIRECTORY)
string(JSON std_module_count LENGTH "${std_manifest}" modules)
math(EXPR std_module_last "${std_module_count} - 1")
foreach(std_module_index RANGE ${std_module_last})
  string(JSON std_module_name GET "${std_manifest}" modules ${std_module_index} logical-name)
  if(std_module_name STREQUAL "std")
    string(JSON std_module_source GET "${std_manifest}" modules ${std_module_index} source-path)
    cmake_path(ABSOLUTE_PATH std_module_source BASE_DIRECTORY "${std_manifest_directory}" NORMALIZE)
    break()
  endif()
endforeach()
if(NOT std_module_source OR NOT EXISTS "${std_module_source}")
  message(FATAL_ERROR "The xclang libc++ manifest does not provide std.cppm")
endif()
get_filename_component(std_module_directory "${std_module_source}" DIRECTORY)
add_library(star_std STATIC)
target_sources(star_std PUBLIC FILE_SET CXX_MODULES
  BASE_DIRS "${std_module_directory}" FILES "${std_module_source}")
# std is a standard-library module, not a project-defined reserved name.
target_compile_options(star_std PRIVATE -Wno-reserved-module-identifier)
target_link_libraries(star_std PRIVATE star_build_config)
set_target_properties(star_std PROPERTIES CXX_SCAN_FOR_MODULES ON)
