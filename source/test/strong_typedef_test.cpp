import star.strong_typedef;

#include "gtest/gtest.h"

struct BaseType {};
template <typename BaseType>
struct DerivedType1Wrapper : BaseType {
  using BaseType::BaseType;

  DerivedType1Wrapper() : BaseType() {}

  DerivedType1Wrapper(DerivedType1Wrapper const& nt) : BaseType(nt) {}

  DerivedType1Wrapper(DerivedType1Wrapper&& nt) : BaseType(std::move(nt)) {}

  explicit DerivedType1Wrapper(BaseType const& bt) : BaseType(bt) {}

  explicit DerivedType1Wrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

  DerivedType1Wrapper& operator=(DerivedType1Wrapper const& rhs) {
    BaseType::operator=(rhs);
    return *this;
  }

  DerivedType1Wrapper& operator=(DerivedType1Wrapper&& rhs) {
    BaseType::operator=(std::move(rhs));
    return *this;
  }

  template <class Arg>
  DerivedType1Wrapper& operator=(Arg&& other) {
    static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
            || std::is_same<DerivedType1Wrapper, typename std::decay<Arg>::type>::value,
        "DerivedType1 can not implicitly be assigned from BaseType-derived classes or strong BaseType typedefs");

    BaseType::operator=(std::forward<Arg>(other));
    return *this;
  }
};
typedef DerivedType1Wrapper<BaseType> DerivedType1;
template <typename BaseType>
struct DerivedType2Wrapper : BaseType {
  using BaseType::BaseType;

  DerivedType2Wrapper() : BaseType() {}

  DerivedType2Wrapper(DerivedType2Wrapper const& nt) : BaseType(nt) {}

  DerivedType2Wrapper(DerivedType2Wrapper&& nt) : BaseType(std::move(nt)) {}

  explicit DerivedType2Wrapper(BaseType const& bt) : BaseType(bt) {}

  explicit DerivedType2Wrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

  DerivedType2Wrapper& operator=(DerivedType2Wrapper const& rhs) {
    BaseType::operator=(rhs);
    return *this;
  }

  DerivedType2Wrapper& operator=(DerivedType2Wrapper&& rhs) {
    BaseType::operator=(std::move(rhs));
    return *this;
  }

  template <class Arg>
  DerivedType2Wrapper& operator=(Arg&& other) {
    static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
            || std::is_same<DerivedType2Wrapper, typename std::decay<Arg>::type>::value,
        "DerivedType2 can not implicitly be assigned from BaseType-derived classes or strong BaseType typedefs");

    BaseType::operator=(std::forward<Arg>(other));
    return *this;
  }
};
typedef DerivedType2Wrapper<BaseType> DerivedType2;
void func(DerivedType1) {}

using AlsoInt = StrongTypedefBuiltin<int, struct AlsoIntTag>;

TEST(StrongTypedefTest, All) {
  AlsoInt i = AlsoInt(0);
  ++i;
  i -= 5;
  EXPECT_EQ(i, -4);

  func((DerivedType1)BaseType());
  func(DerivedType1());

  // Shouldn't compile!  Can't test this automatically!
  // func(BaseType());
  // func(DerivedType2());
  // DerivedType1 dt1 = Basetype();
  // DerivedType2 dt2 = DerivedType1();
}
