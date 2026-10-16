import std;
import star.buffer;
import star.format;
import star.data_stream_devices;
import star.string;
import star.map;

#include "gtest/gtest.h"

using namespace Star;

template <typename T>
void testMap(T const& map) {
  auto byteArray = DataStreamBuffer::serializeMapContainer(map);
  auto mapOut = DataStreamBuffer::deserializeMapContainer<T>(byteArray);
  EXPECT_EQ(map, mapOut);
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

TEST(ExternalBufferTest, ClosedDeviceRejectsZeroLengthRead) {
  ExternalBuffer buffer{std::span<char const>{}};
  EXPECT_EQ(buffer.read(nullptr, 0), 0u);
  buffer.close();
  EXPECT_THROW(buffer.read(nullptr, 0), IOException);
}
