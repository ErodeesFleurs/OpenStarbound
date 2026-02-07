#pragma once

#include "StarHash.hpp"

import std;

namespace Star {

// Reference counted ptr for intrusive reference counted types.  Calls
// unqualified refPtrIncRef and refPtrDecRef functions to manage the reference
// count.
template <typename T>
class RefPtr {
public:
  using element_type = T;

  RefPtr();
  explicit RefPtr(T* p, bool addRef = true);

  RefPtr(RefPtr const& r);
  RefPtr(RefPtr&& r);

  template <typename T2>
  RefPtr(RefPtr<T2> const& r);
  template <typename T2>
  RefPtr(RefPtr<T2>&& r);

  ~RefPtr();

  auto operator=(RefPtr const& r) -> RefPtr&;
  auto operator=(RefPtr&& r) -> RefPtr&;

  template <typename T2>
  auto operator=(RefPtr<T2> const& r) -> RefPtr&;
  template <typename T2>
  auto operator=(RefPtr<T2>&& r) -> RefPtr&;

  void reset();

  void reset(T* r, bool addRef = true);

  auto operator*() const -> T&;
  auto operator->() const -> T*;
  auto get() const -> T*;

  explicit operator bool() const;

private:
  template <typename T2>
  friend class RefPtr;

  T* m_ptr;
};

template <typename T, typename U>
auto operator==(RefPtr<T> const& a, RefPtr<U> const& b) -> bool;

template <typename T, typename U>
auto operator!=(RefPtr<T> const& a, RefPtr<U> const& b) -> bool;

template <typename T>
auto operator==(RefPtr<T> const& a, T* b) -> bool;

template <typename T>
auto operator!=(RefPtr<T> const& a, T* b) -> bool;

template <typename T>
auto operator==(T* a, RefPtr<T> const& b) -> bool;

template <typename T>
auto operator!=(T* a, RefPtr<T> const& b) -> bool;

template <typename T, typename U>
auto operator<(RefPtr<T> const& a, RefPtr<U> const& b) -> bool;

template <typename Type1, typename Type2>
auto is(RefPtr<Type2> const& p) -> bool;

template <typename Type1, typename Type2>
auto is(RefPtr<Type2 const> const& p) -> bool;

template <typename Type1, typename Type2>
auto as(RefPtr<Type2> const& p) -> RefPtr<Type1>;

template <typename Type1, typename Type2>
auto as(RefPtr<Type2 const> const& p) -> RefPtr<Type1 const>;

template <typename T, typename... Args>
auto make_ref(Args&&... args) -> RefPtr<T>;

template <typename T>
struct hash<RefPtr<T>> {
  auto operator()(RefPtr<T> const& a) const -> std::size_t;

  hash<T*> hasher;
};

// Base class for RefPtr that is NOT thread safe.  This can have a performance
// benefit over shared_ptr in single threaded contexts.
class RefCounter {
public:
  friend void refPtrIncRef(RefCounter* p);
  friend void refPtrDecRef(RefCounter* p);

protected:
  RefCounter();
  virtual ~RefCounter() = default;

private:
  std::size_t m_refCounter{};
};

template <typename T>
RefPtr<T>::RefPtr()
    : m_ptr(nullptr) {}

template <typename T>
RefPtr<T>::RefPtr(T* p, bool addRef)
    : m_ptr(nullptr) {
  reset(p, addRef);
}

template <typename T>
RefPtr<T>::RefPtr(RefPtr const& r)
    : RefPtr(r.m_ptr) {}

template <typename T>
RefPtr<T>::RefPtr(RefPtr&& r) {
  m_ptr = r.m_ptr;
  r.m_ptr = nullptr;
}

template <typename T>
template <typename T2>
RefPtr<T>::RefPtr(RefPtr<T2> const& r)
    : RefPtr(r.m_ptr) {}

template <typename T>
template <typename T2>
RefPtr<T>::RefPtr(RefPtr<T2>&& r) {
  m_ptr = r.m_ptr;
  r.m_ptr = nullptr;
}

template <typename T>
RefPtr<T>::~RefPtr() {
  if (m_ptr)
    refPtrDecRef(m_ptr);
}

template <typename T>
auto RefPtr<T>::operator=(RefPtr const& r) -> RefPtr<T>& {
  reset(r.m_ptr);
  return *this;
}

template <typename T>
auto RefPtr<T>::operator=(RefPtr&& r) -> RefPtr<T>& {
  if (m_ptr)
    refPtrDecRef(m_ptr);

  m_ptr = r.m_ptr;
  r.m_ptr = nullptr;
  return *this;
}

template <typename T>
template <typename T2>
auto RefPtr<T>::operator=(RefPtr<T2> const& r) -> RefPtr<T>& {
  reset(r.m_ptr);
  return *this;
}

template <typename T>
template <typename T2>
auto RefPtr<T>::operator=(RefPtr<T2>&& r) -> RefPtr<T>& {
  if (m_ptr)
    refPtrDecRef(m_ptr);

  m_ptr = r.m_ptr;
  r.m_ptr = nullptr;
  return *this;
}

template <typename T>
void RefPtr<T>::reset() {
  reset(nullptr);
}

template <typename T>
void RefPtr<T>::reset(T* r, bool addRef) {
  if (m_ptr == r)
    return;

  if (m_ptr)
    refPtrDecRef(m_ptr);

  m_ptr = r;

  if (m_ptr && addRef)
    refPtrIncRef(m_ptr);
}

template <typename T>
auto RefPtr<T>::operator*() const -> T& {
  return *m_ptr;
}

template <typename T>
auto RefPtr<T>::operator->() const -> T* {
  return m_ptr;
}

template <typename T>
auto RefPtr<T>::get() const -> T* {
  return m_ptr;
}

template <typename T>
RefPtr<T>::operator bool() const {
  return m_ptr != nullptr;
}

template <typename T, typename U>
auto operator==(RefPtr<T> const& a, RefPtr<U> const& b) -> bool {
  return a.get() == b.get();
}

template <typename T, typename U>
auto operator!=(RefPtr<T> const& a, RefPtr<U> const& b) -> bool {
  return a.get() != b.get();
}

template <typename T>
auto operator==(RefPtr<T> const& a, T* b) -> bool {
  return a.get() == b;
}

template <typename T>
auto operator!=(RefPtr<T> const& a, T* b) -> bool {
  return a.get() != b;
}

template <typename T>
auto operator==(T* a, RefPtr<T> const& b) -> bool {
  return a == b.get();
}

template <typename T>
auto operator!=(T* a, RefPtr<T> const& b) -> bool {
  return a != b.get();
}

template <typename T, typename U>
auto operator<(RefPtr<T> const& a, RefPtr<U> const& b) -> bool {
  return a.get() < b.get();
}

template <typename Type1, typename Type2>
auto is(RefPtr<Type2> const& p) -> bool {
  return (bool)dynamic_cast<Type1*>(p.get());
}

template <typename Type1, typename Type2>
auto is(RefPtr<Type2 const> const& p) -> bool {
  return (bool)dynamic_cast<Type1 const*>(p.get());
}

template <typename Type1, typename Type2>
auto as(RefPtr<Type2> const& p) -> RefPtr<Type1> {
  return RefPtr<Type1>(dynamic_cast<Type1*>(p.get()));
}

template <typename Type1, typename Type2>
auto as(RefPtr<Type2 const> const& p) -> RefPtr<Type1 const> {
  return RefPtr<Type1>(dynamic_cast<Type1 const*>(p.get()));
}

template <typename T, typename... Args>
auto make_ref(Args&&... args) -> RefPtr<T> {
  return RefPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
auto hash<RefPtr<T>>::operator()(RefPtr<T> const& a) const -> std::size_t {
  return hasher(a.get());
}

inline void refPtrIncRef(RefCounter* p) {
  ++p->m_refCounter;
}

inline void refPtrDecRef(RefCounter* p) {
  if (--p->m_refCounter == 0)
    delete p;
}

inline RefCounter::RefCounter() = default;

}// namespace Star
