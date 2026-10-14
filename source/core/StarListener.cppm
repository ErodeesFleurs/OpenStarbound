module;
import std;
import star.thread;

namespace Star {

class Listener;
using ListenerPtr = std::shared_ptr<Listener>;
using ListenerConstPtr = std::shared_ptr<const Listener>;
using ListenerWeakPtr = std::weak_ptr<Listener>;
using ListenerConstWeakPtr = std::weak_ptr<const Listener>;
using ListenerUPtr = std::unique_ptr<Listener>;
using ListenerConstUPtr = std::unique_ptr<const Listener>;
class CallbackListener;
using CallbackListenerPtr = std::shared_ptr<CallbackListener>;
using CallbackListenerConstPtr = std::shared_ptr<const CallbackListener>;
using CallbackListenerWeakPtr = std::weak_ptr<CallbackListener>;
using CallbackListenerConstWeakPtr = std::weak_ptr<const CallbackListener>;
using CallbackListenerUPtr = std::unique_ptr<CallbackListener>;
using CallbackListenerConstUPtr = std::unique_ptr<const CallbackListener>;
class TrackerListener;
using TrackerListenerPtr = std::shared_ptr<TrackerListener>;
using TrackerListenerConstPtr = std::shared_ptr<const TrackerListener>;
using TrackerListenerWeakPtr = std::weak_ptr<TrackerListener>;
using TrackerListenerConstWeakPtr = std::weak_ptr<const TrackerListener>;
using TrackerListenerUPtr = std::unique_ptr<TrackerListener>;
using TrackerListenerConstUPtr = std::unique_ptr<const TrackerListener>;
class ListenerGroup;
using ListenerGroupPtr = std::shared_ptr<ListenerGroup>;
using ListenerGroupConstPtr = std::shared_ptr<const ListenerGroup>;
using ListenerGroupWeakPtr = std::weak_ptr<ListenerGroup>;
using ListenerGroupConstWeakPtr = std::weak_ptr<const ListenerGroup>;
using ListenerGroupUPtr = std::unique_ptr<ListenerGroup>;
using ListenerGroupConstUPtr = std::unique_ptr<const ListenerGroup>;

class Listener {
public:
  virtual ~Listener();
  virtual void trigger() = 0;
};

class CallbackListener : public Listener {
public:
  CallbackListener(std::function<void()> callback);

protected:
  virtual void trigger() override;

private:
  std::function<void()> callback;
};

class TrackerListener : public Listener {
public:
  TrackerListener();

  bool pullTriggered();

protected:
  virtual void trigger() override;

private:
  std::atomic<bool> triggered;
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
