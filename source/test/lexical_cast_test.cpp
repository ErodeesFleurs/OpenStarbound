import star.lexical_cast;

import std;

#include "gtest/gtest.h"

using namespace Star;

TEST(LexicalCastTest, BoolRequiresTheExactLiteral) {
  auto cast = [](std::string_view text) {
    return lexicalCast<bool>(text.data(), text.data() + text.size());
  };
  auto tryCast = [](bool& value, std::string_view text) {
    return tryLexicalCast(value, text.data(), text.data() + text.size());
  };

  auto trueResult = cast("true");
  ASSERT_TRUE(trueResult);
  EXPECT_TRUE(*trueResult);
  auto falseResult = cast("false");
  ASSERT_TRUE(falseResult);
  EXPECT_FALSE(*falseResult);

  // Prefixes and trailing bytes must not be accepted as boolean literals.
  for (auto text : {"", "t", "tru", "fals", "truex"}) {
    auto result = cast(text);
    ASSERT_FALSE(result);
    EXPECT_TRUE(result.error().is<BadLexicalCast>());
  }

  bool value = true;
  EXPECT_FALSE(tryCast(value, "tru"));
  EXPECT_TRUE(tryCast(value, "false"));
  EXPECT_FALSE(value);
  EXPECT_TRUE(tryCast(value, "true"));
  EXPECT_TRUE(value);
}
