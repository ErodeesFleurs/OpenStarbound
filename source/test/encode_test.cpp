import std;
import star.string;
import star.byte_array;
import star.encode;

#include "gtest/gtest.h"

import star.sha256;

using namespace Star;

TEST(EncodeTest, Base64) {
  char const* data =
      "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust "
      "of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, "
      "exceeds the short vehemence of any carnal pleasure.";
  ByteArray testSource = ByteArray(data, std::strlen(data));
  String testEncoded =
      "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhl"
      "ciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29u"
      "dGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55"
      "IGNhcm5hbCBwbGVhc3VyZS4=";

  String encoded = base64Encode(testSource);
  ByteArray decoded = base64Decode(encoded);

  EXPECT_EQ(encoded, testEncoded);
  EXPECT_EQ(decoded, testSource);
}

TEST(EncodeTest, SHA256) {
  char const* data =
      "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust "
      "of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, "
      "exceeds the short vehemence of any carnal pleasure.";
  ByteArray testSource = ByteArray(data, std::strlen(data));
  ByteArray testHash = hexDecode("78fe75026c4390ceccc4e9e6a9428ba8ae5968b458e60b5eebecd682cf24bbf2");

  EXPECT_EQ(sha256(testSource), testHash);
}

TEST(EncodeTest, BorrowedSpans) {
  char output[8]{};
  EXPECT_EQ(hexEncode(std::span<char const>{}, std::span<char>{}), 0u);
  EXPECT_EQ(hexDecode(std::span<char const>{}, std::span<char>{}), 0u);
  EXPECT_EQ(nibbleDecode(std::span<char const>{}, std::span<char>{}), 0u);
  EXPECT_EQ(base64Encode(std::span<char const>{}, std::span<char>{}), 0u);
  EXPECT_EQ(base64Decode(std::span<char const>{}, std::span<char>{}), 0u);

  EXPECT_EQ(hexEncode({"ab", 2}, {output, 3}), 2u);
  EXPECT_EQ(std::string_view(output, 2), "61");
  EXPECT_EQ(hexDecode({"g1f", 3}, output), 1u);
  EXPECT_EQ(static_cast<unsigned char>(output[0]), 1u);
  EXPECT_EQ(nibbleDecode({"gF1", 3}, {output, 2}), 2u);
  EXPECT_EQ(output[0], 0);
  EXPECT_EQ(output[1], 15);

  EXPECT_EQ(base64Encode({"M", 1}, output), 4u);
  EXPECT_EQ(std::string_view(output, 4), "TQ==");
  EXPECT_EQ(base64Encode({"Man", 3}, {output, 2}), 2u);
  EXPECT_EQ(std::string_view(output, 2), "TW");
  EXPECT_EQ(base64Decode({"TWE", 3}, output), 2u);
  EXPECT_EQ(std::string_view(output, 2), "Ma");
  EXPECT_EQ(base64Decode({"TQ=ignored", 10}, output), 1u);
  EXPECT_EQ(output[0], 'M');
  EXPECT_EQ(base64Decode({"TQ!ignored", 10}, output), 1u);
  EXPECT_EQ(output[0], 'M');
  EXPECT_EQ(base64Decode({"TWFu", 4}, {output, 1}), 1u);
  EXPECT_EQ(output[0], 'M');
}
