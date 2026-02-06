#pragma once

#include "StarException.hpp"

import std;

namespace Star {

using PointerConvertException = ExceptionDerived<"PointerConvertException">;

template <typename Type1, typename Type2>
auto is(Type2* p) -> bool {
  return (bool)dynamic_cast<Type1*>(p);
}

template <typename Type1, typename Type2>
auto is(Type2 const* p) -> bool {
  return (bool)dynamic_cast<Type1 const*>(p);
}

template <typename Type1, typename Type2>
auto is(std::shared_ptr<Type2> const& p) -> bool {
  return (bool)dynamic_cast<Type1*>(p.get());
}

template <typename Type1, typename Type2>
auto is(std::shared_ptr<Type2 const> const& p) -> bool {
  return (bool)dynamic_cast<Type1 const*>(p.get());
}

template <typename Type1, typename Type2>
auto ris(Type2& r) -> bool {
  return (bool)dynamic_cast<Type1*>(&r);
}

template <typename Type1, typename Type2>
auto ris(Type2 const& r) -> bool {
  return (bool)dynamic_cast<Type1 const*>(&r);
}

template <typename Type1, typename Type2>
auto as(Type2* p) -> Type1* {
  return dynamic_cast<Type1*>(p);
}

template <typename Type1, typename Type2>
auto as(Type2 const* p) -> Type1 const* {
  return dynamic_cast<Type1 const*>(p);
}

template <typename Type1, typename Type2>
auto as(std::shared_ptr<Type2> const& p) -> std::shared_ptr<Type1> {
  return dynamic_pointer_cast<Type1>(p);
}

template <typename Type1, typename Type2>
auto as(std::shared_ptr<Type2 const> const& p) -> std::shared_ptr<Type1 const> {
  return dynamic_pointer_cast<Type1 const>(p);
}

template <typename Type, typename Ptr>
auto convert(Ptr const& p) -> decltype(as<Type>(p)) {
  if (!p)
    throw PointerConvertException::format("Could not convert from nullptr to {}", typeid(Type).name());
  else if (auto a = as<Type>(p))
    return a;
  else
    throw PointerConvertException::format("Could not convert from {} to {}", typeid(*p).name(), typeid(Type).name());
}

template <typename Type1, typename Type2>
auto rconvert(Type2& r) -> Type1& {
  return *dynamic_cast<Type1*>(&r);
}

template <typename Type1, typename Type2>
auto rconvert(Type2 const& r) -> Type1 const& {
  return *dynamic_cast<Type1 const*>(&r);
}

template <typename Type>
auto asWeak(std::shared_ptr<Type> const& p) -> std::weak_ptr<Type> {
  return weak_ptr<Type>(p);
}

template <typename Type>
auto asWeak(std::shared_ptr<Type const> const& p) -> std::weak_ptr<Type const> {
  return weak_ptr<Type>(p);
}

}
