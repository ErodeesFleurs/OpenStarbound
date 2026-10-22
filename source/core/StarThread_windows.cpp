
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <locale.h>
import std;
import star.unique_function;
import star.thread;
import star.string;
import star.format;
import star.output_proxy;
import star.time;
import star.logging;

import star.algorithm;
namespace Star {

// This is the CONDITIONAL_VARIABLE typedef for using Window's native
// conditional variables on kernels 6.0+.
// MinGW does not currently have this typedef.
typedef struct pthread_cond_t { void* ptr; } CONDITIONAL_VARIABLE;

// Static thread initialization
struct ThreadSupport {
  ThreadSupport() {
    HMODULE kernel_dll = GetModuleHandle(TEXT("kernel32.dll"));
    initializeConditionVariable = (InitializeConditionVariablePtr)GetProcAddress(kernel_dll, "InitializeConditionVariable");
    wakeAllConditionVariable = (WakeAllConditionVariablePtr)GetProcAddress(kernel_dll, "WakeAllConditionVariable");
    wakeConditionVariable = (WakeConditionVariablePtr)GetProcAddress(kernel_dll, "WakeConditionVariable");
    sleepConditionVariableSRW = (SleepConditionVariableSRWPtr)GetProcAddress(kernel_dll, "SleepConditionVariableSRW");

    nativeConditionVariables = initializeConditionVariable && wakeAllConditionVariable && wakeConditionVariable && sleepConditionVariableSRW;
  }

  typedef void(WINAPI* InitializeConditionVariablePtr)(CONDITIONAL_VARIABLE* cond);
  typedef void(WINAPI* WakeAllConditionVariablePtr)(CONDITIONAL_VARIABLE* cond);
  typedef void(WINAPI* WakeConditionVariablePtr)(CONDITIONAL_VARIABLE* cond);
  typedef BOOL(WINAPI* SleepConditionVariableSRWPtr)(CONDITIONAL_VARIABLE* cond, PSRWLOCK SRWLock, DWORD milliseconds, ULONG flags);

  // function pointers to conditional variable API on windows 6.0+ kernels
  InitializeConditionVariablePtr initializeConditionVariable;
  WakeAllConditionVariablePtr wakeAllConditionVariable;
  WakeConditionVariablePtr wakeConditionVariable;
  SleepConditionVariableSRWPtr sleepConditionVariableSRW;

  bool nativeConditionVariables;
};
static ThreadSupport g_threadSupport;

struct ThreadImpl {
  static DWORD WINAPI runThread(void* data) {
    ThreadImpl* ptr = static_cast<ThreadImpl*>(data);
    unsigned long exceptionStackSize = 131072;
    SetThreadStackGuarantee(&exceptionStackSize);
    auto result = ptr->function();
    if (!result)
      EmergencyLog::error("Thread {} failed: {}", ptr->name, outputException(ExceptionDiagnostic(result.error()), true));
    ptr->stopped = true;
    return 0;
  }

  ThreadImpl(UniqueFunction<std::expected<void, StarException>()> function, String name)
    : function(std::move(function)), name(std::move(name)), thread(INVALID_HANDLE_VALUE), stopped(true) {}

  [[nodiscard]] std::expected<bool, StarException> start() {
    MutexLocker mutexLocker(mutex);
    if (thread != INVALID_HANDLE_VALUE)
      return false;

    stopped = false;
    if (thread == INVALID_HANDLE_VALUE)
      thread = CreateThread(NULL, 0, runThread, (void*)this, 0, NULL);
    if (thread == NULL)
      thread = INVALID_HANDLE_VALUE;
    if (thread == INVALID_HANDLE_VALUE) {
      stopped = true;
      return std::unexpected(StarException(strf("Failed to create thread, error {}", GetLastError())));
    }
    return true;
  }

  [[nodiscard]] std::expected<bool, StarException> join() {
    MutexLocker mutexLocker(mutex);
    if (thread == INVALID_HANDLE_VALUE)
      return false;
    if (WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0)
      return std::unexpected(StarException(strf("Failed to join thread, error {}", GetLastError())));
    if (!CloseHandle(thread))
      return std::unexpected(StarException(strf("Failed to close joined thread, error {}", GetLastError())));
    thread = INVALID_HANDLE_VALUE;
    return true;
  }

  UniqueFunction<std::expected<void, StarException>()> function;
  String name;
  HANDLE thread;
  std::atomic<bool> stopped;

private:
  ThreadImpl(ThreadImpl const&);
  ThreadImpl& operator=(ThreadImpl const&);

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
    InitializeSRWLock(&srwLock);
  }

  ~MutexImpl() {}

  void lock() {
    AcquireSRWLockExclusive(&srwLock);
  }

  void unlock() {
    ReleaseSRWLockExclusive(&srwLock);
  }

  bool tryLock() {
    return TryAcquireSRWLockExclusive(&srwLock);
  }

  SRWLOCK srwLock;
};

struct ConditionVariableImpl {
  ConditionVariableImpl() {
    if (g_threadSupport.nativeConditionVariables) {
      m_impl = std::make_unique<NativeImpl>();
    } else {
      m_impl = std::make_unique<EmulatedImpl>();
    }
  }

  void wait(Mutex& mutex) {
    m_impl->wait(mutex);
  }

  void wait(Mutex& mutex, unsigned millis) {
    m_impl->wait(mutex, millis);
  }

  void signal() {
    m_impl->signal();
  }

  void broadcast() {
    m_impl->broadcast();
  }

private:
  struct Impl {
    virtual ~Impl() {}

    virtual void wait(Mutex& mutex) = 0;
    virtual void wait(Mutex& mutex, unsigned millis) = 0;
    virtual void signal() = 0;
    virtual void broadcast() = 0;
  };

  struct NativeImpl : Impl {
    NativeImpl() {
      g_threadSupport.initializeConditionVariable(&conditionVariable);
    }

    void wait(Mutex& mutex) override {
      g_threadSupport.sleepConditionVariableSRW(&conditionVariable, &mutex.m_impl->srwLock, INFINITE, 0);
    }

    void wait(Mutex& mutex, unsigned millis) override {
      g_threadSupport.sleepConditionVariableSRW(&conditionVariable, &mutex.m_impl->srwLock, millis, 0);
    }

    void signal() override {
      g_threadSupport.wakeConditionVariable(&conditionVariable);
    }

    void broadcast() override {
      g_threadSupport.wakeAllConditionVariable(&conditionVariable);
    }

    CONDITIONAL_VARIABLE conditionVariable;
  };

  struct EmulatedImpl : Impl {
    EmulatedImpl() {
      numThreads = 0;
      isBroadcasting = 0;
      threadSemaphore = CreateSemaphore(NULL, // no security
          0, // initially 0
          0x7fffffff, // max count
          NULL); // unnamed

      InitializeSRWLock(&numThreadsConditionLock);

      broadcastDone = CreateEvent(NULL, // no security
          FALSE, // auto-reset
          FALSE, // non-signaled initially
          NULL); // unnamed
    }

    virtual ~EmulatedImpl() {
      CloseHandle(threadSemaphore);
      CloseHandle(broadcastDone);
    }

    void wait(Mutex& mutex) override {
      // Avoid race conditions.
      AcquireSRWLockExclusive(&numThreadsConditionLock);
      numThreads++;
      ReleaseSRWLockExclusive(&numThreadsConditionLock);

      // Release the mutex and waits on the semaphore until signal or broadcast
      // are called by another thread.
      ReleaseSRWLockExclusive(&mutex.m_impl->srwLock);
      WaitForSingleObject(threadSemaphore, INFINITE);

      // Reacquire lock to avoid race conditions.
      AcquireSRWLockExclusive(&numThreadsConditionLock);

      // We're no longer waiting...
      numThreads--;

      // Check to see if we're the last waiter after broadcast
      bool last_waiter = isBroadcasting && numThreads == 0;

      ReleaseSRWLockExclusive(&numThreadsConditionLock);

      // If we're the last waiter thread during this particular broadcast
      // then let all the other threads proceed.
      if (last_waiter)
        SetEvent(broadcastDone);
      AcquireSRWLockExclusive(&mutex.m_impl->srwLock);
    }

    void wait(Mutex& mutex, unsigned millis) override {
      // Avoid race conditions.
      AcquireSRWLockExclusive(&numThreadsConditionLock);
      numThreads++;
      ReleaseSRWLockExclusive(&numThreadsConditionLock);

      // Release the mutex and waits on the semaphore until signal or broadcast
      // are called by another thread.
      ReleaseSRWLockExclusive(&mutex.m_impl->srwLock);
      WaitForSingleObject(threadSemaphore, millis);

      // Reacquire lock to avoid race conditions.
      AcquireSRWLockExclusive(&numThreadsConditionLock);

      // We're no longer waiting...
      numThreads--;

      // Check to see if we're the last waiter after broadcast
      bool last_waiter = isBroadcasting && numThreads == 0;

      ReleaseSRWLockExclusive(&numThreadsConditionLock);

      // If we're the last waiter thread during this particular broadcast
      // then let all the other threads proceed.
      if (last_waiter)
        SetEvent(broadcastDone);
      AcquireSRWLockExclusive(&mutex.m_impl->srwLock);
    }

    void signal() override {
      AcquireSRWLockExclusive(&numThreadsConditionLock);
      bool have_waiters = numThreads > 0;
      ReleaseSRWLockExclusive(&numThreadsConditionLock);

      // If there aren't any waiters, then this is a no-op.
      if (have_waiters)
        ReleaseSemaphore(threadSemaphore, 1, 0);
    }

    void broadcast() override {
      // This is needed to ensure that <numThreads> and <isBroadcasting> are
      // consistent relative to each other.
      AcquireSRWLockExclusive(&numThreadsConditionLock);
      bool have_waiters = 0;

      if (numThreads > 0) {
        // We are broadcasting, even if there is just one waiter...
        // Record that we are broadcasting
        isBroadcasting = 1;
        have_waiters = 1;
      }

      if (have_waiters) {
        // Wake up all the waiters atomically.
        ReleaseSemaphore(threadSemaphore, numThreads, 0);

        ReleaseSRWLockExclusive(&numThreadsConditionLock);

        // Wait for all the awakened threads to acquire the counting
        // semaphore.
        WaitForSingleObject(broadcastDone, INFINITE);
        // This assignment is okay, even without the <numThreadsConditionMutex>
        // held
        // because no other waiter threads can wake up to access it.
        isBroadcasting = 0;
      } else {
        ReleaseSRWLockExclusive(&numThreadsConditionLock);
      }
    }

    // Number of waiting threads.
    int numThreads;

    // Serialize access to <numThreads>.
    SRWLOCK numThreadsConditionLock;

    // Semaphore used to queue up threads waiting for the condition to
    // become signaled.
    HANDLE threadSemaphore;

    // An auto-reset event used by the broadcast/signal thread to wait
    // for all the waiting thread(s) to wake up and be released from the
    // semaphore.
    HANDLE broadcastDone;

    // Keeps track of whether we were broadcasting or signaling.  This
    // allows us to optimize the code if we're just signaling.
    std::size_t isBroadcasting;
  };

  std::unique_ptr<Impl> m_impl;
};

struct RecursiveMutexImpl {
  RecursiveMutexImpl() {
    InitializeCriticalSection(&criticalSection);
  }

  ~RecursiveMutexImpl() {
    DeleteCriticalSection(&criticalSection);
  }

  void lock() {
    EnterCriticalSection(&criticalSection);
  }

  void unlock() {
    LeaveCriticalSection(&criticalSection);
  }

  bool tryLock() {
    return TryEnterCriticalSection(&criticalSection);
  }

  CRITICAL_SECTION criticalSection;
};

void Thread::sleepPrecise(unsigned msecs) {
  std::int64_t now = Time::monotonicMilliseconds();
  std::int64_t deadline = now + msecs;

  while (deadline - now > 10) {
    Sleep(deadline - now - 10);
    now = Time::monotonicMilliseconds();
  }

  while (deadline > now) {
    if (deadline - now >= 2)
      Sleep((deadline - now) / 2);
    else
      Sleep(0);
    now = Time::monotonicMilliseconds();
  }
}

void Thread::sleep(unsigned msecs) {
  Sleep(msecs);
}

void Thread::yield() {
  SwitchToThread();
}

std::expected<unsigned, StarException> Thread::numberOfProcessors() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  if (info.dwNumberOfProcessors == 0)
    return std::unexpected(StarException("Could not determine number of CPUs online"));
  return info.dwNumberOfProcessors;
}

Thread::Thread(String const& name) {
  m_impl.reset(new ThreadImpl([this]() { return run(); }, name));
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
  return m_impl->thread == INVALID_HANDLE_VALUE;
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
  return !m_impl || m_impl->thread == INVALID_HANDLE_VALUE;
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

Mutex::Mutex() : m_impl(new MutexImpl()) {}

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

ConditionVariable::ConditionVariable() : m_impl(new ConditionVariableImpl()) {}

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

RecursiveMutex::RecursiveMutex() : m_impl(new RecursiveMutexImpl()) {}

RecursiveMutex::RecursiveMutex(RecursiveMutex&&) = default;

RecursiveMutex::~RecursiveMutex() {}

RecursiveMutex& RecursiveMutex::operator=(RecursiveMutex&&) = default;

#ifdef STAR_MUTEX_LOG
void RecursiveMutex::setLogged(bool logged) {
  m_log = logged;
}
#endif

void RecursiveMutex::lock() {
  #ifdef STAR_MUTEX_LOG
  if (m_log)
    printStack("RecursiveMutex lock waiting");
  #endif
  m_impl->lock();
  #ifdef STAR_MUTEX_LOG
  if (m_log) {
    auto str = strf("RecursiveMutex locked, LockCount {}", m_impl->criticalSection.LockCount);
    printStack(str.c_str());
  }
  #endif
}

bool RecursiveMutex::tryLock() {
  #ifdef STAR_MUTEX_LOG
  if (m_log)
    printStack("RecursiveMutex tryLock waiting");
  #endif
  bool result = m_impl->tryLock();
  #ifdef STAR_MUTEX_LOG
  if (result && m_log) {
    auto str = strf("RecursiveMutex tryLock success, LockCount {}", m_impl->criticalSection.LockCount);
    printStack(str.c_str());
  }
  #endif
  return result;
}

void RecursiveMutex::unlock() {
  m_impl->unlock();
  #ifdef STAR_MUTEX_LOG
  if (m_log) {
    auto str = strf("RecursiveMutex unlocked, LockCount {}", m_impl->criticalSection.LockCount);
    printStack(str.c_str());
  }
  #endif
}

}
