#pragma once

#include "StarConfig.hpp"
#include "StarString.hpp"

namespace Star {

class DynamicLib {
public:
  // Returns the library extension normally used on the current platform
  // including the '.', e.g.  '.dll', '.so', '.dylib'
  static auto libraryExtension() -> String;

  // Load a dll from the given filename.  If the library is found and
  // succesfully loaded, returns a handle to the library, otherwise nullptr.
  static auto loadLibrary(String const& fileName) -> UPtr<DynamicLib>;

  // Load a dll from the given name, minus extension.
  static auto loadLibraryBase(String const& baseName) -> UPtr<DynamicLib>;

  // Should return handle to currently running executable.  Will always
  // succeed.
  static auto currentExecutable() -> UPtr<DynamicLib>;

  virtual ~DynamicLib() = default;

  virtual auto funcPtr(char const* name) -> void* = 0;
};

inline auto DynamicLib::loadLibraryBase(String const& baseName) -> UPtr<DynamicLib> {
  return loadLibrary(baseName + libraryExtension());
}

}
