#pragma once

#include <type_traits>
#include <utility>

namespace Star {

// Behaves nearly identical to BaseType, but remains a distinct type. Values
// implicitly convert to BaseType through inheritance, while construction from
// BaseType itself remains explicit.
template <typename BaseType, typename Tag>
struct StrongTypedef : BaseType {
  using BaseType::BaseType;

  StrongTypedef()
    : BaseType() {}

  StrongTypedef(StrongTypedef const& nt)
    : BaseType(nt) {}

  StrongTypedef(StrongTypedef&& nt)
    : BaseType(std::move(nt)) {}

  explicit StrongTypedef(BaseType const& bt)
    : BaseType(bt) {}

  explicit StrongTypedef(BaseType&& bt)
    : BaseType(std::move(bt)) {}

  StrongTypedef& operator=(StrongTypedef const& rhs) {
    BaseType::operator=(rhs);
    return *this;
  }

  StrongTypedef& operator=(StrongTypedef&& rhs) {
    BaseType::operator=(std::move(rhs));
    return *this;
  }

  template <class Arg>
    requires (!std::is_base_of_v<BaseType, std::decay_t<Arg>> || std::is_same_v<StrongTypedef, std::decay_t<Arg>>)
  StrongTypedef& operator=(Arg&& other) {
    BaseType::operator=(std::forward<Arg>(other));
    return *this;
  }
};

// Strong typedef wrapper for builtin types.
template <typename Type, typename Tag>
struct StrongTypedefBuiltin {
  Type t;

  explicit StrongTypedefBuiltin(Type const t_)
    : t(t_) {}

  StrongTypedefBuiltin()
    : t(Type()) {}

  StrongTypedefBuiltin(StrongTypedefBuiltin const& t_)
    : t(t_.t) {}

  StrongTypedefBuiltin& operator=(StrongTypedefBuiltin const& rhs) {
    t = rhs.t;
    return *this;
  }

  StrongTypedefBuiltin& operator=(Type const& rhs) {
    t = rhs;
    return *this;
  }

  operator Type const&() const {
    return t;
  }

  operator Type&() {
    return t;
  }

  bool operator==(StrongTypedefBuiltin const& rhs) const {
    return t == rhs.t;
  }

  bool operator!=(StrongTypedefBuiltin const& rhs) const {
    return t != rhs.t;
  }

  bool operator<(StrongTypedefBuiltin const& rhs) const {
    return t < rhs.t;
  }

  bool operator>(StrongTypedefBuiltin const& rhs) const {
    return t > rhs.t;
  }

  bool operator<=(StrongTypedefBuiltin const& rhs) const {
    return t <= rhs.t;
  }

  bool operator>=(StrongTypedefBuiltin const& rhs) const {
    return t >= rhs.t;
  }
};

}
