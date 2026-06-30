#pragma once

#include "StarString.hpp"

namespace Star {

class DynamicLib;

class DynamicLib {
public:
  // Returns the library extension normally used on the current platform
  // including the '.', e.g.  '.dll', '.so', '.dylib'
  [[nodiscard]] static String libraryExtension();

  // Load a dll from the given filename.  If the library is found and
  // successfully loaded, returns a handle to the library, otherwise nullptr.
  [[nodiscard]] static UniquePtr<DynamicLib> loadLibrary(String const& fileName);

  // Load a dll from the given name, minus extension.
  [[nodiscard]] static UniquePtr<DynamicLib> loadLibraryBase(String const& baseName);

  // Should return handle to currently running executable.  Will always
  // succeed.
  [[nodiscard]] static UniquePtr<DynamicLib> currentExecutable();

  virtual ~DynamicLib() = default;

  [[nodiscard]] virtual void* funcPtr(char const* name) = 0;
};

inline UniquePtr<DynamicLib> DynamicLib::loadLibraryBase(String const& baseName) {
  return loadLibrary(baseName + libraryExtension());
}

}// namespace Star
