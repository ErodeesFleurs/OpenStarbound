module;
import std;
import star.unique_function;
import star.string;
import star.list;
import star.maybe;
import star.format;
import star.thread;

namespace Star {

struct WorkerPoolExceptionTag {
  static constexpr char const* name() { return "WorkerPoolException"; }
};
using WorkerPoolException = StarError<WorkerPoolExceptionTag, StarException>;

class WorkerPool;

// Shareable handle for a WorkerPool computation that does not produce any
// value.
class WorkerPoolHandle {
public:
  // Returns true if the work is completed (either due to error or actual
  // completion, will not re-throw)
  bool done() const;

  // Waits up to given millis for the computation to finish.  Returns true if
  // the computation finished within the allotted time, false otherwise.  If
  // the computation is finished but it threw an exception, it will be
  // re-thrown here.
  bool wait(unsigned millis) const;

  // synonym for wait(0)
  bool poll() const;

  // Wait until the computation finishes completely.  If the computation threw
  // an exception it will be re-thrown by this method.
  void finish() const;

private:
  friend WorkerPool;

  struct Impl {
    Impl();

    Mutex mutex;
    ConditionVariable condition;
    std::atomic<bool> done;
    std::exception_ptr exception;
  };

  WorkerPoolHandle(std::shared_ptr<Impl> impl);

  std::shared_ptr<Impl> m_impl;
};

// Shareable handle for a WorkerPool computation that produces a value.
template <typename ResultType>
class WorkerPoolPromise {
public:
  // Returns true if the work is completed (either due to error or actual
  // completion, will not re-throw)
  bool done() const;

  // Waits for the given amount of time for the work to be completed.  If the
  // work is completed, returns true.  If the producer function throws for any
  // reason, this method will re-throw the exception.  If millis is zero, does
  // not wait at all simply polls to see if the computation is finished.
  bool wait(unsigned millis) const;

  // synonym for wait(0)
  bool poll() const;

  // Blocks until the work is done, and returns the result.  May be called
  // multiple times to access the result.  If the computation threw
  // an exception it will be re-thrown by this method.
  ResultType& get();
  ResultType const& get() const;

private:
  friend WorkerPool;

  struct Impl {
    Mutex mutex;
    ConditionVariable condition;
    Maybe<ResultType> result;
    std::exception_ptr exception;
    std::atomic<bool> done{false};
  };

  WorkerPoolPromise(std::shared_ptr<Impl> impl);

  std::shared_ptr<Impl> m_impl;
};

class WorkerPool {
public:
  // Creates a pool that queues work until start() or finish().
  WorkerPool(String name);
  // Creates a started pool
  WorkerPool(String name, unsigned threadCount);
  ~WorkerPool();

  // Workers borrow this object's locks and queues; its address must stay stable.
  WorkerPool(WorkerPool const&) = delete;
  WorkerPool& operator=(WorkerPool const&) = delete;
  WorkerPool(WorkerPool&&) = delete;
  WorkerPool& operator=(WorkerPool&&) = delete;

  // Start the thread pool with the given thread count range, or if it is
  // already started, reconfigure the thread counts.
  void start(unsigned threadCount);

  // Stop accepting work, fail queued jobs, and wait for running jobs to finish.
  // Submissions after stop return an already-failed handle or promise until start.
  void stop();

  // Stop accepting work, complete all accepted jobs, then stop the threads.
  // Neither finish nor stop may be called from one of this pool's jobs.
  void finish();

  // Add the given work to the pool and return a handle for the work.  It not
  // required that the caller of this method hold on to the worker handle, the
  // work will be managed and completed regardless of the WorkerPoolHandle
  // lifetime.
  WorkerPoolHandle addWork(UniqueFunction<void()> work);

  // Like addWork, but the worker is expected to produce some result.  The
  // returned promise can be used to get this return value once the producer is
  // complete.
  template <typename ResultType>
  WorkerPoolPromise<ResultType> addProducer(UniqueFunction<ResultType()> producer);

  // Returns the current number of worker threads.
  std::size_t getWorkerCount() const;

  // Detect nested work without a process-global or thread-local current pool.
  bool isWorkerThread() const;

private:
  class WorkerThread : public Thread {
  public:
    // Starts automatically
    WorkerThread(WorkerPool* parent);
    ~WorkerThread();

    void run() override;

    WorkerPool* parent;
    std::atomic<bool> shouldStop;
    std::atomic<bool> waiting;
    std::thread::id id;
  };

  // A non-null exception cancels a queued job without invoking its producer.
  void queueWork(UniqueFunction<void(std::exception_ptr)> work);

  // Lifecycle helpers require m_threadMutex, but never join with m_workMutex held.
  void stopWorkers();
  void joinWorkers();

  String m_name;
  Mutex m_threadMutex;
  List<std::unique_ptr<WorkerThread>> m_workerThreads;

  mutable Mutex m_workMutex;
  ConditionVariable m_workCondition;
  Deque<UniqueFunction<void(std::exception_ptr)>> m_pendingWork;
  std::thread::id m_finishingThread;
  bool m_acceptingWork = true;
};

template <typename ResultType>
bool WorkerPoolPromise<ResultType>::done() const {
  return m_impl->done.load(std::memory_order_acquire);
}

template <typename ResultType>
bool WorkerPoolPromise<ResultType>::wait(unsigned millis) const {
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

  if (m_impl->result)
    return true;

  return false;
}

template <typename ResultType>
bool WorkerPoolPromise<ResultType>::poll() const {
  return wait(0);
}

template <typename ResultType>
ResultType& WorkerPoolPromise<ResultType>::get() {
  m_impl->done.wait(false, std::memory_order_acquire);

  if (m_impl->exception)
    std::rethrow_exception(m_impl->exception);

  return *m_impl->result;
}

template <typename ResultType>
ResultType const& WorkerPoolPromise<ResultType>::get() const {
  return const_cast<WorkerPoolPromise*>(this)->get();
}

template <typename ResultType>
WorkerPoolPromise<ResultType>::WorkerPoolPromise(std::shared_ptr<Impl> impl)
  : m_impl(std::move(impl)) {}

template <typename ResultType>
WorkerPoolPromise<ResultType> WorkerPool::addProducer(UniqueFunction<ResultType()> producer) {
  // Construct a worker pool promise and wrap the producer to signal the
  // promise when finished.
  auto workerPoolPromiseImpl = std::make_shared<typename WorkerPoolPromise<ResultType>::Impl>();
  queueWork([workerPoolPromiseImpl, producer = std::move(producer)](std::exception_ptr cancelled) mutable {
    try {
      if (cancelled)
        std::rethrow_exception(cancelled);
      auto result = producer();
      MutexLocker promiseLocker(workerPoolPromiseImpl->mutex);
      workerPoolPromiseImpl->result = std::move(result);
      workerPoolPromiseImpl->done.store(true, std::memory_order_release);
      workerPoolPromiseImpl->done.notify_all();
      workerPoolPromiseImpl->condition.broadcast();
    } catch (...) {
      MutexLocker promiseLocker(workerPoolPromiseImpl->mutex);
      workerPoolPromiseImpl->exception = std::current_exception();
      workerPoolPromiseImpl->done.store(true, std::memory_order_release);
      workerPoolPromiseImpl->done.notify_all();
      workerPoolPromiseImpl->condition.broadcast();
    }
  });

  return workerPoolPromiseImpl;
}

}

export module star.worker_pool;

export namespace Star {
  using ::Star::WorkerPoolExceptionTag;
  using ::Star::WorkerPoolException;
  using ::Star::WorkerPoolHandle;
  using ::Star::WorkerPoolPromise;
  using ::Star::WorkerPool;
}
