#pragma once

import std;

template <typename Type, typename Tag>
struct StrongTypedefBuiltin {
  Type t;

  explicit StrongTypedefBuiltin(const Type t_)
    : t(t_){}

  StrongTypedefBuiltin()
    : t(Type()) {}

  StrongTypedefBuiltin(const StrongTypedefBuiltin& t_)
    : t(t_.t) {}

  StrongTypedefBuiltin& operator=(const StrongTypedefBuiltin& rhs) {
    t = rhs.t;
    return *this;
  }

  StrongTypedefBuiltin& operator=(Type const& rhs) {
    t = rhs;
    return *this;
  }

  operator const Type&() const {
    return t;
  }

  operator Type&() {
    return t;
  }

  bool operator==(StrongTypedefBuiltin const& rhs) const {
    return t == rhs.t;
  }

  std::weak_ordering operator<=>(StrongTypedefBuiltin const& rhs) const {
    if (t < rhs.t)
      return std::weak_ordering::less;
    if (rhs.t < t)
      return std::weak_ordering::greater;
    return std::weak_ordering::equivalent;
  }
};
