#pragma once

#include <memory>
#include <atomic>

#include "StarThread.hpp"

namespace Star {

template <typename T>
class PacketPool {
public:
  PacketPool() = default;
  PacketPool(PacketPool const&) = delete;
  PacketPool& operator=(PacketPool const&) = delete;

  T* acquire() {
    SpinLocker locker(m_lock);
    if (m_freeList) {
      auto* p = m_freeList;
      m_freeList = *reinterpret_cast<void**>(m_freeList);
      --m_allocCount;
      return static_cast<T*>(p);
    }
    ++m_allocCount;
    return static_cast<T*>(Star::malloc(sizeof(T)));
  }

  void release(T* p) {
    p->~T();
    SpinLocker locker(m_lock);
      *reinterpret_cast<void**>(p) = m_freeList;
    m_freeList = p;
    ++m_allocCount;
  }

  void reserve(size_t count) {
    SpinLocker locker(m_lock);
    for (size_t i = 0; i < count; ++i) {
      auto* p = Star::malloc(sizeof(T));
    *reinterpret_cast<void**>(p) = m_freeList;
      m_freeList = p;
      ++m_allocCount;
    }
  }

  size_t allocated() const {
    return m_allocCount.load();
  }

private:
  void* m_freeList = nullptr;
  SpinLock m_lock;
  std::atomic<size_t> m_allocCount{0};
};

template <typename T>
PacketPool<T>& packetPool() {
  static PacketPool<T> pool;
  return pool;
}

template <typename T, typename... Args>
std::shared_ptr<T> makePooled(Args&&... args) {
  auto& pool = packetPool<T>();
  void* mem = pool.acquire();
  ::new (mem) T(std::forward<Args>(args)...);
  return std::shared_ptr<T>(
      static_cast<T*>(mem),
      [&pool](T* p) { pool.release(p); });
}

}
