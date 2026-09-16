# nixpkgs' opus ships opus.pc but no CMake package config, while upstream's
# source/CMakeLists.txt does `find_package(Opus CONFIG REQUIRED)` and links the
# `Opus::opus` target.  This shim supplies the missing config, so the
# repository's own CMake files can be configured unmodified -- both in the Nix
# sandbox and in `nix develop`.
include(CMakeFindDependencyMacro)
find_dependency(PkgConfig)
pkg_check_modules(OPUS REQUIRED IMPORTED_TARGET GLOBAL opus)

if(NOT TARGET Opus::opus)
  add_library(Opus::opus INTERFACE IMPORTED)
  set_target_properties(Opus::opus PROPERTIES INTERFACE_LINK_LIBRARIES PkgConfig::OPUS)
endif()

set(Opus_FOUND TRUE)
