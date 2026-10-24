module;
import std;
import star.unique_function;
import star.string;
import star.format;
import star.thread;

namespace Star {

struct WorkerPoolExceptionTag {
  static constexpr char const* name() { return "WorkerPoolException"; }
};
using WorkerPoolException = StarError<WorkerPoolExceptionTag, StarException>;

class WorkerPool;

// A shared completion signal carrying an owning failure diagnostic.
class WorkerPoolHandle {
public:
  bool done() const;
  [[nodiscard]] std::expected<bool, StarException> wait(unsigned millis) const;
  [[nodiscard]] std::expected<bool, StarException> poll() const;
  [[nodiscard]] std::expected<void, StarException> finish() const;

private:
  friend WorkerPool;
  struct Impl {
    std::mutex mutex;
    std::condition_variable condition;
    std::atomic<bool> done{false};
    std::optional<StarException> error;
  };
  WorkerPoolHandle(std::shared_ptr<Impl> impl);
  std::shared_ptr<Impl> m_impl;
};

// Results stay owned by the shared promise and can be observed repeatedly.
template <typename ResultType>
class WorkerPoolPromise {
public:
  bool done() const;
  [[nodiscard]] std::expected<bool, StarException> wait(unsigned millis) const;
  [[nodiscard]] std::expected<bool, StarException> poll() const;
  [[nodiscard]] std::expected<ResultType*, StarException> get();
  [[nodiscard]] std::expected<ResultType const*, StarException> get() const;

private:
  friend WorkerPool;
  struct Impl {
    std::mutex mutex;
    std::condition_variable condition;
    std::optional<ResultType> result;
    std::optional<StarException> error;
    std::atomic<bool> done{false};
  };
  WorkerPoolPromise(std::shared_ptr<Impl> impl);
  std::shared_ptr<Impl> m_impl;
};

class WorkerPool {
public:
  // A queue-only pool can execute accepted jobs synchronously in finish().
  WorkerPool(String name);
  [[nodiscard]] static std::expected<std::unique_ptr<WorkerPool>, StarException> create(String name, unsigned threadCount);
  ~WorkerPool();

  // Worker threads borrow this object's locks and queues: its address is fixed.
  WorkerPool(WorkerPool const&) = delete;
  WorkerPool& operator=(WorkerPool const&) = delete;
  WorkerPool(WorkerPool&&) = delete;
  WorkerPool& operator=(WorkerPool&&) = delete;

  [[nodiscard]] std::expected<void, StarException> start(unsigned threadCount);
  // Stop accepting work, fail queued jobs, and join running jobs.
  [[nodiscard]] std::expected<void, StarException> stop();
  // Drain accepted work and join workers. Do not call from this pool's jobs.
  [[nodiscard]] std::expected<void, StarException> finish();
  WorkerPoolHandle addWork(UniqueFunction<std::expected<void, StarException>()> work);
  template <typename ResultType>
  WorkerPoolPromise<ResultType> addProducer(UniqueFunction<std::expected<ResultType, StarException>()> producer);
  std::size_t getWorkerCount() const;
  bool isWorkerThread() const;

private:
  class WorkerThread : public Thread {
  public:
    WorkerThread(WorkerPool* parent);
    ~WorkerThread();
    [[nodiscard]] std::expected<void, StarException> run() override;
    WorkerPool* parent;
    std::atomic<bool> shouldStop{false};
    std::atomic<bool> waiting{false};
    std::thread::id id;
  };

  using QueuedWork = UniqueFunction<std::expected<void, StarException>(std::optional<StarException>)>;
  void queueWork(QueuedWork work);
  // Require m_threadMutex; never join while m_workMutex is held.
  [[nodiscard]] std::expected<void, StarException> stopWorkers();
  [[nodiscard]] std::expected<void, StarException> joinWorkers();

  String m_name;
  std::mutex m_threadMutex;
  std::vector<std::unique_ptr<WorkerThread>> m_workerThreads;
  mutable std::mutex m_workMutex;
  std::condition_variable m_workCondition;
  std::deque<QueuedWork> m_pendingWork;
  std::thread::id m_finishingThread;
  bool m_acceptingWork = true;
};

template <typename ResultType>
bool WorkerPoolPromise<ResultType>::done() const {
  return m_impl->done.load(std::memory_order_acquire);
}

template <typename ResultType>
[[nodiscard]] std::expected<bool, StarException> WorkerPoolPromise<ResultType>::wait(unsigned millis) const {
  std::unique_lock locker(m_impl->mutex);
  if (millis != 0) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(millis);
    m_impl->condition.wait_until(locker, deadline,
        [this] { return m_impl->done.load(std::memory_order_acquire); });
  }
  if (m_impl->error)
    return std::unexpected(*m_impl->error);
  return m_impl->done.load(std::memory_order_acquire);
}

template <typename ResultType>
[[nodiscard]] std::expected<bool, StarException> WorkerPoolPromise<ResultType>::poll() const {
  return wait(0);
}

template <typename ResultType>
[[nodiscard]] std::expected<ResultType*, StarException> WorkerPoolPromise<ResultType>::get() {
  m_impl->done.wait(false, std::memory_order_acquire);
  if (m_impl->error)
    return std::unexpected(*m_impl->error);
  return &*m_impl->result;
}

template <typename ResultType>
[[nodiscard]] std::expected<ResultType const*, StarException> WorkerPoolPromise<ResultType>::get() const {
  auto result = const_cast<WorkerPoolPromise*>(this)->get();
  if (!result)
    return std::unexpected(std::move(result.error()));
  return *result;
}

template <typename ResultType>
WorkerPoolPromise<ResultType>::WorkerPoolPromise(std::shared_ptr<Impl> impl)
  : m_impl(std::move(impl)) {}

template <typename ResultType>
WorkerPoolPromise<ResultType> WorkerPool::addProducer(UniqueFunction<std::expected<ResultType, StarException>()> producer) {
  auto state = std::make_shared<typename WorkerPoolPromise<ResultType>::Impl>();
  queueWork([state, producer = std::move(producer)](std::optional<StarException> cancelled) mutable -> std::expected<void, StarException> {
    auto result = cancelled ? std::expected<ResultType, StarException>(std::unexpected(std::move(*cancelled))) : producer();
    std::lock_guard lock(state->mutex);
    if (result)
      state->result.emplace(std::move(*result));
    else
      state->error.emplace(std::move(result.error()));
    state->done.store(true, std::memory_order_release);
    state->done.notify_all();
    state->condition.notify_all();
    return {};
  });
  return state;
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
