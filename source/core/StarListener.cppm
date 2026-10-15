module;
import std;
import star.thread;

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
  void addListener(std::weak_ptr<Listener> listener);
  void removeListener(std::weak_ptr<Listener> listener);
  void clearExpiredListeners();
  void clearAllListeners();

  void trigger();

private:
  Mutex m_mutex;
  std::set<std::weak_ptr<Listener>, std::owner_less<std::weak_ptr<Listener>>> m_listeners;
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
  using ::Star::CallbackListener;
  using ::Star::TrackerListener;
  using ::Star::ListenerGroup;
}
