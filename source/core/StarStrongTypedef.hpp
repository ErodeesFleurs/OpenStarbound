#pragma once

#include <compare>
#include <type_traits>

// Defines a new type that behaves nearly identical to 'parentType', with the
// added benefit that though the new type can be implicitly converted to the
// base type, it must be explicitly converted *from* the base type, and they
// are two distinct types in the type system.
#define strong_typedef(ParentType, NewType)                                                                         \
  template <typename BaseType>                                                                                      \
  struct NewType##Wrapper : BaseType {                                                                              \
    using BaseType::BaseType;                                                                                       \
                                                                                                                    \
    NewType##Wrapper() : BaseType() {}                                                                              \
                                                                                                                    \
    NewType##Wrapper(NewType##Wrapper const& nt) : BaseType(nt) {}                                                  \
                                                                                                                    \
    NewType##Wrapper(NewType##Wrapper&& nt) : BaseType(std::move(nt)) {}                                            \
                                                                                                                    \
    explicit NewType##Wrapper(BaseType const& bt) : BaseType(bt) {}                                                 \
                                                                                                                    \
    explicit NewType##Wrapper(BaseType&& bt) : BaseType(std::move(bt)) {}                                           \
                                                                                                                    \
    NewType##Wrapper& operator=(NewType##Wrapper const& rhs) {                                                      \
      BaseType::operator=(rhs);                                                                                     \
      return *this;                                                                                                 \
    }                                                                                                               \
                                                                                                                    \
    NewType##Wrapper& operator=(NewType##Wrapper&& rhs) {                                                           \
      BaseType::operator=(std::move(rhs));                                                                          \
      return *this;                                                                                                 \
    }                                                                                                               \
                                                                                                                    \
    template <class Arg>                                                                                            \
    NewType##Wrapper& operator=(Arg&& other) {                                                                      \
      static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false                       \
              || std::is_same<NewType##Wrapper, typename std::decay<Arg>::type>::value,                             \
          "" #NewType " can not implicitly be assigned from " #ParentType "-derived classes or strong " #ParentType \
          " typedefs");                                                                                             \
                                                                                                                    \
      BaseType::operator=(std::forward<Arg>(other));                                                                \
      return *this;                                                                                                 \
    }                                                                                                               \
  };                                                                                                                \
  typedef NewType##Wrapper<ParentType> NewType

// strong_typedef for builtin types, as a real class template rather than a macro
// so that a module can export it.  The tag type keeps two typedefs over the same
// underlying type distinct, which is what a separate struct per name used to do.
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
