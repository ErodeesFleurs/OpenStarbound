#pragma once

#include "StarHash.hpp"

namespace Star {

// Non-owning observer pointer.  Documents that the pointed-to object is
// not owned by this pointer.  Nullable, copyable, zero-overhead wrapper
// around a raw pointer.
template <typename T>
class observer_ptr {
public:
  using element_type = T;

  constexpr observer_ptr() noexcept = default;
  constexpr observer_ptr(std::nullptr_t) noexcept {}
  constexpr explicit observer_ptr(T* p) noexcept : m_ptr(p) {}

  constexpr T* get() const noexcept { return m_ptr; }
  constexpr T& operator*() const noexcept { return *m_ptr; }
  constexpr T* operator->() const noexcept { return m_ptr; }
  constexpr explicit operator bool() const noexcept { return m_ptr != nullptr; }

  constexpr void reset(T* p = nullptr) noexcept { m_ptr = p; }
  constexpr void swap(observer_ptr& other) noexcept {
    auto tmp = m_ptr;
    m_ptr = other.m_ptr;
    other.m_ptr = tmp;
  }

  friend constexpr bool operator==(observer_ptr const& lhs, observer_ptr const& rhs) noexcept = default;
  friend constexpr bool operator!=(observer_ptr const& lhs, observer_ptr const& rhs) noexcept = default;

private:
  T* m_ptr = nullptr;
};

}

namespace std {

template <typename T>
struct hash<Star::observer_ptr<T>> {
  size_t operator()(Star::observer_ptr<T> const& p) const {
    return hash<T*>()(p.get());
  }
};

}
