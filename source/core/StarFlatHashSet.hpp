#pragma once

#include "StarFlatHashTable.hpp"
#include "StarHash.hpp"

import std;

namespace Star {

template <typename Key, typename Hash = hash<Key>, typename Equals = std::equal_to<Key>, typename Allocator = std::allocator<Key>>
class FlatHashSet {
public:
  using key_type = Key;
  using value_type = Key;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using hasher = Hash;
  using key_equal = Equals;
  using allocator_type = Allocator;

  using reference = value_type&;
  using const_reference = const value_type&;

  using pointer         = std::allocator_traits<Allocator>::pointer;
  using const_pointer   = std::allocator_traits<Allocator>::const_pointer;

private:
  struct GetKey {
    auto operator()(value_type const& value) const -> key_type const&;
  };

  using Table = FlatHashTable<Key, Key, GetKey, Hash, Equals, Allocator>;

public:
  struct const_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename FlatHashSet::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    auto operator==(const_iterator const& rhs) const -> bool;
    auto operator!=(const_iterator const& rhs) const -> bool;

    auto operator++() -> const_iterator&;
    auto operator++(int) -> const_iterator;

    auto operator*() const -> value_type&;
    auto operator->() const -> value_type*;

    typename Table::const_iterator inner;
  };

  struct iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename FlatHashSet::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    auto operator==(iterator const& rhs) const -> bool;
    auto operator!=(iterator const& rhs) const -> bool;

    auto operator++() -> iterator&;
    auto operator++(int) -> iterator;

    auto operator*() const -> value_type&;
    auto operator->() const -> value_type*;

    operator const_iterator() const;

    typename Table::iterator inner;
  };

  FlatHashSet();
  explicit FlatHashSet(std::size_t bucketCount, hasher const& hash = hasher(),
      key_equal const& equal = key_equal(), allocator_type const& alloc = allocator_type());
  FlatHashSet(std::size_t bucketCount, allocator_type const& alloc);
  FlatHashSet(std::size_t bucketCount, hasher const& hash, allocator_type const& alloc);
  explicit FlatHashSet(allocator_type const& alloc);

  template <typename InputIt>
  FlatHashSet(InputIt first, InputIt last, std::size_t bucketCount = 0,
      hasher const& hash = hasher(), key_equal const& equal = key_equal(),
      allocator_type const& alloc = allocator_type());
  template <typename InputIt>
  FlatHashSet(InputIt first, InputIt last, std::size_t bucketCount, allocator_type const& alloc);
  template <typename InputIt>
  FlatHashSet(InputIt first, InputIt last, std::size_t bucketCount,
      hasher const& hash, allocator_type const& alloc);

  FlatHashSet(FlatHashSet const& other);
  FlatHashSet(FlatHashSet const& other, allocator_type const& alloc);
  FlatHashSet(FlatHashSet&& other);
  FlatHashSet(FlatHashSet&& other, allocator_type const& alloc);

  FlatHashSet(std::initializer_list<value_type> init, std::size_t bucketCount = 0,
      hasher const& hash = hasher(), key_equal const& equal = key_equal(),
      allocator_type const& alloc = allocator_type());
  FlatHashSet(std::initializer_list<value_type> init, std::size_t bucketCount, allocator_type const& alloc);
  FlatHashSet(std::initializer_list<value_type> init, std::size_t bucketCount, hasher const& hash,
      allocator_type const& alloc);

  auto operator=(FlatHashSet const& other) -> FlatHashSet&;
  auto operator=(FlatHashSet&& other) -> FlatHashSet&;
  auto operator=(std::initializer_list<value_type> init) -> FlatHashSet&;

  auto begin() -> iterator;
  auto end() -> iterator;

  auto begin() const -> const_iterator;
  auto end() const -> const_iterator;

  auto cbegin() const -> const_iterator;
  auto cend() const -> const_iterator;

  [[nodiscard]] auto empty() const -> std::size_t;
  [[nodiscard]] auto size() const -> std::size_t;
  void clear();

  auto insert(value_type const& value) -> std::pair<iterator, bool>;
  auto insert(value_type&& value) -> std::pair<iterator, bool>;
  auto insert(const_iterator hint, value_type const& value) -> iterator;
  auto insert(const_iterator hint, value_type&& value) -> iterator;
  template <typename InputIt>
  void insert(InputIt first, InputIt last);
  void insert(std::initializer_list<value_type> init);

  template <typename... Args>
  auto emplace(Args&&... args) -> std::pair<iterator, bool>;
  template <typename... Args>
  auto emplace_hint(const_iterator hint, Args&&... args) -> iterator;

  auto erase(const_iterator pos) -> iterator;
  auto erase(const_iterator first, const_iterator last) -> iterator;
  auto erase(key_type const& key) -> std::size_t;

  auto count(key_type const& key) const -> std::size_t;
  auto find(key_type const& key) const -> const_iterator;
  auto find(key_type const& key) -> iterator;
  auto equal_range(key_type const& key) -> std::pair<iterator, iterator>;
  auto equal_range(key_type const& key) const -> std::pair<const_iterator, const_iterator>;

  void reserve(std::size_t capacity);

  auto operator==(FlatHashSet const& rhs) const -> bool;
  auto operator!=(FlatHashSet const& rhs) const -> bool;

private:
  Table m_table;
};

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::GetKey::operator()(value_type const& value) const -> key_type const& {
  return value;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::const_iterator::operator==(const_iterator const& rhs) const -> bool {
  return inner == rhs.inner;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::const_iterator::operator!=(const_iterator const& rhs) const -> bool {
  return inner != rhs.inner;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::const_iterator::operator++() -> const_iterator& {
  ++inner;
  return *this;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::const_iterator::operator++(int) -> const_iterator {
  const_iterator copy(*this);
  operator++();
  return copy;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::const_iterator::operator*() const -> value_type& {
  return *inner;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::const_iterator::operator->() const -> value_type* {
  return &operator*();
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::iterator::operator==(iterator const& rhs) const -> bool {
  return inner == rhs.inner;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::iterator::operator!=(iterator const& rhs) const -> bool {
  return inner != rhs.inner;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::iterator::operator++() -> iterator& {
  ++inner;
  return *this;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::iterator::operator++(int) -> iterator {
  iterator copy(*this);
  operator++();
  return copy;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::iterator::operator*() const -> value_type& {
  return *inner;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::iterator::operator->() const -> value_type* {
  return &operator*();
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::iterator::operator typename FlatHashSet<Key, Hash, Equals, Allocator>::const_iterator() const {
  return const_iterator{inner};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet()
  : FlatHashSet(0) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(std::size_t bucketCount, hasher const& hash,
    key_equal const& equal, allocator_type const& alloc)
  : m_table(bucketCount, GetKey(), hash, equal, alloc) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(std::size_t bucketCount, allocator_type const& alloc)
  : FlatHashSet(bucketCount, hasher(), key_equal(), alloc) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(std::size_t bucketCount, hasher const& hash,
    allocator_type const& alloc)
  : FlatHashSet(bucketCount, hash, key_equal(), alloc) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(allocator_type const& alloc)
  : FlatHashSet(0, hasher(), key_equal(), alloc) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
template <typename InputIt>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(InputIt first, InputIt last, std::size_t bucketCount,
    hasher const& hash, key_equal const& equal, allocator_type const& alloc)
  : FlatHashSet(bucketCount, hash, equal, alloc) {
  insert(first, last);
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
template <typename InputIt>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(InputIt first, InputIt last, std::size_t bucketCount,
    allocator_type const& alloc)
  : FlatHashSet(first, last, bucketCount, hasher(), key_equal(), alloc) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
template <typename InputIt>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(InputIt first, InputIt last, std::size_t bucketCount,
    hasher const& hash, allocator_type const& alloc)
  : FlatHashSet(first, last, bucketCount, hash, key_equal(), alloc) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(FlatHashSet const& other)
  : FlatHashSet(other, other.m_table.getAllocator()) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(FlatHashSet const& other, allocator_type const& alloc)
  : FlatHashSet(alloc) {
  operator=(other);
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(FlatHashSet&& other)
  : FlatHashSet(std::move(other), other.m_table.getAllocator()) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(FlatHashSet&& other, allocator_type const& alloc)
  : FlatHashSet(alloc) {
  operator=(std::move(other));
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(std::initializer_list<value_type> init, std::size_t bucketCount,
    hasher const& hash, key_equal const& equal, allocator_type const& alloc)
  : FlatHashSet(bucketCount, hash, equal, alloc) {
  operator=(init);
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(std::initializer_list<value_type> init, std::size_t bucketCount, allocator_type const& alloc)
  : FlatHashSet(init, bucketCount, hasher(), key_equal(), alloc) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
FlatHashSet<Key, Hash, Equals, Allocator>::FlatHashSet(std::initializer_list<value_type> init, std::size_t bucketCount,
    hasher const& hash, allocator_type const& alloc)
  : FlatHashSet(init, bucketCount, hash, key_equal(), alloc) {}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::operator=(FlatHashSet const& other) -> FlatHashSet<Key, Hash, Equals, Allocator>& {
  m_table.clear();
  m_table.reserve(other.size());
  for (auto const& p : other)
    m_table.insert(p);
  return *this;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::operator=(FlatHashSet&& other) -> FlatHashSet<Key, Hash, Equals, Allocator>& {
  m_table = std::move(other.m_table);
  return *this;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::operator=(std::initializer_list<value_type> init) -> FlatHashSet<Key, Hash, Equals, Allocator>& {
  clear();
  insert(init.begin(), init.end());
  return *this;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::begin() -> iterator {
  return iterator{m_table.begin()};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::end() -> iterator {
  return iterator{m_table.end()};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::begin() const -> const_iterator {
  return const_iterator{m_table.begin()};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::end() const -> const_iterator {
  return const_iterator{m_table.end()};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::cbegin() const -> const_iterator {
  return begin();
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::cend() const -> const_iterator {
  return end();
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::empty() const -> std::size_t {
  return m_table.empty();
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::size() const -> std::size_t {
  return m_table.size();
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
void FlatHashSet<Key, Hash, Equals, Allocator>::clear() {
  m_table.clear();
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::insert(value_type const& value) -> std::pair<iterator, bool> {
  auto res = m_table.insert(value);
  return {iterator{res.first}, res.second};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::insert(value_type&& value) -> std::pair<iterator, bool> {
  auto res = m_table.insert(std::move(value));
  return {iterator{res.first}, res.second};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::insert(const_iterator i, value_type const& value) -> iterator {
  return insert(i, value_type(value));
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::insert(const_iterator, value_type&& value) -> iterator {
  return insert(std::move(value)).first;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
template <typename InputIt>
void FlatHashSet<Key, Hash, Equals, Allocator>::insert(InputIt first, InputIt last) {
  m_table.reserve(m_table.size() + std::distance(first, last));
  for (auto i = first; i != last; ++i)
    m_table.insert(*i);
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
void FlatHashSet<Key, Hash, Equals, Allocator>::insert(std::initializer_list<value_type> init) {
  insert(init.begin(), init.end());
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
template <typename... Args>
auto FlatHashSet<Key, Hash, Equals, Allocator>::emplace(Args&&... args) -> std::pair<iterator, bool> {
  return insert(value_type(forward<Args>(args)...));
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
template <typename... Args>
auto FlatHashSet<Key, Hash, Equals, Allocator>::emplace_hint(const_iterator i, Args&&... args) -> iterator {
  return insert(i, value_type(forward<Args>(args)...));
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::erase(const_iterator pos) -> iterator {
  return iterator{m_table.erase(pos.inner)};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::erase(const_iterator first, const_iterator last) -> iterator {
  return iterator{m_table.erase(first.inner, last.inner)};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::erase(key_type const& key) -> std::size_t {
  auto i = m_table.find(key);
  if (i != m_table.end()) {
    m_table.erase(i);
    return 1;
  }
  return 0;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::count(Key const& key) const -> std::size_t {
  if (m_table.find(key) != m_table.end())
    return 1;
  else
    return 0;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::find(key_type const& key) const -> const_iterator {
  return const_iterator{m_table.find(key)};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::find(key_type const& key) -> iterator {
  return iterator{m_table.find(key)};
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::equal_range(key_type const& key) -> std::pair<iterator, iterator> {
  auto i = find(key);
  if (i != end()) {
    auto j = i;
    ++j;
    return {i, j};
  } else {
    return {i, i};
  }
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::equal_range(key_type const& key) const -> std::pair<const_iterator, const_iterator> {
  auto i = find(key);
  if (i != end()) {
    auto j = i;
    ++j;
    return {i, j};
  } else {
    return {i, i};
  }
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
void FlatHashSet<Key, Hash, Equals, Allocator>::reserve(std::size_t capacity) {
  m_table.reserve(capacity);
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::operator==(FlatHashSet const& rhs) const -> bool {
  return m_table == rhs.m_table;
}

template <typename Key, typename Hash, typename Equals, typename Allocator>
auto FlatHashSet<Key, Hash, Equals, Allocator>::operator!=(FlatHashSet const& rhs) const -> bool {
  return m_table != rhs.m_table;
}

}
