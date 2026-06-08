#include "StarThread.hpp"
#include "StarTime.hpp"
#include "StarLogging.hpp"
#include "StarFormat.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#ifndef STAR_SYSTEM_WINDOWS
#include <cstdio>
#include <pthread.h>
#ifdef STAR_SYSTEM_FREEBSD
#include <pthread_np.h>
#endif
#endif

namespace Star {

// ---- Platform-specific helper: set thread name via native handle ----

static void setThreadName(std::thread& t, String const& name) {
#ifndef STAR_SYSTEM_WINDOWS
  char tname[16];
  snprintf(tname, sizeof(tname), "%s", name.utf8Ptr());
#if defined(STAR_SYSTEM_FREEBSD)
  pthread_set_name_np(t.native_handle(), tname);
#elif defined(STAR_SYSTEM_NETBSD)
  pthread_setname_np(t.native_handle(), "%s", tname);
#elif defined(STAR_SYSTEM_MACOS)
  pthread_setname_np(tname);
#else
  pthread_setname_np(t.native_handle(), tname);
#endif
#endif
}

// ---- ThreadImpl ----

struct ThreadImpl {
  static void runThread(ThreadImpl* ptr) {
    try {
      ptr->function();
    } catch (std::exception const& e) {
      if (ptr->name.empty())
        Logger::error("Exception caught in Thread: {}", outputException(e, true));
      else
        Logger::error("Exception caught in Thread {}: {}", ptr->name, outputException(e, true));
    } catch (...) {
      if (ptr->name.empty())
        Logger::error("Unknown exception caught in Thread");
      else
        Logger::error("Unknown exception caught in Thread {}", ptr->name);
    }
    ptr->stopped = true;
  }

  ThreadImpl(std::function<void()> function, String name)
    : function(std::move(function)), name(std::move(name)), stopped(true), joined(true) {}

  bool start() {
    MutexLocker mutexLocker(mutex);
    if (!joined)
      return false;

    stopped = false;
    joined = false;
    try {
      thread = std::thread(runThread, this);
      setThreadName(thread, name);
    } catch (std::system_error const& e) {
      stopped = true;
      joined = true;
      throw StarException(strf("Failed to create thread, error {}", e.what()));
    }
    return true;
  }

  bool join() {
    MutexLocker mutexLocker(mutex);
    if (joined)
      return false;
    if (thread.joinable())
      thread.join();
    joined = true;
    return true;
  }

  std::function<void()> function;
  String name;
  std::thread thread;
  std::atomic<bool> stopped;
  bool joined;
  Mutex mutex;
};

// ---- ThreadFunctionImpl ----

struct ThreadFunctionImpl : ThreadImpl {
  ThreadFunctionImpl(std::function<void()> function, String name)
    : ThreadImpl(wrapFunction(std::move(function)), std::move(name)) {}

  std::function<void()> wrapFunction(std::function<void()> function) {
    return [function = std::move(function), this]() {
      try {
        function();
      } catch (...) {
        exception = std::current_exception();
      }
    };
  }

  std::exception_ptr exception;
};

// ---- MutexImpl ----

struct MutexImpl {
  std::mutex mutex;
};

// ---- ConditionVariableImpl ----

struct ConditionVariableImpl {
  ConditionVariableImpl() = default;

  void wait(Mutex& mutex) {
    std::unique_lock<std::mutex> lock(mutex.m_impl->mutex, std::adopt_lock);
    cond.wait(lock);
    lock.release();
  }

  void wait(Mutex& mutex, unsigned millis) {
    std::unique_lock<std::mutex> lock(mutex.m_impl->mutex, std::adopt_lock);
    cond.wait_for(lock, std::chrono::milliseconds(millis));
    lock.release();
  }

  void signal() {
    cond.notify_one();
  }

  void broadcast() {
    cond.notify_all();
  }

  std::condition_variable cond;
};

// ---- RecursiveMutexImpl ----

struct RecursiveMutexImpl {
  std::recursive_mutex mutex;
};

// ---- Thread static methods ----

void Thread::sleepPrecise(unsigned msecs) {
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(msecs);
  while (std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();
}

void Thread::sleep(unsigned msecs) {
  std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
}

void Thread::yield() {
  std::this_thread::yield();
}

unsigned Thread::numberOfProcessors() {
  return std::thread::hardware_concurrency();
}

// ---- Thread wrapper methods ----

Thread::Thread(String const& name) {
  m_impl = make_unique<ThreadImpl>([this]() { run(); }, name);
}

Thread::Thread(Thread&&) = default;

Thread::~Thread() {}

Thread& Thread::operator=(Thread&&) = default;

bool Thread::start() {
  return m_impl->start();
}

bool Thread::join() {
  return m_impl->join();
}

String Thread::name() {
  return m_impl->name;
}

bool Thread::isJoined() const {
  return m_impl->joined;
}

bool Thread::isRunning() const {
  return !m_impl->stopped;
}

// ---- ThreadFunction<void> methods ----

ThreadFunction<void>::ThreadFunction() {}

ThreadFunction<void>::ThreadFunction(ThreadFunction&&) = default;

ThreadFunction<void>::ThreadFunction(function<void()> function, String const& name) {
  m_impl = make_unique<ThreadFunctionImpl>(std::move(function), name);
  m_impl->start();
}

ThreadFunction<void>::~ThreadFunction() {
  finish();
}

ThreadFunction<void>& ThreadFunction<void>::operator=(ThreadFunction&&) = default;

void ThreadFunction<void>::finish() {
  if (m_impl) {
    m_impl->join();

    if (m_impl->exception)
      std::rethrow_exception(take(m_impl->exception));
  }
}

bool ThreadFunction<void>::isFinished() const {
  return !m_impl || m_impl->joined;
}

bool ThreadFunction<void>::isRunning() const {
  return m_impl && !m_impl->stopped;
}

ThreadFunction<void>::operator bool() const {
  return !isFinished();
}

String ThreadFunction<void>::name() {
  if (m_impl)
    return m_impl->name;
  else
    return "";
}

// ---- Mutex wrapper methods ----

Mutex::Mutex()
  : m_impl(make_unique<MutexImpl>()) {}

Mutex::Mutex(Mutex&&) = default;

Mutex::~Mutex() {}

Mutex& Mutex::operator=(Mutex&&) = default;

void Mutex::lock() {
  m_impl->mutex.lock();
}

bool Mutex::tryLock() {
  return m_impl->mutex.try_lock();
}

void Mutex::unlock() {
  m_impl->mutex.unlock();
}

// ---- ConditionVariable wrapper methods ----

ConditionVariable::ConditionVariable()
  : m_impl(make_unique<ConditionVariableImpl>()) {}

ConditionVariable::ConditionVariable(ConditionVariable&&) = default;

ConditionVariable::~ConditionVariable() {}

ConditionVariable& ConditionVariable::operator=(ConditionVariable&&) = default;

void ConditionVariable::wait(Mutex& mutex, Maybe<unsigned> millis) {
  if (millis)
    m_impl->wait(mutex, *millis);
  else
    m_impl->wait(mutex);
}

void ConditionVariable::signal() {
  m_impl->signal();
}

void ConditionVariable::broadcast() {
  m_impl->broadcast();
}

// ---- RecursiveMutex wrapper methods ----

RecursiveMutex::RecursiveMutex()
  : m_impl(make_unique<RecursiveMutexImpl>()) {}

RecursiveMutex::RecursiveMutex(RecursiveMutex&&) = default;

RecursiveMutex::~RecursiveMutex() {}

RecursiveMutex& RecursiveMutex::operator=(RecursiveMutex&&) = default;

void RecursiveMutex::setLogged(bool logged) {
  if constexpr (LogRecursiveMutex)
    m_log = logged;
}

void RecursiveMutex::lock() {
  if constexpr (LogRecursiveMutex) {
    if (m_log)
      printStack("RecursiveMutex lock waiting");
  }
  m_impl->mutex.lock();
  if constexpr (LogRecursiveMutex) {
    if (m_log)
      printStack("RecursiveMutex locked");
  }
}

bool RecursiveMutex::tryLock() {
  if constexpr (LogRecursiveMutex) {
    if (m_log)
      printStack("RecursiveMutex tryLock waiting");
  }
  bool result = m_impl->mutex.try_lock();
  if constexpr (LogRecursiveMutex) {
    if (result && m_log)
      printStack("RecursiveMutex tryLock success");
  }
  return result;
}

void RecursiveMutex::unlock() {
  m_impl->mutex.unlock();
  if constexpr (LogRecursiveMutex) {
    if (m_log)
      printStack("RecursiveMutex unlocked");
  }
}

// ---- ReadersWriterMutex ----

ReadersWriterMutex::ReadersWriterMutex()
  : m_readers(), m_writers(), m_readWaiters(), m_writeWaiters() {}

void ReadersWriterMutex::readLock() {
  MutexLocker locker(m_mutex);
  if (m_writers || m_writeWaiters) {
    m_readWaiters++;
    while (m_writers || m_writeWaiters)
      m_readCond.wait(m_mutex);
    m_readWaiters--;
  }
  m_readers++;
}

bool ReadersWriterMutex::tryReadLock() {
  MutexLocker locker(m_mutex);
  if (m_writers || m_writeWaiters)
    return false;
  m_readers++;
  return true;
}

void ReadersWriterMutex::readUnlock() {
  MutexLocker locker(m_mutex);
  m_readers--;
  if (m_writeWaiters)
    m_writeCond.signal();
}

void ReadersWriterMutex::writeLock() {
  MutexLocker locker(m_mutex);
  if (m_readers || m_writers) {
    m_writeWaiters++;
    while (m_readers || m_writers)
      m_writeCond.wait(m_mutex);
    m_writeWaiters--;
  }
  m_writers = 1;
}

bool ReadersWriterMutex::tryWriteLock() {
  MutexLocker locker(m_mutex);
  if (m_readers || m_writers)
    return false;
  m_writers = 1;
  return true;
}

void ReadersWriterMutex::writeUnlock() {
  MutexLocker locker(m_mutex);
  m_writers = 0;
  if (m_writeWaiters)
    m_writeCond.signal();
  else if (m_readWaiters)
    m_readCond.broadcast();
}

ReadLocker::ReadLocker(ReadersWriterMutex& rwlock, bool startLocked) : m_lock(rwlock), m_locked(false) {
  if (startLocked)
    lock();
}

ReadLocker::~ReadLocker() {
  unlock();
}

void ReadLocker::unlock() {
  if (m_locked)
    m_lock.readUnlock();
  m_locked = false;
}

void ReadLocker::lock() {
  if (!m_locked)
    m_lock.readLock();
  m_locked = true;
}

bool ReadLocker::tryLock() {
  if (!m_locked) {
    m_locked = m_lock.tryReadLock();
    return m_locked;
  }
  return true;
}

WriteLocker::WriteLocker(ReadersWriterMutex& rwlock, bool startLocked) : m_lock(rwlock), m_locked(false) {
  if (startLocked)
    lock();
}

WriteLocker::~WriteLocker() {
  unlock();
}

void WriteLocker::unlock() {
  if (m_locked)
    m_lock.writeUnlock();
  m_locked = false;
}

void WriteLocker::lock() {
  if (!m_locked)
    m_lock.writeLock();
  m_locked = true;
}

bool WriteLocker::tryLock() {
  if (!m_locked) {
    m_locked = m_lock.tryWriteLock();
    return m_locked;
  }
  return true;
}

}
