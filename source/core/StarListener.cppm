module;

#include "StarThread.hpp"

namespace Star {

STAR_CLASS(Listener);
STAR_CLASS(CallbackListener);
STAR_CLASS(TrackerListener);
STAR_CLASS(ListenerGroup);

class Listener {
public:
  virtual ~Listener();
  virtual void trigger() = 0;
};

class CallbackListener : public Listener {
public:
  CallbackListener(function<void()> callback);

protected:
  virtual void trigger() override;

private:
  function<void()> callback;
};

class TrackerListener : public Listener {
public:
  TrackerListener();

  bool pullTriggered();

protected:
  virtual void trigger() override;

private:
  atomic<bool> triggered;
};

class ListenerGroup {
public:
  void addListener(ListenerWeakPtr listener);
  void removeListener(ListenerWeakPtr listener);
  void clearExpiredListeners();
  void clearAllListeners();

  void trigger();

private:
  Mutex m_mutex;
  std::set<ListenerWeakPtr, std::owner_less<ListenerWeakPtr>> m_listeners;
};

inline bool TrackerListener::pullTriggered() {
  return triggered.exchange(false);
}

inline void TrackerListener::trigger() {
  triggered = true;
}

}

export module star.listener;

export namespace Star {
  using ::Star::Listener;
  using ::Star::ListenerPtr;
  using ::Star::ListenerConstPtr;
  using ::Star::ListenerUPtr;
  using ::Star::ListenerConstUPtr;
  using ::Star::ListenerWeakPtr;
  using ::Star::ListenerConstWeakPtr;
  using ::Star::CallbackListener;
  using ::Star::CallbackListenerPtr;
  using ::Star::CallbackListenerConstPtr;
  using ::Star::CallbackListenerUPtr;
  using ::Star::CallbackListenerConstUPtr;
  using ::Star::CallbackListenerWeakPtr;
  using ::Star::CallbackListenerConstWeakPtr;
  using ::Star::TrackerListener;
  using ::Star::TrackerListenerPtr;
  using ::Star::TrackerListenerConstPtr;
  using ::Star::TrackerListenerUPtr;
  using ::Star::TrackerListenerConstUPtr;
  using ::Star::TrackerListenerWeakPtr;
  using ::Star::TrackerListenerConstWeakPtr;
  using ::Star::ListenerGroup;
  using ::Star::ListenerGroupPtr;
  using ::Star::ListenerGroupConstPtr;
  using ::Star::ListenerGroupUPtr;
  using ::Star::ListenerGroupConstUPtr;
  using ::Star::ListenerGroupWeakPtr;
  using ::Star::ListenerGroupConstWeakPtr;
}
