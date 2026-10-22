module;
import std;
import star.format;

namespace Star {

// Entry-owned native facilities; construct before application objects and keep
// alive through their destruction and the entry point's error diagnostics.
class NativeRuntime {
public:
  [[nodiscard]] static std::expected<std::unique_ptr<NativeRuntime>, StarException> create();
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  ~NativeRuntime();
#else
  ~NativeRuntime() = default;
#endif

  NativeRuntime(NativeRuntime const&) = delete;
  NativeRuntime& operator=(NativeRuntime const&) = delete;
  NativeRuntime(NativeRuntime&&) = delete;
  NativeRuntime& operator=(NativeRuntime&&) = delete;

private:
  NativeRuntime() = default;
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  bool m_winsockInitialized = false;
  bool m_diagnosticsInitialized = false;
#endif
};

}

#ifndef STAR_SYSTEM_FAMILY_WINDOWS
[[nodiscard]] inline std::expected<std::unique_ptr<Star::NativeRuntime>, Star::StarException> Star::NativeRuntime::create() {
  return std::unique_ptr<NativeRuntime>(new NativeRuntime);
}
#endif

export module star.native_runtime;

export namespace Star {
  using ::Star::NativeRuntime;
}
