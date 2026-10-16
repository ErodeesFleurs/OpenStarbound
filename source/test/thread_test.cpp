import std;
import star.unique_function;
import star.thread;
import star.string;
import star.maybe;

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
