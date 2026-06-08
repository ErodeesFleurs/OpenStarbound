#pragma once

#include "StarString.hpp"

namespace Star {

class DynamicLib;

class DynamicLib {
public:
  // Returns the library extension normally used on the current platform
  // including the '.', e.g.  '.dll', '.so', '.dylib'
  static String libraryExtension();

  // Load a dll from the given filename.  If the library is found and
  // succesfully loaded, returns a handle to the library, otherwise nullptr.
  static UniquePtr<DynamicLib> loadLibrary(String const& fileName);

  // Load a dll from the given name, minus extension.
  static UniquePtr<DynamicLib> loadLibraryBase(String const& baseName);

  // Should return handle to currently running executable.  Will always
  // succeed.
  static UniquePtr<DynamicLib> currentExecutable();

  virtual ~DynamicLib() = default;

  virtual void* funcPtr(char const* name) = 0;
};

inline UniquePtr<DynamicLib> DynamicLib::loadLibraryBase(String const& baseName) {
  return loadLibrary(baseName + libraryExtension());
}

}
