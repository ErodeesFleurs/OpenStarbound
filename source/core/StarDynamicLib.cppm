module;

#include "StarString.hpp"

#ifdef STAR_SYSTEM_FAMILY_WINDOWS
#include "StarFormat.hpp"
#include "StarString_windows.hpp"
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

export module star.dynamic_lib;

export namespace Star {

STAR_CLASS(DynamicLib);

class DynamicLib {
public:
  // Returns the library extension normally used on the current platform
  // including the '.', e.g.  '.dll', '.so', '.dylib'
  static String libraryExtension();

  // Load a dll from the given filename.  If the library is found and
  // succesfully loaded, returns a handle to the library, otherwise nullptr.
  static DynamicLibUPtr loadLibrary(String const& fileName);

  // Load a dll from the given name, minus extension.
  static DynamicLibUPtr loadLibraryBase(String const& baseName);

  // Should return handle to currently running executable.  Will always
  // succeed.
  static DynamicLibUPtr currentExecutable();

  virtual ~DynamicLib() = default;

  virtual void* funcPtr(char const* name) = 0;
};

inline DynamicLibUPtr DynamicLib::loadLibraryBase(String const& baseName) {
  return loadLibrary(baseName + libraryExtension());
}

}

namespace Star {

#ifdef STAR_SYSTEM_FAMILY_WINDOWS

class PrivateDynLib : public DynamicLib {
public:
  PrivateDynLib(void* handle)
    : m_handle(handle) {}

  ~PrivateDynLib() {
    FreeLibrary((HMODULE)m_handle);
  }

  void* funcPtr(const char* name) {
    return (void*)GetProcAddress((HMODULE)m_handle, name);
  }

private:
  void* m_handle;
};

String DynamicLib::libraryExtension() {
  return ".dll";
}

DynamicLibUPtr DynamicLib::loadLibrary(String const& libraryName) {
  void* handle = LoadLibraryW(stringToUtf16(libraryName).get());
  if (handle == NULL)
    return {};
  return make_unique<PrivateDynLib>(handle);
}

DynamicLibUPtr DynamicLib::currentExecutable() {
  void* handle = GetModuleHandle(0);
  starAssert(handle);
  return make_unique<PrivateDynLib>(handle);
}

#else

struct PrivateDynLib : public DynamicLib {
  PrivateDynLib(void* handle)
    : m_handle(handle) {}

  ~PrivateDynLib() {
    dlclose(m_handle);
  }

  void* funcPtr(const char* name) {
    return dlsym(m_handle, name);
  }

  void* m_handle;
};

String DynamicLib::libraryExtension() {
#ifdef STAR_SYSTEM_MACOS
  return ".dylib";
#else
  return ".so";
#endif
}

DynamicLibUPtr DynamicLib::loadLibrary(String const& libraryName) {
  void* handle = dlopen(libraryName.utf8Ptr(), RTLD_NOW);
  if (handle == NULL)
    return {};
  return make_unique<PrivateDynLib>(handle);
}

DynamicLibUPtr DynamicLib::currentExecutable() {
  void* handle = dlopen(NULL, 0);
  starAssert(handle);
  return make_unique<PrivateDynLib>(handle);
}

#endif

}
