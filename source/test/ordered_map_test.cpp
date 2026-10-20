import std;
import star.ordered_map;
import star.list;
import star.maybe;
import star.lru_cache;
import star.ttl_cache;
import star.random;

#include "gtest/gtest.h"

using namespace Star;

TEST(OrderedMap, Insert) {
  OrderedMap<int, int> map;
  map.insert({3, 3});
  map.insert({2, 2});
  map.insert({1, 1});

  {
    List<int> comp = {3, 2, 1};
    EXPECT_EQ(comp, map.keys());
  }

  {
    List<int> comp = {3, 2, 1};
    List<int> keys;
    for (auto i = map.begin(); i != map.end(); ++i)
      keys.append(i->first);
    EXPECT_EQ(comp, keys);
  }

  {
    List<int> comp = {1, 2, 3};
    List<int> keys;
    for (auto i = map.rbegin(); i != map.rend(); ++i)
      keys.append(i->first);
    EXPECT_EQ(comp, keys);
  }
}

TEST(OrderedMap, Getters) {
  OrderedMap<int, int> map;
  map[1] = 1;
  map[2] = 2;
  map[3] = 3;

  EXPECT_EQ(map.get(1), 1);
  EXPECT_EQ(map.get(2), 2);
  EXPECT_EQ(map.get(3), 3);

  EXPECT_EQ(map.ptr(3), &map.get(3));
}

TEST(OrderedMap, ConstGetters) {
  OrderedHashMap<int, int> const map{
    {1, 1},
    {2, 2},
    {3, 3}
  };

  EXPECT_EQ(map.get(1), 1);
  EXPECT_EQ(map.get(2), 2);
  EXPECT_EQ(map.get(3), 3);

  EXPECT_EQ(map.ptr(3), &map.get(3));

  EXPECT_EQ(map.value(4, 4), 4);
  EXPECT_EQ(map.maybe(5), Maybe<int>());
}

TEST(OrderedMap, Sorting) {
  OrderedMap<int, int> map{
    {1, 5},
    {3, 3},
    {2, 4},
    {5, 1},
    {4, 2}
  };

  EXPECT_EQ(map.keys(), List<int>({1, 3, 2, 5, 4}));
  map.sortByKey();
  EXPECT_EQ(map.keys(), List<int>({1, 2, 3, 4, 5}));
  map.sortByValue();
  EXPECT_EQ(map.keys(), List<int>({5, 4, 3, 2, 1}));
}

TEST(OrderedMap, Removing) {
  OrderedHashMap<int, int> map{
    {5, 5},
    {4, 4},
    {3, 3},
    {2, 2},
    {1, 1}
  };

  map.remove(3);
  map.remove(1);
  EXPECT_EQ(map.keys(), List<int>({5, 4, 2}));
}

TEST(LruCache, HitsRetainEntriesAndDoNotProduce) {
  HashLruCache<int, int> cache(1);
  unsigned calls = 0;
  auto produce = [&](int const& key) { ++calls; return key * 10; };
  EXPECT_EQ(cache.get(7, produce), 70);
  EXPECT_EQ(cache.get(7, produce), 70);
  EXPECT_EQ(calls, 1u);
  cache.setMaxSize(2);
  cache.set(8, 80);
  EXPECT_EQ(cache.get(8, produce), 80);
  EXPECT_EQ(cache.currentSize(), 2u);
  EXPECT_EQ(cache.keys(), (List<int>{7, 8}));
}

TEST(LruCache, MissEvictsLeastRecentAndPreservesFailureOrdering) {
  HashLruCache<int, int> cache(2);
  cache.set(1, 10);
  cache.set(2, 20);
  EXPECT_EQ(cache.get(1, [](int const&) { return -1; }), 10);
  EXPECT_EQ(cache.get(3, [](int const& key) { return key * 10; }), 30);
  EXPECT_EQ(cache.keys(), (List<int>{1, 3}));
  EXPECT_THROW(cache.get(4, [](int const&) -> int { throw std::runtime_error("producer"); }), std::runtime_error);
  EXPECT_EQ(cache.keys(), (List<int>{3}));
  cache.set(5, 50);
  cache.setMaxSize(0);
  EXPECT_EQ(cache.maxSize(), 1u);
  EXPECT_EQ(cache.keys(), (List<int>{5}));
}

TEST(LruCache, BorrowsMutableNoncopyableCallbacks) {
  struct Producer {
    std::unique_ptr<int> calls = std::make_unique<int>(0);
    int operator()(int const& key) { ++*calls; return key * 10; }
  } producer;
  HashLruCache<int, int> cache(2);
  EXPECT_EQ(cache.get(1, producer), 10);
  EXPECT_EQ(cache.get(1, producer), 10);
  EXPECT_EQ(*producer.calls, 1);
  cache.set(2, 20);
  auto filter = [state = std::make_unique<int>(0)](int const& key, int& value) mutable {
    ++*state;
    value += 1;
    return key == 1;
  };
  cache.removeWhere(filter);
  ASSERT_NE(cache.ptr(2), nullptr);
  EXPECT_EQ(*cache.ptr(2), 21);
  EXPECT_EQ(cache.keys(), (List<int>{2}));
}

TEST(TtlCache, MoveOnlyProducerAndOptionalRefreshFilters) {
  RandomService random;
  HashTtlCache<int, int> cache(random, -1, 0, 4, false);
  EXPECT_EQ(cache.get(1, [value = std::make_unique<int>(42)](int const&) mutable { return *value; }), 42);
  struct Refresh {
    explicit operator bool() const { return false; }
    bool operator()(int const&, int const&) { return true; }
  } refresh;
  cache.cleanup(refresh);
  ASSERT_NE(cache.ptr(1), nullptr);
  EXPECT_EQ(*cache.ptr(1), 42);
  std::function<bool(int const&, int const&)> empty;
  cache.cleanup(empty);
  EXPECT_TRUE(cache.keys().empty());
  cache.set(2, 20);
  bool (*nullFilter)(int const&, int const&) = nullptr;
  cache.cleanup(nullFilter);
  EXPECT_TRUE(cache.keys().empty());
  cache.set(3, 30);
  cache.cleanup({});
  EXPECT_TRUE(cache.keys().empty());
}
