#pragma once

#include "StarException.hpp"
#include "StarHash.hpp"

#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>

namespace Star {

STAR_EXCEPTION(InvalidMaybeAccessException, StarException);

template <typename T>
class Maybe {
public:
  typedef T* PointerType;
  typedef T const* PointerConstType;
  typedef T& RefType;
  typedef T const& RefConstType;

  Maybe() = default;
  Maybe(std::nullopt_t) : m_data(std::nullopt) {}

  Maybe(T const& t) : m_data(t) {}
  Maybe(T&& t) : m_data(std::move(t)) {}

  Maybe(Maybe const& rhs) = default;
  Maybe(Maybe&& rhs) : m_data(std::move(rhs.m_data)) {
    rhs.reset();
  }

  template <typename T2>
  Maybe(Maybe<T2> const& rhs) {
    if (rhs.isValid())
      m_data.emplace(*rhs);
  }

  Maybe(std::optional<T> const& opt) : m_data(opt) {}
  Maybe(std::optional<T>&& opt) : m_data(std::move(opt)) {}

  ~Maybe() = default;

  Maybe& operator=(Maybe const& rhs) = default;
  Maybe& operator=(Maybe&& rhs) {
    m_data = std::move(rhs.m_data);
    rhs.reset();
    return *this;
  }

  template <typename T2>
  Maybe& operator=(Maybe<T2> const& rhs) {
    if (rhs.isValid())
      m_data.emplace(*rhs);
    else
      m_data.reset();
    return *this;
  }

  bool isValid() const {
    return m_data.has_value();
  }

  bool isNothing() const {
    return !m_data.has_value();
  }

  explicit operator bool() const {
    return m_data.has_value();
  }

  PointerConstType ptr() const {
    return m_data.has_value() ? &*m_data : nullptr;
  }

  PointerType ptr() {
    return m_data.has_value() ? &*m_data : nullptr;
  }

  PointerConstType operator->() const {
    if (!m_data.has_value())
      throw InvalidMaybeAccessException();
    return &*m_data;
  }

  PointerType operator->() {
    if (!m_data.has_value())
      throw InvalidMaybeAccessException();
    return &*m_data;
  }

  RefConstType operator*() const {
    return get();
  }

  RefType operator*() {
    return get();
  }

  bool operator==(Maybe const& rhs) const {
    return m_data == rhs.m_data;
  }

  bool operator!=(Maybe const& rhs) const {
    return m_data != rhs.m_data;
  }

  bool operator<(Maybe const& rhs) const {
    return m_data < rhs.m_data;
  }

  RefConstType get() const {
    if (!m_data.has_value())
      throw InvalidMaybeAccessException();
    return *m_data;
  }

  RefType get() {
    if (!m_data.has_value())
      throw InvalidMaybeAccessException();
    return *m_data;
  }

  // Get either the contents of this Maybe or the given default.
  T value(T def = T()) const {
    return m_data.value_or(std::move(def));
  }

  // Get either this value, or if this value is none the given value.
  Maybe orMaybe(Maybe const& other) const {
    if (m_data.has_value())
      return *this;
    else
      return other;
  }

  // Takes the value out of this Maybe, leaving it Nothing.
  T take() {
    if (!m_data.has_value())
      throw InvalidMaybeAccessException();
    T val = std::move(*m_data);
    m_data.reset();
    return val;
  }

  // If this Maybe is set, assigns it to t and leaves this Maybe as Nothing.
  bool put(T& t) {
    if (m_data.has_value()) {
      t = std::move(*m_data);
      m_data.reset();
      return true;
    } else {
      return false;
    }
  }

  void set(T const& t) {
    m_data = t;
  }

  void set(T&& t) {
    m_data = std::move(t);
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    m_data.emplace(std::forward<Args>(args)...);
  }

  void reset() {
    m_data.reset();
  }

  // Apply a function to the contained value if it is not Nothing.
  template <typename Function>
  void exec(Function&& function) {
    if (m_data.has_value())
      function(*m_data);
  }

  // Functor map operator.
  template <typename Function>
  auto apply(Function&& function) const -> Maybe<std::decay_t<decltype(function(std::declval<T>()))>> {
    if (!m_data.has_value())
      return std::nullopt;
    return function(*m_data);
  }

  // Monadic bind operator.
  template <typename Function>
  auto sequence(Function&& function) const -> decltype(function(std::declval<T>())) {
    if (!m_data.has_value())
      return std::nullopt;
    return function(*m_data);
  }

  // Implicit conversion to std::optional
  operator std::optional<T> const&() const { return m_data; }
  operator std::optional<T>&() { return m_data; }

private:
  std::optional<T> m_data;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, Maybe<T> const& v) {
  if (v)
    return os << "Just (" << *v << ")";
  else
    return os << "Nothing";
}

template <typename T>
struct hash<Maybe<T>> {
  size_t operator()(Maybe<T> const& m) const {
    if (!m)
      return 0;
    else
      return hasher(*m);
  }
  hash<T> hasher;
};

}

template <typename T>
struct std::formatter<Star::Maybe<T>> : Star::ostream_formatter {};
