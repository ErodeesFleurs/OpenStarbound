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

namespace {
struct VariantThrowingMove {
  std::shared_ptr<int> value;
  bool fail;

  VariantThrowingMove(std::shared_ptr<int> value, bool fail)
    : value(std::move(value)), fail(fail) {}
  VariantThrowingMove(VariantThrowingMove const&) = default;
  VariantThrowingMove(VariantThrowingMove&& other) {
    if (other.fail)
      throw std::runtime_error("move");
    value = std::move(other.value);
    fail = other.fail;
  }
  VariantThrowingMove& operator=(VariantThrowingMove const&) = default;
  VariantThrowingMove& operator=(VariantThrowingMove&&) noexcept = default;
};

struct VariantThrowingCopy {
  VariantThrowingCopy() = default;
  VariantThrowingCopy(VariantThrowingCopy const&) {
    throw std::runtime_error("copy");
  }
  VariantThrowingCopy(VariantThrowingCopy&&) noexcept = default;
  VariantThrowingCopy& operator=(VariantThrowingCopy const&) = default;
  VariantThrowingCopy& operator=(VariantThrowingCopy&&) noexcept = default;
};
}

TEST(VariantTest, AlternativeOrdering) {
  Variant<int, double> value(2);
  EXPECT_TRUE(value < 3);
  EXPECT_FALSE(value < 2);
  EXPECT_FALSE(value < 1);
  EXPECT_TRUE(value < 0.0);
  value = 2.0;
  EXPECT_FALSE(value < 100);
  EXPECT_TRUE(value < 3.0);
  EXPECT_FALSE(value < 2.0);
}

TEST(VariantTest, ThrowingMoveDuringTypeChange) {
  using V = Variant<int, VariantThrowingMove>;
  static_assert(!std::is_nothrow_move_constructible_v<V>);
  static_assert(!std::is_nothrow_move_assignable_v<V>);
  auto value = std::make_shared<int>(42);
  V source(std::in_place_type<VariantThrowingMove>, value, true);
  V destination(7);
  EXPECT_THROW(destination = std::move(source), std::runtime_error);
  EXPECT_TRUE(destination.invalid());
  EXPECT_TRUE(source.is<VariantThrowingMove>());
  EXPECT_EQ(source.get<VariantThrowingMove>().value, value);
  EXPECT_EQ(value.use_count(), 2);
  destination = 9;
  EXPECT_EQ(destination.get<int>(), 9);
  EXPECT_THROW(static_cast<void>(V(std::move(source))), std::runtime_error);
  EXPECT_EQ(value.use_count(), 2);
  source.get<VariantThrowingMove>().fail = false;
  destination = std::move(source);
  EXPECT_EQ(destination.get<VariantThrowingMove>().value, value);
}

TEST(VariantTest, InvalidSourceThrowsWithoutTermination) {
  using V = Variant<int, VariantThrowingCopy>;
  static_assert(std::is_nothrow_move_constructible_v<VariantThrowingCopy>);
  static_assert(std::is_nothrow_move_assignable_v<VariantThrowingCopy>);
  static_assert(!std::is_nothrow_move_constructible_v<V>);
  static_assert(!std::is_nothrow_move_assignable_v<V>);
  V source(std::in_place_type<VariantThrowingCopy>);
  V invalid(1);
  EXPECT_THROW(invalid = source, std::runtime_error);
  ASSERT_TRUE(invalid.invalid());
  EXPECT_THROW(static_cast<void>(V(invalid)), BadVariantType);
  EXPECT_THROW(static_cast<void>(V(std::move(invalid))), BadVariantType);
  V destination(42);
  EXPECT_THROW(destination = invalid, BadVariantType);
  EXPECT_EQ(destination.get<int>(), 42);
  EXPECT_THROW(destination = std::move(invalid), BadVariantType);
  EXPECT_EQ(destination.get<int>(), 42);
  invalid = 3;
  EXPECT_EQ(invalid.get<int>(), 3);
}

TEST(VariantTest, AlignedStorageReuse) {
  struct alignas(64) Value {
    int value;
  };
  using V = Variant<char, Value>;
  static_assert(alignof(V) == alignof(Value));
  static_assert(sizeof(V) == 2 * sizeof(Value));
  static_assert(sizeof(Variant<char, int>) == 2 * sizeof(int));
  V value(std::in_place_type<Value>, 42);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(value.ptr<Value>()) % alignof(Value), 0u);
  EXPECT_EQ(value.get<Value>().value, 42);
  value = 'x';
  value = Value{7};
  EXPECT_EQ(value.get<Value>().value, 7);
  EXPECT_EQ(std::as_const(value).get<Value>().value, 7);
}

TEST(VariantTest, ThrowingMoveAssignmentPreservesActiveType) {
  struct Value {
    Value() = default;
    Value(Value const&) = default;
    Value(Value&&) noexcept = default;
    Value& operator=(Value const&) = default;
    Value& operator=(Value&&) {
      throw std::runtime_error("assignment");
    }
  };
  using V = Variant<int, Value>;
  static_assert(!std::is_nothrow_move_assignable_v<V>);
  V source(std::in_place_type<Value>);
  V destination(std::in_place_type<Value>);
  EXPECT_THROW(destination = std::move(source), std::runtime_error);
  EXPECT_TRUE(destination.is<Value>());
  EXPECT_TRUE(source.is<Value>());
  auto& same = destination;
  EXPECT_NO_THROW(destination = std::move(same));
}
