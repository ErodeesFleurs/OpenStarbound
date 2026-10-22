#include <stdio.h>

#include <limits.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <dirent.h>
#include <pthread.h>
#ifdef STAR_SYSTEM_FREEBSD
#include <pthread_np.h>
#endif
#include <sys/time.h>
#include <errno.h>
import std;
import star.unique_function;
import star.thread;
import star.string;
import star.format;
import star.output_proxy;
import star.time;
import star.logging;

#ifdef MAXCOMLEN
static constexpr std::size_t MaxThreadNameLength = MAXCOMLEN;
#else
static constexpr std::size_t MaxThreadNameLength = 16;
#endif

import star.algorithm;
namespace Star {

struct ThreadImpl {
  static void* runThread(void* data) {
    ThreadImpl* ptr = static_cast<ThreadImpl*>(data);
#ifdef STAR_SYSTEM_MACOS
    char tname[MaxThreadNameLength];
    std::snprintf(tname, sizeof(tname), "%s", ptr->name.utf8Ptr());
    pthread_setname_np(tname);
#endif
    auto result = ptr->function();
    if (!result)
      EmergencyLog::error("Thread {} failed: {}", ptr->name, outputException(ExceptionDiagnostic(result.error()), true));
    ptr->stopped = true;
    return nullptr;
  }

  ThreadImpl(UniqueFunction<std::expected<void, StarException>()> function, String name)
    : function(std::move(function)), name(std::move(name)), stopped(true), joined(true) {}

  [[nodiscard]] std::expected<bool, StarException> start() {
    MutexLocker mutexLocker(mutex);
    if (!joined)
      return false;

    stopped = false;
    joined = false;
    int ret = pthread_create(&pthread, NULL, &runThread, (void*)this);
    if (ret != 0) {
      stopped = true;
      joined = true;
      return std::unexpected(StarException(strf("Failed to create thread, error {}", ret)));
    }

    // ensure the name is under the max allowed
    char tname[MaxThreadNameLength];
    std::snprintf(tname, sizeof(tname), "%s", name.utf8Ptr());

#ifdef STAR_SYSTEM_FREEBSD
    pthread_set_name_np(pthread, tname);
#elif defined(STAR_SYSTEM_NETBSD)
    pthread_setname_np(pthread, "%s", tname);
#elif not defined STAR_SYSTEM_MACOS
    pthread_setname_np(pthread, tname);
#endif
    return true;
  }

  [[nodiscard]] std::expected<bool, StarException> join() {
    MutexLocker mutexLocker(mutex);
    if (joined)
      return false;
    int ret = pthread_join(pthread, NULL);
    if (ret != 0)
      return std::unexpected(StarException(strf("Failed to join thread, error {}", ret)));
    joined = true;
    return true;
  }

  UniqueFunction<std::expected<void, StarException>()> function;
  String name;
  pthread_t pthread;
  std::atomic<bool> stopped;
  bool joined;
  Mutex mutex;
};

struct ThreadFunctionImpl : ThreadImpl {
  ThreadFunctionImpl(UniqueFunction<std::expected<void, StarException>()> function, String name)
    : ThreadImpl(wrapFunction(std::move(function)), std::move(name)) {}

  UniqueFunction<std::expected<void, StarException>()> wrapFunction(
      UniqueFunction<std::expected<void, StarException>()> function) {
    return [function = std::move(function), this]() mutable -> std::expected<void, StarException> {
      auto result = function();
      if (!result)
        error = std::move(result.error());
      return {};
    };
  }

  std::optional<StarException> error;
};

struct MutexImpl {
  MutexImpl() {
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);

    pthread_mutex_init(&mutex, &mutexattr);

    pthread_mutexattr_destroy(&mutexattr);
  }

  ~MutexImpl() {
    pthread_mutex_destroy(&mutex);
  }

  void lock() {
#ifdef STAR_MUTEX_TIMED
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 15;
    if (pthread_mutex_timedlock(&mutex, &ts) != 0) {
      printStack("Mutex::lock is TAKING TOO LONG 🎃");
#else
    {
#endif
      pthread_mutex_lock(&mutex);
    }
  }

  void unlock() {
    pthread_mutex_unlock(&mutex);
  }

  bool tryLock() {
    if (pthread_mutex_trylock(&mutex) == 0)
      return true;
    else
      return false;
  }

  pthread_mutex_t mutex;
};

struct ConditionVariableImpl {
  ConditionVariableImpl() {
    pthread_cond_init(&condition, NULL);
  }

  ~ConditionVariableImpl() {
    pthread_cond_destroy(&condition);
  }

  void wait(Mutex& mutex) {
    pthread_cond_wait(&condition, &mutex.m_impl->mutex);
  }

  void wait(Mutex& mutex, unsigned millis) {
    std::int64_t time = Time::millisecondsSinceEpoch() + millis;

    timespec ts;
    ts.tv_sec = time / 1000;
    ts.tv_nsec = (time % 1000) * 1000000;

    pthread_cond_timedwait(&condition, &mutex.m_impl->mutex, &ts);
  }

  void signal() {
    pthread_cond_signal(&condition);
  }

  void broadcast() {
    pthread_cond_broadcast(&condition);
  }

  pthread_cond_t condition;
};

struct RecursiveMutexImpl {
  RecursiveMutexImpl() {
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);

    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&mutex, &mutexattr);

    pthread_mutexattr_destroy(&mutexattr);
  }

  ~RecursiveMutexImpl() {
    pthread_mutex_destroy(&mutex);
  }

  void lock() {
#ifndef STAR_SYSTEM_MACOS
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 15;
    if (pthread_mutex_timedlock(&mutex, &ts) != 0) {
      printStack("RecursiveMutex::lock is TAKING TOO LONG 🎃");
#else
    {
#endif
      pthread_mutex_lock(&mutex);
    }
  }

  void unlock() {
    pthread_mutex_unlock(&mutex);
  }

  bool tryLock() {
    if (pthread_mutex_trylock(&mutex) == 0)
      return true;
    else
      return false;
  }

  pthread_mutex_t mutex;
};

void Thread::sleepPrecise(unsigned msecs) {
  std::int64_t now = Time::monotonicMilliseconds();
  std::int64_t deadline = now + msecs;

  while (deadline - now > 10) {
    usleep((deadline - now - 10) * 1000);
    now = Time::monotonicMilliseconds();
  }

  while (deadline > now) {
    usleep((deadline - now) * 500);
    now = Time::monotonicMilliseconds();
  }
}

void Thread::sleep(unsigned msecs) {
  usleep(msecs * 1000);
}

void Thread::yield() {
  sched_yield();
}

std::expected<unsigned, StarException> Thread::numberOfProcessors() {
  long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
  if (nprocs < 1)
    return std::unexpected(StarException(strf("Could not determine number of CPUs online: {}\n", std::strerror(errno))));
  return static_cast<unsigned>(nprocs);
}

Thread::Thread(String const& name) {
  m_impl.reset(new ThreadImpl([this]() {
      return run();
    }, name));
}

Thread::Thread(Thread&&) = default;

Thread::~Thread() {
  if (m_impl && !isJoined()) {
    EmergencyLog::error("Thread {} destroyed without joining", name());
    fatalError("Cannot destroy an unjoined thread", false);
  }
}

Thread& Thread::operator=(Thread&& other) {
  if (this != &other) {
    if (m_impl && !isJoined()) {
      EmergencyLog::error("Thread {} overwritten without joining", name());
      fatalError("Cannot overwrite an unjoined thread", false);
    }
    m_impl = std::move(other.m_impl);
  }
  return *this;
}

std::expected<bool, StarException> Thread::start() {
  return m_impl->start();
}

std::expected<bool, StarException> Thread::join() {
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

ThreadFunction<void>::ThreadFunction() {}

ThreadFunction<void>::ThreadFunction(ThreadFunction&&) = default;

std::expected<ThreadFunction<void>, StarException> ThreadFunction<void>::create(
    UniqueFunction<std::expected<void, StarException>()> function, String const& name) {
  ThreadFunction result;
  result.m_impl = std::make_unique<ThreadFunctionImpl>(std::move(function), name);
  auto started = result.m_impl->start();
  if (!started)
    return std::unexpected(std::move(started.error()));
  return result;
}

ThreadFunction<void>::~ThreadFunction() {
  auto result = finish();
  if (!result)
    EmergencyLog::error("Thread function {} failed during destruction: {}", name(), outputException(ExceptionDiagnostic(result.error()), true));
  if (!isFinished())
    fatalError("Cannot destroy an unjoined thread function", false);
}

ThreadFunction<void>& ThreadFunction<void>::operator=(ThreadFunction&& other) {
  if (this != &other) {
    auto result = finish();
    if (!result)
      EmergencyLog::error("Thread function {} failed during reassignment: {}", name(), outputException(ExceptionDiagnostic(result.error()), true));
    if (!isFinished())
      fatalError("Cannot overwrite an unjoined thread function", false);
    m_impl = std::move(other.m_impl);
  }
  return *this;
}

std::expected<void, StarException> ThreadFunction<void>::finish() {
  if (m_impl) {
    auto joined = m_impl->join();
    if (!joined)
      return std::unexpected(std::move(joined.error()));
    if (m_impl->error) {
      auto error = std::move(*m_impl->error);
      m_impl->error.reset();
      return std::unexpected(std::move(error));
    }
  }
  return {};
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

Mutex::Mutex()
  : m_impl(new MutexImpl()) {}

Mutex::Mutex(Mutex&&) = default;

Mutex::~Mutex() {}

Mutex& Mutex::operator=(Mutex&&) = default;

void Mutex::lock() {
  m_impl->lock();
}

bool Mutex::tryLock() {
  return m_impl->tryLock();
}

void Mutex::unlock() {
  m_impl->unlock();
}

ConditionVariable::ConditionVariable()
  : m_impl(new ConditionVariableImpl()) {}

ConditionVariable::ConditionVariable(ConditionVariable&&) = default;

ConditionVariable::~ConditionVariable() {}

ConditionVariable& ConditionVariable::operator=(ConditionVariable&&) = default;

void ConditionVariable::wait(Mutex& mutex, std::optional<unsigned> millis) {
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

RecursiveMutex::RecursiveMutex()
  : m_impl(new RecursiveMutexImpl()) {}

RecursiveMutex::RecursiveMutex(RecursiveMutex&&) = default;

RecursiveMutex::~RecursiveMutex() {}

RecursiveMutex& RecursiveMutex::operator=(RecursiveMutex&&) = default;

void RecursiveMutex::lock() {
  m_impl->lock();
}

bool RecursiveMutex::tryLock() {
  return m_impl->tryLock();
}

void RecursiveMutex::unlock() {
  m_impl->unlock();
}

}
