#pragma once

import std;

// Defines a new type that behaves nearly identical to 'parentType', with the
// added benefit that though the new type can be implicitly converted to the
// base type, it must be explicitly converted *from* the base type, and they
// are two distinct types in the type system.

// Version of strong_typedef for builtin types.

template <typename BaseType>
class StrongTypedef {
private:
  BaseType value_;

public:
  explicit constexpr StrongTypedef(const BaseType& value = {})
      : value_(value) {}

  explicit constexpr StrongTypedef(BaseType&& value)
      : value_(std::move(value)) {}

  constexpr StrongTypedef(const StrongTypedef&) = default;
  constexpr StrongTypedef(StrongTypedef&&) = default;

  constexpr auto operator=(const StrongTypedef&) -> StrongTypedef& = default;
  constexpr auto operator=(StrongTypedef&&) -> StrongTypedef& = default;

  explicit constexpr operator BaseType() const { return value_; }

  constexpr auto get() const noexcept -> const BaseType& { return value_; }
  constexpr auto get() noexcept -> BaseType& { return value_; }

  constexpr auto operator==(const StrongTypedef& other) const
    noexcept(noexcept(std::declval<BaseType>() == std::declval<BaseType>())) -> bool {
    return value_ == other.value_;
  }

  constexpr auto operator!=(const StrongTypedef& other) const
    noexcept(noexcept(std::declval<BaseType>() != std::declval<BaseType>())) -> bool {
    return value_ != other.value_;
  }

  constexpr auto operator<=>(const StrongTypedef& other) const
    noexcept(noexcept(std::declval<BaseType>() <=> std::declval<BaseType>())) {
    return value_ <=> other.value_;
  }

  template <typename CharT, typename Traits>
  friend auto
  operator<<(std::basic_ostream<CharT, Traits>& os, const StrongTypedef& st) -> std::basic_ostream<CharT, Traits>& {
    return os << st.value_;
  }

  template <typename CharT, typename Traits>
  friend auto
  operator>>(std::basic_istream<CharT, Traits>& is, StrongTypedef& st) -> std::basic_istream<CharT, Traits>& {
    return is >> st.value_;
  }

  friend struct std::hash<StrongTypedef>;
};

template <typename BaseType>
  requires std::is_arithmetic_v<BaseType>
class StrongTypedefBuiltin : public StrongTypedef<BaseType> {
public:
  using StrongTypedef<BaseType>::StrongTypedef;

  // 算术运算符
  constexpr auto operator+() const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(+this->get());
  }

  constexpr auto operator-() const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(-this->get());
  }

  constexpr auto operator+(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() + other.get());
  }

  constexpr auto operator-(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() - other.get());
  }

  constexpr auto operator*(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() * other.get());
  }

  constexpr auto operator/(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() / other.get());
  }

  constexpr auto operator%(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() % other.get());
  }

  // 复合赋值运算符
  constexpr auto operator+=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() += other.get();
    return *this;
  }

  constexpr auto operator-=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() -= other.get();
    return *this;
  }

  constexpr auto operator*=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() *= other.get();
    return *this;
  }

  constexpr auto operator/=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() /= other.get();
    return *this;
  }

  constexpr auto operator%=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() %= other.get();
    return *this;
  }

  // 自增/自减
  constexpr auto operator++() -> StrongTypedefBuiltin& {
    ++this->get();
    return *this;
  }

  constexpr auto operator++(int) -> StrongTypedefBuiltin {
    StrongTypedefBuiltin temp = *this;
    ++this->get();
    return temp;
  }

  constexpr auto operator--() -> StrongTypedefBuiltin& {
    --this->get();
    return *this;
  }

  constexpr auto operator--(int) -> StrongTypedefBuiltin {
    StrongTypedefBuiltin temp = *this;
    --this->get();
    return temp;
  }

  // 位运算符（仅整数类型）
  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator~() const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(~this->get());
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator&(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() & other.get());
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator|(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() | other.get());
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator^(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() ^ other.get());
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator<<(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() << other.get());
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator>>(const StrongTypedefBuiltin& other) const -> StrongTypedefBuiltin {
    return StrongTypedefBuiltin(this->get() >> other.get());
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator&=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() &= other.get();
    return *this;
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator|=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() |= other.get();
    return *this;
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator^=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() ^= other.get();
    return *this;
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator<<=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() <<= other.get();
    return *this;
  }

  template <typename T = BaseType>
    requires std::is_integral_v<T>
  constexpr auto operator>>=(const StrongTypedefBuiltin& other) -> StrongTypedefBuiltin& {
    this->get() >>= other.get();
    return *this;
  }
};

template <typename BaseType>
struct std::hash<StrongTypedef<BaseType>> {
  auto operator()(const StrongTypedef<BaseType>& st) const noexcept -> size_t {
    return std::hash<BaseType>{}(st.get());
  }
};

template <typename BaseType>
  requires std::is_arithmetic_v<BaseType>
struct std::hash<StrongTypedefBuiltin<BaseType>> {
  auto operator()(const StrongTypedefBuiltin<BaseType>& st) const noexcept -> size_t {
    return std::hash<BaseType>{}(st.get());
  }
};
