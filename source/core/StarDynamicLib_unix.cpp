#include "StarConfig.hpp"

#include "StarDynamicLib.hpp"

#include <dlfcn.h>
#include <pthread.h>
#include <sys/time.h>

import std;

namespace Star {

struct PrivateDynLib : public DynamicLib {
  PrivateDynLib(void* handle)
      : m_handle(handle) {}

  ~PrivateDynLib() override {
    dlclose(m_handle);
  }

  auto funcPtr(const char* name) -> void* override {
    return dlsym(m_handle, name);
  }

  void* m_handle;
};

auto DynamicLib::libraryExtension() -> String {
#ifdef STAR_SYSTEM_MACOS
  return ".dylib";
#else
  return ".so";
#endif
}

auto DynamicLib::loadLibrary(String const& libraryName) -> UPtr<DynamicLib> {
  void* handle = dlopen(libraryName.utf8Ptr(), RTLD_NOW);
  if (handle == nullptr)
    return {};
  return std::make_unique<PrivateDynLib>(handle);
}

auto DynamicLib::currentExecutable() -> UPtr<DynamicLib> {
  void* handle = dlopen(nullptr, 0);
  return std::make_unique<PrivateDynLib>(handle);
}

}// namespace Star
