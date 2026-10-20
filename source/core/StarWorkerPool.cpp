import std;
import star.unique_function;
import star.string;
import star.format;
import star.thread;
import star.worker_pool;
import star.list;

namespace Star {

bool WorkerPoolHandle::done() const {
  return m_impl->done.load(std::memory_order_acquire);
}

bool WorkerPoolHandle::wait(unsigned millis) const {
  MutexLocker locker(m_impl->mutex);

  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(millis);
  while (!m_impl->done.load(std::memory_order_acquire) && millis != 0) {
    auto remaining = deadline - std::chrono::steady_clock::now();
    if (remaining <= decltype(remaining)::zero())
      break;
    m_impl->condition.wait(m_impl->mutex,
        static_cast<unsigned>(std::chrono::ceil<std::chrono::milliseconds>(remaining).count()));
  }

  if (m_impl->exception)
    std::rethrow_exception(m_impl->exception);

  return m_impl->done;
}

bool WorkerPoolHandle::poll() const {
  return wait(0);
}

void WorkerPoolHandle::finish() const {
  m_impl->done.wait(false, std::memory_order_acquire);

  if (m_impl->exception)
    std::rethrow_exception(m_impl->exception);

  return;
}

WorkerPoolHandle::Impl::Impl() : done(false) {}

WorkerPoolHandle::WorkerPoolHandle(std::shared_ptr<Impl> impl) : m_impl(std::move(impl)) {}

WorkerPool::WorkerPool(String name) : m_name(std::move(name)) {}

WorkerPool::WorkerPool(String name, unsigned threadCount) : WorkerPool(std::move(name)) {
  start(threadCount);
}

WorkerPool::~WorkerPool() {
  stop();
}


void WorkerPool::start(unsigned threadCount) {
  if (threadCount == 0) {
    stop();
    return;
  }

  MutexLocker threadLock(m_threadMutex);
  {
    MutexLocker workLock(m_workMutex);
    for (auto const& workerThread : m_workerThreads)
      workerThread->shouldStop = true;
    m_workCondition.broadcast();
  }
  joinWorkers();

  try {
    MutexLocker workLock(m_workMutex);
    m_workerThreads.reserve(threadCount);
    for (unsigned i = 0; i < threadCount; ++i)
      m_workerThreads.append(std::make_unique<WorkerThread>(this));
    m_acceptingWork = true;
  } catch (...) {
    stopWorkers();
    throw;
  }
}

void WorkerPool::stop() {
  MutexLocker threadLock(m_threadMutex);
  stopWorkers();
}

void WorkerPool::stopWorkers() {
  Deque<UniqueFunction<void(std::exception_ptr)>> cancelledWork;
  {
    MutexLocker workLock(m_workMutex);
    m_acceptingWork = false;
    for (auto const& workerThread : m_workerThreads)
      workerThread->shouldStop = true;
    cancelledWork = std::move(m_pendingWork);
    m_workCondition.broadcast();
  }

  if (!cancelledWork.empty()) {
    auto cancelled = std::make_exception_ptr(WorkerPoolException("Worker pool stopped before work began"));
    for (auto& work : cancelledWork)
      work(cancelled);
  }
  joinWorkers();
}

void WorkerPool::joinWorkers() {
  for (auto const& workerThread : m_workerThreads)
    workerThread->join();
  MutexLocker workLock(m_workMutex);
  m_workerThreads.clear();
}

void WorkerPool::finish() {
  MutexLocker threadLock(m_threadMutex);
  MutexLocker workLock(m_workMutex);
  m_acceptingWork = false;
  m_finishingThread = std::this_thread::get_id();
  while (!m_pendingWork.empty()) {
    auto work = m_pendingWork.takeFirst();
    workLock.unlock();
    work({});
    Thread::yield();
    workLock.lock();
  }
  m_finishingThread = {};
  for (auto const& workerThread : m_workerThreads)
    workerThread->shouldStop = true;
  m_workCondition.broadcast();
  workLock.unlock();
  joinWorkers();
}

WorkerPoolHandle WorkerPool::addWork(UniqueFunction<void()> work) {
  // Construct a worker pool handle and wrap the work to signal the handle when
  // finished.  Set the result to empty string if successful and to the content
  // of the exception if an exception is thrown.
  auto workerPoolHandleImpl = std::make_shared<WorkerPoolHandle::Impl>();
  queueWork([workerPoolHandleImpl, work = std::move(work)](std::exception_ptr cancelled) mutable {
    try {
      if (cancelled)
        std::rethrow_exception(cancelled);
      work();
      MutexLocker handleLocker(workerPoolHandleImpl->mutex);
      workerPoolHandleImpl->done.store(true, std::memory_order_release);
      workerPoolHandleImpl->done.notify_all();
      workerPoolHandleImpl->condition.broadcast();
    } catch (...) {
      MutexLocker handleLocker(workerPoolHandleImpl->mutex);
      workerPoolHandleImpl->exception = std::current_exception();
      workerPoolHandleImpl->done.store(true, std::memory_order_release);
      workerPoolHandleImpl->done.notify_all();
      workerPoolHandleImpl->condition.broadcast();
    }
  });

  return workerPoolHandleImpl;
}

std::size_t WorkerPool::getWorkerCount() const {
  MutexLocker workLock(m_workMutex);
  return m_workerThreads.size();
}

bool WorkerPool::isWorkerThread() const {
  auto id = std::this_thread::get_id();
  MutexLocker workLock(m_workMutex);
  if (m_finishingThread == id)
    return true;
  for (auto const& workerThread : m_workerThreads) {
    if (workerThread->id == id)
      return true;
  }
  return false;
}

WorkerPool::WorkerThread::WorkerThread(WorkerPool* parent)
  : Thread(strf("WorkerThread for WorkerPool '{}'", parent->m_name)),
    parent(parent),
    shouldStop(false),
    waiting(false) {
  start();
}

WorkerPool::WorkerThread::~WorkerThread() {
  join();
}

void WorkerPool::WorkerThread::run() {
  MutexLocker workLock(parent->m_workMutex);
  id = std::this_thread::get_id();
  while (!shouldStop) {
    if (parent->m_pendingWork.empty()) {
      waiting = true;
      parent->m_workCondition.wait(parent->m_workMutex);
      waiting = false;
    } else {
      auto work = parent->m_pendingWork.takeFirst();
      workLock.unlock();
      work({});
      workLock.lock();
    }
  }
}

void WorkerPool::queueWork(UniqueFunction<void(std::exception_ptr)> work) {
  MutexLocker workLock(m_workMutex);
  if (m_acceptingWork) {
    m_pendingWork.append(std::move(work));
    m_workCondition.signal();
  } else {
    workLock.unlock();
    work(std::make_exception_ptr(WorkerPoolException("Cannot submit work to a stopped worker pool")));
  }
}

}
