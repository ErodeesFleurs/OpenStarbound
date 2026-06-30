#include "StarStrongTypedef.hpp"

#include "gtest/gtest.h"

struct BaseType {};
using DerivedType1 = Star::StrongTypedef<BaseType, struct DerivedType1Tag>;
using DerivedType2 = Star::StrongTypedef<BaseType, struct DerivedType2Tag>;
void func(DerivedType1) {}

using AlsoInt = Star::StrongTypedefBuiltin<int, struct AlsoIntTag>;

TEST(StrongTypedefTest, All) {
  AlsoInt i = AlsoInt(0);
  ++i;
  i -= 5;
  EXPECT_EQ(i, -4);

  func(static_cast<DerivedType1>(BaseType()));
  func(DerivedType1());

  // Shouldn't compile!  Can't test this automatically!
  // func(BaseType());
  // func(DerivedType2());
  // DerivedType1 dt1 = Basetype();
  // DerivedType2 dt2 = DerivedType1();
}
