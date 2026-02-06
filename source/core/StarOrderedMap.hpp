#pragma once

#include "StarMap.hpp"

import std;

namespace Star {

// Wraps a normal map type and provides an element order independent of the
// underlying map order.
template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
class OrderedMapWrapper {
public:
  using key_type = Key;
  using mapped_type = Value;
  using value_type = std::pair<key_type const, mapped_type>;

  using OrderType = LinkedList<value_type, Allocator>;
  using MapType = Map<
      std::reference_wrapper<key_type const>, typename OrderType::iterator, MapArgs...,
      typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<std::reference_wrapper<key_type const> const, typename OrderType::iterator>>
    >;

  using iterator = typename OrderType::iterator;
  using const_iterator = typename OrderType::const_iterator;

  using reverse_iterator = typename OrderType::reverse_iterator;
  using const_reverse_iterator = typename OrderType::const_reverse_iterator;

  using mapped_ptr =  std::decay_t<mapped_type>*;
  using mapped_const_ptr = typename std::decay<mapped_type>::type const*;

  template <typename Collection>
  static auto from(Collection const& c) -> OrderedMapWrapper;

  OrderedMapWrapper();

  OrderedMapWrapper(OrderedMapWrapper const& map);

  template <typename InputIterator>
  OrderedMapWrapper(InputIterator beg, InputIterator end);

  OrderedMapWrapper(std::initializer_list<value_type> list);

  auto keys() const -> List<key_type>;
  auto values() const -> List<mapped_type>;
  auto pairs() const -> List<std::pair<key_type, mapped_type>>;

  auto contains(key_type const& k) const -> bool;

  // Throws MapException if key not found
  auto get(key_type const& k) -> mapped_type&;
  auto get(key_type const& k) const -> mapped_type const&;

  // Return def if key not found
  auto value(key_type const& k, mapped_type d = mapped_type()) const -> mapped_type;

  auto maybe(key_type const& k) const -> std::optional<mapped_type>;

  auto ptr(key_type const& k) const -> mapped_const_ptr;
  auto ptr(key_type const& k) -> mapped_ptr;

  auto operator[](key_type const& k) -> mapped_type&;

  auto operator=(OrderedMapWrapper const& map) -> OrderedMapWrapper&;

  auto operator==(OrderedMapWrapper const& m) const -> bool;

  // Finds first value matching the given value and returns its key, throws
  // MapException if no such value is found.
  auto keyOf(mapped_type const& v) const -> key_type;

  // Finds all of the values matching the given value and returns their keys.
  auto keysOf(mapped_type const& v) const -> List<key_type>;

  auto insert(value_type const& v) -> std::pair<iterator, bool>;
  auto insert(key_type k, mapped_type v) -> std::pair<iterator, bool>;

  auto insertFront(value_type const& v) -> std::pair<iterator, bool>;
  auto insertFront(key_type k, mapped_type v) -> std::pair<iterator, bool>;

  // Add a key / value pair, throw if the key already exists
  auto add(key_type k, mapped_type v) -> mapped_type&;

  // Set a key to a value, always override if it already exists
  auto set(key_type k, mapped_type v) -> mapped_type&;

  // Appends all values of given map into this map.  If overwite is false, then
  // skips values that already exist in this map.  Returns false if any keys
  // previously existed.
  auto merge(OrderedMapWrapper const& m, bool overwrite = false) -> bool;

  // Removes the item with key k and returns true if found, false otherwise.
  auto remove(key_type const& k) -> bool;

  // Remove and return the value with the key k, throws MapException if not
  // found.
  auto take(key_type const& k) -> mapped_type;

  auto maybeTake(key_type const& k) -> std::optional<value_type>;

  auto begin() const -> const_iterator;
  auto end() const -> const_iterator;

  auto begin() -> iterator;
  auto end() -> iterator;

  auto rbegin() const -> const_reverse_iterator;
  auto rend() const -> const_reverse_iterator;

  auto rbegin() -> reverse_iterator;
  auto rend() -> reverse_iterator;

  [[nodiscard]] auto size() const -> std::size_t;

  auto erase(iterator i) -> iterator;
  auto erase(key_type const& k) -> std::size_t;

  auto find(key_type const& k) -> iterator;
  auto find(key_type const& k) const -> const_iterator;

  auto indexOf(key_type const& k) const -> std::optional<std::size_t>;

  auto keyAt(std::size_t i) const -> key_type const&;
  auto valueAt(std::size_t i) const -> mapped_type const&;
  auto valueAt(std::size_t i) -> mapped_type&;

  auto takeFirst() -> value_type;
  void removeFirst();

  auto first() const -> value_type const&;

  auto firstKey() const -> key_type const&;
  auto firstValue() -> mapped_type&;
  auto firstValue() const -> mapped_type const&;

  auto insert(iterator pos, value_type v) -> iterator;

  void clear();

  [[nodiscard]] auto empty() const -> bool;

  auto toBack(iterator i) -> iterator;
  void toBack(key_type const& k);

  auto toFront(iterator i) -> iterator;
  void toFront(key_type const& k);

  template <typename Compare>
  void sort(Compare comp);

  void sortByKey();
  void sortByValue();

private:
  MapType m_map;
  OrderType m_order;
};

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto operator<<(std::ostream& os, OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...> const& m) -> std::ostream&;

template <typename Key, typename Value, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<Key const, Value>>>
using OrderedMap = OrderedMapWrapper<std::map, Key, Value, Allocator, Compare>;

template <typename Key, typename Value, typename Hash = Star::hash<Key>, typename Equals = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<Key const, Value>>>
using OrderedHashMap = OrderedMapWrapper<FlatHashMap, Key, Value, Allocator, Hash, Equals>;

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
template <typename Collection>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::from(Collection const& c) -> OrderedMapWrapper {
  return OrderedMapWrapper(c.begin(), c.end());
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::OrderedMapWrapper() = default;

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::OrderedMapWrapper(OrderedMapWrapper const& map) {
  for (auto const& p : map)
    insert(p);
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
template <typename InputIterator>
OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::OrderedMapWrapper(InputIterator beg, InputIterator end) {
  while (beg != end) {
    insert(*beg);
    ++beg;
  }
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::OrderedMapWrapper(std::initializer_list<value_type> list) {
  for (value_type v : list)
    insert(std::move(v));
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::keys() const -> List<key_type> {
  List<key_type> keys;
  for (auto const& p : *this)
    keys.append(p.first);
  return keys;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::values() const -> List<mapped_type> {
  List<mapped_type> values;
  for (auto const& p : *this)
    values.append(p.second);
  return values;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::pairs() const -> List<std::pair<key_type, mapped_type>> {
  List<std::pair<key_type, mapped_type>> plist;
  for (auto const& p : *this)
    plist.append(p.second);
  return plist;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::contains(key_type const& k) const -> bool {
  return m_map.find(k) != m_map.end();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::get(key_type const& k) -> mapped_type& {
  auto i = m_map.find(k);
  if (i == m_map.end())
    throw MapException(strf("Key '{}' not found in OrderedMap::get()", outputAny(k)));

  return i->second->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::get(key_type const& k) const -> mapped_type const& {
  return const_cast<OrderedMapWrapper*>(this)->get(k);
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::value(key_type const& k, mapped_type d) const -> mapped_type {
  auto i = m_map.find(k);
  if (i == m_map.end())
    return d;
  else
    return i->second->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::maybe(key_type const& k) const -> std::optional<mapped_type> {
  auto i = find(k);
  if (i == end())
    return {};
  else
    return i->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::ptr(key_type const& k) const -> mapped_const_ptr {
  auto i = find(k);
  if (i == end())
    return nullptr;
  else
    return &i->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::ptr(key_type const& k) -> mapped_ptr {
  iterator i = find(k);
  if (i == end())
    return nullptr;
  else
    return &i->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::operator[](key_type const& k) -> mapped_type& {
  auto i = m_map.find(k);
  if (i == m_map.end()) {
    iterator orderIt = m_order.insert(m_order.end(), value_type(k, mapped_type()));
    i = m_map.insert(typename MapType::value_type(std::cref(orderIt->first), orderIt)).first;
    return orderIt->second;
  } else {
    return i->second->second;
  }
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::operator=(OrderedMapWrapper const& map) -> OrderedMapWrapper& {
  if (this != &map) {
    clear();
    for (auto const& p : map)
      insert(p);
  }

  return *this;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::operator==(OrderedMapWrapper const& m) const -> bool {
  return this == &m || mapsEqual(*this, m);
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::keyOf(mapped_type const& v) const -> key_type {
  for (const_iterator i = begin(); i != end(); ++i) {
    if (i->second == v)
      return i->first;
  }
  throw MapException(strf("Value '{}' not found in OrderedMap::keyOf()", outputAny(v)));
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::keysOf(mapped_type const& v) const -> List<key_type> {
  List<key_type> keys;
  for (iterator i = begin(); i != end(); ++i) {
    if (i->second == v)
      keys.append(i->first);
  }
  return keys;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::insert(value_type const& v) -> std::pair<iterator, bool> {
  auto i = m_map.find(v.first);
  if (i == m_map.end()) {
    iterator orderIt = m_order.insert(m_order.end(), v);
    m_map.insert(i, typename MapType::value_type(std::cref(orderIt->first), orderIt));
    return std::make_pair(orderIt, true);
  } else {
    return std::make_pair(i->second, false);
  }
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::insert(key_type k, mapped_type v) -> std::pair<iterator, bool> {
  return insert(value_type(std::move(k), std::move(v)));
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::insertFront(value_type const& v) -> std::pair<iterator, bool> {
  auto i = m_map.find(v.first);
  if (i == m_map.end()) {
    iterator orderIt = m_order.insert(m_order.begin(), v);
    m_map.insert(i, typename MapType::value_type(std::cref(orderIt->first), orderIt));
    return std::make_pair(orderIt, true);
  } else {
    return std::make_pair(i->second, false);
  }
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::insertFront(key_type k, mapped_type v) -> std::pair<iterator, bool> {
  return insertFront(value_type(std::move(k), std::move(v)));
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::add(key_type k, mapped_type v) -> mapped_type& {
  auto pair = insert(value_type(std::move(k), std::move(v)));
  if (!pair.second)
    throw MapException(strf("Entry with key '{}' already present.", outputAny(k)));
  else
    return pair.first->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::set(key_type k, mapped_type v) -> mapped_type& {
  auto i = find(k);
  if (i != end()) {
    i->second = std::move(v);
    return i->second;
  } else {
    return insert(value_type(std::move(k), std::move(v))).first->second;
  }
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::merge(OrderedMapWrapper const& m, bool overwrite) -> bool {
  return mapMerge(*this, m, overwrite);
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::remove(key_type const& k) -> bool {
  auto i = m_map.find(k);
  if (i != m_map.end()) {
    auto orderIt = i->second;
    m_map.erase(i);

    m_order.erase(orderIt);
    return true;
  } else {
    return false;
  }
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::take(key_type const& k) -> mapped_type {
  auto i = m_map.find(k);
  if (i != m_map.end()) {
    auto orderIt = i->second;
    m_map.erase(i);

    mapped_type v = orderIt->second;
    m_order.erase(i->second);
    return v;
  } else {
    throw MapException(strf("Key '{}' not found in OrderedMap::take()", outputAny(k)));
  }
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::maybeTake(key_type const& k) -> std::optional<value_type> {
  iterator i = find(k);
  if (i != end()) {
    value_type v = *i;
    erase(i);
    return v;
  }

  return {};
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::begin() const -> const_iterator {
  return m_order.begin();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::end() const -> const_iterator {
  return m_order.end();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::begin() -> iterator {
  return m_order.begin();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::end() -> iterator {
  return m_order.end();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::rbegin() const -> const_reverse_iterator {
  return m_order.rbegin();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::rend() const -> const_reverse_iterator {
  return m_order.rend();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::rbegin() -> reverse_iterator {
  return m_order.rbegin();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::rend() -> reverse_iterator {
  return m_order.rend();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::size() const -> size_t {
  return m_map.size();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::erase(iterator i) -> iterator {
  m_map.erase(i->first);
  return m_order.erase(i);
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::erase(key_type const& k) -> size_t {
  if (remove(k))
    return 1;
  return 0;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::find(key_type const& k) -> iterator {
  auto i = m_map.find(k);
  if (i == m_map.end())
    return m_order.end();
  else
    return i->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::find(key_type const& k) const -> const_iterator {
  auto i = m_map.find(k);
  if (i == m_map.end())
    return m_order.end();
  else
    return i->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::indexOf(key_type const& k) const -> std::optional<size_t> {
  typename MapType::const_iterator i = m_map.find(k);
  if (i == m_map.end())
    return {};

  return std::distance(begin(), const_iterator(i->second));
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::keyAt(size_t i) const -> key_type const& {
  if (i >= size())
    throw MapException(strf("index {} out of range in OrderedMap::at()", i));

  auto it = begin();
  std::advance(it, i);
  return it->first;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::valueAt(size_t i) const -> mapped_type const& {
  return const_cast<OrderedMapWrapper*>(this)->valueAt(i);
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::valueAt(size_t i) -> mapped_type& {
  if (i >= size())
    throw MapException(strf("index {} out of range in OrderedMap::valueAt()", i));

  auto it = m_order.begin();
  std::advance(it, i);
  return it->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::takeFirst() -> value_type {
  if (empty())
    throw MapException("OrderedMap::takeFirst() called on empty OrderedMap");

  iterator i = m_order.begin();
  m_map.remove(i->first);
  value_type v = *i;
  m_order.erase(i);
  return v;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
void OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::removeFirst() {
  erase(begin());
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::first() const -> value_type const& {
  if (empty())
    throw MapException("OrderedMap::takeFirst() called on empty OrderedMap");

  return *m_order.begin();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::firstValue() -> mapped_type& {
  return begin()->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::firstValue() const -> mapped_type const& {
  return begin()->second;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::firstKey() const -> key_type const& {
  return begin()->first;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::insert(iterator pos, value_type v) -> iterator {
  auto i = m_map.find(v.first);
  if (i == m_map.end()) {
    iterator orderIt = m_order.insert(pos, std::move(v));
    m_map.insert(typename MapType::value_type(std::cref(orderIt->first), orderIt));
    return orderIt;
  } else {
    i->second->second = std::move(v.second);
    m_order.splice(pos, m_order, i->second);
    return i->second;
  }
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
void OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::clear() {
  m_map.clear();
  m_order.clear();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::empty() const -> bool {
  return size() == 0;
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::toBack(iterator i) -> iterator {
  m_order.splice(m_order.end(), m_order, i);
  return prev(m_order.end());
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::toFront(iterator i) -> iterator {
  m_order.splice(m_order.begin(), m_order, i);
  return m_order.begin();
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
void OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::toBack(key_type const& k) {
  auto i = m_map.find(k);
  if (i == m_map.end())
    throw MapException(strf("Key not found in OrderedMap::toBack('{}')", outputAny(k)));

  toBack(i->second);
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
void OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::toFront(key_type const& k) {
  auto i = m_map.find(k);
  if (i == m_map.end())
    throw MapException(strf("Key not found in OrderedMap::toFront('{}')", outputAny(k)));

  toFront(i->second);
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
template <typename Compare>
void OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::sort(Compare comp) {
  m_order.sort(comp);
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
void OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::sortByKey() {
  sort([](value_type const& a, value_type const& b) -> auto {
      return a.first < b.first;
    });
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
void OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>::sortByValue() {
  sort([](value_type const& a, value_type const& b) -> auto {
      return a.second < b.second;
    });
}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
auto operator<<(std::ostream& os, OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...> const& m) -> std::ostream& {
  printMap(os, m);
  return os;
}

}

template <template <typename...> class Map, typename Key, typename Value, typename Allocator, typename... MapArgs>
struct std::formatter<Star::OrderedMapWrapper<Map, Key, Value, Allocator, MapArgs...>> : Star::ostream_formatter {};
