#pragma once

#include "StarThread.hpp"

import std;

namespace Star {

// Thread safe shared_ptr such that is is possible to safely access the
// contents of the shared_ptr while other threads might be updating it.  Makes
// it possible to safely do Read Copy Update.
template <typename T>
class AtomicSharedPtr {
public:
  using SharedPtr = std::shared_ptr<T>;
  using WeakPtr = std::weak_ptr<T>;

  AtomicSharedPtr();
  AtomicSharedPtr(AtomicSharedPtr const& p);
  AtomicSharedPtr(AtomicSharedPtr&& p);
  AtomicSharedPtr(SharedPtr p);

  auto load() const -> SharedPtr;
  auto weak() const -> WeakPtr;
  void store(SharedPtr p);
  void reset();

  explicit operator bool() const;
  auto unique() const -> bool;

  auto operator->() const -> SharedPtr;

  auto operator=(AtomicSharedPtr const& p) -> AtomicSharedPtr&;
  auto operator=(AtomicSharedPtr&& p) -> AtomicSharedPtr&;
  auto operator=(SharedPtr p) -> AtomicSharedPtr&;

private:
  SharedPtr m_ptr;
  mutable SpinLock m_lock;
};

template <typename T>
AtomicSharedPtr<T>::AtomicSharedPtr() = default;

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
  SpinLocker locker(m_lock);
  return m_ptr;
}

template <typename T>
auto AtomicSharedPtr<T>::weak() const -> WeakPtr {
  SpinLocker locker(m_lock);
  return WeakPtr(m_ptr);
}

template <typename T>
void AtomicSharedPtr<T>::store(SharedPtr p) {
  SpinLocker locker(m_lock);
  m_ptr = std::move(p);
}

template <typename T>
void AtomicSharedPtr<T>::reset() {
  SpinLocker locker(m_lock);
  m_ptr.reset();
}

template <typename T>
AtomicSharedPtr<T>::operator bool() const {
  SpinLocker locker(m_lock);
  return (bool)m_ptr;
}

template <typename T>
auto AtomicSharedPtr<T>::unique() const -> bool {
  SpinLocker locker(m_lock);
  return m_ptr.use_count() == 1;
}

template <typename T>
auto AtomicSharedPtr<T>::operator-> () const -> SharedPtr {
  SpinLocker locker(m_lock);
  return m_ptr;
}

template <typename T>
auto AtomicSharedPtr<T>::operator=(AtomicSharedPtr const& p) -> AtomicSharedPtr<T>& {
  SpinLocker locker(m_lock);
  m_ptr = p.load();
  return *this;
}

template <typename T>
auto AtomicSharedPtr<T>::operator=(AtomicSharedPtr&& p) -> AtomicSharedPtr<T>& {
  SpinLocker locker(m_lock);
  m_ptr = std::move(p.m_ptr);
  return *this;
}

template <typename T>
auto AtomicSharedPtr<T>::operator=(SharedPtr p) -> AtomicSharedPtr<T>& {
  SpinLocker locker(m_lock);
  m_ptr = std::move(p);
  return *this;
}

}
