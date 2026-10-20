import std;
import star.format;
import star.flat_hash_set;
import star.flat_hash_map;
import star.random;
import star.vector;
import star.map;
import star.string;
import star.string_view;
import star.color;

#include "gtest/gtest.h"

import star.list;
using namespace Star;

namespace {
struct ParsedMapEntry {
  int key = 0;
  operator std::pair<int, int>() const { return {key, key * 2}; }
  friend std::istream& operator>>(std::istream& input, ParsedMapEntry& entry) {
    return input >> entry.key;
  }
};
struct CollisionHash {
  std::size_t operator()(int) const { return 0; }
};
}

TEST(FlatHashMap, SinglePassInputPreservesEveryElement) {
  std::istringstream input("1 2 3 4");
  FlatHashMap<int, int> values;
  values.insert(std::istream_iterator<ParsedMapEntry>(input), {});
  EXPECT_EQ(values.size(), 4u);
  for (int key = 1; key <= 4; ++key)
    EXPECT_EQ(values.at(key), key * 2);
  std::istringstream setInput("1 2 3 4");
  FlatHashSet<int> keys(std::istream_iterator<int>(setInput), {});
  EXPECT_EQ(keys.size(), 4u);
  for (int key = 1; key <= 4; ++key)
    EXPECT_NE(keys.find(key), keys.end());
}

TEST(FlatHashMap, RemoveAllValuesAcrossCollisionShift) {
  HashMap<int, int, CollisionHash> values;
  for (int key = 0; key < 40; ++key)
    values.set(key, key % 2);
  EXPECT_TRUE(values.removeValues(0));
  EXPECT_EQ(values.size(), 20u);
  for (int key = 0; key < 40; ++key)
    EXPECT_EQ(values.contains(key), key % 2 != 0);
  EXPECT_TRUE(values.removeValues(1));
  EXPECT_TRUE(values.empty());
}

TEST(FlatHashMap, TryEmplaceKeepsOwnershipOnDuplicate) {
  HashMap<int, std::unique_ptr<int>, CollisionHash> values;
  values.try_emplace(1, std::make_unique<int>(42));
  auto candidate = std::make_unique<int>(99);
  auto result = values.try_emplace(1, std::move(candidate));
  EXPECT_FALSE(result.second);
  ASSERT_TRUE(candidate);
  EXPECT_EQ(*candidate, 99);
  EXPECT_EQ(*result.first->second, 42);
  values.insert_or_assign(1, std::move(candidate));
  EXPECT_FALSE(candidate);
  EXPECT_EQ(*values.get(1), 99);

  StringMap<int> keyed;
  keyed.try_emplace("present", 1);
  String key("present");
  auto assigned = keyed.insert_or_assign(std::move(key), 2);
  EXPECT_FALSE(assigned.second);
  EXPECT_EQ(key, "present");
  EXPECT_EQ(assigned.first->second, 2);
}

TEST(FlatHashMap, BorrowedLookupPreservesBytesAndCaseRules) {
  String key("x\0中", 5);
  std::string bytes = "!" + key.utf8() + "?";
  StringView query(bytes.data() + 1, key.utf8Size());
  StringMap<int> values{{key, 7}};
  StableStringMap<int> stable{{key, 8}};
  EXPECT_EQ(hash<String>{}(key), hash<String>{}(query));
  EXPECT_EQ(values.get(query), 7);
  EXPECT_EQ(stable.get(query), 8);
  *values.ptr(query) = 9;
  EXPECT_EQ(values.value(query), 9);
  EXPECT_EQ(values.maybe(query).get(), 9);
  CaseInsensitiveStringMap<int> insensitive{{"MiXeD中", 10}};
  EXPECT_EQ(insensitive.get(StringView("mixed中")), 10);
  EXPECT_EQ(Color(StringView("RED")), Color::Red);
}

TEST(FlatHashSet, Preset) {
  FlatHashSet<int> testSet = {42, 63};
  ASSERT_EQ(testSet.find(41), testSet.end());
  ASSERT_EQ(*testSet.find(42), 42);
  ASSERT_EQ(*testSet.find(63), 63);
  ASSERT_EQ(testSet.find(64), testSet.end());
  ASSERT_EQ(testSet.size(), 2u);

  testSet.erase(testSet.find(42));
  ASSERT_EQ(testSet.find(42), testSet.end());
  ASSERT_EQ(*testSet.find(63), 63);
  ASSERT_EQ(testSet.size(), 1u);

  testSet.erase(testSet.find(63));
  ASSERT_EQ(testSet.find(42), testSet.end());
  ASSERT_EQ(testSet.find(63), testSet.end());
  ASSERT_EQ(testSet.size(), 0u);

  testSet.insert(12);
  testSet.insert(24);
  ASSERT_EQ(*testSet.find(12), 12);
  ASSERT_EQ(testSet.size(), 2u);
  testSet.clear();

  ASSERT_EQ(testSet.find(12), testSet.end());
  ASSERT_EQ(testSet.size(), 0u);

  EXPECT_TRUE(testSet.insert(7).second);
  EXPECT_TRUE(testSet.insert(11).second);
  EXPECT_FALSE(testSet.insert(7).second);

  ASSERT_EQ(testSet.size(), 2u);

  FlatHashSet<int> testSet2(testSet.begin(), testSet.end());
  ASSERT_EQ(testSet, testSet2);

  ASSERT_EQ(testSet.erase(testSet.begin(), testSet.end()), testSet.end());
  ASSERT_EQ(testSet.size(), 0u);

  ASSERT_NE(testSet, testSet2);

  FlatHashSet<int> testSet3(testSet.begin(), testSet.end());
  ASSERT_EQ(testSet3.size(), 0u);

  testSet2 = testSet;
  ASSERT_EQ(testSet, testSet2);
}

TEST(FlatHashSet, Random) {
  RandomService random;
  List<Vec2I> keys;
  for (unsigned i = 0; i < 100000; ++i)
    keys.append(Vec2I(i * 743202097, i * 205495087));
  random.shuffle(keys);

  FlatHashSet<Vec2I> testSet;
  for (std::size_t i = 0; i < keys.size(); ++i)
    testSet.insert(keys[i]);

  random.shuffle(keys);
  for (std::size_t i = 0; i < keys.size() / 2; ++i)
    testSet.erase(keys[i]);

  random.shuffle(keys);
  for (std::size_t i = 0; i < keys.size() / 3; ++i)
    testSet.insert(keys[i]);

  random.shuffle(keys);
  for (std::size_t i = 0; i < keys.size() / 2; ++i)
    testSet.erase(keys[i]);

  random.shuffle(keys);
  for (auto k : keys) {
    auto i = testSet.find(k);
    if (i != testSet.end()) {
      ASSERT_TRUE(*i == k);
    }
  }

  random.shuffle(keys);
  for (auto k : keys) {
    testSet.insert(k);
    ASSERT_TRUE(testSet.find(k) != testSet.end());
  }

  List<Vec2I> cmp;
  for (auto const& k : testSet)
    cmp.append(k);
  cmp.sort();
  keys.sort();
  ASSERT_TRUE(cmp == keys);

  random.shuffle(keys);
  for (auto k : keys) {
    testSet.erase(k);
    ASSERT_TRUE(testSet.find(k) == testSet.end());
  }

  ASSERT_TRUE(testSet.empty());
}

TEST(FlatHashMap, Preset) {
  FlatHashMap<int, int> testMap = {{42, 42}, {63, 63}};
  ASSERT_EQ(testMap.find(41), testMap.end());
  ASSERT_EQ(testMap.find(42)->second, 42);
  ASSERT_EQ(testMap.find(63)->second, 63);
  ASSERT_EQ(testMap.find(64), testMap.end());
  ASSERT_EQ(testMap.size(), 2u);

  testMap.erase(testMap.find(42));
  ASSERT_EQ(testMap.find(42), testMap.end());
  ASSERT_EQ(testMap.find(63)->second, 63);
  ASSERT_EQ(testMap.size(), 1u);

  testMap.erase(testMap.find(63));
  ASSERT_EQ(testMap.find(42), testMap.end());
  ASSERT_EQ(testMap.find(63), testMap.end());
  ASSERT_EQ(testMap.size(), 0u);

  testMap.insert({12, 12});
  testMap.insert({24, 24});
  ASSERT_EQ(testMap.find(12)->second, 12);
  ASSERT_EQ(testMap.size(), 2u);
  testMap.clear();

  ASSERT_EQ(testMap.find(12), testMap.end());
  ASSERT_EQ(testMap.size(), 0u);

  EXPECT_TRUE(testMap.insert({7, 7}).second);
  EXPECT_TRUE(testMap.insert({11, 11}).second);
  EXPECT_FALSE(testMap.insert({7, 7}).second);

  ASSERT_EQ(testMap.size(), 2u);

  FlatHashMap<int, int> testMap2(testMap.begin(), testMap.end());
  ASSERT_EQ(testMap, testMap2);

  ASSERT_EQ(testMap.erase(testMap.begin(), testMap.end()), testMap.end());
  ASSERT_EQ(testMap.size(), 0u);

  ASSERT_NE(testMap, testMap2);

  FlatHashMap<int, int> testMap3(testMap.begin(), testMap.end());
  ASSERT_EQ(testMap3.size(), 0u);

  testMap2 = testMap;
  ASSERT_EQ(testMap, testMap2);
}

TEST(FlatHashMap, Random) {
  RandomService random;
  List<std::pair<Vec2I, int>> values;
  for (unsigned i = 0; i < 100000; ++i)
    values.append({Vec2I(i * 743202097, i * 205495087), i});
  random.shuffle(values);

  FlatHashMap<Vec2I, int> testMap;
  for (auto v : values)
    testMap.insert(v);

  random.shuffle(values);
  for (std::size_t i = 0; i < values.size() / 2; ++i)
    testMap.erase(values[i].first);

  random.shuffle(values);
  for (std::size_t i = 0; i < values.size() / 3; ++i)
    testMap.insert(values[i]);

  random.shuffle(values);
  for (std::size_t i = 0; i < values.size() / 2; ++i)
    testMap.erase(values[i].first);

  random.shuffle(values);
  for (auto v : values) {
    auto i = testMap.find(v.first);
    if (i != testMap.end()) {
      ASSERT_TRUE(i->second == v.second);
    }
  }

  random.shuffle(values);
  for (auto v : values) {
    ASSERT_TRUE(testMap.insert(v).first->second == v.second);
    ASSERT_TRUE(testMap.at(v.first) == v.second);
  }

  random.shuffle(values);
  for (auto v : values) {
    ASSERT_EQ(testMap.erase(v.first), 1u);
    ASSERT_TRUE(testMap.find(v.first) == testMap.end());
  }

  ASSERT_TRUE(testMap.empty());
}

TEST(FlatHashMap, EraseWhileIterating) {
  List<std::pair<Vec2I, int>> values;
  for (unsigned i = 0; i < 100000; ++i)
    values.append({Vec2I(i * 743202097, i * 205495087), i});

  FlatHashMap<Vec2I, int> testMap;
  for (auto v : values)
    testMap.insert(v);

  for (auto it = testMap.begin(); it != testMap.end();) {
    if (it->second % 3 == 0)
      it = testMap.erase(it);
    else
      ++it;
  }

  ASSERT_EQ(testMap.size(), values.size() - (values.size() + 2) / 3);
  for (auto const& value : values) {
    auto found = testMap.find(value.first);
    if (value.second % 3 == 0)
      ASSERT_EQ(found, testMap.end());
    else {
      ASSERT_NE(found, testMap.end());
      EXPECT_EQ(found->second, value.second);
    }
  }
}

TEST(FlatHashMap, NonDefaultAlignedValueLifetime) {
  struct alignas(32) Payload {
    Payload() = delete;
    explicit Payload(std::shared_ptr<int> value) : value(std::move(value)) {}
    std::shared_ptr<int> value;
  };
  struct CollisionHash {
    std::size_t operator()(int) const { return 0; }
  };
  using Map = FlatHashMap<int, Payload, CollisionHash>;

  std::array<std::weak_ptr<int>, 32> lifetimes;
  Map original;
  for (int i = 0; i < 32; ++i) {
    auto value = std::make_shared<int>(i);
    lifetimes[i] = value;
    ASSERT_TRUE(original.insert({i, Payload(std::move(value))}).second);
  }
  original.reserve(128);
  for (auto const& entry : original) {
    EXPECT_EQ(*entry.second.value, entry.first);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&entry.second) % alignof(Payload), 0u);
    EXPECT_EQ(lifetimes[entry.first].use_count(), 1);
  }

  Map copy = original;
  copy.reserve(256);
  for (int i = 0; i < 32; ++i) {
    ASSERT_NE(copy.find(i), copy.end());
    EXPECT_EQ(*copy.find(i)->second.value, i);
    EXPECT_EQ(lifetimes[i].use_count(), 2);
  }
  ASSERT_EQ(original.erase(0), 1u);
  EXPECT_EQ(lifetimes[0].use_count(), 1);

  Map moved = std::move(copy);
  ASSERT_EQ(moved.size(), 32u);
  moved.clear();
  EXPECT_TRUE(lifetimes[0].expired());
  for (int i = 1; i < 32; ++i)
    EXPECT_EQ(lifetimes[i].use_count(), 1);

  moved = original;
  for (int i = 1; i < 32; ++i)
    EXPECT_EQ(lifetimes[i].use_count(), 2);
  original.clear();
  for (int i = 1; i < 32; ++i)
    EXPECT_EQ(lifetimes[i].use_count(), 1);
  moved = original;
  for (auto const& lifetime : lifetimes)
    EXPECT_TRUE(lifetime.expired());

  auto reused = std::make_shared<int>(99);
  lifetimes[0] = reused;
  ASSERT_TRUE(original.insert({99, Payload(std::move(reused))}).second);
  ASSERT_NE(original.find(99), original.end());
  EXPECT_EQ(*original.find(99)->second.value, 99);
  original.clear();
  EXPECT_TRUE(lifetimes[0].expired());
}
