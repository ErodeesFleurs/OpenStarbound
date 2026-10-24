import std;
import star.buffer;
import star.format;
import star.data_stream_devices;
import star.string;
import star.map;
import star.list;
import star.data_stream_extra;
import star.byte_array;

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
