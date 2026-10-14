import std;
import star.variant;
import star.maybe;

#include "gtest/gtest.h"

#ifdef STAR_COMPILER_CLANG
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif

using namespace Star;

TEST(VariantTest, All) {
  struct VariantTester {
    std::shared_ptr<int> intptr;
  };

  MVariant<int, double, char, VariantTester> a, b;
  EXPECT_EQ(a.typeIndex(), 0);
  a = 'c';
  EXPECT_EQ(a.typeIndex(), 3u);
  EXPECT_TRUE(a.is<char>());
  a.makeType(1);
  EXPECT_EQ(a.get<int>(), 0);
  EXPECT_TRUE(a.is<int>());
  a = b;
  EXPECT_TRUE(a.empty());

  std::shared_ptr<int> intptr = std::make_shared<int>(42);
  a = VariantTester{intptr};
  b = VariantTester{intptr};
  a = b;
  a = a;
  b = std::move(a);
  a = std::move(b);
  EXPECT_EQ(intptr.use_count(), 2);
  a.reset();
  EXPECT_EQ(intptr.use_count(), 1);

  Variant<int, double, char> v(1.0);
  MVariant<int, double, char> mv(v);
  EXPECT_EQ(mv, 1.0);
  v = 2;
  mv = v;
  EXPECT_EQ(mv, 2);
  mv = '3';
  v = mv.takeValue();
  EXPECT_EQ(v, '3');
  EXPECT_TRUE(mv.empty());
}

TEST(MaybeTest, All) {
  struct MaybeTester {
    std::shared_ptr<int> intptr;
  };

  Maybe<MaybeTester> a, b;

  EXPECT_FALSE(a.isValid());

  std::shared_ptr<int> intptr = std::make_shared<int>(42);
  a = MaybeTester{intptr};
  b = MaybeTester{intptr};
  EXPECT_TRUE(a.isValid());
  a = b;
  a = a;
  b = std::move(a);
  a = std::move(b);
  EXPECT_EQ(intptr.use_count(), 2);
  a = {};
  EXPECT_FALSE(a.isValid());
  EXPECT_EQ(intptr.use_count(), 1);
}
