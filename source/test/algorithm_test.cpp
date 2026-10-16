import star.list;

import std;

#include "gtest/gtest.h"

import star.algorithm;
import star.pythonic;
import star.data_stream;
import star.data_stream_devices;
import star.data_stream_extra;
import star.bytes;
using namespace Star;

TEST(any, allTests) {
  int a = 60;
  int asdf[] = {1, 2, 3, 4, 5, 6};

  EXPECT_TRUE(any(asdf, [&](int b) { return b < a; }));
  EXPECT_FALSE(any(asdf, [&](int b) { return b > a; }));
  EXPECT_TRUE(any(asdf, [&](int b) { return a % b == 0; }));

  bool b[] = {false, false, false, true};
  bool c[] = {false, false, false, false};
  bool d[] = {false, false, true, true};
  bool e[] = {true, true, true, true};
  int f[] = {0, 1, 0, 0, 0, 3};

  EXPECT_TRUE(any(b));
  EXPECT_FALSE(any(c));
  EXPECT_TRUE(any(d));
  EXPECT_TRUE(any(e));
  EXPECT_TRUE(any(f));
}

TEST(all, allTests) {
  int a = 60;
  int asdf[] = {1, 2, 3, 4, 5, 6};

  EXPECT_TRUE(all(asdf, [&](int b) { return b < a; }));
  EXPECT_FALSE(all(asdf, [&](int b) { return b > a; }));
  EXPECT_TRUE(all(asdf, [&](int b) { return a % b == 0; }));

  bool b[] = {false, false, false, true};
  bool c[] = {false, false, false, false};
  bool d[] = {false, false, true, true};
  bool e[] = {true, true, true, true};
  int f[] = {0, 1, 0, 0, 0, 3};

  EXPECT_FALSE(all(b));
  EXPECT_FALSE(all(c));
  EXPECT_FALSE(all(d));
  EXPECT_TRUE(all(e));
  EXPECT_FALSE(all(f));
}

TEST(ContainerOperators, allTests) {
  List<bool> a{false, false, true, false};
  List<int> b{1, 1, 0, 1};
  List<int> c = a.transformed([](bool a) { return a ? 0 : 1; });
  List<int> d{1, 2, 3, 5};
  List<int> e{1, 3, 5};
  List<int> f = d.filtered([](int i) { return i % 2 == 1; });

  EXPECT_TRUE(a.any());
  EXPECT_FALSE(a.all());
  EXPECT_EQ(b, c);
  EXPECT_EQ(e, f);
}

template <int Amount>
struct Times {
  template <typename T>
  T operator()(T t) {
    return t * Amount;
  }
};

template <int Amount>
struct Add {
  template <typename T>
  T operator()(T t) {
    return t + Amount;
  }
};

TEST(Compose, All) {
  auto f = compose(Times<2>(), Add<1>(), Times<2>(), Add<3>());
  EXPECT_EQ(f(5), 34);
}

TEST(ZipTest, All) {
  List<int> a{1, 2, 3};
  std::vector<std::uint64_t> b{5, 4, 3, 2, 1};
  std::deque<std::int64_t> c{3, 2, 2};
  std::list<unsigned> d{0, 0, 0, 0, 4, 8};

  auto zipResult = zip(a, b, c, d);

  EXPECT_EQ(zipResult.size(), 3u);
  EXPECT_EQ(std::get<0>(zipResult[0]), 1);
  EXPECT_EQ(std::get<1>(zipResult[0]), 5u);
  EXPECT_EQ(std::get<2>(zipResult[0]), 3);
  EXPECT_EQ(std::get<3>(zipResult[0]), 0u);
  EXPECT_EQ(std::get<0>(zipResult[2]), 3);
  EXPECT_EQ(std::get<1>(zipResult[2]), 3u);
  EXPECT_EQ(std::get<2>(zipResult[2]), 2);
  EXPECT_EQ(std::get<3>(zipResult[2]), 0u);
}

TEST(ZipWith, All) {
  List<int> a{1, 1, 2, 3, 5, 8};
  List<int> b{5, 4, 3, 2, 1, 0};
  List<int> c{6, 5, 5, 5, 6, 8};
  List<int> d = zipWith<List<int>>(std::plus<int>(), a, b);
  EXPECT_EQ(c, d);
}


TEST(PythonicPredicates, NoncopyableExplicitBoolShortCircuit) {
  struct Truth {
    Truth(bool value, int& visits) : value(value), visits(visits) {}
    Truth(Truth const&) = delete;
    explicit operator bool() const {
      ++visits;
      return value;
    }
    bool value;
    int& visits;
  };

  int visits = 0;
  Truth values[] = {{false, visits}, {true, visits}, {false, visits}};
  EXPECT_TRUE(any(std::begin(values), std::end(values)));
  EXPECT_EQ(visits, 2);
  visits = 0;
  EXPECT_FALSE(all(std::begin(values), std::end(values)));
  EXPECT_EQ(visits, 1);
  visits = 0;
  EXPECT_TRUE(any(values));
  EXPECT_EQ(visits, 2);
  visits = 0;
  EXPECT_FALSE(all(values));
  EXPECT_EQ(visits, 1);
  visits = 0;
  EXPECT_FALSE(any(std::begin(values), std::begin(values)));
  EXPECT_TRUE(all(std::begin(values), std::begin(values)));
  EXPECT_EQ(visits, 0);

  int calls = 0;
  auto predicate = [&](Truth const& value) {
    ++calls;
    return static_cast<bool>(value);
  };
  EXPECT_TRUE(any(values, predicate));
  EXPECT_EQ(calls, 2);
  calls = 0;
  EXPECT_FALSE(all(values, predicate));
  EXPECT_EQ(calls, 1);
  calls = 0;
  EXPECT_FALSE(any(std::begin(values), std::begin(values), predicate));
  EXPECT_TRUE(all(std::begin(values), std::begin(values), predicate));
  EXPECT_EQ(calls, 0);
}

TEST(TupleSerialization, OrderedWireAndEnumWidths) {
  enum class Signed : std::int8_t { Negative = -2 };
  enum class Unsigned : std::uint32_t { High = 0x89abcdefu };
  auto values = std::tuple{Signed::Negative, std::uint16_t{0x1234}, Unsigned::High};
  DataStreamBuffer stream;
  stream << values;
  std::array<unsigned char, 7> expected{0xfe, 0x12, 0x34, 0x89, 0xab, 0xcd, 0xef};
  ASSERT_EQ(stream.size(), expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i)
    EXPECT_EQ(static_cast<unsigned char>(stream.ptr()[i]), expected[i]);

  stream.seek(0);
  decltype(values) restored{};
  stream >> restored;
  EXPECT_EQ(restored, values);
  EXPECT_TRUE(stream.atEnd());

  DataStreamBuffer empty;
  std::tuple<> emptyTuple;
  empty << emptyTuple;
  EXPECT_TRUE(empty.empty());
  empty >> emptyTuple;
  EXPECT_EQ(empty.pos(), 0u);
}

TEST(ListFiltering, MoveOnlyValuesKeepStableOrder) {
  List<std::unique_ptr<int>> values;
  for (int value = 0; value < 8; ++value)
    values.append(std::make_unique<int>(value));
  values.filter([](std::unique_ptr<int>& value) { return *value % 2 != 0; });
  ASSERT_EQ(values.size(), 4u);
  for (std::size_t i = 0; i < values.size(); ++i)
    EXPECT_EQ(*values[i], static_cast<int>(i * 2 + 1));
  values.filter([](auto const&) { return true; });
  EXPECT_EQ(values.size(), 4u);
  values.filter([](auto const&) { return false; });
  EXPECT_TRUE(values.empty());
}

TEST(ByteOrder, FloatingPayloadAndIntegerRepresentations) {
  auto opposite = platformByteOrder() == ByteOrder::LittleEndian ? ByteOrder::BigEndian : ByteOrder::LittleEndian;
  for (auto bits : {std::uint32_t{0}, std::uint32_t{0x80000000}, std::uint32_t{0x7fc12345}, std::uint32_t{0x3f800000}}) {
    auto value = std::bit_cast<float>(bits);
    EXPECT_EQ(std::bit_cast<std::uint32_t>(toByteOrder(opposite, value)), std::byteswap(bits));
    EXPECT_EQ(std::bit_cast<std::uint32_t>(fromByteOrder(opposite, toByteOrder(opposite, value))), bits);
  }
  EXPECT_EQ(toByteOrder(opposite, std::uint16_t{0x1234}), 0x3412u);
  EXPECT_EQ(toByteOrder(opposite, std::int32_t{-2}), std::byteswap(std::int32_t{-2}));
  EXPECT_EQ(toByteOrder(ByteOrder::NoConversion, std::uint64_t{0x12345678}), 0x12345678u);
}
