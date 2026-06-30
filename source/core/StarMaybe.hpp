#pragma once

#include "StarException.hpp"
#include "StarHash.hpp"

#include <optional>
#include <type_traits>
#include <utility>

namespace Star {

struct InvalidMaybeAccessExceptionTag {
  static constexpr char const* typeName = "InvalidMaybeAccessException";
};
using InvalidMaybeAccessException = TypedException<StarException, InvalidMaybeAccessExceptionTag>;

template <typename T>
class Maybe {
public:
  using PointerType = T*;
  using PointerConstType = T const*;
  using RefType = T&;
  using RefConstType = T const&;

  Maybe() = default;

  Maybe(T const& t);
  Maybe(T&& t);
  Maybe(std::nullopt_t);

  Maybe(Maybe const& rhs);
  Maybe(Maybe&& rhs) noexcept(std::is_nothrow_move_constructible_v<T>);
  template <typename T2>
  Maybe(Maybe<T2> const& rhs);

  ~Maybe();

  Maybe& operator=(Maybe const& rhs);
  Maybe& operator=(Maybe&& rhs) noexcept(std::is_nothrow_move_constructible_v<T>);
  Maybe& operator=(std::nullopt_t);
  template <typename T2>
  Maybe& operator=(Maybe<T2> const& rhs);

  [[nodiscard]] static Maybe fromOptional(std::optional<T> const& t);
  [[nodiscard]] static Maybe fromOptional(std::optional<T>&& t);

  [[nodiscard]] bool isValid() const;
  [[nodiscard]] bool isNothing() const;
  explicit operator bool() const;

  [[nodiscard]] PointerConstType ptr() const;
  [[nodiscard]] PointerType ptr();

  PointerConstType operator->() const;
  PointerType operator->();

  RefConstType operator*() const;
  RefType operator*();

  bool operator==(Maybe const& rhs) const;
  bool operator!=(Maybe const& rhs) const;
  bool operator<(Maybe const& rhs) const;

  RefConstType get() const;
  RefType get();

  [[nodiscard]] std::optional<T> optional() const&;
  [[nodiscard]] std::optional<T> optional() &&;

  // Get either the contents of this Maybe or the given default.
  T value(T def = T()) const;

  // Get either this value, or if this value is none the given value.
  Maybe orMaybe(Maybe const& other) const;

  // Takes the value out of this Maybe, leaving it Nothing.
  T take();

  // If this Maybe is set, assigns it to t and leaves this Maybe as Nothing.
  bool put(T& t);

  void set(T const& t);
  void set(T&& t);

  template <typename... Args>
  void emplace(Args&&... t);

  void reset();

  // Apply a function to the contained value if it is not Nothing.
  template <typename Function>
  void exec(Function&& function);

  // Functor map operator.  If this maybe is not Nothing, then applies the
  // given function to it and returns the result, otherwise returns Nothing (of
  // the type the function would normally return).
  template <typename Function>
  auto apply(Function&& function) const -> Maybe<std::decay_t<decltype(function(std::declval<T>()))>>;

  // Monadic bind operator.  Given function should return another Maybe.
  template <typename Function>
  auto sequence(Function function) const -> decltype(function(std::declval<T>()));

private:
  std::optional<T> m_data;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, Maybe<T> const& v);

template <typename T>
struct hash<Maybe<T>> {
  size_t operator()(Maybe<T> const& m) const;
  hash<T> hasher;
};

template <typename T>
Maybe<T>::Maybe(T const& t)
    : m_data(t) {}

template <typename T>
Maybe<T>::Maybe(T&& t)
    : m_data(std::move(t)) {}

template <typename T>
Maybe<T>::Maybe(std::nullopt_t)
    : m_data(std::nullopt) {}

template <typename T>
Maybe<T>::Maybe(Maybe const& rhs)
    : m_data(rhs.m_data) {}

template <typename T>
Maybe<T>::Maybe(Maybe&& rhs) noexcept(std::is_nothrow_move_constructible_v<T>)
    : m_data(std::move(rhs.m_data)) {
  rhs.reset();
}

template <typename T>
template <typename T2>
Maybe<T>::Maybe(Maybe<T2> const& rhs)
    : m_data(rhs ? std::optional<T>(*rhs) : std::nullopt) {}

template <typename T>
Maybe<T>::~Maybe() = default;

template <typename T>
Maybe<T>& Maybe<T>::operator=(Maybe const& rhs) {
  if (&rhs == this)
    return *this;

  if (rhs)
    emplace(*rhs);
  else
    reset();

  return *this;
}

template <typename T>
template <typename T2>
Maybe<T>& Maybe<T>::operator=(Maybe<T2> const& rhs) {
  if (rhs)
    emplace(*rhs);
  else
    reset();

  return *this;
}

template <typename T>
Maybe<T>& Maybe<T>::operator=(Maybe&& rhs) noexcept(std::is_nothrow_move_constructible_v<T>) {
  if (&rhs == this)
    return *this;

  if (rhs)
    emplace(rhs.take());
  else
    reset();

  return *this;
}

template <typename T>
Maybe<T>& Maybe<T>::operator=(std::nullopt_t) {
  reset();
  return *this;
}

template <typename T>
Maybe<T> Maybe<T>::fromOptional(std::optional<T> const& t) {
  if (t)
    return Maybe(*t);
  return {};
}

template <typename T>
Maybe<T> Maybe<T>::fromOptional(std::optional<T>&& t) {
  if (t)
    return Maybe(std::move(*t));
  return {};
}

template <typename T>
bool Maybe<T>::isValid() const {
  return m_data.has_value();
}

template <typename T>
bool Maybe<T>::isNothing() const {
  return !m_data.has_value();
}

template <typename T>
Maybe<T>::operator bool() const {
  return m_data.has_value();
}

template <typename T>
auto Maybe<T>::ptr() const -> PointerConstType {
  return m_data ? &*m_data : nullptr;
}

template <typename T>
auto Maybe<T>::ptr() -> PointerType {
  return m_data ? &*m_data : nullptr;
}

template <typename T>
auto Maybe<T>::operator->() const -> PointerConstType {
  if (!m_data)
    throw InvalidMaybeAccessException();

  return ptr();
}

template <typename T>
auto Maybe<T>::operator->() -> PointerType {
  if (!m_data)
    throw InvalidMaybeAccessException();

  return ptr();
}

template <typename T>
auto Maybe<T>::operator*() const -> RefConstType {
  return get();
}

template <typename T>
auto Maybe<T>::operator*() -> RefType {
  return get();
}

template <typename T>
bool Maybe<T>::operator==(Maybe const& rhs) const {
  if (!m_data && !rhs.m_data)
    return true;
  if (m_data && rhs.m_data)
    return get() == rhs.get();
  return false;
}

template <typename T>
bool Maybe<T>::operator!=(Maybe const& rhs) const {
  return !operator==(rhs);
}

template <typename T>
bool Maybe<T>::operator<(Maybe const& rhs) const {
  if (m_data && rhs.m_data)
    return get() < rhs.get();
  if (!m_data && rhs.m_data)
    return true;
  return false;
}

template <typename T>
auto Maybe<T>::get() const -> RefConstType {
  if (!m_data)
    throw InvalidMaybeAccessException();

  return *m_data;
}

template <typename T>
auto Maybe<T>::get() -> RefType {
  if (!m_data)
    throw InvalidMaybeAccessException();

  return *m_data;
}

template <typename T>
[[nodiscard]] std::optional<T> Maybe<T>::optional() const& {
  return m_data;
}

template <typename T>
[[nodiscard]] std::optional<T> Maybe<T>::optional() && {
  if (m_data)
    return take();
  return std::nullopt;
}

template <typename T>
[[nodiscard]] T Maybe<T>::value(T def) const {
  if (m_data)
    return *m_data;
  else
    return def;
}

template <typename T>
[[nodiscard]] Maybe<T> Maybe<T>::orMaybe(Maybe const& other) const {
  if (m_data)
    return *this;
  else
    return other;
}

template <typename T>
[[nodiscard]] T Maybe<T>::take() {
  if (!m_data)
    throw InvalidMaybeAccessException();

  T val(std::move(*m_data));

  reset();

  return val;
}

template <typename T>
[[nodiscard]] bool Maybe<T>::put(T& t) {
  if (m_data) {
    t = std::move(*m_data);

    reset();

    return true;
  } else {
    return false;
  }
}

template <typename T>
void Maybe<T>::set(T const& t) {
  emplace(t);
}

template <typename T>
void Maybe<T>::set(T&& t) {
  emplace(std::forward<T>(t));
}

template <typename T>
template <typename... Args>
void Maybe<T>::emplace(Args&&... t) {
  m_data.emplace(std::forward<Args>(t)...);
}

template <typename T>
void Maybe<T>::reset() {
  m_data.reset();
}

template <typename T>
template <typename Function>
[[nodiscard]] auto Maybe<T>::apply(Function&& function) const
  -> Maybe<std::decay_t<decltype(function(std::declval<T>()))>> {
  if (!isValid())
    return {};
  return function(get());
}

template <typename T>
template <typename Function>
void Maybe<T>::exec(Function&& function) {
  if (isValid())
    function(get());
}

template <typename T>
template <typename Function>
[[nodiscard]] auto Maybe<T>::sequence(Function function) const -> decltype(function(std::declval<T>())) {
  if (!isValid())
    return {};
  return function(get());
}

template <typename T>
std::ostream& operator<<(std::ostream& os, Maybe<T> const& v) {
  if (v)
    return os << "Just (" << *v << ")";
  else
    return os << "Nothing";
}

template <typename T>
size_t hash<Maybe<T>>::operator()(Maybe<T> const& m) const {
  if (!m)
    return 0;
  else
    return hasher(*m);
}

}// namespace Star

template <typename T>
struct std::formatter<Star::Maybe<T>> : Star::OstreamFormatter {};
