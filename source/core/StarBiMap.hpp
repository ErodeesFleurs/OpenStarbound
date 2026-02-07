#pragma once

#include "StarString.hpp"

import std;

namespace Star {

// Bi-directional map of unique sets of elements with quick map access from
// either the left or right element to the other side.  Every left side value
// must be unique from every other left side value and the same for the right
// side.
template <typename LeftT,
          typename RightT,
          typename LeftMapT = Map<LeftT, RightT const*>,
          typename RightMapT = Map<RightT, LeftT const*>>
class BiMap {
public:
  using Left = LeftT;
  using Right = RightT;
  using LeftMap = LeftMapT;
  using RightMap = RightMapT;

  using value_type = std::pair<Left, Right>;

  struct BiMapIterator {
    auto operator++() -> BiMapIterator&;
    auto operator++(int) -> BiMapIterator;

    auto operator==(BiMapIterator const& rhs) const -> bool;
    auto operator!=(BiMapIterator const& rhs) const -> bool;

    auto operator*() const -> std::pair<Left const&, Right const&>;

    typename LeftMap::const_iterator iterator;
  };

  using iterator = BiMapIterator;
  using const_iterator = iterator;

  template <typename Collection>
  static auto from(Collection const& c) -> BiMap;

  BiMap();
  BiMap(BiMap const& map);

  template <typename InputIterator>
  BiMap(InputIterator beg, InputIterator end);

  BiMap(std::initializer_list<value_type> list);

  auto leftValues() const -> List<Left>;
  auto rightValues() const -> List<Right>;
  auto pairs() const -> List<value_type>;

  auto hasLeftValue(Left const& left) const -> bool;
  auto hasRightValue(Right const& right) const -> bool;

  auto getRight(Left const& left) const -> Right const&;
  auto getLeft(Right const& right) const -> Left const&;

  auto valueRight(Left const& left, Right const& def = Right()) const -> Right;
  auto valueLeft(Right const& right, Left const& def = Left()) const -> Left;

  auto maybeRight(Left const& left) const -> std::optional<Right>;

  auto maybeLeft(Right const& right) const -> std::optional<Left>;

  auto takeRight(Left const& left) -> Right;
  auto takeLeft(Right const& right) -> Left;

  auto maybeTakeRight(Left const& left) -> std::optional<Right>;
  auto maybeTakeLeft(Right const& right) -> std::optional<Left>;

  auto rightPtr(Left const& left) const -> Right const*;
  auto leftPtr(Right const& right) const -> Left const*;

  auto operator=(BiMap const& map) -> BiMap&;

  auto insert(value_type const& val) -> std::pair<iterator, bool>;

  // Returns true if value was inserted, false if either the left or right side
  // already existed.
  auto insert(Left const& left, Right const& right) -> bool;

  // Throws an exception if the pair cannot be inserted
  void add(Left const& left, Right const& right);
  void add(value_type const& value);

  // Overwrites the left / right mapping regardless of whether each side
  // already exists.
  void overwrite(Left const& left, Right const& right);
  void overwrite(value_type const& value);

  // Removes the pair with the given left side, returns true if this pair was
  // found, false otherwise.
  auto removeLeft(Left const& left) -> bool;

  // Removes the pair with the given right side, returns true if this pair was
  // found, false otherwise.
  auto removeRight(Right const& right) -> bool;

  auto begin() const -> const_iterator;
  auto end() const -> const_iterator;

  [[nodiscard]] auto size() const -> std::size_t;

  void clear();

  [[nodiscard]] auto empty() const -> bool;

  auto operator==(BiMap const& m) const -> bool;

private:
  LeftMap m_leftMap;
  RightMap m_rightMap;
};

template <typename Left, typename Right, typename LeftHash = Star::hash<Left>, typename RightHash = Star::hash<Right>>
using BiHashMap = BiMap<Left, Right, StableHashMap<Left, Right const*, LeftHash>, StableHashMap<Right, Left const*, RightHash>>;

// Case insensitive Enum <-> String map
template <typename EnumType>
using EnumMap = BiMap<EnumType,
                      String,
                      Map<EnumType, String const*>,
                      StableHashMap<String, EnumType const*, CaseInsensitiveStringHash, CaseInsensitiveStringCompare>>;

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::BiMapIterator::operator++() -> BiMapIterator& {
  ++iterator;
  return *this;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::BiMapIterator::operator++(int) -> BiMapIterator {
  BiMapIterator last{iterator};
  ++iterator;
  return last;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::BiMapIterator::operator==(BiMapIterator const& rhs) const -> bool {
  return iterator == rhs.iterator;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::BiMapIterator::operator!=(BiMapIterator const& rhs) const -> bool {
  return iterator != rhs.iterator;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::BiMapIterator::operator*() const -> std::pair<LeftT const&, RightT const&> {
  return {iterator->first, *iterator->second};
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
template <typename Collection>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::from(Collection const& c) -> BiMap<LeftT, RightT, LeftMapT, RightMapT> {
  return BiMap(c.begin(), c.end());
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
BiMap<LeftT, RightT, LeftMapT, RightMapT>::BiMap() = default;

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
BiMap<LeftT, RightT, LeftMapT, RightMapT>::BiMap(BiMap const& map)
    : BiMap(map.begin(), map.end()) {}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
template <typename InputIterator>
BiMap<LeftT, RightT, LeftMapT, RightMapT>::BiMap(InputIterator beg, InputIterator end) {
  while (beg != end) {
    insert(*beg);
    ++beg;
  }
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
BiMap<LeftT, RightT, LeftMapT, RightMapT>::BiMap(std::initializer_list<value_type> list) {
  for (value_type const& v : list) {
    if (!insert(v.first, v.second))
      throw MapException::format("Repeat pair in BiMap initializer_list construction: ({}, {})", outputAny(v.first), outputAny(v.second));
  }
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::leftValues() const -> List<LeftT> {
  return m_leftMap.keys();
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::rightValues() const -> List<RightT> {
  return m_rightMap.keys();
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::pairs() const -> List<value_type> {
  List<value_type> values;
  for (auto const& p : *this)
    values.append(p);
  return values;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::hasLeftValue(Left const& left) const -> bool {
  return m_leftMap.contains(left);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::hasRightValue(Right const& right) const -> bool {
  return m_rightMap.contains(right);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::getRight(Left const& left) const -> RightT const& {
  return *m_leftMap.get(left);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::getLeft(Right const& right) const -> LeftT const& {
  return *m_rightMap.get(right);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::valueRight(Left const& left, Right const& def) const -> RightT {
  return maybeRight(left).value_or(def);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::valueLeft(Right const& right, Left const& def) const -> LeftT {
  return maybeLeft(right).value_or(def);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::maybeRight(Left const& left) const -> std::optional<RightT> {
  auto i = m_leftMap.find(left);
  if (i != m_leftMap.end())
    return *i->second;
  return std::nullopt;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::maybeLeft(Right const& right) const -> std::optional<LeftT> {
  auto i = m_rightMap.find(right);
  if (i != m_rightMap.end())
    return *i->second;
  return std::nullopt;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::takeRight(Left const& left) -> RightT {
  if (auto right = maybeTakeRight(left))
    return std::move(*right);
  throw MapException::format("No such key in BiMap::takeRight", outputAny(left));
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::takeLeft(Right const& right) -> LeftT {
  if (auto left = maybeTakeLeft(right))
    return std::move(*left);
  throw MapException::format("No such key in BiMap::takeLeft", outputAny(right));
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::maybeTakeRight(Left const& left) -> std::optional<RightT> {
  if (auto rightPtr = m_leftMap.maybeTake(left).value_or(nullptr)) {
    Right right = *rightPtr;
    m_rightMap.remove(*rightPtr);
    return right;
  } else {
    return std::nullopt;
  }
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::maybeTakeLeft(Right const& right) -> std::optional<LeftT> {
  if (auto leftPtr = m_rightMap.maybeTake(right).value_or(nullptr)) {
    Left left = *leftPtr;
    m_leftMap.remove(*leftPtr);
    return left;
  } else {
    return std::nullopt;
  }
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::rightPtr(Left const& left) const -> RightT const* {
  return m_leftMap.value(left);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::leftPtr(Right const& right) const -> LeftT const* {
  return m_rightMap.value(right);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::operator=(BiMap const& map) -> BiMap<LeftT, RightT, LeftMapT, RightMapT>& {
  if (this != &map) {
    clear();
    for (auto const& p : map)
      insert(p);
  }
  return *this;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::insert(value_type const& val) -> std::pair<iterator, bool> {
  auto leftRes = m_leftMap.insert(std::make_pair(val.first, nullptr));
  if (!leftRes.second)
    return {BiMapIterator{leftRes.first}, false};

  auto rightRes = m_rightMap.insert(std::make_pair(val.second, nullptr));
  leftRes.first->second = &rightRes.first->first;
  rightRes.first->second = &leftRes.first->first;
  return {BiMapIterator{leftRes.first}, true};
};

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::insert(Left const& left, Right const& right) -> bool {
  return insert(std::make_pair(left, right)).second;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
void BiMap<LeftT, RightT, LeftMapT, RightMapT>::add(Left const& left, Right const& right) {
  if (m_leftMap.contains(left))
    throw MapException(strf("BiMap already contains left side value '{}'", outputAny(left)));

  if (m_rightMap.contains(right))
    throw MapException(strf("BiMap already contains right side value '{}'", outputAny(right)));

  insert(left, right);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
void BiMap<LeftT, RightT, LeftMapT, RightMapT>::add(value_type const& value) {
  add(value.first, value.second);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
void BiMap<LeftT, RightT, LeftMapT, RightMapT>::overwrite(Left const& left, Right const& right) {
  removeLeft(left);
  removeRight(right);
  insert(left, right);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
void BiMap<LeftT, RightT, LeftMapT, RightMapT>::overwrite(value_type const& value) {
  return overwrite(value.first, value.second);
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::removeLeft(Left const& left) -> bool {
  if (auto right = m_leftMap.value(left)) {
    m_rightMap.remove(*right);
    m_leftMap.remove(left);
    return true;
  }

  return false;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::removeRight(Right const& right) -> bool {
  if (auto left = m_rightMap.value(right)) {
    m_leftMap.remove(*left);
    m_rightMap.remove(right);
    return true;
  }

  return false;
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::begin() const -> const_iterator {
  return BiMapIterator{m_leftMap.begin()};
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::end() const -> const_iterator {
  return BiMapIterator{m_leftMap.end()};
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::size() const -> std::size_t {
  return m_leftMap.size();
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
void BiMap<LeftT, RightT, LeftMapT, RightMapT>::clear() {
  m_leftMap.clear();
  m_rightMap.clear();
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::empty() const -> bool {
  return m_leftMap.empty();
}

template <typename LeftT, typename RightT, typename LeftMapT, typename RightMapT>
auto BiMap<LeftT, RightT, LeftMapT, RightMapT>::operator==(BiMap const& m) const -> bool {
  if (&m == this)
    return true;

  if (size() != m.size())
    return false;

  for (auto const& pair : *this) {
    if (auto p = m.rightPtr(pair.first))
      if (!p || *p != pair.second)
        return false;
  }

  return true;
}

}// namespace Star
