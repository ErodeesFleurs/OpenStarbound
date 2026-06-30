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
  Maybe(T const& t) : m_data(t) {}
  Maybe(T&& t) : m_data(std::move(t)) {}
  Maybe(std::nullopt_t) : m_data(std::nullopt) {}

  Maybe(Maybe const&) = default;
  Maybe(Maybe&& rhs) noexcept(std::is_nothrow_move_constructible_v<T>) : m_data(std::move(rhs.m_data)) {}
  template <typename T2>
  Maybe(Maybe<T2> const& rhs) : m_data(rhs ? std::optional<T>(*rhs) : std::nullopt) {}

  ~Maybe() = default;

  Maybe& operator=(Maybe const&) = default;
  Maybe& operator=(Maybe&& rhs) noexcept(std::is_nothrow_move_constructible_v<T>) {
    if (this != &rhs) {
      if (rhs)
        emplace(rhs.take());
      else
        reset();
    }
    return *this;
  }
  Maybe& operator=(std::nullopt_t) {
    m_data.reset();
    return *this;
  }
  template <typename T2>
  Maybe& operator=(Maybe<T2> const& rhs) {
    if (rhs)
      m_data.emplace(*rhs);
    else
      m_data.reset();
    return *this;
  }

  [[nodiscard]] static Maybe fromOptional(std::optional<T> const& t) { return t ? Maybe(*t) : Maybe(); }
  [[nodiscard]] static Maybe fromOptional(std::optional<T>&& t) { return t ? Maybe(std::move(*t)) : Maybe(); }

  [[nodiscard]] bool isValid() const { return m_data.has_value(); }
  [[nodiscard]] bool isNothing() const { return !m_data.has_value(); }
  [[nodiscard]] explicit operator bool() const { return m_data.has_value(); }

  [[nodiscard]] PointerConstType ptr() const { return m_data ? &*m_data : nullptr; }
  [[nodiscard]] PointerType ptr() { return m_data ? &*m_data : nullptr; }

  [[nodiscard]] PointerConstType operator->() const { return &m_data.value(); }
  [[nodiscard]] PointerType operator->() { return &m_data.value(); }

  [[nodiscard]] RefConstType operator*() const { return m_data.value(); }
  [[nodiscard]] RefType operator*() { return m_data.value(); }

  bool operator==(Maybe const& rhs) const { return m_data == rhs.m_data; }
  bool operator!=(Maybe const& rhs) const { return m_data != rhs.m_data; }
  [[nodiscard]] bool operator<(Maybe const& rhs) const { return m_data < rhs.m_data; }

  [[nodiscard]] RefConstType get() const { return m_data.value(); }
  [[nodiscard]] RefType get() { return m_data.value(); }

  [[nodiscard]] std::optional<T> optional() const& { return m_data; }
  [[nodiscard]] std::optional<T> optional() && { auto tmp = std::move(m_data); m_data.reset(); return tmp; }

  [[nodiscard]] T value(T def = T()) const { return m_data.value_or(std::move(def)); }
  [[nodiscard]] Maybe orMaybe(Maybe const& other) const { return m_data ? *this : other; }

  [[nodiscard]] T take() {
    if (!m_data)
      throw InvalidMaybeAccessException();
    T val(std::move(*m_data));
    m_data.reset();
    return val;
  }

  [[nodiscard]] bool put(T& t) {
    if (m_data) {
      t = std::move(*m_data);
      m_data.reset();
      return true;
    }
    return false;
  }

  void set(T const& t) { m_data.emplace(t); }
  void set(T&& t) { m_data.emplace(std::move(t)); }
  template <typename... Args>
  void emplace(Args&&... t) { m_data.emplace(std::forward<Args>(t)...); }
  void reset() { m_data.reset(); }

  template <typename Function>
  void exec(Function&& function) { if (m_data) function(*m_data); }

  template <typename Function>
  [[nodiscard]] auto apply(Function&& function) const -> Maybe<std::decay_t<decltype(function(std::declval<T>()))>> {
    if (m_data) return function(*m_data);
    return {};
  }

  template <typename Function>
  [[nodiscard]] auto sequence(Function function) const -> decltype(function(std::declval<T>())) {
    if (m_data) return function(*m_data);
    return {};
  }

private:
  std::optional<T> m_data;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, Maybe<T> const& v);

template <typename T>
struct hash<Maybe<T>> {
  [[nodiscard]] size_t operator()(Maybe<T> const& m) const;
  hash<T> hasher;
};

// --- out-of-line implementations (only non-trivial ones) ---

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

} // namespace Star

template <typename T>
struct std::formatter<Star::Maybe<T>> : Star::OstreamFormatter {};
