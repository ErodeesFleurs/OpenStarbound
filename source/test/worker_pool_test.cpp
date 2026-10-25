import std;
import star.thread;
import star.list;
import star.worker_pool;
import star.unique_function;
import star.format;

#include "gtest/gtest.h"

using namespace Star;

TEST(WorkerPoolTest, All) {
  int counter = 0;
  std::mutex counterMutex;

  auto incCounter = [&counter, &counterMutex]() -> std::expected<void, StarException> {
    Thread::sleep(100);
    std::lock_guard locker(counterMutex);
    counter += 1;
    return {};
  };

  Deque<WorkerPoolHandle> handles;

  WorkerPool workerPool("WorkerPoolTest");
  for (std::size_t i = 0; i < 10; ++i)
    handles.append(workerPool.addWork(incCounter));

  ASSERT_TRUE(workerPool.start(10));

  for (std::size_t i = 0; i < 90; ++i)
    handles.append(workerPool.addWork(incCounter));

  while (handles.size() > 20) {
    auto handle = handles.takeFirst();
    ASSERT_TRUE(handle);
    ASSERT_TRUE(handle->finish());
  }

  ASSERT_TRUE(workerPool.finish());

  EXPECT_EQ(counter, 100);
}

TEST(WorkerPoolTest, MoveOnlyTasksPublishResultsToAllWaiters) {
  WorkerPool pool("move-only");
  ASSERT_TRUE(pool.start(2));
  std::atomic<bool> release{false};
  auto result = pool.addProducer<int>([value = std::make_unique<int>(42), &release]() -> std::expected<int, StarException> {
    release.wait(false);
    return *value;
  });
  auto waiting = result.wait(1);
  EXPECT_TRUE(waiting);
  if (waiting)
    EXPECT_FALSE(*waiting);
  std::array<int, 4> observations{};
  std::vector<std::thread> waiters;
  for (std::size_t i = 0; i < observations.size(); ++i) {
    waiters.emplace_back([&, i] {
      auto value = result.get();
      ASSERT_TRUE(value);
      observations[i] = **value;
    });
  }
  release.store(true);
  release.notify_all();
  for (auto& waiter : waiters)
    waiter.join();
  for (auto value : observations)
    EXPECT_EQ(value, 42);
  EXPECT_TRUE(result.done());
  auto value = result.get();
  ASSERT_TRUE(value);
  EXPECT_EQ(**value, 42);
  auto empty = pool.addWork(UniqueFunction<std::expected<void, StarException>()>{});
  auto emptyFinish = empty.finish();
  ASSERT_FALSE(emptyFinish);
  auto repeatedFinish = empty.finish();
  ASSERT_FALSE(repeatedFinish);
  EXPECT_STREQ(repeatedFinish.error().what(), emptyFinish.error().what());
  auto emptyProducer = pool.addProducer<int>(UniqueFunction<std::expected<int, StarException>()>{});
  auto emptyValue = emptyProducer.get();
  ASSERT_FALSE(emptyValue);
  auto recovery = pool.addProducer<int>([]() -> std::expected<int, StarException> { return 19; });
  auto recoveredValue = recovery.get();
  ASSERT_TRUE(recoveredValue);
  EXPECT_EQ(**recoveredValue, 19);
  ASSERT_TRUE(pool.finish());
}

namespace {
struct StoredLvalueTask {
  std::unique_ptr<int> value;
  int* observed;
  explicit operator bool() const { return false; }
  std::expected<void, StarException> operator()() & {
    *observed = *value;
    return {};
  }
  std::expected<void, StarException> operator()() && = delete;
};

struct StoredLvalueProducer {
  std::unique_ptr<int> value;
  std::expected<int, StarException> operator()() & { return *value; }
  std::expected<int, StarException> operator()() && = delete;
};

template <typename Callable>
concept AcceptsWork = requires(WorkerPool& pool, Callable&& callable) {
  pool.addWork(std::forward<Callable>(callable));
};

template <typename Callable>
concept AcceptsIntProducer = requires(WorkerPool& pool, Callable&& callable) {
  pool.addProducer<int>(std::forward<Callable>(callable));
};

static_assert(AcceptsWork<StoredLvalueTask>);
static_assert(!AcceptsWork<StoredLvalueTask&>);
static_assert(AcceptsIntProducer<StoredLvalueProducer>);
static_assert(!AcceptsWork<decltype([] {})>);
static_assert(!AcceptsWork<StoredLvalueProducer>);
static_assert(!AcceptsIntProducer<decltype([] { return 1; })>);
}

TEST(WorkerPoolTest, ConcreteMoveOnlyCallablesAreInvokedAsStoredLvalues) {
  WorkerPool pool("stored-lvalue");
  int observed = 0;
  auto task = pool.addWork(StoredLvalueTask{std::make_unique<int>(23), &observed});
  auto producer = pool.addProducer<int>(StoredLvalueProducer{std::make_unique<int>(47)});
  ASSERT_TRUE(pool.finish());
  ASSERT_TRUE(task.finish());
  EXPECT_EQ(observed, 23);
  auto result = producer.get();
  ASSERT_TRUE(result);
  EXPECT_EQ(**result, 47);
}

TEST(WorkerPoolTest, NullFunctionPointersPublishRepeatableErrorsAndDrainContinues) {
  WorkerPool pool("null-functions");
  std::expected<void, StarException> (*work)() = nullptr;
  std::expected<int, StarException> (*producer)() = nullptr;
  auto task = pool.addWork(work);
  auto result = pool.addProducer<int>(producer);
  int observed = 0;
  auto recovery = pool.addWork([&]() -> std::expected<void, StarException> {
    observed = 1;
    return {};
  });
  ASSERT_TRUE(pool.finish());
  auto failure = task.finish();
  ASSERT_FALSE(failure);
  auto repeatedFailure = task.finish();
  ASSERT_FALSE(repeatedFailure);
  EXPECT_STREQ(failure.error().what(), repeatedFailure.error().what());
  auto value = result.get();
  ASSERT_FALSE(value);
  auto repeatedValue = result.get();
  ASSERT_FALSE(repeatedValue);
  EXPECT_STREQ(value.error().what(), repeatedValue.error().what());
  ASSERT_TRUE(recovery.finish());
  EXPECT_EQ(observed, 1);
}

TEST(WorkerPoolTest, CancelledConcreteCallablesNeverRun) {
  WorkerPool pool("cancelled-callables");
  int calls = 0;
  auto task = pool.addWork([value = std::make_unique<int>(1), &calls]() -> std::expected<void, StarException> {
    calls += *value;
    return {};
  });
  auto producer = pool.addProducer<int>([value = std::make_unique<int>(2), &calls]() -> std::expected<int, StarException> {
    calls += *value;
    return *value;
  });
  ASSERT_TRUE(pool.stop());
  EXPECT_FALSE(task.finish());
  EXPECT_FALSE(producer.get());
  EXPECT_EQ(calls, 0);
}
