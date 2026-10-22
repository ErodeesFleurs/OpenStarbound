import std;
import star.unique_function;
import star.string;
import star.format;
import star.output_proxy;
import star.thread;
import star.worker_pool;
import star.logging;

namespace Star {

bool WorkerPoolHandle::done() const {
  return m_impl->done.load(std::memory_order_acquire);
}

std::expected<bool, StarException> WorkerPoolHandle::wait(unsigned millis) const {
  MutexLocker locker(m_impl->mutex);
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(millis);
  while (!m_impl->done.load(std::memory_order_acquire) && millis != 0) {
    auto remaining = deadline - std::chrono::steady_clock::now();
    if (remaining <= decltype(remaining)::zero())
      break;
    m_impl->condition.wait(m_impl->mutex,
        static_cast<unsigned>(std::chrono::ceil<std::chrono::milliseconds>(remaining).count()));
  }
  if (m_impl->error)
    return std::unexpected(*m_impl->error);
  return m_impl->done.load(std::memory_order_acquire);
}

std::expected<bool, StarException> WorkerPoolHandle::poll() const {
  return wait(0);
}

std::expected<void, StarException> WorkerPoolHandle::finish() const {
  m_impl->done.wait(false, std::memory_order_acquire);
  if (m_impl->error)
    return std::unexpected(*m_impl->error);
  return {};
}

WorkerPoolHandle::WorkerPoolHandle(std::shared_ptr<Impl> impl) : m_impl(std::move(impl)) {}

WorkerPool::WorkerPool(String name) : m_name(std::move(name)) {}

std::expected<std::unique_ptr<WorkerPool>, StarException> WorkerPool::create(String name, unsigned threadCount) {
  auto result = std::make_unique<WorkerPool>(std::move(name));
  auto started = result->start(threadCount);
  if (!started)
    return std::unexpected(std::move(started.error()));
  return result;
}

WorkerPool::~WorkerPool() {
  // Unlike explicit stop(), final cleanup must attempt every independent join,
  // including when destruction itself violates the no-own-job lifecycle rule.
  MutexLocker threadLock(m_threadMutex);
  auto result = stopWorkers();
  if (!result)
    EmergencyLog::error("Worker pool {} failed during destruction: {}", m_name, outputException(ExceptionDiagnostic(result.error()), true));
  if (!m_workerThreads.empty())
    fatalError("Cannot destroy a worker pool with unjoined workers", false);
}

std::expected<void, StarException> WorkerPool::start(unsigned threadCount) {
  if (isWorkerThread())
    return std::unexpected(WorkerPoolException("Cannot reconfigure a worker pool from one of its jobs"));
  if (threadCount == 0)
    return stop();

  MutexLocker threadLock(m_threadMutex);
  {
    MutexLocker workLock(m_workMutex);
    for (auto const& worker : m_workerThreads)
      worker->shouldStop = true;
    m_workCondition.broadcast();
  }
  auto joined = joinWorkers();
  if (!joined)
    return std::unexpected(std::move(joined.error()));

  std::optional<StarException> failure;
  {
    MutexLocker workLock(m_workMutex);
    m_workerThreads.reserve(threadCount);
    for (unsigned i = 0; i < threadCount; ++i) {
      auto worker = std::make_unique<WorkerThread>(this);
      auto started = worker->start();
      if (!started) {
        failure.emplace(std::move(started.error()));
        break;
      }
      m_workerThreads.push_back(std::move(worker));
    }
    if (!failure)
      m_acceptingWork = true;
  }
  if (failure) {
    auto stopped = stopWorkers();
    if (!stopped)
      EmergencyLog::error("Worker pool {} rollback failed: {}", m_name, outputException(ExceptionDiagnostic(stopped.error()), true));
    return std::unexpected(std::move(*failure));
  }
  return {};
}

std::expected<void, StarException> WorkerPool::stop() {
  if (isWorkerThread())
    return std::unexpected(WorkerPoolException("Cannot stop a worker pool from one of its jobs"));
  MutexLocker threadLock(m_threadMutex);
  return stopWorkers();
}

std::expected<void, StarException> WorkerPool::stopWorkers() {
  std::deque<QueuedWork> cancelledWork;
  {
    MutexLocker workLock(m_workMutex);
    m_acceptingWork = false;
    for (auto const& worker : m_workerThreads)
      worker->shouldStop = true;
    cancelledWork = std::move(m_pendingWork);
    m_pendingWork.clear();
    m_workCondition.broadcast();
  }
  std::optional<StarException> failure;
  std::optional<StarException> cancelled;
  if (!cancelledWork.empty())
    cancelled.emplace(WorkerPoolException("Worker pool stopped before work began"));
  for (auto& work : cancelledWork) {
    auto result = work(cancelled);
    if (!result) {
      if (!failure)
        failure.emplace(std::move(result.error()));
      else
        EmergencyLog::error("Worker pool {} cancellation failed: {}", m_name, outputException(ExceptionDiagnostic(result.error()), true));
    }
  }
  auto joined = joinWorkers();
  if (!joined) {
    if (failure)
      EmergencyLog::error("Worker pool {} cancellation also failed: {}", m_name, outputException(ExceptionDiagnostic(*failure), true));
    return std::unexpected(std::move(joined.error()));
  }
  if (failure)
    return std::unexpected(std::move(*failure));
  return {};
}

std::expected<void, StarException> WorkerPool::joinWorkers() {
  std::optional<StarException> failure;
  for (auto const& worker : m_workerThreads) {
    auto result = worker->join();
    if (!result) {
      if (!failure)
        failure.emplace(std::move(result.error()));
      else
        EmergencyLog::error("Worker pool {} join failed: {}", m_name, outputException(ExceptionDiagnostic(result.error()), true));
    }
  }
  {
    MutexLocker workLock(m_workMutex);
    std::erase_if(m_workerThreads, [](auto const& worker) { return worker->isJoined(); });
  }
  if (failure)
    return std::unexpected(std::move(*failure));
  return {};
}

std::expected<void, StarException> WorkerPool::finish() {
  if (isWorkerThread())
    return std::unexpected(WorkerPoolException("Cannot finish a worker pool from one of its jobs"));
  MutexLocker threadLock(m_threadMutex);
  MutexLocker workLock(m_workMutex);
  m_acceptingWork = false;
  m_finishingThread = std::this_thread::get_id();
  std::optional<StarException> failure;
  while (!m_pendingWork.empty()) {
    auto work = std::move(m_pendingWork.front());
    m_pendingWork.pop_front();
    workLock.unlock();
    auto result = work({});
    if (!result) {
      if (!failure)
        failure.emplace(std::move(result.error()));
      else
        EmergencyLog::error("Worker pool {} completion failed: {}", m_name, outputException(ExceptionDiagnostic(result.error()), true));
    }
    Thread::yield();
    workLock.lock();
  }
  m_finishingThread = {};
  for (auto const& worker : m_workerThreads)
    worker->shouldStop = true;
  m_workCondition.broadcast();
  workLock.unlock();
  auto joined = joinWorkers();
  if (!joined) {
    if (failure)
      EmergencyLog::error("Worker pool {} completion also failed: {}", m_name, outputException(ExceptionDiagnostic(*failure), true));
    return std::unexpected(std::move(joined.error()));
  }
  if (failure)
    return std::unexpected(std::move(*failure));
  return {};
}

WorkerPoolHandle WorkerPool::addWork(UniqueFunction<std::expected<void, StarException>()> work) {
  auto state = std::make_shared<WorkerPoolHandle::Impl>();
  queueWork([state, work = std::move(work)](std::optional<StarException> cancelled) mutable -> std::expected<void, StarException> {
    auto result = cancelled ? std::expected<void, StarException>(std::unexpected(std::move(*cancelled))) : work();
    MutexLocker lock(state->mutex);
    if (!result)
      state->error.emplace(std::move(result.error()));
    state->done.store(true, std::memory_order_release);
    state->done.notify_all();
    state->condition.broadcast();
    return {};
  });
  return state;
}

std::size_t WorkerPool::getWorkerCount() const {
  MutexLocker lock(m_workMutex);
  return m_workerThreads.size();
}

bool WorkerPool::isWorkerThread() const {
  auto id = std::this_thread::get_id();
  MutexLocker lock(m_workMutex);
  if (m_finishingThread == id)
    return true;
  for (auto const& worker : m_workerThreads) {
    if (worker->id == id)
      return true;
  }
  return false;
}

WorkerPool::WorkerThread::WorkerThread(WorkerPool* parent)
  : Thread(strf("WorkerThread for WorkerPool '{}'", parent->m_name)), parent(parent) {}

WorkerPool::WorkerThread::~WorkerThread() {
  auto result = join();
  if (!result)
    EmergencyLog::error("Worker thread failed during destruction: {}", outputException(ExceptionDiagnostic(result.error()), true));
}

std::expected<void, StarException> WorkerPool::WorkerThread::run() {
  MutexLocker workLock(parent->m_workMutex);
  id = std::this_thread::get_id();
  while (!shouldStop) {
    if (parent->m_pendingWork.empty()) {
      waiting = true;
      parent->m_workCondition.wait(parent->m_workMutex);
      waiting = false;
    } else {
      auto work = std::move(parent->m_pendingWork.front());
      parent->m_pendingWork.pop_front();
      workLock.unlock();
      auto result = work({});
      if (!result)
        return std::unexpected(std::move(result.error()));
      workLock.lock();
    }
  }
  return {};
}

void WorkerPool::queueWork(QueuedWork work) {
  MutexLocker workLock(m_workMutex);
  if (m_acceptingWork) {
    m_pendingWork.push_back(std::move(work));
    m_workCondition.signal();
  } else {
    workLock.unlock();
    auto result = work(WorkerPoolException("Cannot submit work to a stopped worker pool"));
    if (!result)
      EmergencyLog::error("Worker pool {} cancellation failed: {}", m_name, outputException(ExceptionDiagnostic(result.error()), true));
  }
}

}
