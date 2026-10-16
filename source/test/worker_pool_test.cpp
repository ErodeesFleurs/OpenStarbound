import std;
import star.thread;
import star.list;
import star.worker_pool;

#include "gtest/gtest.h"

using namespace Star;

TEST(WorkerPoolTest, All) {
  int counter = 0;
  Mutex counterMutex;

  auto incCounter = [&counter, &counterMutex]() {
    Thread::sleep(100);
    MutexLocker locker(counterMutex);
    counter += 1;
  };

  Deque<WorkerPoolHandle> handles;

  WorkerPool workerPool("WorkerPoolTest");
  for (std::size_t i = 0; i < 10; ++i)
    handles.append(workerPool.addWork(incCounter));

  workerPool.start(10);

  for (std::size_t i = 0; i < 90; ++i)
    handles.append(workerPool.addWork(incCounter));

  while (handles.size() > 20)
    handles.takeFirst().finish();

  workerPool.finish();

  EXPECT_EQ(counter, 100);
}

TEST(WorkerPoolTest, MoveOnlyTasksPublishResultsToAllWaiters) {
  WorkerPool pool("move-only");
  pool.start(2);
  std::atomic<bool> release{false};
  auto result = pool.addProducer<int>([value = std::make_unique<int>(42), &release] {
    release.wait(false);
    return *value;
  });
  EXPECT_FALSE(result.wait(1));
  std::array<int, 4> observations{};
  std::vector<std::thread> waiters;
  for (std::size_t i = 0; i < observations.size(); ++i)
    waiters.emplace_back([&, i] { observations[i] = result.get(); });
  release.store(true);
  release.notify_all();
  for (auto& waiter : waiters)
    waiter.join();
  for (auto value : observations)
    EXPECT_EQ(value, 42);
  EXPECT_TRUE(result.done());
  EXPECT_EQ(result.get(), 42);
  auto empty = pool.addWork({});
  EXPECT_THROW(empty.finish(), std::bad_function_call);
  EXPECT_THROW(empty.finish(), std::bad_function_call);
  auto emptyProducer = pool.addProducer<int>({});
  EXPECT_THROW(emptyProducer.get(), std::bad_function_call);
  pool.finish();
}
