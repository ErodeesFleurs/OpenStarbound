#pragma once

#include "StarConfig.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

class Listener {
public:
  virtual ~Listener();
  virtual void trigger() = 0;
};

class CallbackListener : public Listener {
public:
  CallbackListener(std::function<void()> callback);

protected:
  void trigger() override;

private:
  std::function<void()> callback;
};

class TrackerListener : public Listener {
public:
  TrackerListener();

  auto pullTriggered() -> bool;

protected:
  void trigger() override;

private:
  std::atomic<bool> triggered;
};

class ListenerGroup {
public:
  void addListener(WeakPtr<Listener> listener);
  void removeListener(WeakPtr<Listener> listener);
  void clearExpiredListeners();
  void clearAllListeners();

  void trigger();

private:
  Mutex m_mutex;
  std::set<WeakPtr<Listener>, std::owner_less<WeakPtr<Listener>>> m_listeners;
};

inline auto TrackerListener::pullTriggered() -> bool {
  return triggered.exchange(false);
}

inline void TrackerListener::trigger() {
  triggered = true;
}

}
