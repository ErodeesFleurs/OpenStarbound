#pragma once

#include "StarFlatHashSet.hpp"
#include "StarList.hpp"

import std;

namespace Star {

using SetException = ExceptionDerived<"SetException">;

template <typename BaseSet>
class SetMixin : public BaseSet {
public:
  using Base = BaseSet;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;

  using value_type = typename Base::value_type;

  using Base::Base;

  auto values() const -> List<value_type>;

  auto contains(value_type const& v) const -> bool;

  auto add(value_type const& v) -> bool;

  // Like add, but always adds new value, potentially replacing another equal
  // (comparing equal, may not be actually equal) value.  Returns whether an
  // existing value was replaced.
  auto replace(value_type v) -> bool;

  template <typename Container>
  void addAll(Container const& s);

  auto remove(value_type const& v) -> bool;

  template <typename Container>
  void removeAll(Container const& s);

  auto first() -> value_type;
  auto maybeFirst() -> std::optional<value_type>;
  auto takeFirst() -> value_type;
  auto maybeTakeFirst() -> std::optional<value_type>;

  auto last() -> value_type;
  auto maybeLast() -> std::optional<value_type>;
  auto takeLast() -> value_type;
  auto maybeTakeLast() -> std::optional<value_type>;

  auto hasIntersection(SetMixin const& s) const -> bool;
};

template <typename BaseSet>
auto operator<<(std::ostream& os, SetMixin<BaseSet> const& set) -> std::ostream&;

template <typename Value, typename Compare = std::less<Value>, typename Allocator = std::allocator<Value>>
class Set : public SetMixin<std::set<Value, Compare, Allocator>> {
public:
  using Base = SetMixin<std::set<Value, Compare, Allocator>>;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;

  using value_type = typename Base::value_type;

  template <typename Container>
  static auto from(Container const& c) -> Set;

  using Base::Base;

  // Returns set of elements that are in this set and the given set.
  auto intersection(Set const& s) const -> Set;
  auto intersection(Set const& s, std::function<bool(Value const&, Value const&)> compare) const -> Set;

  // Returns elements in this set that are not in the given set
  auto difference(Set const& s) const -> Set;
  auto difference(Set const& s, std::function<bool(Value const&, Value const&)> compare) const -> Set;

  // Returns elements in either this set or the given set
  auto combination(Set const& s) const -> Set;
};

template <typename BaseSet>
class HashSetMixin : public SetMixin<BaseSet> {
public:
  using Base = SetMixin<BaseSet>;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;

  using value_type = typename Base::value_type;

  template <typename Container>
  static auto from(Container const& c) -> HashSetMixin;

  using Base::Base;

  auto intersection(HashSetMixin const& s) const -> HashSetMixin;
  auto difference(HashSetMixin const& s) const -> HashSetMixin;
  auto combination(HashSetMixin const& s) const -> HashSetMixin;
};

template <typename Value, typename Hash = hash<Value>, typename Equals = std::equal_to<Value>, typename Allocator = std::allocator<Value>>
using HashSet = HashSetMixin<FlatHashSet<Value, Hash, Equals, Allocator>>;

template <typename Value, typename Hash = hash<Value>, typename Equals = std::equal_to<Value>, typename Allocator = std::allocator<Value>>
using StableHashSet = HashSetMixin<std::unordered_set<Value, Hash, Equals, Allocator>>;

template <typename BaseSet>
auto SetMixin<BaseSet>::values() const -> List<value_type> {
  return List<value_type>(Base::begin(), Base::end());
}

template <typename BaseSet>
auto SetMixin<BaseSet>::contains(value_type const& v) const -> bool {
  return Base::find(v) != Base::end();
}

template <typename BaseSet>
auto SetMixin<BaseSet>::add(value_type const& v) -> bool {
  return Base::insert(v).second;
}

template <typename BaseSet>
auto SetMixin<BaseSet>::replace(value_type v) -> bool {
  bool replaced = remove(v);
  Base::insert(std::move(v));
  return replaced;
}

template <typename BaseSet>
template <typename Container>
void SetMixin<BaseSet>::addAll(Container const& s) {
  return Base::insert(s.begin(), s.end());
}

template <typename BaseSet>
auto SetMixin<BaseSet>::remove(value_type const& v) -> bool {
  return Base::erase(v) != 0;
}

template <typename BaseSet>
template <typename Container>
void SetMixin<BaseSet>::removeAll(Container const& s) {
  for (auto const& v : s)
    remove(v);
}

template <typename BaseSet>
auto SetMixin<BaseSet>::first() -> value_type {
  if (Base::empty())
    throw SetException("first called on empty set");
  return *Base::begin();
}

template <typename BaseSet>
auto SetMixin<BaseSet>::maybeFirst() -> std::optional<value_type> {
  if (Base::empty())
    return std::nullopt;
  return *Base::begin();
}

template <typename BaseSet>
auto SetMixin<BaseSet>::takeFirst() -> value_type {
  if (Base::empty())
    throw SetException("takeFirst called on empty set");
  auto i = Base::begin();
  value_type v = std::move(*i);
  Base::erase(i);
  return v;
}

template <typename BaseSet>
auto SetMixin<BaseSet>::maybeTakeFirst() -> std::optional<value_type> {
  if (Base::empty())
    return std::nullopt;
  auto i = Base::begin();
  value_type v = std::move(*i);
  Base::erase(i);
  return v;
}

template <typename BaseSet>
auto SetMixin<BaseSet>::last() -> value_type {
  if (Base::empty())
    throw SetException("last called on empty set");
  return *prev(Base::end());
}

template <typename BaseSet>
auto SetMixin<BaseSet>::maybeLast() -> std::optional<value_type> {
  if (Base::empty())
    return std::nullopt;
  return *prev(Base::end());
}

template <typename BaseSet>
auto SetMixin<BaseSet>::takeLast() -> value_type {
  if (Base::empty())
    throw SetException("takeLast called on empty set");
  auto i = prev(Base::end());
  value_type v = std::move(*i);
  Base::erase(i);
  return v;
}

template <typename BaseSet>
auto SetMixin<BaseSet>::maybeTakeLast() -> std::optional<value_type> {
  if (Base::empty())
    return std::nullopt;
  auto i = prev(Base::end());
  value_type v = std::move(*i);
  Base::erase(i);
  return v;
}

template <typename BaseSet>
auto SetMixin<BaseSet>::hasIntersection(SetMixin const& s) const -> bool {
  for (auto const& v : s) {
    if (contains(v)) {
      return true;
    }
  }
  return false;
}

template <typename BaseSet>
auto operator<<(std::ostream& os, SetMixin<BaseSet> const& set) -> std::ostream& {
  os << "(";
  for (auto i = set.begin(); i != set.end(); ++i) {
    if (i != set.begin())
      os << ", ";
    os << *i;
  }
  os << ")";
  return os;
}

template <typename Value, typename Compare, typename Allocator>
template <typename Container>
auto Set<Value, Compare, Allocator>::from(Container const& c) -> Set<Value, Compare, Allocator> {
  return Set(c.begin(), c.end());
}

template <typename Value, typename Compare, typename Allocator>
auto Set<Value, Compare, Allocator>::intersection(Set const& s) const -> Set<Value, Compare, Allocator> {
  Set res;
  std::set_intersection(Base::begin(), Base::end(), s.begin(), s.end(), std::inserter(res, res.end()));
  return res;
}

template <typename Value, typename Compare, typename Allocator>
auto Set<Value, Compare, Allocator>::intersection(Set const& s, std::function<bool(Value const&, Value const&)> compare) const -> Set<Value, Compare, Allocator> {
  Set res;
  std::set_intersection(Base::begin(), Base::end(), s.begin(), s.end(), std::inserter(res, res.end()), compare);
  return res;
}

template <typename Value, typename Compare, typename Allocator>
auto Set<Value, Compare, Allocator>::difference(Set const& s) const -> Set<Value, Compare, Allocator> {
  Set res;
  std::set_difference(Base::begin(), Base::end(), s.begin(), s.end(), std::inserter(res, res.end()));
  return res;
}

template <typename Value, typename Compare, typename Allocator>
auto Set<Value, Compare, Allocator>::difference(Set const& s, std::function<bool(Value const&, Value const&)> compare) const -> Set<Value, Compare, Allocator> {
  Set res;
  std::set_difference(Base::begin(), Base::end(), s.begin(), s.end(), std::inserter(res, res.end()), compare);
  return res;
}

template <typename Value, typename Compare, typename Allocator>
auto Set<Value, Compare, Allocator>::combination(Set const& s) const -> Set<Value, Compare, Allocator> {
  Set ret(*this);
  ret.addAll(s);
  return ret;
}

template <typename BaseMap>
template <typename Container>
auto HashSetMixin<BaseMap>::from(Container const& c) -> HashSetMixin<BaseMap> {
  return HashSetMixin(c.begin(), c.end());
}

template <typename BaseMap>
auto HashSetMixin<BaseMap>::intersection(HashSetMixin const& s) const -> HashSetMixin<BaseMap> {
  // Can't use std::set_intersection, since not sorted, naive version is fine.
  HashSetMixin ret;
  for (auto const& e : s) {
    if (contains(e))
      ret.add(e);
  }
  return ret;
}

template <typename BaseMap>
auto HashSetMixin<BaseMap>::difference(HashSetMixin const& s) const -> HashSetMixin<BaseMap> {
  // Can't use std::set_difference, since not sorted, naive version is fine.
  HashSetMixin ret;
  for (auto const& e : *this) {
    if (!s.contains(e))
      ret.add(e);
  }
  return ret;
}

template <typename BaseMap>
auto HashSetMixin<BaseMap>::combination(HashSetMixin const& s) const -> HashSetMixin<BaseMap> {
  HashSetMixin ret(*this);
  ret.addAll(s);
  return ret;
}

}// namespace Star

template <typename BaseSet>
struct std::formatter<Star::SetMixin<BaseSet>> : Star::ostream_formatter {};

template <typename BaseSet>
struct std::formatter<Star::HashSetMixin<BaseSet>> : Star::ostream_formatter {};
