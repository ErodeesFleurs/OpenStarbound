#include "StarDataStreamDevices.hpp"
#include "StarEncode.hpp"

#include "gtest/gtest.h"

using namespace Star;

namespace {

void expectSerializedHex(ByteArray const& data, String const& expectedHex) {
  EXPECT_EQ(hexEncode(data), expectedHex);
}

}

template <typename T>
void testMap(T const& map) {
  auto byteArray = DataStreamBuffer::serializeMapContainer(map);
  auto mapOut = DataStreamBuffer::deserializeMapContainer<T>(byteArray);
  EXPECT_EQ(map, mapOut);
}

TEST(DataStreamTest, PrimitiveGoldenBytes) {
  DataStreamBuffer ds;
  ds << uint8_t(0x12);
  ds << uint16_t(0x3456);
  ds << uint32_t(0x789abcde);
  ds << uint64_t(0x0102030405060708);
  ds << int16_t(-2);
  ds << int32_t(-3);

  expectSerializedHex(ds.data(), "123456789abcde0102030405060708fffefffffffd");

  DataStreamBuffer readDs(ds.data());
  EXPECT_EQ(readDs.read<uint8_t>(), uint8_t(0x12));
  EXPECT_EQ(readDs.read<uint16_t>(), uint16_t(0x3456));
  EXPECT_EQ(readDs.read<uint32_t>(), uint32_t(0x789abcde));
  EXPECT_EQ(readDs.read<uint64_t>(), uint64_t(0x0102030405060708));
  EXPECT_EQ(readDs.read<int16_t>(), -2);
  EXPECT_EQ(readDs.read<int32_t>(), -3);
  EXPECT_TRUE(readDs.atEnd());
}

TEST(DataStreamTest, VlqGoldenBytes) {
  DataStreamBuffer ds;
  ds.writeVlqU(0);
  ds.writeVlqU(127);
  ds.writeVlqU(128);
  ds.writeVlqU(16383);
  ds.writeVlqI(-1);
  ds.writeVlqI(1);
  ds.writeVlqI(-64);
  ds.writeVlqI(64);
  ds.writeVlqS(NPos);
  ds.writeVlqS(0);

  expectSerializedHex(ds.data(), "007f8100ff7f01027f81000001");

  DataStreamBuffer readDs(ds.data());
  EXPECT_EQ(readDs.readVlqU(), uint64_t(0));
  EXPECT_EQ(readDs.readVlqU(), uint64_t(127));
  EXPECT_EQ(readDs.readVlqU(), uint64_t(128));
  EXPECT_EQ(readDs.readVlqU(), uint64_t(16383));
  EXPECT_EQ(readDs.readVlqI(), -1);
  EXPECT_EQ(readDs.readVlqI(), 1);
  EXPECT_EQ(readDs.readVlqI(), -64);
  EXPECT_EQ(readDs.readVlqI(), 64);
  EXPECT_EQ(readDs.readVlqS(), NPos);
  EXPECT_EQ(readDs.readVlqS(), size_t(0));
  EXPECT_TRUE(readDs.atEnd());
}

TEST(DataStreamTest, StringGoldenBytes) {
  DataStreamBuffer ds;
  ds << String("abc");
  expectSerializedHex(ds.data(), "03616263");

  DataStreamBuffer readDs(ds.data());
  EXPECT_EQ(readDs.read<String>(), "abc");
  EXPECT_TRUE(readDs.atEnd());

  DataStreamBuffer nullTerminatedDs;
  nullTerminatedDs.setNullTerminatedStrings(true);
  nullTerminatedDs << String("abc");
  expectSerializedHex(nullTerminatedDs.data(), "61626300");

  DataStreamBuffer nullTerminatedReadDs(nullTerminatedDs.data());
  nullTerminatedReadDs.setNullTerminatedStrings(true);
  EXPECT_EQ(nullTerminatedReadDs.read<String>(), "abc");
  EXPECT_TRUE(nullTerminatedReadDs.atEnd());
}

TEST(DataStreamTest, MapGoldenBytes) {
  Map<int, int> map = {
      {1, 2}, {3, 4}, {5, 6},
  };

  auto byteArray = DataStreamBuffer::serializeMapContainer(map);
  expectSerializedHex(byteArray, "03000000010000000200000003000000040000000500000006");
  auto mapOut = DataStreamBuffer::deserializeMapContainer<Map<int, int>>(byteArray);
  EXPECT_EQ(mapOut, map);
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
