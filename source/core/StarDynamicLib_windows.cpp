#include "StarDynamicLib.hpp"
#include "StarFormat.hpp"
#include "StarString_windows.hpp"

#include <windows.h>

namespace Star {

class PrivateDynLib : public DynamicLib {
public:
  explicit PrivateDynLib(void* handle, bool closeOnDestroy = true)
      : m_handle(handle), m_closeOnDestroy(closeOnDestroy) {}

  ~PrivateDynLib() override {
    if (m_closeOnDestroy)
      FreeLibrary(static_cast<HMODULE>(m_handle));
  }

  void* funcPtr(const char* name) override {
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_handle), name));
  }

private:
  void* m_handle;
  bool m_closeOnDestroy;
};

String DynamicLib::libraryExtension() {
  return ".dll";
}

UniquePtr<DynamicLib> DynamicLib::loadLibrary(String const& libraryName) {
  void* handle = LoadLibraryW(stringToUtf16(libraryName).get());
  if (handle == nullptr)
    return {};
  return make_unique<PrivateDynLib>(handle);
}

UniquePtr<DynamicLib> DynamicLib::currentExecutable() {
  void* handle = GetModuleHandle(0);
  assert(handle);
  return make_unique<PrivateDynLib>(handle, false);
}

}// namespace Star
