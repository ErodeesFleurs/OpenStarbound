#pragma once

#include "StarBlockAllocator.hpp"
#include "StarOrderedMap.hpp"

import std;

namespace Star {

template <typename OrderedMapType>
class LruCacheBase {
public:
  using Key = typename OrderedMapType::key_type;
  using Value = typename OrderedMapType::mapped_type;

  using ProducerFunction = std::function<Value(Key const&)>;

  LruCacheBase(std::size_t maxSize = 256);

  // Max size cannot be zero, it will be clamped to at least 1 in order to hold
  // the most recent element returned by get.
  [[nodiscard]] auto maxSize() const -> std::size_t;
  void setMaxSize(std::size_t maxSize);

  [[nodiscard]] auto currentSize() const -> std::size_t;

  auto keys() const -> List<Key>;
  auto values() const -> List<Value>;

  // If the value is in the cache, returns a pointer to it and marks it as
  // accessed, otherwise returns nullptr.
  auto ptr(Key const& key) -> Value*;

  // Put the given value into the cache.
  void set(Key const& key, Value value);
  // Removes the given value from the cache.  If found and removed, returns
  // true.
  auto remove(Key const& key) -> bool;

  // Remove all key / value pairs matching a filter.
  void removeWhere(std::function<bool(Key const&, Value&)> filter);

  // If the value for the key is not found in the cache, produce it with the
  // given producer.  Producer shold take the key as an argument and return the
  // value.
  template <typename Producer>
  auto get(Key const& key, Producer producer) -> Value&;

  // Clear all cached entries.
  void clear();

private:
  OrderedMapType m_map;
  std::size_t m_maxSize;
};

template <typename Key, typename Value, typename Compare = std::less<Key>, typename Allocator = BlockAllocator<std::pair<Key const, Value>, 1024>>
using LruCache = LruCacheBase<OrderedMap<Key, Value, Compare, Allocator>>;

template <typename Key, typename Value, typename Hash = Star::hash<Key>, typename Equals = std::equal_to<Key>, typename Allocator = BlockAllocator<std::pair<Key const, Value>, 1024>>
using HashLruCache = LruCacheBase<OrderedHashMap<Key, Value, Hash, Equals, Allocator>>;

template <typename OrderedMapType>
LruCacheBase<OrderedMapType>::LruCacheBase(std::size_t maxSize) {
  setMaxSize(maxSize);
}

template <typename OrderedMapType>
auto LruCacheBase<OrderedMapType>::maxSize() const -> std::size_t {
  return m_maxSize;
}

template <typename OrderedMapType>
void LruCacheBase<OrderedMapType>::setMaxSize(std::size_t maxSize) {
  m_maxSize = std::max<std::size_t>(maxSize, 1);

  while (m_map.size() > m_maxSize)
    m_map.removeFirst();
}

template <typename OrderedMapType>
auto LruCacheBase<OrderedMapType>::currentSize() const -> std::size_t {
  return m_map.size();
}

template <typename OrderedMapType>
auto LruCacheBase<OrderedMapType>::keys() const -> List<Key> {
  return m_map.keys();
}

template <typename OrderedMapType>
auto LruCacheBase<OrderedMapType>::values() const -> List<Value> {
  return m_map.values();
}

template <typename OrderedMapType>
auto LruCacheBase<OrderedMapType>::ptr(Key const& key) -> Value* {
  auto i = m_map.find(key);
  if (i == m_map.end())
    return nullptr;
  i = m_map.toBack(i);
  return &i->second;
}

template <typename OrderedMapType>
void LruCacheBase<OrderedMapType>::set(Key const& key, Value value) {
  auto i = m_map.find(key);
  if (i == m_map.end()) {
    m_map.add(key, std::move(value));
  } else {
    i->second = std::move(value);
    m_map.toBack(i);
  }
}

template <typename OrderedMapType>
auto LruCacheBase<OrderedMapType>::remove(Key const& key) -> bool {
  return m_map.remove(key);
}

template <typename OrderedMapType>
void LruCacheBase<OrderedMapType>::removeWhere(std::function<bool(Key const&, Value&)> filter) {
  eraseWhere(m_map, [&filter](auto& p) -> auto {
    return filter(p.first, p.second);
  });
}

template <typename OrderedMapType>
template <typename Producer>
auto LruCacheBase<OrderedMapType>::get(Key const& key, Producer producer) -> Value& {
  while (m_map.size() > m_maxSize - 1)
    m_map.removeFirst();

  auto i = m_map.find(key);
  if (i == m_map.end())
    i = m_map.insert({key, producer(key)}).first;
  else
    i = m_map.toBack(i);

  return i->second;
}

template <typename OrderedMapType>
void LruCacheBase<OrderedMapType>::clear() {
  m_map.clear();
}

}// namespace Star
