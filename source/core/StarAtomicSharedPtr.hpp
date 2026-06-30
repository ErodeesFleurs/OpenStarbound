#pragma once

#include "StarThread.hpp"

namespace Star {

// Thread safe shared_ptr such that is is possible to safely access the
// contents of the shared_ptr while other threads might be updating it.  Makes
// it possible to safely do Read Copy Update.
template <typename T>
class AtomicSharedPtr {
public:
  using SharedPtr = shared_ptr<T>;
  using WeakPtr = weak_ptr<T>;

  AtomicSharedPtr() = default;
  AtomicSharedPtr(AtomicSharedPtr const& p);
  AtomicSharedPtr(AtomicSharedPtr&& p);
  AtomicSharedPtr(SharedPtr p);

  [[nodiscard]] SharedPtr load() const;
  [[nodiscard]] WeakPtr weak() const;
  void store(SharedPtr p);
  void reset();

  [[nodiscard]] explicit operator bool() const;
  [[nodiscard]] bool unique() const;

  [[nodiscard]] SharedPtr operator->() const;

  AtomicSharedPtr& operator=(AtomicSharedPtr const& p);
  AtomicSharedPtr& operator=(AtomicSharedPtr&& p);
  AtomicSharedPtr& operator=(SharedPtr p);

private:
  SharedPtr m_ptr;
  mutable SpinLock m_lock;
};

template <typename T>
AtomicSharedPtr<T>::AtomicSharedPtr(AtomicSharedPtr const& p)
  : m_ptr(p.load()) {}

template <typename T>
AtomicSharedPtr<T>::AtomicSharedPtr(AtomicSharedPtr&& p)
  : m_ptr(std::move(p.m_ptr)) {}

template <typename T>
AtomicSharedPtr<T>::AtomicSharedPtr(SharedPtr p)
  : m_ptr(std::move(p)) {}

template <typename T>
auto AtomicSharedPtr<T>::load() const -> SharedPtr {
  [[nodiscard]] SpinLocker locker(m_lock);
  return m_ptr;
}

template <typename T>
auto AtomicSharedPtr<T>::weak() const -> WeakPtr {
  [[nodiscard]] SpinLocker locker(m_lock);
  return WeakPtr(m_ptr);
}

template <typename T>
void AtomicSharedPtr<T>::store(SharedPtr p) {
  [[nodiscard]] SpinLocker locker(m_lock);
  m_ptr = std::move(p);
}

template <typename T>
void AtomicSharedPtr<T>::reset() {
  [[nodiscard]] SpinLocker locker(m_lock);
  m_ptr.reset();
}

template <typename T>
AtomicSharedPtr<T>::operator bool() const {
  [[nodiscard]] SpinLocker locker(m_lock);
  return m_ptr != nullptr;
}

template <typename T>
bool AtomicSharedPtr<T>::unique() const {
  [[nodiscard]] SpinLocker locker(m_lock);
  return m_ptr.unique();
}

template <typename T>
auto AtomicSharedPtr<T>::operator-> () const -> SharedPtr {
  [[nodiscard]] SpinLocker locker(m_lock);
  return m_ptr;
}

template <typename T>
AtomicSharedPtr<T>& AtomicSharedPtr<T>::operator=(AtomicSharedPtr const& p) {
  [[nodiscard]] SpinLocker locker(m_lock);
  m_ptr = p.load();
  return *this;
}

template <typename T>
AtomicSharedPtr<T>& AtomicSharedPtr<T>::operator=(AtomicSharedPtr&& p) {
  [[nodiscard]] SpinLocker locker(m_lock);
  m_ptr = std::move(p.m_ptr);
  return *this;
}

template <typename T>
AtomicSharedPtr<T>& AtomicSharedPtr<T>::operator=(SharedPtr p) {
  [[nodiscard]] SpinLocker locker(m_lock);
  m_ptr = std::move(p);
  return *this;
}

}
