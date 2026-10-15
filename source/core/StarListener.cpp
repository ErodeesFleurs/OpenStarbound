import std;
import star.thread;
import star.listener;

import star.algorithm;
namespace Star {

Listener::~Listener() {}

CallbackListener::CallbackListener(std::function<void()> callback)
  : callback(std::move(callback)) {}

void CallbackListener::trigger() {
  if (callback)
    callback();
}

TrackerListener::TrackerListener() : triggered(false) {}

void ListenerGroup::addListener(std::weak_ptr<Listener> listener) {
  MutexLocker locker(m_mutex);
  m_listeners.insert(std::move(listener));
}

void ListenerGroup::removeListener(std::weak_ptr<Listener> listener) {
  MutexLocker locker(m_mutex);
  m_listeners.erase(std::move(listener));
}

void ListenerGroup::clearExpiredListeners() {
  MutexLocker locker(m_mutex);
  eraseWhere(m_listeners, std::mem_fn(&std::weak_ptr<Listener>::expired));
};

void ListenerGroup::clearAllListeners() {
  MutexLocker locker(m_mutex);
  m_listeners.clear();
}

void ListenerGroup::trigger() {
  MutexLocker locker(m_mutex);
  filter(m_listeners, [](std::weak_ptr<Listener> const& wl) {
      if (auto lock = wl.lock()) {
        lock->trigger();
        return true;
      } else {
        return false;
      }
    });
}

}
