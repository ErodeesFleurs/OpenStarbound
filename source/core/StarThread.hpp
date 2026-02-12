#pragma once

import std;

namespace Star {

template <typename Return>
class ThreadFunction;

class Thread {
public:
  // Implementations of this method should sleep for at least the given amount
  // of time, but may sleep for longer due to scheduling.
  static void sleep(unsigned millis) {
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
  }

  // Sleep a more precise amount of time, but uses more resources to do so.
  // Should be less likely to sleep much longer than the given amount of time.
  static void sleepPrecise(unsigned millis);

  // Yield this thread, offering the opportunity to reschedule.
  static void yield() {
    std::this_thread::yield();
  }

  static auto numberOfProcessors() -> unsigned {
    unsigned n = std::thread::hardware_concurrency();
    return n > 0 ? n : 1;
  }

  template <typename Function, typename... Args>
  static auto invoke(String const& name, Function&& f, Args&&... args) -> ThreadFunction<std::invoke_result_t<Function, Args...>>;

  Thread(String const& name);
  Thread(Thread&&) noexcept;
  // Will not automatically join!  ALL implementations of this class MUST call
  // join() in their most derived constructors, or not rely on the destructor
  // joining.
  virtual ~Thread();

  auto operator=(Thread&&) noexcept -> Thread&;

  // Start a thread that is currently in the joined state.  Returns true if the
  // thread was joined and is now started, false if the thread was not joined.
  auto start() -> bool;

  // Wait for a thread to finish and re-join with the thread, on completion
  // isJoined() will be true.  Returns true if the thread was joinable, and is
  // now joined, false if the thread was already joined.
  auto join() -> bool;

  // Returns false when this thread has been started without being joined.
  [[nodiscard]] auto isJoined() const -> bool;

  // Returns false before start() has been called, true immediately after
  // start() has been called, and false once the run() method returns.
  [[nodiscard]] auto isRunning() const -> bool;

  [[nodiscard]] auto name() const -> String;

protected:
  virtual void run() = 0;

private:
  struct ThreadImpl;
  std::unique_ptr<ThreadImpl> m_impl;
};

// Wraps a function call and calls in another thread, very nice lightweight
// one-shot alternative to deriving from Thread.  Handles exceptions in a
// different way from Thread, instead of logging the exception, the exception
// is forwarded and re-thrown during the call to finish().
template <>
class ThreadFunction<void> {
public:
  ThreadFunction();
  ThreadFunction(ThreadFunction&&) noexcept;

  // Automatically starts the given function, ThreadFunction can also be
  // constructed with Thread::invoke, which is a shorthand.
  ThreadFunction(std::function<void()> function, String const& name);

  // Automatically calls finish, though BEWARE that often times this is quite
  // dangerous, and this is here mostly as a fallback.  The natural destructor
  // order for members of a class is often wrong, and if the function throws,
  // since this destructor calls finish it will throw.
  ~ThreadFunction();

  auto operator=(ThreadFunction&&) noexcept -> ThreadFunction&;

  // Waits on function finish if function is assigned and started, otherwise
  // does nothing.  If the function threw an exception, it will be re-thrown
  // here (on the first call to finish() only).
  void finish();

  // Returns whether the ThreadFunction::finish method been called and the
  // ThreadFunction has stopped.  Also returns true when the ThreadFunction has
  // been default constructed.
  [[nodiscard]] auto isFinished() const -> bool;
  // Returns false if the thread function has stopped running, whether or not
  // finish() has been called.
  [[nodiscard]] auto isRunning() const -> bool;

  // Equivalent to !isFinished()
  explicit operator bool() const;

  [[nodiscard]] auto name() const -> String;

private:
  struct ThreadFunctionImpl;
  std::unique_ptr<ThreadFunctionImpl> m_impl;
};

template <typename Return>
class ThreadFunction {
public:
  ThreadFunction() = default;
  ThreadFunction(ThreadFunction&&) noexcept = default;
  ThreadFunction(std::function<Return()> function, String const& name);

  ~ThreadFunction();

  auto operator=(ThreadFunction&&) noexcept -> ThreadFunction& = default;

  // Finishes the thread, moving and returning the final value of the function.
  // If the function threw an exception, finish() will rethrow that exception.
  // May only be called once, otherwise will throw runtime_error.
  auto finish() -> Return;

  [[nodiscard]] auto isFinished() const -> bool;
  [[nodiscard]] auto isRunning() const -> bool;

  explicit operator bool() const;

  [[nodiscard]] auto name() const -> String;

private:
  ThreadFunction<void> m_function;
  std::shared_ptr<std::optional<Return>> m_return;
};

// *Non* recursive mutex lock, for use with ConditionVariable
class Mutex {
public:
  Mutex() : m_mutex(std::make_unique<std::mutex>()) {}
  Mutex(Mutex const&) = delete;
  Mutex(Mutex&&) noexcept = default;
  ~Mutex() = default;

  auto operator=(Mutex const&) -> Mutex& = delete;
  auto operator=(Mutex&&) noexcept -> Mutex& = default;

  void lock() { m_mutex->lock(); }

  // Attempt to acquire the mutex without blocking.
  auto tryLock() -> bool { return m_mutex->try_lock(); }

  void unlock() { m_mutex->unlock(); }

private:
  friend class ConditionVariable;
  std::unique_ptr<std::mutex> m_mutex;
};

class ConditionVariable {
public:
  ConditionVariable() : m_cv(std::make_unique<std::condition_variable>()) {}
  ConditionVariable(ConditionVariable const&) = delete;
  ConditionVariable(ConditionVariable&&) noexcept = default;
  ~ConditionVariable() = default;

  auto operator=(ConditionVariable const&) -> ConditionVariable& = delete;
  auto operator=(ConditionVariable&&) noexcept -> ConditionVariable& = default;

  // Atomically unlocks the mutex argument and waits on the condition.  On
  // acquiring the condition, atomically returns and re-locks the mutex.  Must
  // lock the mutex before calling.  If millis is given, waits for a maximum of
  // the given milliseconds only.
  void wait(Mutex& mutex, std::optional<unsigned> millis = {}) {
    std::unique_lock<std::mutex> lock(*mutex.m_mutex, std::adopt_lock);
    if (millis)
      m_cv->wait_for(lock, std::chrono::milliseconds(*millis));
    else
      m_cv->wait(lock);
    lock.release(); // Return ownership to caller as per Starbound API
  }

  // Wake one waiting thread.  The calling thread for is allowed to either hold
  // or not hold the mutex that the threads waiting on the condition are using,
  // both will work and result in slightly different scheduling.
  void signal() { m_cv->notify_one(); }

  // Wake all threads, policy for holding the mutex is the same for signal().
  void broadcast() { m_cv->notify_all(); }

private:
  std::unique_ptr<std::condition_variable> m_cv;
};

// Recursive mutex lock.  lock() may be called many times freely by the same
// thread, but unlock() must be called an equal number of times to unlock it.
class RecursiveMutex {
public:
  RecursiveMutex() : m_mutex(std::make_unique<std::recursive_mutex>()) {}
  RecursiveMutex(RecursiveMutex const&) = delete;
  RecursiveMutex(RecursiveMutex&&) noexcept = default;
  ~RecursiveMutex() = default;

  auto operator=(RecursiveMutex const&) -> RecursiveMutex& = delete;
  auto operator=(RecursiveMutex&&) noexcept -> RecursiveMutex& = default;

  void lock() { m_mutex->lock(); }

  // Attempt to acquire the mutex without blocking.
  auto tryLock() -> bool { return m_mutex->try_lock(); }

  void unlock() { m_mutex->unlock(); }

private:
  std::unique_ptr<std::recursive_mutex> m_mutex;
};

// RAII for mutexes.  Locking and unlocking are always safe, MLocker will never
// attempt to lock the held mutex more than once, or unlock more than once, and
// destruction will always unlock the mutex *iff* it is actually locked.
// (Locked here refers to one specific MLocker *itself* locking the mutex, not
// whether the mutex is locked *at all*, so it is sensible to use with
// RecursiveMutex)
template <typename MutexType>
class MLocker {
public:
  // Pass false to lock to start unlocked
  explicit MLocker(MutexType& ref, bool lock = true)
    : m_mutex(ref)  {
    if (lock)
      this->lock();
  }

  ~MLocker() {
    unlock();
  }

  MLocker(MLocker const&) = delete;
  auto operator=(MLocker const&) -> MLocker& = delete;

  auto mutex() -> MutexType& { return m_mutex; }

  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }

  void lock() {
    if (!m_locked) {
      m_mutex.lock();
      m_locked = true;
    }
  }

  auto tryLock() -> bool {
    if (!m_locked) {
      if (m_mutex.tryLock())
        m_locked = true;
    }

    return m_locked;
  }

  [[nodiscard]] auto isLocked() const -> bool { return m_locked; }

private:
  MutexType& m_mutex;
  bool m_locked{};
};
using MutexLocker = MLocker<Mutex>;
using RecursiveMutexLocker = MLocker<RecursiveMutex>;

class ReadersWriterMutex {
public:
  ReadersWriterMutex() : m_mutex(std::make_unique<std::shared_mutex>()) {}
  ReadersWriterMutex(ReadersWriterMutex const&) = delete;
  ReadersWriterMutex(ReadersWriterMutex&&) noexcept = default;

  auto operator=(ReadersWriterMutex const&) -> ReadersWriterMutex& = delete;
  auto operator=(ReadersWriterMutex&&) noexcept -> ReadersWriterMutex& = default;

  void readLock() { m_mutex->lock_shared(); }
  auto tryReadLock() -> bool { return m_mutex->try_lock_shared(); }
  void readUnlock() { m_mutex->unlock_shared(); }

  void writeLock() { m_mutex->lock(); }
  auto tryWriteLock() -> bool { return m_mutex->try_lock(); }
  void writeUnlock() { m_mutex->unlock(); }

private:
  std::unique_ptr<std::shared_mutex> m_mutex;
};

class ReadLocker {
public:
  explicit ReadLocker(ReadersWriterMutex& rwlock, bool startLocked = true)
    : m_lock(rwlock) {
    if (startLocked)
      lock();
  }
  ~ReadLocker() { unlock(); }

  ReadLocker(ReadLocker const&) = delete;
  auto operator=(ReadLocker const&) -> ReadLocker& = delete;

  void unlock() {
    if (m_locked) {
      m_lock.readUnlock();
      m_locked = false;
    }
  }

  void lock() {
    if (!m_locked) {
      m_lock.readLock();
      m_locked = true;
    }
  }

  auto tryLock() -> bool;

  [[nodiscard]] auto isLocked() const -> bool { return m_locked; }

private:
  ReadersWriterMutex& m_lock;
  bool m_locked{};
};

class WriteLocker {
public:
  explicit WriteLocker(ReadersWriterMutex& rwlock, bool startLocked = true)
    : m_lock(rwlock)  {
    if (startLocked)
      lock();
  }
  ~WriteLocker() { unlock(); }

  WriteLocker(WriteLocker const&) = delete;
  auto operator=(WriteLocker const&) -> WriteLocker& = delete;

  void unlock() {
    if (m_locked) {
      m_lock.writeUnlock();
      m_locked = false;
    }
  }

  void lock() {
    if (!m_locked) {
      m_lock.writeLock();
      m_locked = true;
    }
  }

  auto tryLock() -> bool {
    if (!m_locked)
      m_locked = m_lock.tryWriteLock();
    return m_locked;
  }

  [[nodiscard]] auto isLocked() const -> bool { return m_locked; }

private:
  ReadersWriterMutex& m_lock;
  bool m_locked{};
};

class SpinLock {
public:
  SpinLock() = default;

  void lock() {
    while (m_lock.test_and_set(std::memory_order::acquire)) {
      m_lock.wait(true, std::memory_order::relaxed);
    }
  }

  auto tryLock() -> bool {
    return !m_lock.test_and_set(std::memory_order::acquire);
  }

  void unlock() {
    m_lock.clear(std::memory_order::release);
    m_lock.notify_one();
  }

private:
  std::atomic_flag m_lock{};
};
using SpinLocker = MLocker<SpinLock>;

template <typename Function, typename... Args>
auto Thread::invoke(String const& name, Function&& f, Args&&... args) -> ThreadFunction<std::invoke_result_t<Function, Args...>> {
  using Return = std::invoke_result_t<Function, Args...>;
  return ThreadFunction<Return>(std::bind_front(std::forward<Function>(f), std::forward<Args>(args)...), name);
}

template <typename Return>
ThreadFunction<Return>::ThreadFunction(std::function<Return()> function, String const& name) {
  m_return = std::make_shared<std::optional<Return>>();
  m_function = ThreadFunction<void>([function = std::move(function), retValue = m_return]() -> auto {
    *retValue = function();
  }, name);
}

template <typename Return>
ThreadFunction<Return>::~ThreadFunction() {
  m_function.finish();
}

template <typename Return>
auto ThreadFunction<Return>::finish() -> Return {
  m_function.finish();
  if (!m_return->has_value())
    throw std::runtime_error("ThreadFunction::finish called but no return value available");
  Return ret = std::move(**m_return);
  m_return->reset();
  return ret;
}

template <typename Return>
auto ThreadFunction<Return>::isFinished() const -> bool {
  return m_function.isFinished();
}

template <typename Return>
auto ThreadFunction<Return>::isRunning() const -> bool {
  return m_function.isRunning();
}

template <typename Return>
ThreadFunction<Return>::operator bool() const {
  return !isFinished();
}

template <typename Return>
auto ThreadFunction<Return>::name() const -> String {
  return m_function.name();
}

inline ThreadFunction<void>::operator bool() const {
  return !isFinished();
}

}
