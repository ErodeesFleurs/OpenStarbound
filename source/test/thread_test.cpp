import std;
import star.unique_function;
import star.thread;
import star.string;
import star.maybe;
import star.listener;

#include "gtest/gtest.h"

using namespace Star;

TEST(Thread, InvokeErrors) {
  struct TestException {};

  auto function = Thread::invoke("test", []() {
      throw TestException();
    });

  EXPECT_THROW(function.finish(), TestException);
}

TEST(Thread, InvokeReturn) {
  auto functionRet = Thread::invoke("test", []() {
      return String("TestValue");
    });

  EXPECT_EQ(functionRet.finish(), String("TestValue"));
  EXPECT_THROW(functionRet.finish(), InvalidMaybeAccessException);
}

TEST(Thread, MoveOnlyCallableAndMemberInvocation) {
  auto owned = std::make_unique<int>(42);
  auto task = Thread::invoke("move-only", [value = std::move(owned)] { return *value; });
  EXPECT_FALSE(owned);
  EXPECT_EQ(task.finish(), 42);
  struct Target {
    int value = 7;
    int add(int amount) { return value += amount; }
  } target;
  auto member = Thread::invoke("member", &Target::add, std::ref(target), 5);
  EXPECT_EQ(member.finish(), 12);
  EXPECT_EQ(target.value, 12);
  ThreadFunction<void> empty(UniqueFunction<void()>{}, "empty");
  EXPECT_THROW(empty.finish(), std::bad_function_call);
}

TEST(Thread, UniqueCallableOwnershipAlignmentAndEmptyState) {
  struct Witness {
    int* deaths;
    ~Witness() { ++*deaths; }
  };
  int deaths = 0;
  {
    UniqueFunction<int()> small([owner = std::unique_ptr<Witness>(new Witness{&deaths})] { return 42; });
    UniqueFunction<int()> moved(std::move(small));
    EXPECT_FALSE(small);
    EXPECT_EQ(moved(), 42);
    EXPECT_THROW(small(), std::bad_function_call);
    struct alignas(64) Aligned {
      std::unique_ptr<Witness> owner;
      int operator()() {
        return reinterpret_cast<std::uintptr_t>(this) % alignof(Aligned) == 0 ? 7 : -1;
      }
    };
    UniqueFunction<int()> large(Aligned{std::unique_ptr<Witness>(new Witness{&deaths})});
    EXPECT_EQ(large(), 7);
    moved = std::move(large);
    EXPECT_EQ(deaths, 1);
    EXPECT_FALSE(large);
    EXPECT_EQ(moved(), 7);
    moved = nullptr;
    EXPECT_EQ(deaths, 2);
    int (*empty)() = nullptr;
    UniqueFunction<int()> nullFunction(empty);
    EXPECT_FALSE(nullFunction);
    EXPECT_THROW(nullFunction(), std::bad_function_call);
    int referenced = 1;
    UniqueFunction<int&()> reference([&]() -> int& { return referenced; });
    reference() = 17;
    EXPECT_EQ(referenced, 17);
    struct ReferenceSource {
      std::string text;
      std::string& operator()() { return text; }
    };
    static_assert(!std::is_constructible_v<UniqueFunction<std::string_view const&()>, ReferenceSource>);
  }
  EXPECT_EQ(deaths, 2);
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

TEST(Listener, DisconnectWaitsForInvocationAndPreventsLaterTriggers) {
  ListenerGroup group;
  std::latch entered(1);
  std::latch release(1);
  std::atomic<int> calls = 0;
  auto listener = std::make_shared<CallbackListener>([&] {
    ++calls;
    entered.count_down();
    release.wait();
  });
  auto subscription = group.addListener(listener);
  std::thread triggering([&] { group.trigger(); });
  entered.wait();
  std::latch disconnecting(1);
  auto disconnected = std::async(std::launch::async, [&] {
    disconnecting.count_down();
    subscription.disconnect();
  });
  disconnecting.wait();
  EXPECT_EQ(disconnected.wait_for(std::chrono::milliseconds(25)), std::future_status::timeout);
  release.count_down();
  triggering.join();
  disconnected.get();
  group.trigger();
  EXPECT_EQ(calls.load(), 1);
}

TEST(Listener, ReentrantTriggerAndSelfDisconnect) {
  ListenerGroup group;
  ListenerSubscription subscription;
  int calls = 0;
  auto listener = std::make_shared<CallbackListener>([&] {
    if (++calls == 1)
      group.trigger();
    else
      subscription.disconnect();
  });
  subscription = group.addListener(listener);
  group.trigger();
  group.trigger();
  EXPECT_EQ(calls, 2);
}

TEST(Listener, RegistryMutationFromCallbackAndExceptionRelease) {
  ListenerGroup group;
  int calls = 0;
  bool throwNow = true;
  auto listener = std::make_shared<CallbackListener>([&] {
    ++calls;
    // A callback may mutate the registry without holding its lock, including
    // adding and immediately cancelling another listener.
    auto tracker = std::make_shared<TrackerListener>();
    auto temporary = group.addListener(tracker);
    group.clearExpiredListeners();
    if (throwNow)
      throw std::runtime_error("listener failure");
  });
  auto subscription = group.addListener(listener);
  EXPECT_THROW(group.trigger(), std::runtime_error);
  throwNow = false;
  group.trigger();
  EXPECT_EQ(calls, 2);
  auto disconnected = std::async(std::launch::async, [&] { subscription.disconnect(); });
  EXPECT_EQ(disconnected.wait_for(std::chrono::seconds(1)), std::future_status::ready);
  disconnected.get();
  group.trigger();
  EXPECT_EQ(calls, 2);
}

TEST(Listener, DuplicateOwnerIdentityAndScopedCancellation) {
  ListenerGroup group;
  int calls = 0;
  auto listener = std::make_shared<CallbackListener>([&] { ++calls; });
  auto first = group.addListener(listener);
  // An aliasing shared_ptr must not create a second registration.
  std::shared_ptr<Listener> alias(listener, listener.get());
  auto duplicate = group.addListener(alias);
  group.trigger();
  EXPECT_EQ(calls, 1);
  duplicate.disconnect();
  group.trigger();
  EXPECT_EQ(calls, 1);
  {
    auto replacement = group.addListener(listener);
    group.trigger();
    EXPECT_EQ(calls, 2);
    // Disconnecting an old registration cannot cancel its replacement.
    first.disconnect();
    group.trigger();
    EXPECT_EQ(calls, 3);
  }
  group.trigger();
  EXPECT_EQ(calls, 3);
}

TEST(Listener, TrackerCoalescesAndOwningCallbackDoesNotKeepOwnerAlive) {
  ListenerGroup group;
  auto tracker = std::make_shared<TrackerListener>();
  auto tracking = group.addListener(tracker);
  EXPECT_FALSE(tracker->pullTriggered());
  group.trigger();
  group.trigger();
  EXPECT_TRUE(tracker->pullTriggered());
  EXPECT_FALSE(tracker->pullTriggered());
  auto cache = std::make_shared<int>(0);
  std::weak_ptr<int> weakCache = cache;
  auto listener = std::make_shared<CallbackListener>([cache] { ++*cache; });
  auto subscription = group.addListener(listener);
  cache.reset();
  group.trigger();
  EXPECT_EQ(*weakCache.lock(), 1);
  listener.reset();
  EXPECT_TRUE(weakCache.expired());
  group.clearExpiredListeners();
  group.clearAllListeners();
  EXPECT_TRUE(tracker->pullTriggered());
  group.trigger();
  EXPECT_FALSE(tracker->pullTriggered());
}

TEST(Listener, ClearAllFromCallbackCancelsRemainingSnapshot) {
  ListenerGroup group;
  int calls = 0;
  auto first = std::make_shared<CallbackListener>([&] {
    ++calls;
    group.clearAllListeners();
  });
  auto second = std::make_shared<CallbackListener>([&] {
    ++calls;
    group.clearAllListeners();
  });
  auto firstSubscription = group.addListener(first);
  auto secondSubscription = group.addListener(second);
  group.trigger();
  EXPECT_EQ(calls, 1);
  group.trigger();
  EXPECT_EQ(calls, 1);
}

TEST(Listener, ConcurrentTriggersRemainSerial) {
  ListenerGroup group;
  std::latch entered(1);
  std::latch release(1);
  std::latch secondStarting(1);
  std::atomic<int> calls = 0;
  std::atomic<int> running = 0;
  std::atomic<bool> overlapped = false;
  auto listener = std::make_shared<CallbackListener>([&] {
    if (running.fetch_add(1) != 0)
      overlapped = true;
    if (calls.fetch_add(1) == 0) {
      entered.count_down();
      release.wait();
    }
    running.fetch_sub(1);
  });
  auto subscription = group.addListener(listener);
  std::thread first([&] { group.trigger(); });
  entered.wait();
  std::thread second([&] {
    secondStarting.count_down();
    group.trigger();
  });
  secondStarting.wait();
  release.count_down();
  first.join();
  second.join();
  EXPECT_EQ(calls.load(), 2);
  EXPECT_FALSE(overlapped.load());
}
