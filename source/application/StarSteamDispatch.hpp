#pragma once

import std;

#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"

namespace Star {
// The flat ReadP2PPacket export writes this SDK-defined eight-byte data record
// through a pointer, never by value. Its constructors/accessors stay local.
static_assert(std::is_trivially_copyable_v<CSteamID>);
static_assert(std::is_standard_layout_v<CSteamID>);
static_assert(sizeof(CSteamID) == sizeof(uint64));

class SteamDispatch;

// Local registrations are never passed to Steam. Only the manual-dispatch C API
// crosses the SDK boundary; this also works with the MSVC-built Windows DLL.
class SteamRegistration {
public:
  SteamRegistration(SteamRegistration const&) = delete;
  SteamRegistration& operator=(SteamRegistration const&) = delete;
  ~SteamRegistration();
  void cancel();

protected:
  using Deliver = void (*)(SteamRegistration&, void const*, HSteamPipe);
  SteamRegistration(int callback, Deliver deliver) : m_callback(callback), m_deliver(deliver) {}
  void attach(SteamDispatch& dispatch, SteamAPICall_t call = k_uAPICallInvalid);

private:
  friend class SteamDispatch;
  SteamDispatch* m_dispatch = nullptr;
  SteamRegistration* m_previous = nullptr;
  SteamRegistration* m_next = nullptr;
  int m_callback;
  SteamAPICall_t m_call = k_uAPICallInvalid;
  Deliver m_deliver;
};

// Owned by the platform state (or uploader). Pump and register on the UI thread,
// as with the previous SteamAPI_RunCallbacks loop. Registrations must die first.
class SteamDispatch {
public:
  SteamDispatch() = default;
  SteamDispatch(SteamDispatch const&) = delete;
  SteamDispatch& operator=(SteamDispatch const&) = delete;

  void initialize() {
    SteamAPI_ManualDispatch_Init();
    m_pipe = SteamAPI_GetHSteamPipe();
  }

  void runCallbacks() {
    if (!m_pipe || m_pumping)
      return;
    m_pumping = true;
    struct PumpGuard {
      SteamDispatch& dispatch;
      ~PumpGuard() {
        dispatch.m_cursor = nullptr;
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
          continue;
        std::memcpy(&completed, callback.m_pubParam, sizeof(completed));
      }

      // Advance before delivery. Removal of the next registration updates the
      // cursor, so callbacks may cancel themselves or another registration.
      m_cursor = m_first;
      while (m_cursor) {
        auto registration = m_cursor;
        m_cursor = registration->m_next;
        if (callResult) {
          if (registration->m_call == completed.m_hAsyncCall
              && registration->m_callback == completed.m_iCallback)
            registration->m_deliver(*registration, &completed, m_pipe);
        } else if (registration->m_call == k_uAPICallInvalid
                   && registration->m_callback == callback.m_iCallback) {
          registration->m_deliver(*registration, &callback, m_pipe);
        }
      }
    }
  }

private:
  friend class SteamRegistration;
  HSteamPipe m_pipe = 0;
  SteamRegistration* m_first = nullptr;
  SteamRegistration* m_cursor = nullptr;
  bool m_pumping = false;
};

inline SteamRegistration::~SteamRegistration() {
  cancel();
}

inline void SteamRegistration::cancel() {
  if (!m_dispatch)
    return;
  if (m_dispatch->m_cursor == this)
    m_dispatch->m_cursor = m_next;
  if (m_previous)
    m_previous->m_next = m_next;
  else
    m_dispatch->m_first = m_next;
  if (m_next)
    m_next->m_previous = m_previous;
  m_dispatch = nullptr;
  m_previous = nullptr;
  m_next = nullptr;
  m_call = k_uAPICallInvalid;
}

inline void SteamRegistration::attach(SteamDispatch& dispatch, SteamAPICall_t call) {
  cancel();
  m_dispatch = &dispatch;
  m_call = call;
  m_next = dispatch.m_first;
  if (m_next)
    m_next->m_previous = this;
  dispatch.m_first = this;
}

template <typename Object, typename Event>
class SteamCallback : public SteamRegistration {
public:
  using Method = void (Object::*)(Event*);
  SteamCallback(SteamDispatch& dispatch, Object* object, Method method)
      : SteamRegistration(Event::k_iCallback, &deliver), m_object(object), m_method(method) {
    static_assert(std::is_trivially_copyable_v<Event>);
    attach(dispatch);
  }

private:
  static void deliver(SteamRegistration& registration, void const* payload, HSteamPipe) {
    auto& self = static_cast<SteamCallback&>(registration);
    auto const& callback = *static_cast<CallbackMsg_t const*>(payload);
    if (callback.m_cubParam != sizeof(Event))
      return;
    Event event;
    std::memcpy(&event, callback.m_pubParam, sizeof(event));
    (self.m_object->*self.m_method)(&event);
  }
  Object* m_object;
  Method m_method;
};

template <typename Object, typename Event>
class SteamCallResult : public SteamRegistration {
public:
  using Method = void (Object::*)(Event*, bool);
  SteamCallResult(SteamDispatch& dispatch, SteamAPICall_t call, Object* object, Method method)
      : SteamRegistration(Event::k_iCallback, &deliver), m_object(object), m_method(method) {
    static_assert(std::is_trivially_copyable_v<Event>);
    if (call != k_uAPICallInvalid)
      attach(dispatch, call);
  }

private:
  static void deliver(SteamRegistration& registration, void const* payload, HSteamPipe pipe) {
    auto& self = static_cast<SteamCallResult&>(registration);
    auto const& completed = *static_cast<SteamAPICallCompleted_t const*>(payload);
    if (completed.m_cubParam != sizeof(Event))
      return;
    Event event{};
    bool failed = false;
    if (!SteamAPI_ManualDispatch_GetAPICallResult(pipe, completed.m_hAsyncCall,
                                                  &event, sizeof(event), Event::k_iCallback, &failed))
      return;
    auto object = self.m_object;
    auto method = self.m_method;
    self.cancel();// One-shot, including if the handler starts another call.
    (object->*method)(&event, failed);
  }
  Object* m_object;
  Method m_method;
};

}// namespace Star
