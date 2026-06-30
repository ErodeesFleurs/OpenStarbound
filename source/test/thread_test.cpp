#include "StarThread.hpp"

#include "gtest/gtest.h"

#include <atomic>

using namespace Star;

TEST(Thread, InvokeErrors) {
  struct TestException {};

  auto function = Thread::invoke("test", []() {
    throw TestException();
  });

  EXPECT_THROW(function.finish(), TestException);
}

TEST(Thread, DestructorSuppressesInvokeErrors) {
  struct TestException {};

  {
    auto function = Thread::invoke("test", []() {
      throw TestException();
    });
  }
}

TEST(Thread, InvokeReturn) {
  auto functionRet = Thread::invoke("test", []() {
    return String("TestValue");
  });

  EXPECT_EQ(functionRet.finish(), String("TestValue"));
  EXPECT_THROW(functionRet.finish(), InvalidMaybeAccessException);
}

TEST(Thread, MoveAssignFinishesExistingFunction) {
  std::atomic<bool> firstStarted = false;
  std::atomic<bool> allowFirstFinish = false;
  std::atomic<bool> firstFinished = false;

  ThreadFunction<void> function = Thread::invoke("first", [&]() {
    firstStarted = true;
    while (!allowFirstFinish)
      Thread::yield();
    firstFinished = true;
  });

  while (!firstStarted)
    Thread::yield();

  auto replacement = Thread::invoke("second", []() {});
  allowFirstFinish = true;
  function = std::move(replacement);

  EXPECT_TRUE(firstFinished);
  EXPECT_NO_THROW(function.finish());
}

TEST(Thread, ReadersWriterMutex) {
  ReadersWriterMutex mutex;
  ReadLocker rl1(mutex);
  ReadLocker rl2(mutex);
  WriteLocker wl(mutex, false);
  EXPECT_FALSE(wl.tryLock());
  rl1.unlock();
  EXPECT_FALSE(wl.tryLock());
  rl2.unlock();
  EXPECT_TRUE(wl.tryLock());
}
