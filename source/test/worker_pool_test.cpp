import std;
import star.thread;
import star.list;
import star.worker_pool;
import star.format;

#include "gtest/gtest.h"

using namespace Star;

TEST(WorkerPoolTest, All) {
  int counter = 0;
  Mutex counterMutex;

  auto incCounter = [&counter, &counterMutex]() -> std::expected<void, StarException> {
    Thread::sleep(100);
    MutexLocker locker(counterMutex);
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
  auto empty = pool.addWork({});
  auto emptyFinish = empty.finish();
  ASSERT_FALSE(emptyFinish);
  auto repeatedFinish = empty.finish();
  ASSERT_FALSE(repeatedFinish);
  EXPECT_STREQ(repeatedFinish.error().what(), emptyFinish.error().what());
  auto emptyProducer = pool.addProducer<int>({});
  auto emptyValue = emptyProducer.get();
  ASSERT_FALSE(emptyValue);
  auto recovery = pool.addProducer<int>([]() -> std::expected<int, StarException> { return 19; });
  auto recoveredValue = recovery.get();
  ASSERT_TRUE(recoveredValue);
  EXPECT_EQ(**recoveredValue, 19);
  ASSERT_TRUE(pool.finish());
}
