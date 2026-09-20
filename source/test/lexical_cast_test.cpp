#include "StarLexicalCast.hpp"

#include "gtest/gtest.h"

using namespace Star;

TEST(LexicalCastTest, BoolRequiresTheExactLiteral) {
  EXPECT_TRUE(lexicalCast<bool>("true", "true" + 4));
  EXPECT_FALSE(lexicalCast<bool>("false", "false" + 5));

  // These used to be accepted: the comparison only looked at as many characters
  // as the input had, so any prefix of the literal matched.
  EXPECT_THROW(lexicalCast<bool>("", ""), BadLexicalCast);
  EXPECT_THROW(lexicalCast<bool>("t", "t" + 1), BadLexicalCast);
  EXPECT_THROW(lexicalCast<bool>("tru", "tru" + 3), BadLexicalCast);
  EXPECT_THROW(lexicalCast<bool>("fals", "fals" + 4), BadLexicalCast);
  EXPECT_THROW(lexicalCast<bool>("truex", "truex" + 5), BadLexicalCast);

  bool value = true;
  EXPECT_FALSE(tryLexicalCast(value, "tru", "tru" + 3));
  EXPECT_TRUE(tryLexicalCast(value, "false", "false" + 5));
  EXPECT_FALSE(value);
  EXPECT_TRUE(tryLexicalCast(value, "true", "true" + 4));
  EXPECT_TRUE(value);
}
