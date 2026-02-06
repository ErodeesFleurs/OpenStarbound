#pragma once

#include "StarFlatHashMap.hpp"
#include "StarList.hpp"

import std;

namespace Star {

using MapException = ExceptionDerived<"MapException">;

template <typename BaseMap>
class MapMixin : public BaseMap {
public:
  using Base = BaseMap;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;

  using key_type = typename Base::key_type;
  using mapped_type = typename Base::mapped_type;
  using value_type = typename Base::value_type;

  using mapped_ptr =  std::decay_t<mapped_type>*;
  using mapped_const_ptr = typename std::decay<mapped_type>::type const*;

  template <typename MapType>
  static auto from(MapType const& m) -> MapMixin;

  using Base::Base;

  auto keys() const -> List<key_type>;
  auto values() const -> List<mapped_type>;
  auto pairs() const -> List<std::pair<key_type, mapped_type>>;

  auto contains(key_type const& k) const -> bool;

  // Removes the item with key k and returns true if contains(k) is true,
  // false otherwise.
  auto remove(key_type const& k) -> bool;

  // Removes *all* items that have a value matching the given one.  Returns
  // true if any elements were removed.
  auto removeValues(mapped_type const& v) -> bool;

  // Throws exception if key not found
  auto take(key_type const& k) -> mapped_type;

  auto maybeTake(key_type const& k) -> std::optional<mapped_type>;

  // Throws exception if key not found
  auto get(key_type const& k) -> mapped_type&;
  auto get(key_type const& k) const -> mapped_type const&;

  // Return d if key not found
  auto value(key_type const& k, mapped_type d = mapped_type()) const -> mapped_type;

  auto maybe(key_type const& k) const -> std::optional<mapped_type>;

  auto ptr(key_type const& k) const -> mapped_const_ptr;
  auto ptr(key_type const& k) -> mapped_ptr;

  // Finds first value matching the given value and returns its key.
  auto keyOf(mapped_type const& v) const -> key_type;

  // Finds all of the values matching the given value and returns their keys.
  auto keysOf(mapped_type const& v) const -> List<key_type>;

  auto hasValue(mapped_type const& v) const -> bool;

  using Base::insert;

  // Same as insert(value_type), returns the iterator to either the newly
  // inserted value or the existing value, and then a bool that is true if the
  // new element was inserted.
  auto insert(key_type k, mapped_type v) -> std::pair<iterator, bool>;

  // Add a key / value std::pair, throw if the key already exists
  auto add(key_type k, mapped_type v) -> mapped_type&;

  // Set a key to a value, always override if it already exists
  auto set(key_type k, mapped_type v) -> mapped_type&;

  // Appends all values of given map into this map.  If overwite is false, then
  // skips values that already exist in this map.  Returns false if any keys
  // previously existed.
  template <typename MapType>
  auto merge(MapType const& m, bool overwrite = false) -> bool;

  auto operator==(MapMixin const& m) const -> bool;
};

template <typename BaseMap>
auto operator<<(std::ostream& os, MapMixin<BaseMap> const& m) -> std::ostream&;

template <typename Key, typename Value, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<Key const, Value>>>
using Map = MapMixin<std::map<Key, Value, Compare, Allocator>>;

template <typename Key, typename Value, typename Hash = hash<Key>, typename Equals = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<Key const, Value>>>
using HashMap = MapMixin<FlatHashMap<Key, Value, Hash, Equals, Allocator>>;

template <typename Key, typename Value, typename Hash = hash<Key>, typename Equals = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<Key const, Value>>>
using StableHashMap = MapMixin<std::unordered_map<Key, Value, Hash, Equals, Allocator>>;

template <typename BaseMap>
template <typename MapType>
auto MapMixin<BaseMap>::from(MapType const& m) -> MapMixin {
  return MapMixin(m.begin(), m.end());
}

template <typename BaseMap>
auto MapMixin<BaseMap>::keys() const -> List<key_type> {
  List<key_type> klist;
  for (const_iterator i = Base::begin(); i != Base::end(); ++i)
    klist.push_back(i->first);
  return klist;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::values() const -> List<mapped_type> {
  List<mapped_type> vlist;
  for (const_iterator i = Base::begin(); i != Base::end(); ++i)
    vlist.push_back(i->second);
  return vlist;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::pairs() const -> List<std::pair<key_type, mapped_type>> {
  List<std::pair<key_type, mapped_type>> plist;
  for (const_iterator i = Base::begin(); i != Base::end(); ++i)
    plist.push_back(*i);
  return plist;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::contains(key_type const& k) const -> bool {
  return Base::find(k) != Base::end();
}

template <typename BaseMap>
auto MapMixin<BaseMap>::remove(key_type const& k) -> bool {
  return Base::erase(k) != 0;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::removeValues(mapped_type const& v) -> bool {
  bool removed = false;
  const_iterator i = Base::begin();
  while (i != Base::end()) {
    if (i->second == v) {
      Base::erase(i++);
      removed = true;
    } else {
      ++i;
    }
  }
  return removed;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::take(key_type const& k) -> mapped_type {
  if (auto v = maybeTake(k))
    return std::move(*v);
  throw MapException(strf("Key '{}' not found in Map::take()", outputAny(k)));
}

template <typename BaseMap>
auto MapMixin<BaseMap>::maybeTake(key_type const& k) -> std::optional<mapped_type> {
  const_iterator i = Base::find(k);
  if (i != Base::end()) {
    mapped_type v = std::move(i->second);
    Base::erase(i);
    return v;
  }

  return std::nullopt;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::get(key_type const& k) -> mapped_type& {
  iterator i = Base::find(k);
  if (i == Base::end())
    throw MapException(strf("Key '{}' not found in Map::get()", outputAny(k)));
  return i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::get(key_type const& k) const -> mapped_type const& {
  const_iterator i = Base::find(k);
  if (i == Base::end())
    throw MapException(strf("Key '{}' not found in Map::get()", outputAny(k)));
  return i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::value(key_type const& k, mapped_type d) const -> mapped_type {
  const_iterator i = Base::find(k);
  if (i == Base::end())
    return d;
  else
    return i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::maybe(key_type const& k) const -> std::optional<mapped_type> {
  auto i = Base::find(k);
  if (i == Base::end())
    return std::nullopt;
  else
    return i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::ptr(key_type const& k) const -> mapped_const_ptr {
  auto i = Base::find(k);
  if (i == Base::end())
    return nullptr;
  else
    return &i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::ptr(key_type const& k) -> mapped_ptr {
  auto i = Base::find(k);
  if (i == Base::end())
    return nullptr;
  else
    return &i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::keyOf(mapped_type const& v) const -> key_type {
  for (const_iterator i = Base::begin(); i != Base::end(); ++i) {
    if (i->second == v)
      return i->first;
  }
  throw MapException(strf("Value '{}' not found in Map::keyOf()", outputAny(v)));
}

template <typename BaseMap>
auto MapMixin<BaseMap>::keysOf(mapped_type const& v) const -> List<key_type> {
  List<key_type> keys;
  for (const_iterator i = Base::begin(); i != Base::end(); ++i) {
    if (i->second == v)
      keys.append(i->first);
  }
  return keys;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::hasValue(mapped_type const& v) const -> bool {
  for (const_iterator i = Base::begin(); i != Base::end(); ++i) {
    if (i->second == v)
      return true;
  }
  return false;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::insert(key_type k, mapped_type v) -> std::pair<iterator, bool> {
  return Base::insert(value_type(std::move(k), std::move(v)));
}

template <typename BaseMap>
auto MapMixin<BaseMap>::add(key_type k, mapped_type v) -> mapped_type& {
  auto pair = Base::insert(value_type(std::move(k), std::move(v)));
  if (!pair.second)
    throw MapException(strf("Entry with key '{}' already present.", outputAny(k)));
  else
    return pair.first->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::set(key_type k, mapped_type v) -> mapped_type& {
  auto i = Base::find(k);
  if (i != Base::end()) {
    i->second = std::move(v);
    return i->second;
  } else {
    return Base::insert(value_type(std::move(k), std::move(v))).first->second;
  }
}

template <typename BaseMap>
template <typename OtherMapType>
auto MapMixin<BaseMap>::merge(OtherMapType const& m, bool overwrite) -> bool {
  return mapMerge(*this, m, overwrite);
}

template <typename BaseMap>
auto MapMixin<BaseMap>::operator==(MapMixin const& m) const -> bool {
  return this == &m || mapsEqual(*this, m);
}

template <typename MapType>
void printMap(std::ostream& os, MapType const& m) {
  os << "{ ";
  for (auto i = m.begin(); i != m.end(); ++i) {
    if (m.begin() == i)
      os << "\"";
    else
      os << ", \"";
    os << i->first << "\" : \"" << i->second << "\"";
  }
  os << " }";
}

template <typename BaseMap>
auto operator<<(std::ostream& os, MapMixin<BaseMap> const& m) -> std::ostream& {
  printMap(os, m);
  return os;
}

}

template <typename BaseMap>
struct std::formatter<Star::MapMixin<BaseMap>> : Star::ostream_formatter {};
