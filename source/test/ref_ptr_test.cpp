import std;
import star.ref_ptr;
import star.casting;
import star.type_identity;

#include "gtest/gtest.h"

#ifdef STAR_COMPILER_CLANG
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif 

using namespace Star;

int g_test1Count = 0;
int g_test2Count = 0;

struct Base : RefCounter {
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<Base, RefCounter>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return typeName<Base>();
  }
};

struct Test1 : Base {
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<Test1, Base>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return typeName<Test1>();
  }
  Test1() {
    ++g_test1Count;
  }

  ~Test1() {
    --g_test1Count;
  }
};

struct Test2 : Base {
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<Test2, Base>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return typeName<Test2>();
  }
  Test2() {
    ++g_test2Count;
  }

  ~Test2() {
    --g_test2Count;
  }
};

TEST(IntrusivePtr, All) {
  {
    RefPtr<Base> p1 = make_ref<Test1>();
    RefPtr<Base> p2 = make_ref<Test2>();

    EXPECT_TRUE(is<Test1>(p1));
    EXPECT_FALSE(is<Test2>(p1));
    EXPECT_TRUE(is<Test2>(p2));
    EXPECT_FALSE(is<Test1>(p2));

    RefPtr<Base> p3 = p1;
    RefPtr<Base> p4 = p2;

    p3 = p3;
    std::swap(p3, p4);

    EXPECT_TRUE(is<Test1>(p4));
    EXPECT_FALSE(is<Test2>(p4));
    EXPECT_TRUE(is<Test2>(p3));
    EXPECT_FALSE(is<Test1>(p3));

    EXPECT_EQ(p3, p2);
    EXPECT_EQ(p4, p1);

    std::swap(p3, p4);

    EXPECT_EQ(p3, p1);
    EXPECT_EQ(p4, p2);

    RefPtr<Base> p5;
    EXPECT_FALSE(p5);

    EXPECT_NE(p4, p1);
    EXPECT_NE(p3, p2);
    EXPECT_NE(p3, p5);

    p5 = p2;
    p2 = std::move(p5);
    EXPECT_TRUE(is<Test2>(p2));

    RefPtr<Test1> p6 = as<Test1>(p1);
    RefPtr<Test2> p7 = as<Test2>(p2);
    RefPtr<Test2> p8 = as<Test2>(p1);
    EXPECT_TRUE((bool)p6);
    EXPECT_TRUE((bool)p7);
    EXPECT_FALSE((bool)p8);
  }

  EXPECT_EQ(0, g_test1Count);
  EXPECT_EQ(0, g_test2Count);
}

namespace {
struct QueryLeft : virtual RefCounter {
  int left = 17;
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<QueryLeft, RefCounter>(this, key);
  }
};

struct QueryRight : virtual RefCounter {
  int right = 29;
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<QueryRight, RefCounter>(this, key);
  }
};

struct QueryBoth : QueryLeft, QueryRight {
  explicit QueryBoth(bool& destroyed) : destroyed(destroyed) {}
  ~QueryBoth() { destroyed = true; }
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<QueryBoth, QueryLeft, QueryRight>(this, key);
  }
  bool& destroyed;
};
}

TEST(IntrusivePtr, VirtualCrossCastSharesObject) {
  bool destroyed = false;
  {
    RefPtr<QueryLeft> left(new QueryBoth(destroyed));
    auto right = as<QueryRight>(left);
    ASSERT_TRUE(right);
    EXPECT_EQ(29, right->right);
    EXPECT_EQ(as<RefCounter>(left).get(), as<RefCounter>(right).get());
    EXPECT_FALSE(as<Test2>(left));
    left.reset();
    EXPECT_FALSE(destroyed);
    EXPECT_EQ(29, right->right);
  }
  EXPECT_TRUE(destroyed);
}

TEST(Casting, SharedCrossCastAliasesOwnerAndPreservesConst) {
  static_assert(std::same_as<
      decltype(as<QueryRight>(std::declval<std::shared_ptr<QueryLeft const>>())),
      std::shared_ptr<QueryRight const>>);
  bool destroyed = false;
  auto object = std::make_shared<QueryBoth>(destroyed);
  std::weak_ptr<QueryBoth> observer = object;
  std::shared_ptr<QueryLeft> left = object;
  auto right = as<QueryRight>(left);
  ASSERT_TRUE(right);
  EXPECT_EQ(static_cast<QueryRight*>(object.get()), right.get());
  EXPECT_EQ(29, right->right);
  EXPECT_FALSE(left.owner_before(right));
  EXPECT_FALSE(right.owner_before(left));
  {
    std::shared_ptr<QueryLeft const> readOnly = left;
    auto constRight = as<QueryRight>(readOnly);
    ASSERT_TRUE(constRight);
    EXPECT_EQ(right.get(), constRight.get());
    EXPECT_EQ(29, constRight->right);
    EXPECT_FALSE(readOnly.owner_before(constRight));
    EXPECT_FALSE(constRight.owner_before(readOnly));
  }
  auto failed = as<Test2>(left);
  EXPECT_EQ(nullptr, failed.get());
  EXPECT_EQ(0, failed.use_count());
  auto rootFromLeft = as<RefCounter>(left);
  auto rootFromRight = as<RefCounter>(right);
  EXPECT_EQ(rootFromLeft.get(), rootFromRight.get());
  rootFromLeft.reset();
  rootFromRight.reset();
  object.reset();
  left.reset();
  EXPECT_FALSE(observer.expired());
  right->right = 41;
  EXPECT_EQ(41, as<QueryRight>(observer.lock())->right);
  right.reset();
  EXPECT_TRUE(observer.expired());
  EXPECT_TRUE(destroyed);
}

TEST(Casting, BorrowedCrossCastAdjustsWithoutExtendingLifetime) {
  bool destroyed = false;
  auto owner = std::make_shared<QueryBoth>(destroyed);
  QueryLeft& left = *owner;

  auto right = asRef<QueryRight>(left);
  ASSERT_TRUE(right);
  right->get().right = 43;
  EXPECT_EQ(43, owner->right);

  auto readOnly = rconvert<QueryRight>(std::as_const(left));
  ASSERT_TRUE(readOnly);
  EXPECT_EQ(43, readOnly->get().right);

  EXPECT_FALSE(asRef<Test2>(left));
  auto missing = rconvert<Test2>(left);
  ASSERT_FALSE(missing);
  EXPECT_TRUE(missing.error().is<PointerConvertException>());

  owner.reset();
  EXPECT_TRUE(destroyed);
  // Borrow values may remain in scope, but must not be dereferenced again.
}
