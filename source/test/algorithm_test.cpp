import star.list;
import star.format;

import std;

#include "gtest/gtest.h"

import star.algorithm;
import star.data_stream;
import star.data_stream_devices;
import star.data_stream_extra;
import star.bytes;
import star.array;
import star.vector;
using namespace Star;

TEST(ListPredicates, Any) {
  int a = 60;
  List<int> asdf{1, 2, 3, 4, 5, 6};

  EXPECT_TRUE(asdf.any([&](int b) { return b < a; }));
  EXPECT_FALSE(asdf.any([&](int b) { return b > a; }));
  EXPECT_TRUE(asdf.any([&](int b) { return a % b == 0; }));

  List<bool> b{false, false, false, true};
  List<bool> c{false, false, false, false};
  List<bool> d{false, false, true, true};
  List<bool> e{true, true, true, true};
  List<int> f{0, 1, 0, 0, 0, 3};

  EXPECT_TRUE(b.any());
  EXPECT_FALSE(c.any());
  EXPECT_TRUE(d.any());
  EXPECT_TRUE(e.any());
  EXPECT_TRUE(f.any());
}

TEST(ListPredicates, All) {
  int a = 60;
  List<int> asdf{1, 2, 3, 4, 5, 6};

  EXPECT_TRUE(asdf.all([&](int b) { return b < a; }));
  EXPECT_FALSE(asdf.all([&](int b) { return b > a; }));
  EXPECT_TRUE(asdf.all([&](int b) { return a % b == 0; }));

  List<bool> b{false, false, false, true};
  List<bool> c{false, false, false, false};
  List<bool> d{false, false, true, true};
  List<bool> e{true, true, true, true};
  List<int> f{0, 1, 0, 0, 0, 3};

  EXPECT_FALSE(b.all());
  EXPECT_FALSE(c.all());
  EXPECT_FALSE(d.all());
  EXPECT_TRUE(e.all());
  EXPECT_FALSE(f.all());
}

TEST(ListPredicates, ExpectedBoolPayloadErrorsAndShortCircuit) {
  List<std::expected<bool, StarException>> falseValues{false, false};
  List<std::expected<bool, StarException>> trueValues{true, true};
  EXPECT_EQ(falseValues.any(), false);
  EXPECT_EQ(falseValues.all(), false);
  EXPECT_EQ(trueValues.any(), true);
  EXPECT_EQ(trueValues.all(), true);

  List<std::expected<bool, StarException>> anyError{
      false, std::unexpected(OutOfRangeException("any predicate marker", false)), true};
  auto anyResult = anyError.any();
  ASSERT_FALSE(anyResult);
  EXPECT_TRUE(anyResult.error().is<OutOfRangeException>());
  EXPECT_NE(std::string_view(anyResult.error().what()).find("any predicate marker"), std::string_view::npos);
  // The false first payload settles all() before reaching the stored error.
  EXPECT_EQ(anyError.all(), false);

  List<std::expected<bool, StarException>> allError{
      true, std::unexpected(OutOfRangeException("all predicate marker", false)), false};
  auto allResult = allError.all();
  ASSERT_FALSE(allResult);
  EXPECT_TRUE(allResult.error().is<OutOfRangeException>());
  EXPECT_NE(std::string_view(allResult.error().what()).find("all predicate marker"), std::string_view::npos);
  // The true first payload settles any() before reaching the stored error.
  EXPECT_EQ(allError.any(), true);
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

TEST(ListPredicates, NoncopyableExplicitBoolShortCircuit) {
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
  LinkedList<Truth> values;
  EXPECT_FALSE(values.emplaceAppend(false, visits).value);
  EXPECT_TRUE(values.emplaceAppend(true, visits).value);
  EXPECT_FALSE(values.emplaceAppend(false, visits).value);
  EXPECT_TRUE(values.any());
  EXPECT_EQ(visits, 2);
  visits = 0;
  EXPECT_FALSE(values.all());
  EXPECT_EQ(visits, 1);
  visits = 0;
  LinkedList<Truth> empty;
  EXPECT_FALSE(empty.any());
  EXPECT_TRUE(empty.all());
  EXPECT_EQ(visits, 0);

  int calls = 0;
  auto predicate = [&](Truth const& value) {
    ++calls;
    return static_cast<bool>(value);
  };
  EXPECT_TRUE(values.any(predicate));
  EXPECT_EQ(calls, 2);
  calls = 0;
  EXPECT_FALSE(values.all(predicate));
  EXPECT_EQ(calls, 1);
  calls = 0;
  EXPECT_FALSE(empty.any(predicate));
  EXPECT_TRUE(empty.all(predicate));
  EXPECT_EQ(calls, 0);
}

TEST(TupleSerialization, OrderedWireAndEnumWidths) {
  enum class Signed : std::int8_t { Negative = -2 };
  enum class Unsigned : std::uint32_t { High = 0x89abcdefu };
  auto values = std::tuple{Signed::Negative, std::uint16_t{0x1234}, Unsigned::High};
  DataStreamBuffer stream;
  ASSERT_TRUE(stream << values);
  std::array<unsigned char, 7> expected{0xfe, 0x12, 0x34, 0x89, 0xab, 0xcd, 0xef};
  ASSERT_EQ(stream.size(), expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i)
    EXPECT_EQ(static_cast<unsigned char>(stream.ptr()[i]), expected[i]);

  ASSERT_TRUE(stream.seek(0));
  decltype(values) restored{};
  ASSERT_TRUE(stream >> restored);
  EXPECT_EQ(restored, values);
  auto atEnd = stream.atEnd();
  ASSERT_TRUE(atEnd);
  EXPECT_TRUE(*atEnd);

  DataStreamBuffer empty;
  std::tuple<> emptyTuple;
  ASSERT_TRUE(empty << emptyTuple);
  EXPECT_TRUE(empty.empty());
  ASSERT_TRUE(empty >> emptyTuple);
  auto position = empty.pos();
  ASSERT_TRUE(position);
  EXPECT_EQ(*position, 0u);
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

TEST(FixedContainerCopy, PreservesSinglePassInputPosition) {
  std::istringstream arrayInput("1 2 3 4 5");
  auto array = Array<int, 3>::copyFrom(std::istream_iterator<int>(arrayInput), 2);
  EXPECT_EQ(array, (Array<int, 3>{1, 2, 0}));
  int next = 0;
  arrayInput >> next;
  EXPECT_EQ(next, 4);

  std::istringstream vectorInput("1 2 3 4 5");
  auto vector = Vector<int, 2>::copyFrom(std::istream_iterator<int>(vectorInput));
  EXPECT_EQ(vector, (Vector<int, 2>{1, 2}));
  vectorInput >> next;
  EXPECT_EQ(next, 4);
}
