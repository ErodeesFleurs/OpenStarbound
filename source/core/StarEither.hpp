#pragma once

#include "StarException.hpp"
#include "StarVariant.hpp"

import std;

namespace Star {

using EitherException = ExceptionDerived<"EitherException">;

template <typename Value>
struct EitherLeftValue {
  Value value;
};

template <typename Value>
struct EitherRightValue {
  Value value;
};

template <typename Value>
auto makeLeft(Value value) -> EitherLeftValue<Value>;

template <typename Value>
auto makeRight(Value value) -> EitherRightValue<Value>;

// Container that contains exactly one of either Left or Right.
template <typename Left, typename Right>
class Either {
public:
  // Constructs Either that contains a default constructed Left value
  Either();

  Either(EitherLeftValue<Left> left);
  Either(EitherRightValue<Right> right);

  template <typename T>
  Either(EitherLeftValue<T> left);

  template <typename T>
  Either(EitherRightValue<T> right);

  Either(Either const& rhs);
  Either(Either&& rhs);

  auto operator=(Either const& rhs) -> Either&;
  auto operator=(Either&& rhs) -> Either&;

  template <typename T>
  auto operator=(EitherLeftValue<T> left) -> Either&;

  template <typename T>
  auto operator=(EitherRightValue<T> right) -> Either&;

  [[nodiscard]] auto isLeft() const -> bool;
  [[nodiscard]] auto isRight() const -> bool;

  void setLeft(Left left);
  void setRight(Right left);

  // left() and right() throw EitherException on invalid access

  auto left() const -> Left const&;
  auto right() const -> Right const&;

  auto left() -> Left&;
  auto right() -> Right&;

  auto maybeLeft() const -> std::optional<Left>;
  auto maybeRight() const -> std::optional<Right>;

  // leftPtr() and rightPtr() do not throw on invalid access

  auto leftPtr() const -> Left const*;
  auto rightPtr() const -> Right const*;

  auto leftPtr() -> Left*;
  auto rightPtr() -> Right*;

private:
  using LeftType = EitherLeftValue<Left>;
  using RightType = EitherRightValue<Right>;

  Variant<LeftType, RightType> m_value;
};

template <typename Value>
auto makeLeft(Value value) -> EitherLeftValue<Value> {
  return {std::move(value)};
}

template <typename Value>
auto makeRight(Value value) -> EitherRightValue<Value> {
  return {std::move(value)};
}

template <typename Left, typename Right>
Either<Left, Right>::Either() = default;

template <typename Left, typename Right>
Either<Left, Right>::Either(EitherLeftValue<Left> left)
  : m_value(std::move(left)) {}

template <typename Left, typename Right>
Either<Left, Right>::Either(EitherRightValue<Right> right)
  : m_value(std::move(right)) {}

template <typename Left, typename Right>
template <typename T>
Either<Left, Right>::Either(EitherLeftValue<T> left)
  : Either(LeftType{std::move(left.value)}) {}

template <typename Left, typename Right>
template <typename T>
Either<Left, Right>::Either(EitherRightValue<T> right)
  : Either(RightType{std::move(right.value)}) {}

template <typename Left, typename Right>
Either<Left, Right>::Either(Either const& rhs)
  : m_value(rhs.m_value) {}

template <typename Left, typename Right>
Either<Left, Right>::Either(Either&& rhs)
  : m_value(std::move(rhs.m_value)) {}

template <typename Left, typename Right>
auto Either<Left, Right>::operator=(Either const& rhs) -> Either<Left, Right>& = default;

template <typename Left, typename Right>
auto Either<Left, Right>::operator=(Either&& rhs) -> Either<Left, Right>& {
  m_value = std::move(rhs.m_value);
  return *this;
}

template <typename Left, typename Right>
template <typename T>
auto Either<Left, Right>::operator=(EitherLeftValue<T> left) -> Either<Left, Right>& {
  m_value = LeftType{std::move(left.value)};
  return *this;
}

template <typename Left, typename Right>
template <typename T>
auto Either<Left, Right>::operator=(EitherRightValue<T> right) -> Either<Left, Right>& {
  m_value = RightType{std::move(right.value)};
  return *this;
}

template <typename Left, typename Right>
auto Either<Left, Right>::isLeft() const -> bool {
  return m_value.template is<LeftType>();
}

template <typename Left, typename Right>
auto Either<Left, Right>::isRight() const -> bool {
  return m_value.template is<RightType>();
}

template <typename Left, typename Right>
void Either<Left, Right>::setLeft(Left left) {
  m_value = LeftType{std::move(left)};
}

template <typename Left, typename Right>
void Either<Left, Right>::setRight(Right right) {
  m_value = RightType{std::move(right)};
}

template <typename Left, typename Right>
auto Either<Left, Right>::left() const -> Left const& {
  if (auto l = leftPtr())
    return *l;
  throw EitherException("Improper access of left side of Either");
}

template <typename Left, typename Right>
auto Either<Left, Right>::right() const -> Right const& {
  if (auto r = rightPtr())
    return *r;
  throw EitherException("Improper access of right side of Either");
}

template <typename Left, typename Right>
auto Either<Left, Right>::left() -> Left& {
  if (auto l = leftPtr())
    return *l;
  throw EitherException("Improper access of left side of Either");
}

template <typename Left, typename Right>
auto Either<Left, Right>::right() -> Right& {
  if (auto r = rightPtr())
    return *r;
  throw EitherException("Improper access of right side of Either");
}

template <typename Left, typename Right>
auto Either<Left, Right>::maybeLeft() const -> std::optional<Left> {
  if (auto l = leftPtr())
    return *l;
  return std::nullopt;
}

template <typename Left, typename Right>
auto Either<Left, Right>::maybeRight() const -> std::optional<Right> {
  if (auto r = rightPtr())
    return *r;
  return std::nullopt;
}

template <typename Left, typename Right>
auto Either<Left, Right>::leftPtr() const -> Left const* {
  if (auto l = m_value.template ptr<LeftType>())
    return &l->value;
  return nullptr;
}

template <typename Left, typename Right>
auto Either<Left, Right>::rightPtr() const -> Right const* {
  if (auto r = m_value.template ptr<RightType>())
    return &r->value;
  return nullptr;
}

template <typename Left, typename Right>
auto Either<Left, Right>::leftPtr() -> Left* {
  if (auto l = m_value.template ptr<LeftType>())
    return &l->value;
  return nullptr;
}

template <typename Left, typename Right>
auto Either<Left, Right>::rightPtr() -> Right* {
  if (auto r = m_value.template ptr<RightType>())
    return &r->value;
  return nullptr;
}

}
