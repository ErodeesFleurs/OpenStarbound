#include "StarDynamicLib.hpp"

#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

namespace Star {

struct PrivateDynLib : public DynamicLib {
  explicit PrivateDynLib(void* handle, bool closeOnDestroy = true)
      : m_handle(handle), m_closeOnDestroy(closeOnDestroy) {}

  ~PrivateDynLib() override {
    if (m_closeOnDestroy)
      dlclose(m_handle);
  }

  void* funcPtr(const char* name) override {
    return dlsym(m_handle, name);
  }

  void* m_handle;
  bool m_closeOnDestroy;
};

String DynamicLib::libraryExtension() {
#ifdef STAR_SYSTEM_MACOS
  return ".dylib";
#else
  return ".so";
#endif
}

UniquePtr<DynamicLib> DynamicLib::loadLibrary(String const& libraryName) {
  void* handle = dlopen(libraryName.utf8Ptr(), RTLD_NOW);
  if (handle == nullptr)
    return {};
  return make_unique<PrivateDynLib>(handle);
}

UniquePtr<DynamicLib> DynamicLib::currentExecutable() {
  void* handle = dlopen(nullptr, 0);
  assert(handle);
  return make_unique<PrivateDynLib>(handle, false);
}

}// namespace Star
