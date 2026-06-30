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

#include <type_traits>

using namespace Star;

namespace {

template <typename T>
constexpr void checkMovableValueType() {
  static_assert(std::is_move_constructible_v<T>);
  static_assert(std::is_move_assignable_v<T>);
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

}
