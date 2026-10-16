import star.small_vector;
import std;
import star.format;

#include "gtest/gtest.h"

using namespace Star;

TEST(SmallVectorTest, InsertErase) {
  typedef SmallVector<int, 2> SV;
  SV a = {1, 2, 3, 4};
  EXPECT_EQ(a.size(), 4u);
  EXPECT_EQ(a, SV({1, 2, 3, 4}));
  EXPECT_NE(a, SV({1, 2, 3}));
  a.insert(a.begin(), 0);
  a.insert(a.begin(), -1);
  EXPECT_EQ(a, SV({-1, 0, 1, 2, 3, 4}));
  a.insert(a.begin(), {-3, -2});
  EXPECT_EQ(a, SV({-3, -2, -1, 0, 1, 2, 3, 4}));
  a.erase(a.begin() + 1);
  EXPECT_EQ(a, SV({-3, -1, 0, 1, 2, 3, 4}));
  a.erase(a.begin(), a.begin() + 3);
  EXPECT_EQ(a, SV({1, 2, 3, 4}));
  a.insert(a.end(), {5, 6, 7, 8});
  EXPECT_EQ(a, SV({1, 2, 3, 4, 5, 6, 7, 8}));
  a.erase(a.begin() + 2, a.end() - 2);
  EXPECT_EQ(a, SV({1, 2, 7, 8}));
  a.insert(a.begin() + 2, 6);
  a.insert(a.begin() + 2, 5);
  a.insert(a.begin() + 2, 4);
  a.insert(a.begin() + 2, 3);
  EXPECT_EQ(a, SV({1, 2, 3, 4, 5, 6, 7, 8}));

  SV b(a.begin(), a.end());
  EXPECT_EQ(b, SV({1, 2, 3, 4, 5, 6, 7, 8}));
}

TEST(SmallVectorTest, Comparators) {
  typedef SmallVector<int, 3> SV;

  EXPECT_TRUE(SV({1, 2, 3, 4}) < SV({1, 2, 3, 5}));
  EXPECT_FALSE(SV({1, 2, 3, 4}) < SV({1, 2, 3, 4}));
  EXPECT_FALSE(SV({1, 2, 3, 4}) < SV({1, 2, 3, 3}));
  EXPECT_TRUE(SV({1, 2, 3}) < SV({1, 2, 3, 4}));
  EXPECT_FALSE(SV({1, 2, 3, 4, 5}) < SV({1, 2, 3, 4}));
}

TEST(SmallVectorTest, Destructors) {
  auto i = std::make_shared<int>(0);
  SmallVector<std::shared_ptr<int>, 1> v;
  v.push_back(i);
  v.push_back(i);
  v.push_back(i);
  EXPECT_EQ(i.use_count(), 4);
  v.pop_back();
  EXPECT_EQ(i.use_count(), 3);
  v.pop_back();
  EXPECT_EQ(i.use_count(), 2);
  v.clear();
  EXPECT_EQ(i.use_count(), 1);
}

namespace {
struct SmallVectorLifetime {
  int alive = 0;
  int copies = 0;
  int moves = 0;
  int failCopy = -1;
  int failMove = -1;
};

struct CopyableVectorValue {
  SmallVectorLifetime* state;
  int value;

  CopyableVectorValue(SmallVectorLifetime& state, int value) : state(&state), value(value) {
    ++state.alive;
  }
  CopyableVectorValue(CopyableVectorValue const& other) : state(other.state), value(other.value) {
    if (state->copies++ == state->failCopy)
      throw std::runtime_error("copy");
    ++state->alive;
  }
  CopyableVectorValue(CopyableVectorValue&& other) : state(other.state), value(other.value) {
    ++state->moves;
    other.value = -1;
    ++state->alive;
  }
  ~CopyableVectorValue() {
    --state->alive;
  }
};

struct MoveOnlyVectorValue {
  SmallVectorLifetime* state;
  int value;

  MoveOnlyVectorValue(SmallVectorLifetime& state, int value) : state(&state), value(value) {
    ++state.alive;
  }
  MoveOnlyVectorValue(MoveOnlyVectorValue const&) = delete;
  MoveOnlyVectorValue(MoveOnlyVectorValue&& other) : state(other.state), value(other.value) {
    if (state->moves++ == state->failMove)
      throw std::runtime_error("move");
    other.value = -1;
    ++state->alive;
  }
  ~MoveOnlyVectorValue() {
    --state->alive;
  }
};
}

TEST(SmallVectorTest, FailedGrowthCopiesAndReclaimsPrefix) {
  SmallVectorLifetime state;
  {
    SmallVector<CopyableVectorValue, 2> values;
    values.emplace_back(state, 10);
    values.emplace_back(state, 20);
    auto original = values.ptr();
    state.failCopy = 1;
    EXPECT_THROW(values.reserve(3), std::runtime_error);
    EXPECT_EQ(values.ptr(), original);
    EXPECT_EQ(values.size(), 2u);
    EXPECT_EQ(values[0].value, 10);
    EXPECT_EQ(values[1].value, 20);
    EXPECT_EQ(state.alive, 2);
    EXPECT_EQ(state.moves, 0);
    state.failCopy = state.copies + 1;
    EXPECT_THROW(values.emplace_back(state, 30), std::runtime_error);
    EXPECT_EQ(state.alive, 2);
    EXPECT_EQ(values.size(), 2u);
    state.failCopy = -1;
    values.emplace_back(state, 30);
    EXPECT_EQ(state.alive, 3);
    EXPECT_EQ(state.moves, 0);
    original = values.ptr();
    state.failCopy = state.copies + 1;
    EXPECT_THROW(values.reserve(9), std::runtime_error);
    EXPECT_EQ(values.ptr(), original);
    EXPECT_EQ(values.size(), 3u);
    EXPECT_EQ(values[0].value, 10);
    EXPECT_EQ(values[1].value, 20);
    EXPECT_EQ(values[2].value, 30);
    EXPECT_EQ(state.alive, 3);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, ThrowingMoveOnlyGrowthKeepsLiveOriginalStorage) {
  SmallVectorLifetime state;
  {
    SmallVector<MoveOnlyVectorValue, 2> values;
    values.emplace_back(state, 10);
    values.emplace_back(state, 20);
    auto original = values.ptr();
    state.failMove = 1;
    EXPECT_THROW(values.reserve(3), std::runtime_error);
    EXPECT_EQ(values.ptr(), original);
    EXPECT_EQ(values.size(), 2u);
    EXPECT_EQ(state.alive, 2);
    EXPECT_EQ(values[0].value, -1);
    state.failMove = -1;
    values.reserve(3);
    EXPECT_EQ(values.size(), 2u);
    EXPECT_EQ(state.alive, 2);
    values.clear();
    EXPECT_EQ(state.alive, 0);
    values.emplace_back(state, 42);
    EXPECT_EQ(values[0].value, 42);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, ZeroCapacityAndOveralignedStorage) {
  struct alignas(64) Value {
    int value;
  };
  static_assert(alignof(SmallVector<Value, 1>) == alignof(Value));
  static_assert(sizeof(SmallVector<Value, 1>) == 2 * sizeof(Value));
  static_assert(sizeof(SmallVector<Value, 0>) == 2 * sizeof(Value));
  static_assert(sizeof(SmallVector<int, 2>) == 2 * sizeof(int) + 3 * sizeof(int*));
  SmallVector<Value, 0> empty;
  EXPECT_NE(empty.ptr(), nullptr);
  EXPECT_TRUE(empty.empty());
  empty.emplace_back(42);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(empty.ptr()) % alignof(Value), 0u);
  EXPECT_EQ(empty[0].value, 42);
  SmallVector<Value, 1> values;
  values.emplace_back(1);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(values.ptr()) % alignof(Value), 0u);
  values.emplace_back(2);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(values.ptr()) % alignof(Value), 0u);
  EXPECT_EQ(values[0].value, 1);
  EXPECT_EQ(values[1].value, 2);
}

TEST(SmallVectorTest, NothrowMoveIsPreferredToCopy) {
  struct Value {
    int* moves;

    explicit Value(int& moves) : moves(&moves) {}
    Value(Value const&) {
      throw std::runtime_error("unexpected copy");
    }
    Value(Value&& other) noexcept : moves(other.moves) {
      ++*moves;
    }
  };
  int moves = 0;
  SmallVector<Value, 1> values;
  values.emplace_back(moves);
  values.reserve(2);
  EXPECT_EQ(moves, 1);
  EXPECT_EQ(values.size(), 1u);
}

TEST(SmallVectorTest, InsertReturnsNewStoragePosition) {
  SmallVector<int, 1> values{1};
  auto first = values.insert(values.begin(), 2);
  ASSERT_EQ(first, values.begin());
  EXPECT_EQ(*first, 2);
  std::array<int, 3> more{3, 4, 5};
  auto inserted = values.insert(values.begin() + 1, more.begin(), more.end());
  ASSERT_EQ(inserted, values.begin() + 1);
  EXPECT_EQ(values, (SmallVector<int, 1>{2, 3, 4, 5, 1}));
}

TEST(SmallVectorTest, SinglePassAndMoveOnlyInsertion) {
  std::istringstream stream("2 3 4");
  SmallVector<int, 1> values{1};
  auto inserted = values.insert(values.end(), std::istream_iterator<int>(stream), std::istream_iterator<int>{});
  ASSERT_EQ(inserted, values.begin() + 1);
  EXPECT_EQ(values, (SmallVector<int, 1>{1, 2, 3, 4}));
  SmallVector<std::unique_ptr<int>, 1> owners;
  owners.emplace_back(std::make_unique<int>(7));
  owners.emplace(owners.begin(), std::make_unique<int>(9));
  EXPECT_EQ(*owners[0], 9);
  EXPECT_EQ(*owners[1], 7);
}
