#include "StarLexicalCast.hpp"

#include <string_view>

#include "gtest/gtest.h"

using namespace Star;

TEST(LexicalCastTest, BoolRequiresTheExactLiteral) {
  auto cast = [](std::string_view text) {
    return lexicalCast<bool>(text.data(), text.data() + text.size());
  };
  auto tryCast = [](bool& value, std::string_view text) {
    return tryLexicalCast(value, text.data(), text.data() + text.size());
  };

  EXPECT_TRUE(cast("true"));
  EXPECT_FALSE(cast("false"));

  // These used to be accepted: the comparison only looked at as many characters
  // as the input had, so any prefix of the literal matched.
  EXPECT_THROW(cast(""), BadLexicalCast);
  EXPECT_THROW(cast("t"), BadLexicalCast);
  EXPECT_THROW(cast("tru"), BadLexicalCast);
  EXPECT_THROW(cast("fals"), BadLexicalCast);
  EXPECT_THROW(cast("truex"), BadLexicalCast);

  bool value = true;
  EXPECT_FALSE(tryCast(value, "tru"));
  EXPECT_TRUE(tryCast(value, "false"));
  EXPECT_FALSE(value);
  EXPECT_TRUE(tryCast(value, "true"));
  EXPECT_TRUE(value);
}
