#include "StarThread.hpp"
#include "StarLogging.hpp"

import std;

namespace Star {

struct Thread::ThreadImpl {
  std::function<void()> function;
  String name;
  std::optional<std::thread> thread;
  std::atomic<bool> running{false};
  bool joined{true};
  mutable std::mutex mutex;

  ThreadImpl(std::function<void()> f, String n)
    : function(std::move(f)), name(std::move(n)) {}

  ~ThreadImpl() {
    // If the thread was started but never joined, we must detach to avoid std::terminate.
    // The Starbound API contract says join() MUST be called, but we handle it safely.
    if (thread && thread->joinable())
      thread->detach();
  }
};

struct ThreadFunction<void>::ThreadFunctionImpl {
  std::function<void()> function;
  String name;
  std::optional<std::thread> thread;
  std::atomic<bool> running{false};
  bool joined{true};
  std::exception_ptr exception;
  mutable std::mutex mutex;

  ThreadFunctionImpl(std::function<void()> f, String n)
    : function(std::move(f)), name(std::move(n)) {}

  ~ThreadFunctionImpl() {
    // ThreadFunction usually expects to be finished/joined.
    if (thread && thread->joinable())
      thread->join();
  }
};

void Thread::sleepPrecise(unsigned millis) {
  auto const start = std::chrono::steady_clock::now();
  auto const duration = std::chrono::milliseconds(millis);
  auto const end = start + duration;

  // Sleep for the bulk of the time
  if (millis > 10)
    std::this_thread::sleep_for(duration - std::chrono::milliseconds(10));

  // Spin/yield for the remainder to maintain precision
  while (std::chrono::steady_clock::now() < end) {
    std::this_thread::yield();
  }
}

Thread::Thread(String const& name) {
  m_impl = std::make_unique<ThreadImpl>([this]() -> void { run(); }, name);
}

Thread::Thread(Thread&&) noexcept = default;

Thread::~Thread() = default;

auto Thread::operator=(Thread&&) noexcept -> Thread& = default;

auto Thread::start() -> bool {
  std::lock_guard lock(m_impl->mutex);
  if (!m_impl->joined)
    return false;

  m_impl->joined = false;
  m_impl->running = true;

  m_impl->thread.emplace([impl = m_impl.get()]() -> void {
    try {
      impl->function();
    } catch (std::exception const& e) {
      Logger::error("Exception caught in Thread {}: {}", impl->name, outputException(e, true));
    } catch (...) {
      Logger::error("Unknown exception caught in Thread {}", impl->name);
    }
    impl->running = false;
  });
  return true;
}

auto Thread::join() -> bool {
  std::lock_guard lock(m_impl->mutex);
  if (m_impl->joined)
    return false;

  if (m_impl->thread && m_impl->thread->joinable())
    m_impl->thread->join();

  m_impl->joined = true;
  m_impl->thread.reset();
  return true;
}

auto Thread::isJoined() const -> bool {
  std::lock_guard lock(m_impl->mutex);
  return m_impl->joined;
}

auto Thread::isRunning() const -> bool {
  return m_impl->running;
}

auto Thread::name() const -> String {
  return m_impl->name;
}

// ThreadFunction<void> implementation

ThreadFunction<void>::ThreadFunction() = default;

ThreadFunction<void>::ThreadFunction(ThreadFunction&&) noexcept = default;

ThreadFunction<void>::ThreadFunction(std::function<void()> function, String const& name) {
  m_impl = std::make_unique<ThreadFunctionImpl>(std::move(function), name);
  m_impl->joined = false;
  m_impl->running = true;
  m_impl->thread.emplace([impl = m_impl.get()]() -> void {
    try {
      impl->function();
    } catch (...) {
      impl->exception = std::current_exception();
    }
    impl->running = false;
  });
}

ThreadFunction<void>::~ThreadFunction() {
  // We don't want to throw in destructor if we can avoid it, but finish() may throw.
  // We'll catch and ignore if it throws during destruction to avoid terminate().
  try {
    finish();
  } catch (...) {}
}

auto ThreadFunction<void>::operator=(ThreadFunction&&) noexcept -> ThreadFunction<void>& = default;

void ThreadFunction<void>::finish() {
  if (m_impl) {
    if (!m_impl->joined) {
      if (m_impl->thread && m_impl->thread->joinable())
        m_impl->thread->join();
      m_impl->joined = true;
    }

    if (m_impl->exception) {
      std::exception_ptr ex = std::move(m_impl->exception);
      m_impl->exception = nullptr;
      std::rethrow_exception(ex);
    }
  }
}

auto ThreadFunction<void>::isFinished() const -> bool {
  return !m_impl || m_impl->joined;
}

auto ThreadFunction<void>::isRunning() const -> bool {
  return m_impl && m_impl->running;
}

auto ThreadFunction<void>::name() const -> String {
  return m_impl ? m_impl->name : "";
}

auto ReadLocker::tryLock() -> bool {
  if (!m_locked)
    m_locked = m_lock.tryReadLock();
  return m_locked;
}
}// namespace Star
