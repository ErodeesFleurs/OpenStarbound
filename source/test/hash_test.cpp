import std;
import star.string;
import star.xx_hash;
import star.static_random;

#include "gtest/gtest.h"


TEST(HashTest, StringHashCoversEveryUtf8Byte) {
  auto hashOf = [](std::string_view str) {
    Star::XXHash64 hash(0);
    Star::xxHash64Push(hash, Star::String(str));
    return hash.digest();
  };

  // These two share their first two bytes but differ afterwards.  They used to
  // hash identically because the byte count hashed was String::size(), which is
  // a code point count, not the number of utf8 bytes.
  EXPECT_NE(hashOf("a\xC3\xA9"), hashOf("a\xC3\x83"));
}

TEST(HashTest, TextSeedsDoNotCollapseToBoolean) {
  EXPECT_EQ(620815165u, Star::staticRandomHash32("SystemWorldGeneration"));
  EXPECT_EQ(15694224458012596113ull, Star::staticRandomHash64("SystemWorldGeneration"));

  std::string bounded("a\0\xC3\xA9", 4);
  EXPECT_EQ(3636065869u, Star::staticRandomHash32(bounded));
  EXPECT_EQ(15767761046075041944ull, Star::staticRandomHash64(bounded));
  EXPECT_EQ(708170612u, Star::staticRandomHash32("a\0\xC3\xA9"));
  EXPECT_EQ(1230765182578524663ull, Star::staticRandomHash64("a\0\xC3\xA9"));

  EXPECT_EQ(2612236624u, Star::staticRandomHash32(true));
  EXPECT_EQ(2438509545873450692ull, Star::staticRandomHash64(true));
}
