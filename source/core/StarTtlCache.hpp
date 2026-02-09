#pragma once

#include "StarLruCache.hpp"
#include "StarRandom.hpp"
#include "StarTime.hpp"

import std;

namespace Star {

template <typename LruCacheType>
class TtlCacheBase {
public:
  using Key = typename LruCacheType::Key;
  using Value = typename LruCacheType::Value::second_type;

  using ProducerFunction = std::function<Value(Key const&)>;

  TtlCacheBase(std::int64_t timeToLive = 10000, int timeSmear = 1000, std::size_t maxSize = std::numeric_limits<std::size_t>::max(), bool ttlUpdateEnabled = true);

  [[nodiscard]] auto timeToLive() const -> std::int64_t;
  void setTimeToLive(std::int64_t timeToLive);

  [[nodiscard]] auto timeSmear() const -> int;
  void setTimeSmear(int timeSmear);

  // If a max size is set, this cache also acts as an LRU cache with the given
  // maximum size.
  [[nodiscard]] auto maxSize() const -> std::size_t;
  void setMaxSize(std::size_t maxSize = std::numeric_limits<std::size_t>::max());

  [[nodiscard]] auto currentSize() const -> std::size_t;

  auto keys() const -> List<Key>;
  auto values() const -> List<Value>;

  // If ttlUpdateEnabled is false, then the time to live for entries will not
  // be updated on access.
  [[nodiscard]] auto ttlUpdateEnabled() const -> bool;
  void setTtlUpdateEnabled(bool enabled);

  // If the value is in the cache, returns it and updates the access time,
  // otherwise returns nullptr.
  auto ptr(Key const& key) -> Value*;

  // Put the given value into the cache.
  void set(Key const& key, Value value);
  // Removes the given value from the cache.  If found and removed, returns
  // true.
  auto remove(Key const& key) -> bool;

  // Remove all key / value pairs matching a filter.
  void removeWhere(std::function<bool(Key const&, Value&)> filter);

  // If the value for the key is not found in the cache, produce it with the
  // given producer.  Producer should take the key as an argument and return
  // the Value.
  template <typename Producer>
  auto get(Key const& key, Producer producer) -> Value&;

  void clear();

  // Cleanup any cached entries that are older than their time to live, if the
  // refreshFilter is given, things that match the refreshFilter instead have
  // their ttl refreshed rather than being removed.
  void cleanup(std::function<bool(Key const&, Value const&)> refreshFilter = {});

private:
  LruCacheType m_cache;
  std::int64_t m_timeToLive;
  int m_timeSmear;
  bool m_ttlUpdateEnabled;
};

template <typename Key, typename Value, typename Compare = std::less<Key>, typename Allocator = BlockAllocator<std::pair<Key const, std::pair<std::int64_t, Value>>, 1024>>
using TtlCache = TtlCacheBase<LruCache<Key, std::pair<std::int64_t, Value>, Compare, Allocator>>;

template <typename Key, typename Value, typename Hash = Star::hash<Key>, typename Equals = std::equal_to<Key>, typename Allocator = BlockAllocator<std::pair<Key const, std::pair<std::int64_t, Value>>, 1024>>
using HashTtlCache = TtlCacheBase<HashLruCache<Key, std::pair<std::int64_t, Value>, Hash, Equals, Allocator>>;

template <typename LruCacheType>
TtlCacheBase<LruCacheType>::TtlCacheBase(std::int64_t timeToLive, int timeSmear, std::size_t maxSize, bool ttlUpdateEnabled) {
  m_cache.setMaxSize(maxSize);
  m_timeToLive = timeToLive;
  m_timeSmear = timeSmear;
  m_ttlUpdateEnabled = ttlUpdateEnabled;
}

template <typename LruCacheType>
auto TtlCacheBase<LruCacheType>::timeToLive() const -> std::int64_t {
  return m_timeToLive;
}

template <typename LruCacheType>
void TtlCacheBase<LruCacheType>::setTimeToLive(std::int64_t timeToLive) {
  m_timeToLive = timeToLive;
}

template <typename LruCacheType>
auto TtlCacheBase<LruCacheType>::timeSmear() const -> int {
  return m_timeSmear;
}

template <typename LruCacheType>
void TtlCacheBase<LruCacheType>::setTimeSmear(int timeSmear) {
  m_timeSmear = timeSmear;
}

template <typename LruCacheType>
auto TtlCacheBase<LruCacheType>::ttlUpdateEnabled() const -> bool {
  return m_ttlUpdateEnabled;
}

template <typename LruCacheType>
auto TtlCacheBase<LruCacheType>::maxSize() const -> std::size_t {
  return m_cache.maxSize();
}

template <typename LruCacheType>
void TtlCacheBase<LruCacheType>::setMaxSize(std::size_t maxSize) {
  m_cache.setMaxSize(maxSize);
}

template <typename LruCacheType>
auto TtlCacheBase<LruCacheType>::currentSize() const -> std::size_t {
  return m_cache.currentSize();
}

template <typename LruCacheType>
auto TtlCacheBase<LruCacheType>::keys() const -> List<Key> {
  return m_cache.keys();
}

template <typename LruCacheType>
auto TtlCacheBase<LruCacheType>::values() const -> List<Value> {
  List<Value> values;
  for (auto& p : m_cache.values())
    values.append(std::move(p.second));
  return values;
}

template <typename LruCacheType>
void TtlCacheBase<LruCacheType>::setTtlUpdateEnabled(bool enabled) {
  m_ttlUpdateEnabled = enabled;
}

template <typename LruCacheType>
auto TtlCacheBase<LruCacheType>::ptr(Key const& key) -> Value* {
  if (auto p = m_cache.ptr(key)) {
    if (m_ttlUpdateEnabled)
      p->first = Time::monotonicMilliseconds() + Random::randInt(-m_timeSmear, m_timeSmear);
    return &p->second;
  }
  return nullptr;
}

template <typename LruCacheType>
void TtlCacheBase<LruCacheType>::set(Key const& key, Value value) {
  m_cache.set(key, std::make_pair(Time::monotonicMilliseconds() + Random::randInt(-m_timeSmear, m_timeSmear), value));
}

template <typename LruCacheType>
auto TtlCacheBase<LruCacheType>::remove(Key const& key) -> bool {
  return m_cache.remove(key);
}

template <typename LruCacheType>
void TtlCacheBase<LruCacheType>::removeWhere(std::function<bool(Key const&, Value&)> filter) {
  m_cache.removeWhere([&filter](auto const& key, auto& value) -> auto { return filter(key, value.second); });
}

template <typename LruCacheType>
template <typename Producer>
auto TtlCacheBase<LruCacheType>::get(Key const& key, Producer producer) -> Value& {
  auto& value = m_cache.get(key, [producer](Key const& key) -> auto {
    return std::pair<std::int64_t, Value>(0, producer(key));
  });
  if (value.first == 0 || m_ttlUpdateEnabled)
    value.first = Time::monotonicMilliseconds() + Random::randInt(-m_timeSmear, m_timeSmear);
  return value.second;
}

template <typename LruCacheType>
void TtlCacheBase<LruCacheType>::clear() {
  m_cache.clear();
}

template <typename LruCacheType>
void TtlCacheBase<LruCacheType>::cleanup(std::function<bool(Key const&, Value const&)> refreshFilter) {
  std::int64_t currentTime = Time::monotonicMilliseconds();
  m_cache.removeWhere([&](auto const& key, auto& value) -> auto {
    if (refreshFilter && refreshFilter(key, value.second)) {
      value.first = currentTime;
    } else {
      if (currentTime - value.first > m_timeToLive)
        return true;
    }
    return false;
  });
}

}// namespace Star
