#pragma once

#include "StarException.hpp"

import std;

namespace Star {

using BadVariantCast = ExceptionDerived<"BadVariantCast">;
using BadVariantType = ExceptionDerived<"BadVariantType">;

std::uint8_t const InvalidVariantType = 255;

namespace detail {
  template <typename T, typename... Args>
  struct HasType;

  template <typename T>
  struct HasType<T> : std::false_type {};

  template <typename T, typename Head, typename... Args>
  struct HasType<T, Head, Args...> {
    static constexpr bool value = std::is_same_v<T, Head> || HasType<T, Args...>::value;
  };

  template <typename... Args>
  struct IsNothrowMoveConstructible;

  template <>
  struct IsNothrowMoveConstructible<> : std::true_type {};

  template <typename Head, typename... Args>
  struct IsNothrowMoveConstructible<Head, Args...> {
    static constexpr bool value = std::is_nothrow_move_constructible_v<Head> && IsNothrowMoveConstructible<Args...>::value;
  };

  template <typename... Args>
  struct IsNothrowMoveAssignable;

  template <>
  struct IsNothrowMoveAssignable<> : std::true_type {};

  template <typename Head, typename... Args>
  struct IsNothrowMoveAssignable<Head, Args...> {
    static constexpr bool value = std::is_nothrow_move_assignable_v<Head> && IsNothrowMoveAssignable<Args...>::value;
  };
}

// Stack based variant type container that can be inhabited by one of a limited
// number of types.
template <typename FirstType, typename... RestTypes>
class Variant {
public:
  template <typename T>
  using ValidateType =  std::enable_if_t<detail::HasType<T, FirstType, RestTypes...>::value, void>;

  template <typename T, typename = ValidateType<T>>
  static constexpr auto typeIndexOf() -> std::uint8_t;

  // If the first type has a default constructor, constructs an Variant which
  // contains a default constructed value of that type.
  Variant();

  template <typename T, typename = ValidateType<T>>
  Variant(T const& x);
  template <typename T, typename = ValidateType<T>>
  Variant(T&& x);

  template <typename T, typename = ValidateType<T>, typename... Args>
  Variant(std::in_place_type_t<T>, Args&&... args) requires std::is_constructible_v<T, Args...> {
    new (&m_buffer) T(std::forward<Args>(args)...);
    m_typeIndex = TypeIndex<T>::value;
  }

  template <typename T, typename U, typename = ValidateType<T>, typename... Args>
  Variant(std::in_place_type_t<T>, std::initializer_list<U> il, Args&&... args) requires std::is_constructible_v<T, std::initializer_list<U>&, Args...> {
    new (&m_buffer) T(il, std::forward<Args>(args)...);
    m_typeIndex = TypeIndex<T>::value;
  }

  Variant(Variant const& x);
  Variant(Variant&& x) noexcept(detail::IsNothrowMoveConstructible<FirstType, RestTypes...>::value);

  ~Variant();

  // Implementations of operator= may invalidate the Variant if the copy or
  // move constructor of the assigned value throws.
  auto operator=(Variant const& x) -> Variant&;
  auto operator=(Variant&& x) noexcept(detail::IsNothrowMoveAssignable<FirstType, RestTypes...>::value) -> Variant&;
  template <typename T, typename = ValidateType<T>>
  auto operator=(T const& x) -> Variant&;
  template <typename T, typename = ValidateType<T>>
  auto operator=(T&& x) -> Variant&;

  // Returns true if this Variant contains the given type.
  template <typename T, typename = ValidateType<T>>
  [[nodiscard]] auto is() const -> bool;

  // get throws BadVariantCast on bad casts

  template <typename T, typename = ValidateType<T>>
  auto get() const -> T const&;

  template <typename T, typename = ValidateType<T>>
  auto get() -> T&;

  template <typename T, typename = ValidateType<T>>
  auto maybe() const -> std::optional<T>;

  // ptr() does not throw if this Variant does not hold the given type, instead
  // simply returns nullptr.

  template <typename T, typename = ValidateType<T>>
  auto ptr() const -> T const*;

  template <typename T, typename = ValidateType<T>>
  auto ptr() -> T*;

  // Calls the given function with the type currently being held, and returns
  // the value returned by that function.  Will throw if this Variant has been
  // invalidated.
  template <typename Function>
  auto call(Function&& function) -> decltype(auto);
  template <typename Function>
  auto call(Function&& function) const -> decltype(auto);

  // Returns an index for the held type, which can be passed into makeType to
  // make this Variant hold a specific type.  Returns InvalidVariantType if
  // invalidated.
  [[nodiscard]] auto typeIndex() const -> std::uint8_t;

  // Make this Variant hold a new default constructed type of the given type
  // index.  Can only be used if every alternative type has a default
  // constructor.  Throws if given an out of range type index or
  // InvalidVariantType.
  void makeType(std::uint8_t typeIndex);

  // True if this Variant has been invalidated.  If the copy or move
  // constructor of a type throws an exception during assignment, there is no
  // *good* way to ensure that the Variant has a valid type, so it may become
  // invalidated.  It is not possible to directly construct an invalidated
  // Variant.
  [[nodiscard]] auto invalid() const -> bool;

  // Requires that every type included in this Variant has operator==
  auto operator==(Variant const& x) const -> bool;
  auto operator!=(Variant const& x) const -> bool;

  // Requires that every type included in this Variant has operator<
  auto operator<(Variant const& x) const -> bool;

  template <typename T, typename = ValidateType<T>>
  auto operator==(T const& x) const -> bool;
  template <typename T, typename = ValidateType<T>>
  auto operator!=(T const& x) const -> bool;
  template <typename T, typename = ValidateType<T>>
  auto operator<(T const& x) const -> bool;

private:
  template <typename MatchType, std::uint8_t Index, typename... Rest>
  struct LookupTypeIndex;

  template <typename MatchType, std::uint8_t Index>
  struct LookupTypeIndex<MatchType, Index> {
    static std::uint8_t const value = InvalidVariantType;
  };

  template <typename MatchType, std::uint8_t Index, typename Head, typename... Rest>
  struct LookupTypeIndex<MatchType, Index, Head, Rest...> {
    static std::uint8_t const value = std::is_same_v<MatchType, Head> ? Index : LookupTypeIndex<MatchType, Index + 1, Rest...>::value;
  };

  template <typename MatchType>
  struct TypeIndex {
    static std::uint8_t const value = LookupTypeIndex<MatchType, 0, FirstType, RestTypes...>::value;
  };

  void destruct();

  template <typename T>
  void assign(T&& x);

  template <typename Function, typename T>
  auto doCall(Function&& function) -> decltype(auto);
  template <typename Function, typename T1, typename T2, typename... TL>
  auto doCall(Function&& function) -> decltype(auto);

  template <typename Function, typename T>
  auto doCall(Function&& function) const -> decltype(auto);
  template <typename Function, typename T1, typename T2, typename... TL>
  auto doCall(Function&& function) const -> decltype(auto);

  template <typename First>
  void doMakeType(std::uint8_t);
  template <typename First, typename Second, typename... Rest>
  void doMakeType(std::uint8_t typeIndex);

  static constexpr std::size_t BufferAlignment = std::max({alignof(FirstType), alignof(RestTypes)...});
  static constexpr std::size_t BufferSize = std::max({sizeof(FirstType), sizeof(RestTypes)...});

  alignas(BufferAlignment) std::array<std::byte, BufferSize> m_buffer;
  std::uint8_t m_typeIndex = InvalidVariantType;
};

// A version of Variant that has always has a default "empty" state, useful
// when there is no good default type for a Variant but it needs to be default
// constructed, and is slightly more convenient than Maybe<Variant<Types...>>.
template <typename... Types>
class MVariant {
public:
  template <typename T>
  using ValidateType =  std::enable_if_t<detail::HasType<T, Types...>::value, void>;

  template <typename T, typename = ValidateType<T>>
  static constexpr auto typeIndexOf() -> std::uint8_t;

  MVariant();
  MVariant(MVariant const& x);
  MVariant(MVariant&& x);

  template <typename T, typename = ValidateType<T>>
  MVariant(T const& x);
  template <typename T, typename = ValidateType<T>>
  MVariant(T&& x);

  template <typename T, typename = ValidateType<T>, typename... Args>
  MVariant(std::in_place_type_t<T>, Args&&... args)
    requires std::is_constructible_v<T, Args...> : m_variant(std::in_place_type<T>, std::forward<Args>(args)...) {}

  template <typename T, typename U, typename = ValidateType<T>, typename... Args>
  MVariant(std::in_place_type_t<T>, std::initializer_list<U> il, Args&&... args)
      requires std::is_constructible_v<T, std::initializer_list<U>&, Args...> : m_variant(std::in_place_type<T>, il, std::forward<Args>(args)...) {}

  MVariant(Variant<Types...> const& x);
  MVariant(Variant<Types...>&& x);

  ~MVariant();

  // MVariant::operator= will never invalidate the MVariant, instead it will
  // just become empty.
  auto operator=(MVariant const& x) -> MVariant&;
  auto operator=(MVariant&& x) -> MVariant&;

  template <typename T, typename = ValidateType<T>>
  auto operator=(T const& x) -> MVariant&;
  template <typename T, typename = ValidateType<T>>
  auto operator=(T&& x) -> MVariant&;

  auto operator=(Variant<Types...> const& x) -> MVariant&;
  auto operator=(Variant<Types...>&& x) -> MVariant&;

  // Requires that every type included in this MVariant has operator==
  auto operator==(MVariant const& x) const -> bool;
  auto operator!=(MVariant const& x) const -> bool;

  // Requires that every type included in this MVariant has operator<
  auto operator<(MVariant const& x) const -> bool;

  template <typename T, typename = ValidateType<T>>
  auto operator==(T const& x) const -> bool;
  template <typename T, typename = ValidateType<T>>
  auto operator!=(T const& x) const -> bool;
  template <typename T, typename = ValidateType<T>>
  auto operator<(T const& x) const -> bool;

  // get throws BadVariantCast on bad casts

  template <typename T, typename = ValidateType<T>>
  auto get() const -> T const&;

  template <typename T, typename = ValidateType<T>>
  auto get() -> T&;

  // maybe() and ptr() do not throw if this MVariant does not hold the given
  // type, instead simply returns Nothing / nullptr.

  template <typename T, typename = ValidateType<T>>
  auto maybe() const -> std::optional<T>;

  template <typename T, typename = ValidateType<T>>
  auto ptr() const -> T const*;

  template <typename T, typename = ValidateType<T>>
  auto ptr() -> T*;

  template <typename T, typename = ValidateType<T>>
  [[nodiscard]] auto is() const -> bool;

  // Takes the given value out and leaves this empty
  template <typename T, typename = ValidateType<T>>
  auto take() -> T;

  // Returns a Variant of all the allowed types if non-empty, throws
  // BadVariantCast if empty.
  auto value() const -> Variant<Types...>;

  // Moves the contents of this MVariant into the given Variant if non-empty,
  // throws BadVariantCast if empty.
  auto takeValue() -> Variant<Types...>;

  [[nodiscard]] auto empty() const -> bool;
  void reset();

  // Equivalent to !empty()
  explicit operator bool() const;

  // If this MVariant holds a type, calls the given function with the type
  // being held.  If nothing is currently held, the function is not called.
  template <typename Function>
  void call(Function&& function);

  template <typename Function>
  void call(Function&& function) const;

  // Returns an index for the held type, which can be passed into makeType to
  // make this MVariant hold a specific type.  Types are always indexed in the
  // order they are specified starting from 1.  A type index of 0 indicates an
  // empty MVariant.
  [[nodiscard]] auto typeIndex() const -> std::uint8_t;

  // Make this MVariant hold a new default constructed type of the given type
  // index.  Can only be used if every alternative type has a default
  // constructor.
  void makeType(std::uint8_t typeIndex);

private:
  struct MVariantEmpty {
    auto operator==(MVariantEmpty const& rhs) const -> bool;
    auto operator<(MVariantEmpty const& rhs) const -> bool;
  };

  template <typename Function>
  struct RefCaller {
    Function&& function;

    RefCaller(Function&& function);

    void operator()(MVariantEmpty& empty);

    template <typename T>
    void operator()(T& t);
  };

  template <typename Function>
  struct ConstRefCaller {
    Function&& function;

    ConstRefCaller(Function&& function);

    void operator()(MVariantEmpty const& empty);

    template <typename T>
    void operator()(T const& t);
  };

  Variant<MVariantEmpty, Types...> m_variant;
};

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
constexpr auto Variant<FirstType, RestTypes...>::typeIndexOf() -> std::uint8_t {
  return TypeIndex<T>::value;
}

template <typename FirstType, typename... RestTypes>
Variant<FirstType, RestTypes...>::Variant()
  : Variant(FirstType()) {}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
Variant<FirstType, RestTypes...>::Variant(T const& x) {
  assign(x);
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
Variant<FirstType, RestTypes...>::Variant(T&& x) {
  assign(std::forward<T>(x));
}

template <typename FirstType, typename... RestTypes>
Variant<FirstType, RestTypes...>::Variant(Variant const& x) {
  x.call([&](auto const& t) -> auto {
      assign(t);
    });
}

template <typename FirstType, typename... RestTypes>
Variant<FirstType, RestTypes...>::Variant(Variant&& x)
  noexcept(detail::IsNothrowMoveConstructible<FirstType, RestTypes...>::value) {
  x.call([&](auto& t) -> auto {
      assign(std::move(t));
    });
}

template <typename FirstType, typename... RestTypes>
Variant<FirstType, RestTypes...>::~Variant() {
  destruct();
}

template <typename FirstType, typename... RestTypes>
auto Variant<FirstType, RestTypes...>::operator=(Variant const& x) -> Variant<FirstType, RestTypes...>& {
  if (&x == this)
    return *this;

  x.call([&](auto const& t) -> auto {
      assign(t);
    });

  return *this;
}

template <typename FirstType, typename... RestTypes>
auto Variant<FirstType, RestTypes...>::operator=(Variant&& x)
  noexcept(detail::IsNothrowMoveAssignable<FirstType, RestTypes...>::value) -> Variant<FirstType, RestTypes...>& {
  if (&x == this)
    return *this;

  x.call([&](auto& t) -> auto {
      assign(std::move(t));
    });

  return *this;
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::operator=(T const& x) -> Variant<FirstType, RestTypes...>& {
  assign(x);
  return *this;
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::operator=(T&& x) -> Variant<FirstType, RestTypes...>& {
  assign(std::forward<T>(x));
  return *this;
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::get() const -> T const& {
  if (!is<T>())
    throw BadVariantCast();
  return *(T*)(&m_buffer);
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::get() -> T& {
  if (!is<T>())
    throw BadVariantCast();
  return *(T*)(&m_buffer);
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::maybe() const -> std::optional<T> {
  if (is<T>())
    return get<T>();
  return std::nullopt;
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::ptr() const -> T const* {
  if (!is<T>())
    return nullptr;
  return (T*)(&m_buffer);
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::ptr() -> T* {
  if (!is<T>())
    return nullptr;
  return (T*)(&m_buffer);
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::is() const -> bool {
  return m_typeIndex == TypeIndex<T>::value;
}

template <typename FirstType, typename... RestTypes>
template <typename Function>
auto Variant<FirstType, RestTypes...>::call(Function&& function) -> decltype(auto) {
  return doCall<Function, FirstType, RestTypes...>(std::forward<Function>(function));
}

template <typename FirstType, typename... RestTypes>
template <typename Function>
auto Variant<FirstType, RestTypes...>::call(Function&& function) const -> decltype(auto) {
  return doCall<Function, FirstType, RestTypes...>(std::forward<Function>(function));
}

template <typename FirstType, typename... RestTypes>
auto Variant<FirstType, RestTypes...>::typeIndex() const -> std::uint8_t {
  return m_typeIndex;
}

template <typename FirstType, typename... RestTypes>
void Variant<FirstType, RestTypes...>::makeType(std::uint8_t typeIndex) {
  return doMakeType<FirstType, RestTypes...>(typeIndex);
}

template <typename FirstType, typename... RestTypes>
auto Variant<FirstType, RestTypes...>::invalid() const -> bool {
  return m_typeIndex == InvalidVariantType;
}

template <typename FirstType, typename... RestTypes>
auto Variant<FirstType, RestTypes...>::operator==(Variant const& x) const -> bool {
  if (this == &x) {
    return true;
  } else if (typeIndex() != x.typeIndex()) {
    return false;
  } else {
    return call([&x](auto const& t) -> auto {
        using T =  std::decay_t<decltype(t)>;
        return t == x.template get<T>();
      });
  }
}

template <typename FirstType, typename... RestTypes>
auto Variant<FirstType, RestTypes...>::operator!=(Variant const& x) const -> bool {
  return !operator==(x);
}

template <typename FirstType, typename... RestTypes>
auto Variant<FirstType, RestTypes...>::operator<(Variant const& x) const -> bool {
  if (this == &x) {
    return false;
  } else {
    auto sti = typeIndex();
    auto xti = x.typeIndex();
    if (sti != xti) {
      return sti < xti;
    } else {
      return call([&x](auto const& t) -> auto {
          using T =  std::decay_t<decltype(t)>;
          return t < x.template get<T>();
        });
    }
  }
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::operator==(T const& x) const -> bool {
  if (auto p = ptr<T>())
    return *p == x;
  return false;
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::operator!=(T const& x) const -> bool {
  return !operator==(x);
}

template <typename FirstType, typename... RestTypes>
template <typename T, typename>
auto Variant<FirstType, RestTypes...>::operator<(T const& x) const -> bool {
  if (auto p = ptr<T>())
    return *p == x;
  return m_typeIndex < TypeIndex<T>::value;
}

template <typename FirstType, typename... RestTypes>
void Variant<FirstType, RestTypes...>::destruct() {
  if (m_typeIndex != InvalidVariantType) {
    try {
      call([](auto& t) -> auto {
          using T =  std::decay_t<decltype(t)>;
          t.~T();
        });
      m_typeIndex = InvalidVariantType;
    } catch (...) {
      m_typeIndex = InvalidVariantType;
      throw;
    }
  }
}

template <typename FirstType, typename... RestTypes>
template <typename T>
void Variant<FirstType, RestTypes...>::assign(T&& x) {
  using AssignType =  std::decay_t<T>;
  if (auto p = ptr<AssignType>()) {
    *p = std::forward<T>(x);
  } else {
    destruct();
    new (&m_buffer) AssignType(std::forward<T>(x));
    m_typeIndex = TypeIndex<AssignType>::value;
  }
}

template <typename FirstType, typename... RestTypes>
template <typename Function, typename T>
auto Variant<FirstType, RestTypes...>::doCall(Function&& function) -> decltype(auto) {
  if (T* p = ptr<T>())
    return function(*p);
  else
    throw BadVariantType();
}

template <typename FirstType, typename... RestTypes>
template <typename Function, typename T1, typename T2, typename... TL>
auto Variant<FirstType, RestTypes...>::doCall(Function&& function) -> decltype(auto) {
  if (T1* p = ptr<T1>())
    return function(*p);
  else
    return doCall<Function, T2, TL...>(std::forward<Function>(function));
}

template <typename FirstType, typename... RestTypes>
template <typename Function, typename T>
auto Variant<FirstType, RestTypes...>::doCall(Function&& function) const -> decltype(auto) {
  if (T const* p = ptr<T>())
    return function(*p);
  else
    throw BadVariantType();
}

template <typename FirstType, typename... RestTypes>
template <typename Function, typename T1, typename T2, typename... TL>
auto Variant<FirstType, RestTypes...>::doCall(Function&& function) const -> decltype(auto) {
  if (T1 const* p = ptr<T1>())
    return function(*p);
  else
    return doCall<Function, T2, TL...>(std::forward<Function>(function));
}

template <typename FirstType, typename... RestTypes>
template <typename First>
void Variant<FirstType, RestTypes...>::doMakeType(std::uint8_t typeIndex) {
  if (typeIndex == 0)
    *this = First();
  else
    throw BadVariantType();
}

template <typename FirstType, typename... RestTypes>
template <typename First, typename Second, typename... Rest>
void Variant<FirstType, RestTypes...>::doMakeType(std::uint8_t typeIndex) {
  if (typeIndex == 0)
    *this = First();
  else
    return doMakeType<Second, Rest...>(typeIndex - 1);
}

template <typename... Types>
template <typename T, typename>
constexpr auto MVariant<Types...>::typeIndexOf() -> std::uint8_t {
  return Variant<MVariantEmpty, Types...>::template typeIndexOf<T>();
}

template <typename... Types>
MVariant<Types...>::MVariant() = default;

template <typename... Types>
MVariant<Types...>::MVariant(MVariant const& x)
  : m_variant(x.m_variant) {}

template <typename... Types>
MVariant<Types...>::MVariant(MVariant&& x) {
  m_variant = std::move(x.m_variant);
  x.m_variant = MVariantEmpty();
}

template <typename... Types>
MVariant<Types...>::MVariant(Variant<Types...> const& x) {
  operator=(x);
}

template <typename... Types>
MVariant<Types...>::MVariant(Variant<Types...>&& x) {
  operator=(std::move(x));
}

template <typename... Types>
template <typename T, typename>
MVariant<Types...>::MVariant(T const& x)
  : m_variant(x) {}

template <typename... Types>
template <typename T, typename>
MVariant<Types...>::MVariant(T&& x)
  : m_variant(std::forward<T>(x)) {}

template <typename... Types>
MVariant<Types...>::~MVariant() = default;

template <typename... Types>
auto MVariant<Types...>::operator=(MVariant const& x) -> MVariant<Types...>& {
  try {
    m_variant = x.m_variant;
  } catch (...) {
    if (m_variant.invalid())
      m_variant = MVariantEmpty();
    throw;
  }
  return *this;
}

template <typename... Types>
auto MVariant<Types...>::operator=(MVariant&& x) -> MVariant<Types...>& {
  try {
    m_variant = std::move(x.m_variant);
  } catch (...) {
    if (m_variant.invalid())
      m_variant = MVariantEmpty();
    throw;
  }
  return *this;
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::operator=(T const& x) -> MVariant<Types...>& {
  try {
    m_variant = x;
  } catch (...) {
    if (m_variant.invalid())
      m_variant = MVariantEmpty();
    throw;
  }
  return *this;
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::operator=(T&& x) -> MVariant<Types...>& {
  try {
    m_variant = std::forward<T>(x);
  } catch (...) {
    if (m_variant.invalid())
      m_variant = MVariantEmpty();
    throw;
  }
  return *this;
}

template <typename... Types>
auto MVariant<Types...>::operator=(Variant<Types...> const& x) -> MVariant<Types...>& {
  x.call([this](auto const& t) -> auto {
      *this = t;
    });
  return *this;
}

template <typename... Types>
auto MVariant<Types...>::operator=(Variant<Types...>&& x) -> MVariant<Types...>& {
  x.call([this](auto& t) -> auto {
      *this = std::move(t);
    });
  return *this;
}

template <typename... Types>
auto MVariant<Types...>::operator==(MVariant const& x) const -> bool {
  return m_variant == x.m_variant;
}

template <typename... Types>
auto MVariant<Types...>::operator!=(MVariant const& x) const -> bool {
  return m_variant != x.m_variant;
}

template <typename... Types>
auto MVariant<Types...>::operator<(MVariant const& x) const -> bool {
  return m_variant < x.m_variant;
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::operator==(T const& x) const -> bool {
  return m_variant == x;
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::operator!=(T const& x) const -> bool {
  return m_variant != x;
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::operator<(T const& x) const -> bool {
  return m_variant < x;
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::get() const -> T const& {
  return m_variant.template get<T>();
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::get() -> T& {
  return m_variant.template get<T>();
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::maybe() const -> std::optional<T> {
  return m_variant.template maybe<T>();
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::ptr() const -> T const* {
  return m_variant.template ptr<T>();
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::ptr() -> T* {
  return m_variant.template ptr<T>();
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::is() const -> bool {
  return m_variant.template is<T>();
}

template <typename... Types>
template <typename T, typename>
auto MVariant<Types...>::take() -> T {
  T t = std::move(m_variant.template get<T>());
  m_variant = MVariantEmpty();
  return t;
}

template <typename... Types>
auto MVariant<Types...>::value() const -> Variant<Types...> {
  if (empty())
    throw BadVariantCast();

  Variant<Types...> r;
  call([&r](auto const& v) -> auto {
      r = v;
    });
  return r;
}

template <typename... Types>
auto MVariant<Types...>::takeValue() -> Variant<Types...> {
  if (empty())
    throw BadVariantCast();

  Variant<Types...> r;
  call([&r](auto& v) -> auto {
      r = std::move(v);
    });
  m_variant = MVariantEmpty();
  return r;
}

template <typename... Types>
auto MVariant<Types...>::empty() const -> bool {
  return m_variant.template is<MVariantEmpty>();
}

template <typename... Types>
void MVariant<Types...>::reset() {
  m_variant = MVariantEmpty();
}

template <typename... Types>
MVariant<Types...>::operator bool() const {
  return !empty();
}

template <typename... Types>
template <typename Function>
void MVariant<Types...>::call(Function&& function) {
  m_variant.call(RefCaller<Function>(std::forward<Function>(function)));
}

template <typename... Types>
template <typename Function>
void MVariant<Types...>::call(Function&& function) const {
  m_variant.call(ConstRefCaller<Function>(std::forward<Function>(function)));
}

template <typename... Types>
auto MVariant<Types...>::typeIndex() const -> std::uint8_t {
  return m_variant.typeIndex();
}

template <typename... Types>
void MVariant<Types...>::makeType(std::uint8_t typeIndex) {
  m_variant.makeType(typeIndex);
}

template <typename... Types>
auto MVariant<Types...>::MVariantEmpty::operator==(MVariantEmpty const&) const -> bool {
  return true;
}

template <typename... Types>
auto MVariant<Types...>::MVariantEmpty::operator<(MVariantEmpty const&) const -> bool {
  return false;
}

template <typename... Types>
template <typename Function>
MVariant<Types...>::RefCaller<Function>::RefCaller(Function&& function)
  : function(std::forward<Function>(function)) {}

template <typename... Types>
template <typename Function>
void MVariant<Types...>::RefCaller<Function>::operator()(MVariantEmpty&) {}

template <typename... Types>
template <typename Function>
template <typename T>
void MVariant<Types...>::RefCaller<Function>::operator()(T& t) {
  function(t);
}

template <typename... Types>
template <typename Function>
MVariant<Types...>::ConstRefCaller<Function>::ConstRefCaller(Function&& function)
  : function(std::forward<Function>(function)) {}

template <typename... Types>
template <typename Function>
void MVariant<Types...>::ConstRefCaller<Function>::operator()(MVariantEmpty const&) {}

template <typename... Types>
template <typename Function>
template <typename T>
void MVariant<Types...>::ConstRefCaller<Function>::operator()(T const& t) {
  function(t);
}

}

template <typename FirstType, typename... RestTypes>
struct std::formatter<Star::Variant<FirstType, RestTypes...>> : Star::ostream_formatter {};
