module;

namespace Star {

// Entry-owned native facilities; construct before application objects and keep
// alive through their destruction and the entry point's exception diagnostics.
class NativeRuntime {
public:
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  NativeRuntime();
  ~NativeRuntime();
#else
  NativeRuntime() = default;
  ~NativeRuntime() = default;
#endif

  NativeRuntime(NativeRuntime const&) = delete;
  NativeRuntime& operator=(NativeRuntime const&) = delete;
  NativeRuntime(NativeRuntime&&) = delete;
  NativeRuntime& operator=(NativeRuntime&&) = delete;
};

}

export module star.native_runtime;

export namespace Star {
  using ::Star::NativeRuntime;
}
