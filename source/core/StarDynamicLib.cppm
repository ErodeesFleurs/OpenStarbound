module;

#ifdef STAR_SYSTEM_FAMILY_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#include <stddef.h>
#endif
import std;
import star.string;
import star.format;

#ifdef STAR_SYSTEM_FAMILY_WINDOWS
import star.string_windows;
#endif

export module star.dynamic_lib;

export namespace Star {

class DynamicLib {
public:
  // Returns the library extension normally used on the current platform
  // including the '.', e.g.  '.dll', '.so', '.dylib'
  static String libraryExtension();

  // Load a dll from the given filename.  If the library is found and
  // succesfully loaded, returns a handle to the library, otherwise nullptr.
  static std::unique_ptr<DynamicLib> loadLibrary(String const& fileName);

  // Load a dll from the given name, minus extension.
  static std::unique_ptr<DynamicLib> loadLibraryBase(String const& baseName);

  // Should return handle to currently running executable.  Will always
  // succeed.
  static std::unique_ptr<DynamicLib> currentExecutable();

  virtual ~DynamicLib() = default;

  virtual void* funcPtr(char const* name) = 0;
};

inline std::unique_ptr<DynamicLib> DynamicLib::loadLibraryBase(String const& baseName) {
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

std::unique_ptr<DynamicLib> DynamicLib::loadLibrary(String const& libraryName) {
  void* handle = LoadLibraryW(stringToUtf16(libraryName).get());
  if (handle == NULL)
    return {};
  return std::make_unique<PrivateDynLib>(handle);
}

std::unique_ptr<DynamicLib> DynamicLib::currentExecutable() {
  void* handle = GetModuleHandle(0);
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((handle));
  }
  return std::make_unique<PrivateDynLib>(handle);
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

std::unique_ptr<DynamicLib> DynamicLib::loadLibrary(String const& libraryName) {
  void* handle = dlopen(libraryName.utf8Ptr(), RTLD_NOW);
  if (handle == NULL)
    return {};
  return std::make_unique<PrivateDynLib>(handle);
}

std::unique_ptr<DynamicLib> DynamicLib::currentExecutable() {
  void* handle = dlopen(NULL, 0);
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((handle));
  }
  return std::make_unique<PrivateDynLib>(handle);
}

#endif

}
