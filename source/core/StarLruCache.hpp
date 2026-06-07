#pragma once

#include "StarOrderedMap.hpp"
#include "StarBlockAllocator.hpp"

namespace Star {

template <typename OrderedMapType>
class LruCacheBase {
public:
  using Key = typename OrderedMapType::key_type;
  using Value = typename OrderedMapType::mapped_type;

  using ProducerFunction = function<Value(Key const&)>;

  static constexpr size_t DefaultLruCacheSize = 256;

  LruCacheBase(size_t maxSize = DefaultLruCacheSize);

  // Max size cannot be zero, it will be clamped to at least 1 in order to hold
  // the most recent element returned by get.
  [[nodiscard]] size_t maxSize() const;
  void setMaxSize(size_t maxSize);

  [[nodiscard]] size_t currentSize() const;

  [[nodiscard]] List<Key> keys() const;
  [[nodiscard]] List<Value> values() const;

  // If the value is in the cache, returns a pointer to it and marks it as
  // accessed, otherwise returns nullptr.
  [[nodiscard]] Value* ptr(Key const& key);

  // Put the given value into the cache.
  void set(Key const& key, Value value);
  // Removes the given value from the cache.  If found and removed, returns
  // true.
  [[nodiscard]] bool remove(Key const& key);

  // Remove all key / value pairs matching a filter.
  void removeWhere(function<bool(Key const&, Value&)> filter);

  // If the value for the key is not found in the cache, produce it with the
  // given producer.  Producer shold take the key as an argument and return the
  // value.
  template <typename Producer>
  Value& get(Key const& key, Producer producer);

  // Clear all cached entries.
  void clear();

private:
  OrderedMapType m_map;
  size_t m_maxSize;
};

template <typename Key, typename Value, typename Compare = std::less<Key>, typename Allocator = BlockAllocator<pair<Key const, Value>, 1024>>
using LruCache = LruCacheBase<OrderedMap<Key, Value, Compare, Allocator>>;

template <typename Key, typename Value, typename Hash = Star::hash<Key>, typename Equals = std::equal_to<Key>, typename Allocator = BlockAllocator<pair<Key const, Value>, 1024>>
using HashLruCache = LruCacheBase<OrderedHashMap<Key, Value, Hash, Equals, Allocator>>;

template <typename OrderedMapType>
LruCacheBase<OrderedMapType>::LruCacheBase(size_t maxSize) {
  setMaxSize(maxSize);
}

template <typename OrderedMapType>
[[nodiscard]] size_t LruCacheBase<OrderedMapType>::maxSize() const {
  return m_maxSize;
}

template <typename OrderedMapType>
void LruCacheBase<OrderedMapType>::setMaxSize(size_t maxSize) {
  m_maxSize = max<size_t>(maxSize, 1);

  while (m_map.size() > m_maxSize)
    m_map.removeFirst();
}

template <typename OrderedMapType>
[[nodiscard]] size_t LruCacheBase<OrderedMapType>::currentSize() const {
  return m_map.size();
}

template <typename OrderedMapType>
[[nodiscard]] auto LruCacheBase<OrderedMapType>::keys() const -> List<Key> {
  return m_map.keys();
}

template <typename OrderedMapType>
[[nodiscard]] auto LruCacheBase<OrderedMapType>::values() const -> List<Value> {
  return m_map.values();
}

template <typename OrderedMapType>
[[nodiscard]] auto LruCacheBase<OrderedMapType>::ptr(Key const& key) -> Value * {
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
[[nodiscard]] bool LruCacheBase<OrderedMapType>::remove(Key const& key) {
  return m_map.remove(key);
}

template <typename OrderedMapType>
void LruCacheBase<OrderedMapType>::removeWhere(function<bool(Key const&, Value&)> filter) {
  eraseWhere(m_map, [&filter](auto& p) {
      return filter(p.first, p.second);
    });
}

template <typename OrderedMapType>
template <typename Producer>
auto LruCacheBase<OrderedMapType>::get(Key const& key, Producer producer) -> Value & {
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

}
