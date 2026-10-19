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
  int copyAssignments = 0;
  int moveAssignments = 0;
  int destroyed = 0;
  int failAssignment = -1;
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
  CopyableVectorValue& operator=(CopyableVectorValue const& other) {
    if (state->copyAssignments++ == state->failAssignment)
      throw std::runtime_error("copy assignment");
    value = other.value;
    return *this;
  }
  CopyableVectorValue& operator=(CopyableVectorValue&& other) {
    if (state->moveAssignments++ == state->failAssignment)
      throw std::runtime_error("move assignment");
    value = std::exchange(other.value, -1);
    return *this;
  }
  ~CopyableVectorValue() {
    ++state->destroyed;
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
  MoveOnlyVectorValue& operator=(MoveOnlyVectorValue&& other) {
    if (state->moveAssignments++ == state->failAssignment)
      throw std::runtime_error("move assignment");
    value = std::exchange(other.value, -1);
    return *this;
  }
  ~MoveOnlyVectorValue() {
    ++state->destroyed;
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

TEST(SmallVectorTest, NondefaultAssignmentConstructsOnlyNewElements) {
  SmallVectorLifetime state;
  {
    SmallVector<CopyableVectorValue, 4> source;
    source.emplace_back(state, 10);
    source.emplace_back(state, 20);
    source.emplace_back(state, 30);
    SmallVector<CopyableVectorValue, 4> target;
    target.emplace_back(state, 1);
    target = source;
    EXPECT_EQ(state.copyAssignments, 1);
    EXPECT_EQ(state.copies, 2);
    EXPECT_EQ(state.alive, 6);
    ASSERT_EQ(target.size(), 3u);
    EXPECT_EQ(target[0].value, 10);
    EXPECT_EQ(target[2].value, 30);

    source.pop_back();
    source.pop_back();
    auto destroyed = state.destroyed;
    target = source;
    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(state.copyAssignments, 2);
    EXPECT_EQ(state.copies, 2);
    EXPECT_EQ(state.destroyed - destroyed, 2);
    EXPECT_EQ(state.alive, 2);
    auto& alias = target;
    target = alias;
    EXPECT_EQ(state.copyAssignments, 2);

    target = {CopyableVectorValue(state, 40), CopyableVectorValue(state, 50)};
    ASSERT_EQ(target.size(), 2u);
    EXPECT_EQ(target[0].value, 40);
    EXPECT_EQ(target[1].value, 50);
    EXPECT_EQ(state.copyAssignments, 3);
    EXPECT_EQ(state.copies, 3);
    EXPECT_EQ(state.alive, 3);
    target = {};
    EXPECT_TRUE(target.empty());
    EXPECT_EQ(state.alive, 1);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, NondefaultCopyAssignmentGrowsHeapAndReusesCapacity) {
  SmallVectorLifetime state;
  {
    SmallVector<CopyableVectorValue, 1> source;
    source.emplace_back(state, 10);
    source.emplace_back(state, 20);
    source.emplace_back(state, 30);
    SmallVector<CopyableVectorValue, 1> target;
    target.emplace_back(state, 1);
    target = source;
    ASSERT_EQ(target.size(), 3u);
    EXPECT_EQ(target[0].value, 10);
    EXPECT_EQ(target[1].value, 20);
    EXPECT_EQ(target[2].value, 30);
    EXPECT_EQ(state.alive, 6);
    auto storage = target.ptr();
    source.pop_back();
    source.pop_back();
    target = source;
    EXPECT_EQ(target.ptr(), storage);
    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(state.alive, 2);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, HeapMoveTakesStorageEvenAfterShrinkingAndReleasesTarget) {
  SmallVectorLifetime state;
  {
    SmallVector<MoveOnlyVectorValue, 2> source;
    source.reserve(8);
    source.emplace_back(state, 10);
    source.emplace_back(state, 20);
    source.emplace_back(state, 30);
    source.pop_back();
    source.pop_back();
    auto storage = source.ptr();
    SmallVector<MoveOnlyVectorValue, 2> target;
    target.reserve(9);
    target.emplace_back(state, 40);
    target.emplace_back(state, 50);
    auto destroyed = state.destroyed;
    state.failMove = state.moves;
    state.failAssignment = state.moveAssignments;
    target = std::move(source);
    EXPECT_EQ(target.ptr(), storage);
    ASSERT_EQ(target.size(), 1u);
    EXPECT_EQ(target[0].value, 10);
    EXPECT_EQ(state.moves, 0);
    EXPECT_EQ(state.moveAssignments, 0);
    EXPECT_EQ(state.destroyed - destroyed, 2);
    EXPECT_EQ(state.alive, 1);
    EXPECT_TRUE(source.empty());
    EXPECT_NE(source.ptr(), storage);
    source.emplace_back(state, 60);
    EXPECT_EQ(source[0].value, 60);
    auto& alias = target;
    target = std::move(alias);
    EXPECT_EQ(target.ptr(), storage);
    EXPECT_EQ(target[0].value, 10);
    EXPECT_EQ(state.alive, 2);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, HeapMoveConstructionAndEmptyHeapKeepCapacity) {
  SmallVectorLifetime state;
  {
    SmallVector<MoveOnlyVectorValue, 2> source;
    source.reserve(8);
    source.emplace_back(state, 10);
    auto storage = source.ptr();
    state.failMove = state.moves;
    SmallVector<MoveOnlyVectorValue, 2> target(std::move(source));
    EXPECT_EQ(target.ptr(), storage);
    EXPECT_EQ(target[0].value, 10);
    EXPECT_TRUE(source.empty());
    source.emplace_back(state, 20);
    target.clear();
    SmallVector<MoveOnlyVectorValue, 2> empty(std::move(target));
    EXPECT_TRUE(empty.empty());
    EXPECT_EQ(empty.ptr(), storage);
    for (int i = 0; i < 8; ++i)
      empty.emplace_back(state, i);
    EXPECT_EQ(empty.ptr(), storage);
    EXPECT_EQ(state.moves, 0);
    EXPECT_TRUE(target.empty());
    target.emplace_back(state, 30);
    EXPECT_EQ(state.alive, 10);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, InlineMoveOnlyAssignmentAndEraseNeedNoCopyOrDefault) {
  SmallVectorLifetime state;
  {
    SmallVector<MoveOnlyVectorValue, 4> source;
    source.emplace_back(state, 10);
    source.emplace_back(state, 20);
    source.emplace_back(state, 30);
    SmallVector<MoveOnlyVectorValue, 4> target;
    target.emplace_back(state, 1);
    auto sourceStorage = source.ptr();
    target = std::move(source);
    EXPECT_NE(target.ptr(), sourceStorage);
    EXPECT_TRUE(source.empty());
    EXPECT_EQ(state.moveAssignments, 1);
    EXPECT_EQ(state.moves, 2);
    EXPECT_EQ(state.alive, 3);
    ASSERT_EQ(target.size(), 3u);
    EXPECT_EQ(target[0].value, 10);
    EXPECT_EQ(target[2].value, 30);
    auto assignments = state.moveAssignments;
    EXPECT_EQ(target.erase(target.begin() + 1, target.begin() + 1), target.begin() + 1);
    EXPECT_EQ(state.moveAssignments, assignments);
    EXPECT_EQ(target[1].value, 20);
    EXPECT_EQ(target.erase(target.begin()), target.begin());
    EXPECT_EQ(target[0].value, 20);
    EXPECT_EQ(target.erase(target.begin(), target.end()), target.begin());
    EXPECT_TRUE(target.empty());
    EXPECT_EQ(state.alive, 0);
    source.emplace_back(state, 40);
    target.emplace_back(state, 50);
    target.emplace_back(state, 60);
    auto destroyed = state.destroyed;
    target = std::move(source);
    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(target[0].value, 40);
    EXPECT_TRUE(source.empty());
    EXPECT_EQ(state.destroyed - destroyed, 2);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, ThrowingAssignmentAndConstructionKeepLivePrefixes) {
  SmallVectorLifetime state;
  {
    SmallVector<CopyableVectorValue, 4> source;
    source.emplace_back(state, 10);
    source.emplace_back(state, 20);
    source.emplace_back(state, 30);
    SmallVector<CopyableVectorValue, 4> target;
    target.emplace_back(state, 1);
    state.failAssignment = 0;
    EXPECT_THROW(target = source, std::runtime_error);
    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(target[0].value, 1);
    EXPECT_EQ(state.alive, 4);
    state.failAssignment = -1;
    state.failCopy = 1;
    EXPECT_THROW(target = source, std::runtime_error);
    EXPECT_EQ(target.size(), 2u);
    EXPECT_EQ(target[0].value, 10);
    EXPECT_EQ(target[1].value, 20);
    EXPECT_EQ(state.alive, 5);
    state.failCopy = -1;
    target = source;
    EXPECT_EQ(target.size(), 3u);
    EXPECT_EQ(state.alive, 6);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, ThrowingInlineMoveKeepsLivePrefixesAndIsNotNoexcept) {
  using Vector = SmallVector<MoveOnlyVectorValue, 4>;
  static_assert(!std::is_nothrow_move_constructible_v<Vector>);
  static_assert(!std::is_nothrow_move_assignable_v<Vector>);
  SmallVectorLifetime state;
  {
    Vector source;
    source.emplace_back(state, 10);
    source.emplace_back(state, 20);
    source.emplace_back(state, 30);
    state.failMove = 1;
    EXPECT_THROW((Vector(std::move(source))), std::runtime_error);
    EXPECT_EQ(source.size(), 3u);
    EXPECT_EQ(state.alive, 3);
    EXPECT_EQ(source[0].value, -1);
    Vector target;
    target.emplace_back(state, 40);
    state.failMove = -1;
    state.failAssignment = 0;
    EXPECT_THROW(target = std::move(source), std::runtime_error);
    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(target[0].value, 40);
    EXPECT_EQ(source.size(), 3u);
    EXPECT_EQ(state.alive, 4);
    state.failAssignment = -1;
    state.failMove = state.moves + 1;
    EXPECT_THROW(target = std::move(source), std::runtime_error);
    EXPECT_EQ(target.size(), 2u);
    EXPECT_EQ(source.size(), 3u);
    EXPECT_EQ(state.alive, 5);
    state.failMove = -1;
    target = std::move(source);
    EXPECT_EQ(target.size(), 3u);
    EXPECT_TRUE(source.empty());
    EXPECT_EQ(state.alive, 3);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, InlineMoveConstructionOnlyRequiresMoveConstruction) {
  struct Value {
    int value;
    explicit Value(int value) : value(value) {}
    Value(Value const&) = delete;
    Value(Value&& other) : value(std::exchange(other.value, -1)) {}
    Value& operator=(Value&&) = delete;
  };
  SmallVector<Value, 2> source;
  source.emplace_back(10);
  SmallVector<Value, 2> target(std::move(source));
  ASSERT_EQ(target.size(), 1u);
  EXPECT_EQ(target[0].value, 10);
  EXPECT_TRUE(source.empty());
  source.emplace_back(20);
  EXPECT_EQ(source[0].value, 20);
}

TEST(SmallVectorTest, InlineMoveIntoHeapTargetReusesStorageAndDestroysTail) {
  SmallVectorLifetime state;
  {
    SmallVector<MoveOnlyVectorValue, 2> source;
    source.emplace_back(state, 10);
    SmallVector<MoveOnlyVectorValue, 2> target;
    target.reserve(8);
    target.emplace_back(state, 20);
    target.emplace_back(state, 30);
    target.emplace_back(state, 40);
    auto storage = target.ptr();
    auto destroyed = state.destroyed;
    target = std::move(source);
    EXPECT_EQ(target.ptr(), storage);
    ASSERT_EQ(target.size(), 1u);
    EXPECT_EQ(target[0].value, 10);
    EXPECT_EQ(state.moveAssignments, 1);
    EXPECT_EQ(state.moves, 0);
    EXPECT_EQ(state.destroyed - destroyed, 3);
    EXPECT_EQ(state.alive, 1);
    EXPECT_TRUE(source.empty());
    source.emplace_back(state, 50);
    EXPECT_EQ(source[0].value, 50);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, ResizePreservesDefaultTemporaryCopyAndDestructionOrder) {
  struct Value {
    static std::vector<char>& events() {
      static std::vector<char> log;
      return log;
    }
    Value() { events().push_back('D'); }
    Value(Value const&) { events().push_back('C'); }
    ~Value() { events().push_back('X'); }
  };
  SmallVector<Value, 4> values;
  Value::events().clear();
  values.resize(2);
  EXPECT_EQ(Value::events(), (std::vector<char>{'D', 'C', 'C', 'X'}));
  Value::events().clear();
  values.resize(2);
  EXPECT_EQ(Value::events(), (std::vector<char>{'D', 'X'}));
  Value::events().clear();
  values.resize(0);
  EXPECT_EQ(Value::events(), (std::vector<char>{'D', 'X', 'X', 'X'}));
}

TEST(SmallVectorTest, AliasedEmplaceStagesValueBeforeGrowthAndRangeFailureRollsBack) {
  SmallVector<std::string, 1> text{"first"};
  text.emplace(text.begin(), text[0]);
  EXPECT_EQ(text, (SmallVector<std::string, 1>{"first", "first"}));
  SmallVectorLifetime state;
  {
    SmallVector<CopyableVectorValue, 4> values;
    values.emplace_back(state, 10);
    SmallVector<CopyableVectorValue, 4> more;
    more.emplace_back(state, 20);
    more.emplace_back(state, 30);
    state.failCopy = 1;
    EXPECT_THROW(values.insert(values.begin(), more.begin(), more.end()), std::runtime_error);
    EXPECT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].value, 10);
    EXPECT_EQ(state.alive, 3);
    EXPECT_EQ(state.destroyed, 1);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(SmallVectorTest, ThrowingEraseDoesNotDestroyAnUnshiftedTail) {
  SmallVectorLifetime state;
  {
    SmallVector<MoveOnlyVectorValue, 4> values;
    values.emplace_back(state, 10);
    values.emplace_back(state, 20);
    values.emplace_back(state, 30);
    state.failAssignment = 1;
    EXPECT_THROW(values.erase(values.begin()), std::runtime_error);
    EXPECT_EQ(values.size(), 3u);
    EXPECT_EQ(state.alive, 3);
    EXPECT_EQ(state.destroyed, 0);
    state.failAssignment = -1;
    values.erase(values.begin() + 1, values.end());
    EXPECT_EQ(values.size(), 1u);
    EXPECT_EQ(state.alive, 1);
    EXPECT_EQ(state.destroyed, 2);
  }
  EXPECT_EQ(state.alive, 0);
  EXPECT_EQ(state.destroyed, 3);
}
