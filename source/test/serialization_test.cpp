import std;
import star.buffer;
import star.format;
import star.data_stream_devices;
import star.string;
import star.map;
import star.list;
import star.data_stream_extra;
import star.byte_array;
import star.multi_array;

#include "gtest/gtest.h"

using namespace Star;

template <typename T>
void testMap(T const& map) {
  auto byteArray = DataStreamBuffer::serializeMapContainer(map);
  ASSERT_TRUE(byteArray);
  auto mapOut = DataStreamBuffer::deserializeMapContainer<T>(std::move(*byteArray));
  ASSERT_TRUE(mapOut);
  EXPECT_EQ(map, *mapOut);
}

TEST(DataStreamTest, All) {
  Map<int, int> map1 = {
      {1, 2}, {3, 4}, {5, 6},
  };

  Map<String, int> map2 = {
      {"asdf", 1}, {"asdf1", 2}, {"omg", 2},
  };

  Map<String, int> map3 = {};

  testMap(map1);
  testMap(map2);
  testMap(map3);
}

TEST(DataStreamTest, SequenceWireBytesAndTruncatedPrefix) {
  List<std::uint8_t> values{0x10, 0x20};
  auto encoded = DataStreamBuffer::serialize(values);
  ASSERT_TRUE(encoded);
  EXPECT_EQ(*encoded, ByteArray("\x02\x10\x20", 3));

  List<std::uint8_t> decoded{0x7f};
  auto read = DataStreamBuffer::deserialize(decoded, ByteArray("\x03\x10\x20", 3));
  ASSERT_FALSE(read);
  EXPECT_EQ(decoded, values);
}

TEST(DataStreamTest, BoundedDecodeRetainsAcceptedPrefixOnCapacityError) {
  StaticList<std::uint8_t, 2> decoded;
  auto read = DataStreamBuffer::deserialize(decoded, ByteArray("\x03\x10\x20\x30", 4));
  ASSERT_FALSE(read);
  ASSERT_EQ(decoded.size(), 2u);
  EXPECT_EQ(decoded[0], 0x10);
  EXPECT_EQ(decoded[1], 0x20);
}

TEST(DataStreamTest, OrderedMapWireBytesAndDuplicateKeys) {
  Map<std::uint8_t, std::uint8_t> values{{2, 0x22}, {1, 0x11}};
  auto encoded = DataStreamBuffer::serialize(values);
  ASSERT_TRUE(encoded);
  EXPECT_EQ(*encoded, ByteArray("\x02\x01\x11\x02\x22", 5));

  Map<std::uint8_t, std::uint8_t> decoded{{9, 9}};
  auto read = DataStreamBuffer::deserialize(decoded, ByteArray("\x02\x07\x0b\x07\x16", 5));
  ASSERT_TRUE(read);
  ASSERT_EQ(decoded.size(), 1u);
  auto value = decoded.get(7);
  ASSERT_TRUE(value);
  EXPECT_EQ(**value, 0x0b);
}

TEST(ExternalBufferTest, ClosedDeviceRejectsZeroLengthRead) {
  ExternalBuffer buffer{std::span<char const>{}};
  auto read = buffer.read(nullptr, 0);
  ASSERT_TRUE(read);
  EXPECT_EQ(*read, 0u);
  ASSERT_TRUE(buffer.close());
  auto closedRead = buffer.read(nullptr, 0);
  ASSERT_FALSE(closedRead);
  EXPECT_TRUE(closedRead.error().is<IOException>());
}

TEST(DataStreamTest, ExternalSpanBorrowsSlicesAndResetRewinds) {
  std::array<char, 4> bytes{0x10, 0x20, 0x30, 0x40};
  DataStreamExternalBuffer stream{std::span<char const>(bytes).subspan(1, 2)};
  EXPECT_EQ(stream.ptr(), bytes.data() + 1);
  EXPECT_EQ(stream.size(), 2u);
  bytes[1] = 0x25;
  auto first = stream.read<std::uint8_t>();
  ASSERT_TRUE(first);
  EXPECT_EQ(*first, 0x25);
  auto remaining = stream.remaining();
  ASSERT_TRUE(remaining);
  EXPECT_EQ(*remaining, 1u);
  auto write = stream.write<std::uint8_t>(0x7f);
  ASSERT_FALSE(write);
  EXPECT_EQ(bytes[2], 0x30);

  stream.reset(std::span<char const>(bytes).last(1));
  auto position = stream.pos();
  ASSERT_TRUE(position);
  EXPECT_EQ(*position, 0u);
  auto last = stream.read<std::uint8_t>();
  ASSERT_TRUE(last);
  EXPECT_EQ(*last, 0x40);
  EXPECT_FALSE(stream.read<std::uint8_t>());

  stream.reset(std::span<char const>{});
  auto atEnd = stream.atEnd();
  ASSERT_TRUE(atEnd);
  EXPECT_TRUE(*atEnd);
  EXPECT_FALSE(stream.read<std::uint8_t>());
}

TEST(DataStreamTest, MultiArrayPreservesDimensionsAndLastIndexContiguousWireOrder) {
  MultiArray<std::uint8_t, 2> array(2, 3);
  {
    auto view = array.view();
    for (std::size_t x = 0; x < view.extent(0); ++x)
      for (std::size_t y = 0; y < view.extent(1); ++y)
        view[x, y] = static_cast<std::uint8_t>((x + 1) * 0x10 + y);
  }

  auto encoded = DataStreamBuffer::serialize(array);
  ASSERT_TRUE(encoded);
  EXPECT_EQ(*encoded, ByteArray("\x02\x03\x10\x11\x12\x20\x21\x22", 8));
  auto decoded = DataStreamBuffer::deserialize<MultiArray<std::uint8_t, 2>>(*encoded);
  ASSERT_TRUE(decoded);
  auto const view = std::as_const(*decoded).view();
  ASSERT_EQ(view.extent(0), 2u);
  ASSERT_EQ(view.extent(1), 3u);
  EXPECT_EQ((view[0, 2]), 0x12);
  EXPECT_EQ((view[1, 0]), 0x20);
  EXPECT_EQ((view[1, 2]), 0x22);
}
