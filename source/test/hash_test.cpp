#include "StarHash.hpp"
#include "StarXXHash.hpp"

#include "gtest/gtest.h"

TEST(HashTest, All) {
  enum SomeEnum { Foo, Bar };

  std::tuple<int, int, bool> testTuple(1, 2, false);
  std::pair<SomeEnum, int> testPair(SomeEnum::Bar, 10);

  // Yeah yeah, I know that it's technically possible for the hash to be zero,
  // but it's not!
  EXPECT_NE(Star::hash<decltype(testTuple)>()(testTuple), 0u);
  EXPECT_NE(Star::hash<decltype(testPair)>()(testPair), 0u);
}

TEST(HashTest, StringHashCoversEveryUtf8Byte) {
  auto hashOf = [](char const* str) {
    Star::XXHash64 hash(0);
    Star::xxHash64Push(hash, Star::String(str));
    return hash.digest();
  };

  // These two share their first two bytes but differ afterwards.  They used to
  // hash identically because the byte count hashed was String::size(), which is
  // a code point count, not the number of utf8 bytes.
  EXPECT_NE(hashOf("a\xC3\xA9"), hashOf("a\xC3\x83"));
  EXPECT_EQ(hashOf("a\xC3\xA9"), hashOf("a\xC3\xA9"));
}
