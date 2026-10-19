import star.static_vector;
import std;

#include "gtest/gtest.h"

using namespace Star;

TEST(StaticVectorTest, InsertErase) {
  typedef StaticVector<int, 64> SV;
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

TEST(StaticVectorTest, Comparators) {
  typedef StaticVector<int, 64> SV;

  EXPECT_TRUE(SV({1, 2, 3, 4}) < SV({1, 2, 3, 5}));
  EXPECT_FALSE(SV({1, 2, 3, 4}) < SV({1, 2, 3, 4}));
  EXPECT_FALSE(SV({1, 2, 3, 4}) < SV({1, 2, 3, 3}));
  EXPECT_TRUE(SV({1, 2, 3}) < SV({1, 2, 3, 4}));
  EXPECT_FALSE(SV({1, 2, 3, 4, 5}) < SV({1, 2, 3, 4}));
}

TEST(StaticVectorTest, SizeLimits) {
  StaticVector<int, 0> a;
  StaticVector<int, 1> b;
  b.push_back(0);
  StaticVector<int, 2> c;
  c.resize(2, 0);

  EXPECT_THROW(a.push_back(0), StaticVectorSizeException);
  EXPECT_THROW(b.push_back(0), StaticVectorSizeException);
  EXPECT_THROW(c.push_back(0), StaticVectorSizeException);
}

TEST(StaticVectorTest, ThrowingConstructionPreservesLivePrefix) {
  struct Value {
    int* alive;
    int value;

    Value(int& alive, int value) : alive(&alive), value(value) {
      if (value < 0)
        throw std::runtime_error("construction");
      ++alive;
    }
    ~Value() {
      --*alive;
    }
  };
  int alive = 0;
  {
    StaticVector<Value, 2> values;
    values.emplace_back(alive, 42);
    EXPECT_THROW(values.emplace_back(alive, -1), std::runtime_error);
    EXPECT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].value, 42);
    EXPECT_EQ(alive, 1);
    values.emplace_back(alive, 7);
    EXPECT_EQ(values.size(), 2u);
    EXPECT_EQ(alive, 2);
    EXPECT_THROW(values.emplace_back(alive, 9), StaticVectorSizeException);
    EXPECT_EQ(alive, 2);
  }
  EXPECT_EQ(alive, 0);
}

TEST(StaticVectorTest, AlignedStorageAndZeroCapacity) {
  struct alignas(64) Value {
    int value;
  };
  static_assert(alignof(StaticVector<Value, 2>) == alignof(Value));
  static_assert(sizeof(StaticVector<Value, 2>) == 3 * sizeof(Value));
  static_assert(sizeof(StaticVector<int, 2>) == sizeof(std::size_t) + 2 * sizeof(int));
  static_assert(sizeof(StaticVector<Value, 0>) == 2 * sizeof(Value));
  StaticVector<Value, 0> empty;
  EXPECT_NE(empty.ptr(), nullptr);
  EXPECT_EQ(empty.begin(), empty.end());
  EXPECT_THROW(empty.emplace_back(42), StaticVectorSizeException);
  StaticVector<Value, 2> values;
  values.emplace_back(1);
  values.emplace_back(2);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(values.ptr()) % alignof(Value), 0u);
  EXPECT_EQ(values[0].value, 1);
  EXPECT_EQ(values[1].value, 2);
  values.clear();
  values.emplace_back(3);
  EXPECT_EQ(values[0].value, 3);
}

TEST(StaticVectorTest, SinglePassAndMoveOnlyInsertion) {
  std::istringstream stream("2 3 4");
  StaticVector<int, 5> values{1};
  auto inserted = values.insert(values.end(), std::istream_iterator<int>(stream), std::istream_iterator<int>{});
  ASSERT_EQ(inserted, values.begin() + 1);
  EXPECT_EQ(values, (StaticVector<int, 5>{1, 2, 3, 4}));
  StaticVector<std::unique_ptr<int>, 2> owners;
  owners.emplace_back(std::make_unique<int>(7));
  owners.emplace(owners.begin(), std::make_unique<int>(9));
  EXPECT_EQ(*owners[0], 9);
  EXPECT_EQ(*owners[1], 7);
}

TEST(StaticVectorTest, RangeOverflowRollsBackAppendedElements) {
  StaticVector<int, 3> values{1, 2};
  std::array<int, 2> more{3, 4};
  EXPECT_THROW(values.insert(values.begin(), more.begin(), more.end()), StaticVectorSizeException);
  EXPECT_EQ(values, (StaticVector<int, 3>{1, 2}));
  std::istringstream stream("3 4");
  EXPECT_THROW(values.insert(values.begin(), std::istream_iterator<int>(stream), std::istream_iterator<int>{}), StaticVectorSizeException);
  EXPECT_EQ(values, (StaticVector<int, 3>{1, 2}));
}

TEST(StaticVectorTest, CapacityFailureDoesNotConsumeEmplaceArgument) {
  StaticVector<std::unique_ptr<int>, 1> values;
  values.emplace_back(std::make_unique<int>(7));
  auto candidate = std::make_unique<int>(9);
  EXPECT_THROW(values.emplace(values.begin(), std::move(candidate)), StaticVectorSizeException);
  ASSERT_TRUE(candidate);
  EXPECT_EQ(*candidate, 9);
  EXPECT_EQ(*values[0], 7);
}

namespace {
struct StaticVectorLifetime {
  int alive = 0;
  int copies = 0;
  int moves = 0;
  int assignments = 0;
  int destroyed = 0;
  int failCopy = -1;
  int failMove = -1;
  int failAssignment = -1;
};

struct NondefaultStaticValue {
  StaticVectorLifetime* state;
  int value;

  NondefaultStaticValue(StaticVectorLifetime& state, int value) : state(&state), value(value) {
    ++state.alive;
  }
  NondefaultStaticValue(NondefaultStaticValue const& other) : state(other.state), value(other.value) {
    if (state->copies++ == state->failCopy)
      throw std::runtime_error("copy");
    ++state->alive;
  }
  NondefaultStaticValue(NondefaultStaticValue&& other) : state(other.state), value(other.value) {
    if (state->moves++ == state->failMove)
      throw std::runtime_error("move");
    other.value = -1;
    ++state->alive;
  }
  NondefaultStaticValue& operator=(NondefaultStaticValue const& other) {
    if (state->assignments++ == state->failAssignment)
      throw std::runtime_error("copy assignment");
    value = other.value;
    return *this;
  }
  NondefaultStaticValue& operator=(NondefaultStaticValue&& other) {
    if (state->assignments++ == state->failAssignment)
      throw std::runtime_error("move assignment");
    value = std::exchange(other.value, -1);
    return *this;
  }
  ~NondefaultStaticValue() {
    ++state->destroyed;
    --state->alive;
  }
};
}

TEST(StaticVectorTest, NondefaultAssignmentConstructsOnlyNewElements) {
  StaticVectorLifetime state;
  {
    StaticVector<NondefaultStaticValue, 4> source;
    source.emplace_back(state, 10);
    source.emplace_back(state, 20);
    source.emplace_back(state, 30);
    StaticVector<NondefaultStaticValue, 4> target;
    target.emplace_back(state, 1);
    target = source;
    EXPECT_EQ(state.assignments, 1);
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
    EXPECT_EQ(state.assignments, 2);
    EXPECT_EQ(state.copies, 2);
    EXPECT_EQ(state.destroyed - destroyed, 2);
    EXPECT_EQ(state.alive, 2);
    auto& alias = target;
    target = alias;
    target = std::move(alias);
    EXPECT_EQ(state.assignments, 2);
    EXPECT_EQ(target[0].value, 10);

    target = {NondefaultStaticValue(state, 40), NondefaultStaticValue(state, 50)};
    ASSERT_EQ(target.size(), 2u);
    EXPECT_EQ(target[0].value, 40);
    EXPECT_EQ(target[1].value, 50);
    EXPECT_EQ(state.assignments, 3);
    EXPECT_EQ(state.copies, 3);
    EXPECT_EQ(state.alive, 3);
    target = {};
    EXPECT_TRUE(target.empty());
    EXPECT_EQ(state.alive, 1);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(StaticVectorTest, MoveOnlyAssignmentAndEraseNeedNoCopyOrDefault) {
  struct Value {
    int* alive;
    int value;
    Value(int& alive, int value) : alive(&alive), value(value) { ++alive; }
    Value(Value const&) = delete;
    Value(Value&& other) : alive(other.alive), value(std::exchange(other.value, -1)) { ++*alive; }
    Value& operator=(Value&& other) {
      value = std::exchange(other.value, -1);
      return *this;
    }
    ~Value() { --*alive; }
  };
  int alive = 0;
  {
    StaticVector<Value, 4> source;
    source.emplace_back(alive, 10);
    source.emplace_back(alive, 20);
    source.emplace_back(alive, 30);
    StaticVector<Value, 4> target;
    target.emplace_back(alive, 1);
    target = std::move(source);
    ASSERT_EQ(target.size(), 3u);
    EXPECT_EQ(target[0].value, 10);
    EXPECT_EQ(target[2].value, 30);
    EXPECT_EQ(alive, 6);
    EXPECT_EQ(target.erase(target.begin() + 1, target.begin() + 1), target.begin() + 1);
    EXPECT_EQ(target[1].value, 20);
    EXPECT_EQ(target.erase(target.begin()), target.begin());
    EXPECT_EQ(target[0].value, 20);
    EXPECT_EQ(alive, 5);
    EXPECT_EQ(target.erase(target.begin(), target.end()), target.begin());
    EXPECT_TRUE(target.empty());
    EXPECT_EQ(alive, 3);
    target.emplace_back(alive, 40);
    target.emplace_back(alive, 50);
    source.clear();
    source.emplace_back(alive, 60);
    target = std::move(source);
    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(target[0].value, 60);
    EXPECT_EQ(alive, 2);
  }
  EXPECT_EQ(alive, 0);
}

TEST(StaticVectorTest, ThrowingAssignmentAndConstructionKeepLivePrefixes) {
  StaticVectorLifetime state;
  {
    StaticVector<NondefaultStaticValue, 4> source;
    source.emplace_back(state, 10);
    source.emplace_back(state, 20);
    source.emplace_back(state, 30);
    StaticVector<NondefaultStaticValue, 4> target;
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
    target.clear();
    target.emplace_back(state, 40);
    state.failMove = 1;
    EXPECT_THROW(target = std::move(source), std::runtime_error);
    EXPECT_EQ(target.size(), 2u);
    EXPECT_EQ(source.size(), 3u);
    EXPECT_EQ(state.alive, 5);
    state.failMove = -1;
    target = std::move(source);
    EXPECT_EQ(target.size(), 3u);
    EXPECT_EQ(state.alive, 6);
  }
  EXPECT_EQ(state.alive, 0);
}

TEST(StaticVectorTest, InitializerAssignmentChecksCapacityBeforeMutation) {
  StaticVector<int, 2> values{1};
  EXPECT_THROW((values = {2, 3, 4}), StaticVectorSizeException);
  EXPECT_EQ(values, (StaticVector<int, 2>{1}));
}

TEST(StaticVectorTest, ResizePreservesDefaultTemporaryCopyAndDestructionOrder) {
  struct Value {
    static std::vector<char>& events() {
      static std::vector<char> log;
      return log;
    }
    Value() { events().push_back('D'); }
    Value(Value const&) { events().push_back('C'); }
    ~Value() { events().push_back('X'); }
  };
  StaticVector<Value, 4> values;
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

TEST(StaticVectorTest, AliasedEmplaceStagesValueAndRangeFailureRollsBack) {
  StaticVector<std::string, 4> text{"first", "second"};
  text.emplace(text.begin() + 1, text[0]);
  EXPECT_EQ(text, (StaticVector<std::string, 4>{"first", "first", "second"}));
  StaticVectorLifetime state;
  {
    StaticVector<NondefaultStaticValue, 4> values;
    values.emplace_back(state, 10);
    StaticVector<NondefaultStaticValue, 4> more;
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

TEST(StaticVectorTest, ThrowingEraseDoesNotDestroyAnUnshiftedTail) {
  StaticVectorLifetime state;
  {
    StaticVector<NondefaultStaticValue, 4> values;
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
