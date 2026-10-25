module;
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
import std;
import star.format;


namespace Star {
// The flat ReadP2PPacket export writes this SDK-defined eight-byte data record
// through a pointer, never by value. Its constructors/accessors stay local.
static_assert(std::is_trivially_copyable_v<CSteamID>);
static_assert(std::is_standard_layout_v<CSteamID>);
static_assert(sizeof(CSteamID) == sizeof(uint64));

class SteamDispatch;

// Local registrations are never passed to Steam. Only the manual-dispatch C API
// crosses the SDK boundary; this also works with the MSVC-built Windows DLL.
// Intrusive links borrow live registrations. Cancellation unlinks before the
// registration is destroyed and repairs the pump cursor before user delivery.
class SteamRegistration {
public:
  SteamRegistration(SteamRegistration const&) = delete;
  SteamRegistration& operator=(SteamRegistration const&) = delete;
  ~SteamRegistration();
  void cancel();

protected:
  using Payload = std::variant<std::reference_wrapper<CallbackMsg_t const>,
      std::reference_wrapper<SteamAPICallCompleted_t const>>;
  using Deliver = std::expected<void, StarException>(SteamRegistration&, Payload const&, HSteamPipe);
  SteamRegistration(int callback, Deliver& deliver) : m_callback(callback), m_deliver(deliver) {}
  void attach(SteamDispatch& dispatch, SteamAPICall_t call = k_uAPICallInvalid);

private:
  friend class SteamDispatch;
  std::optional<std::reference_wrapper<SteamDispatch>> m_dispatch;
  std::optional<std::reference_wrapper<SteamRegistration>> m_previous;
  std::optional<std::reference_wrapper<SteamRegistration>> m_next;
  int m_callback;
  SteamAPICall_t m_call = k_uAPICallInvalid;
  std::reference_wrapper<Deliver> m_deliver;
};

// Owned by the platform state (or uploader). Pump and register on the UI thread,
// as with the previous SteamAPI_RunCallbacks loop. Registrations must die first.
class SteamDispatch {
public:
  SteamDispatch() = default;
  SteamDispatch(SteamDispatch const&) = delete;
  SteamDispatch& operator=(SteamDispatch const&) = delete;

  [[nodiscard]] std::expected<void, StarException> initialize() {
    SteamAPI_ManualDispatch_Init();
    m_pipe = SteamAPI_GetHSteamPipe();
    if (!m_pipe)
      return std::unexpected(StarException("Steam manual dispatch has no active pipe"));
    return {};
  }

  [[nodiscard]] std::expected<void, StarException> runCallbacks() {
    if (!m_pipe)
      return std::unexpected(StarException("Steam manual dispatch is not initialized"));
    if (m_pumping)
      return {};
    m_pumping = true;
    struct PumpGuard {
      SteamDispatch& dispatch;
      ~PumpGuard() {
        dispatch.m_cursor.reset();
        dispatch.m_pumping = false;
        SteamAPI_ReleaseCurrentThreadMemory();
      }
    } pumpGuard{*this};
    SteamAPI_ManualDispatch_RunFrame(m_pipe);
    CallbackMsg_t callback;
    while (SteamAPI_ManualDispatch_GetNextCallback(m_pipe, &callback)) {
      struct CallbackGuard {
        HSteamPipe pipe;
        ~CallbackGuard() { SteamAPI_ManualDispatch_FreeLastCallback(pipe); }
      } callbackGuard{m_pipe};

      SteamAPICallCompleted_t completed{};
      bool callResult = callback.m_iCallback == SteamAPICallCompleted_t::k_iCallback;
      if (callResult) {
        if (callback.m_cubParam != sizeof(completed))
          return std::unexpected(StarException("Invalid Steam asynchronous completion payload size"));
        std::memcpy(&completed, callback.m_pubParam, sizeof(completed));
      }

      // Advance before delivery. Removal of the next registration updates the
      // cursor, so callbacks may cancel themselves or another registration.
      m_cursor = m_first;
      while (m_cursor) {
        auto& registration = m_cursor->get();
        m_cursor = registration.m_next;
        std::expected<void, StarException> delivered;
        if (callResult) {
          if (registration.m_call == completed.m_hAsyncCall
              && registration.m_callback == completed.m_iCallback)
            delivered = registration.m_deliver.get()(registration, std::cref(completed), m_pipe);
        } else if (registration.m_call == k_uAPICallInvalid
                   && registration.m_callback == callback.m_iCallback) {
          delivered = registration.m_deliver.get()(registration, std::cref(callback), m_pipe);
        }
        if (!delivered)
          return std::unexpected(std::move(delivered.error()));
      }
    }
    return {};
  }

private:
  friend class SteamRegistration;
  HSteamPipe m_pipe = 0;
  std::optional<std::reference_wrapper<SteamRegistration>> m_first;
  std::optional<std::reference_wrapper<SteamRegistration>> m_cursor;
  bool m_pumping = false;
};

inline SteamRegistration::~SteamRegistration() {
  cancel();
}

inline void SteamRegistration::cancel() {
  if (!m_dispatch)
    return;
  auto& dispatch = m_dispatch->get();
  if (dispatch.m_cursor && std::addressof(dispatch.m_cursor->get()) == this)
    dispatch.m_cursor = m_next;
  if (m_previous)
    m_previous->get().m_next = m_next;
  else
    dispatch.m_first = m_next;
  if (m_next)
    m_next->get().m_previous = m_previous;
  m_dispatch.reset();
  m_previous.reset();
  m_next.reset();
  m_call = k_uAPICallInvalid;
}

inline void SteamRegistration::attach(SteamDispatch& dispatch, SteamAPICall_t call) {
  cancel();
  m_dispatch = std::ref(dispatch);
  m_call = call;
  m_next = dispatch.m_first;
  if (m_next)
    m_next->get().m_previous = std::ref(*this);
  dispatch.m_first = std::ref(*this);
}

// These adapters borrow their fixed-address receiver until cancellation.
// Business handlers receive a lexical reference to a copied SDK event; only
// manual-dispatch payload acquisition and copying use native buffer pointers.
template <typename Object, typename Event>
class SteamCallback : public SteamRegistration {
public:
  using Method = std::expected<void, StarException> (Object::*)(Event&);
  SteamCallback(SteamDispatch& dispatch, Object& object, Method method)
      : SteamRegistration(Event::k_iCallback, deliver), m_object(object), m_method(method) {
    static_assert(std::is_trivially_copyable_v<Event>);
    attach(dispatch);
  }

private:
  [[nodiscard]] static std::expected<void, StarException> deliver(SteamRegistration& registration, Payload const& payload, HSteamPipe) {
    auto& self = static_cast<SteamCallback&>(registration);
    auto const& callback = std::get<std::reference_wrapper<CallbackMsg_t const>>(payload).get();
    if (callback.m_cubParam != sizeof(Event))
      return std::unexpected(StarException("Invalid Steam callback payload size"));
    Event event;
    std::memcpy(&event, callback.m_pubParam, sizeof(event));
    return (self.m_object.get().*self.m_method)(event);
  }
  std::reference_wrapper<Object> m_object;
  Method m_method;
};

template <typename Object, typename Event>
class SteamCallResult : public SteamRegistration {
public:
  using Method = std::expected<void, StarException> (Object::*)(Event&, bool);
  SteamCallResult(SteamDispatch& dispatch, SteamAPICall_t call, Object& object, Method method)
      : SteamRegistration(Event::k_iCallback, deliver), m_object(object), m_method(method) {
    static_assert(std::is_trivially_copyable_v<Event>);
    if (call != k_uAPICallInvalid)
      attach(dispatch, call);
  }

private:
  [[nodiscard]] static std::expected<void, StarException> deliver(SteamRegistration& registration, Payload const& payload, HSteamPipe pipe) {
    auto& self = static_cast<SteamCallResult&>(registration);
    auto const& completed = std::get<std::reference_wrapper<SteamAPICallCompleted_t const>>(payload).get();
    auto object = self.m_object;
    auto method = self.m_method;
    if (completed.m_cubParam != sizeof(Event))
      return std::unexpected(StarException("Invalid Steam call result payload size"));
    Event event{};
    bool failed = false;
    if (!SteamAPI_ManualDispatch_GetAPICallResult(pipe, completed.m_hAsyncCall,
                                                  &event, sizeof(event), Event::k_iCallback, &failed))
      failed = true;
    self.cancel();// One-shot only after receiving a correctly sized completion.
    return (object.get().*method)(event, failed);
  }
  std::reference_wrapper<Object> m_object;
  Method m_method;
};

}// namespace Star


export module star.steam_dispatch;

export namespace Star {
  using ::Star::SteamRegistration;
  using ::Star::SteamDispatch;
  using ::Star::SteamCallback;
  using ::Star::SteamCallResult;
}
