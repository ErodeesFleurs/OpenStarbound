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
