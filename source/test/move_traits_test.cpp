#include "StarByteArray.hpp"
#include "StarFlatHashMap.hpp"
#include "StarFlatHashSet.hpp"
#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarMaybe.hpp"
#include "StarOrderedMap.hpp"
#include "StarOrderedSet.hpp"
#include "StarString.hpp"
#include "StarVariant.hpp"

#include "gtest/gtest.h"

#include <type_traits>
#include <utility>

using namespace Star;

namespace {

template <typename T>
constexpr void checkMovableValueType() {
  static_assert(std::is_move_constructible_v<T>);
  static_assert(std::is_move_assignable_v<T>);
}

template <typename T>
constexpr void checkNoThrowMovableValueType() {
  static_assert(std::is_nothrow_move_constructible_v<T>);
  static_assert(std::is_nothrow_move_assignable_v<T>);
}

static_assert((checkMovableValueType<String>(), true));
static_assert((checkMovableValueType<ByteArray>(), true));
static_assert((checkMovableValueType<Json>(), true));
static_assert((checkMovableValueType<List<String>>(), true));
static_assert((checkMovableValueType<Maybe<String>>(), true));
static_assert((checkMovableValueType<Variant<int, String>>(), true));
static_assert((checkMovableValueType<FlatHashMap<String, int>>(), true));
static_assert((checkMovableValueType<FlatHashSet<String>>(), true));
static_assert((checkMovableValueType<OrderedMap<String, int>>(), true));
static_assert((checkMovableValueType<OrderedSet<String>>(), true));

static_assert((checkNoThrowMovableValueType<ByteArray>(), true));
static_assert((checkNoThrowMovableValueType<Maybe<ByteArray>>(), true));

}

TEST(MoveTraitsTest, movedFromContainersRemainAssignable) {
  String stringSource = "source";
  String stringTarget = std::move(stringSource);
  EXPECT_EQ(stringTarget, "source");
  stringSource = "reused";
  EXPECT_EQ(stringSource, "reused");

  List<String> listSource = {"a", "b"};
  List<String> listTarget = std::move(listSource);
  EXPECT_EQ(listTarget, (List<String>{"a", "b"}));
  listSource = {"reused"};
  EXPECT_EQ(listSource, (List<String>{"reused"}));

  Maybe<String> maybeSource = String("value");
  Maybe<String> maybeTarget = std::move(maybeSource);
  ASSERT_TRUE(maybeTarget);
  EXPECT_EQ(*maybeTarget, "value");
  maybeSource = String("reused");
  ASSERT_TRUE(maybeSource);
  EXPECT_EQ(*maybeSource, "reused");
}
