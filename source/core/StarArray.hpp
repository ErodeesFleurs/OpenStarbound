#pragma once

#include <array>

#include "StarException.hpp"
#include "StarHash.hpp"

namespace Star {

// Somewhat nicer form of std::array, always initializes values, uses nicer
// constructor pattern.
template <typename ElementT, size_t SizeN>
class Array : public std::array<ElementT, SizeN> {
public:
  using Base = std::array<ElementT, SizeN>;

  using Element = ElementT;
  static size_t const ArraySize = SizeN;

  using iterator = Element*;
  using const_iterator = Element const*;

  using reference = Element&;
  using const_reference = Element const&;

  using value_type = Element;

  static constexpr Array filled(Element const& e);

  template <typename Iterator>
  static constexpr Array copyFrom(Iterator p, size_t n = NPos);

  constexpr Array();

  explicit constexpr Array(Element const& e1);

  template <typename... T>
  constexpr Array(Element const& e1, T const&... rest);

  template <typename Element2>
  explicit constexpr Array(Array<Element2, SizeN> const& a);

  template <size_t i>
  reference get();

  template <size_t i>
  const_reference get() const;

  template <typename T2>
  Array& operator=(Array<T2, SizeN> const& array);

  constexpr Element* ptr();
  constexpr Element const* ptr() const;

  constexpr bool operator==(Array const& a) const;
  constexpr bool operator!=(Array const& a) const;
  constexpr bool operator<(Array const& a) const;
  constexpr bool operator<=(Array const& a) const;
  constexpr bool operator>(Array const& a) const;
  constexpr bool operator>=(Array const& a) const;

  template <size_t Size2>
  Array<ElementT, Size2> toSize() const;

private:
  // Instead of {} array initialization, use recursive assignment to mimic old
  // C++ style construction with less strict narrowing rules.
  template <typename T, typename... TL>
  constexpr void set(T const& e, TL const&... rest);
  constexpr void set();
};

template <typename DataT, size_t SizeT>
struct hash<Array<DataT, SizeT>> {
  size_t operator()(Array<DataT, SizeT> const& a) const;
  Star::hash<DataT> dataHasher;
};

using Array2I = Array<int, 2>;
using Array2S = Array<size_t, 2>;
using Array2U = Array<unsigned, 2>;
using Array2F = Array<float, 2>;
using Array2D = Array<double, 2>;

using Array3I = Array<int, 3>;
using Array3S = Array<size_t, 3>;
using Array3U = Array<unsigned, 3>;
using Array3F = Array<float, 3>;
using Array3D = Array<double, 3>;

using Array4I = Array<int, 4>;
using Array4S = Array<size_t, 4>;
using Array4U = Array<unsigned, 4>;
using Array4F = Array<float, 4>;
using Array4D = Array<double, 4>;

template <typename Element, size_t Size>
constexpr Array<Element, Size> Array<Element, Size>::filled(Element const& e) {
  Array a;
  a.fill(e);
  return a;
}

template <typename Element, size_t Size>
template <typename Iterator>
constexpr Array<Element, Size> Array<Element, Size>::copyFrom(Iterator p, size_t n) {
  Array a;
  for (size_t i = 0; i < n && i < Size; ++i)
    a[i] = *(p++);
  return a;
}

template <typename Element, size_t Size>
constexpr Array<Element, Size>::Array()
  : Base() {}

template <typename Element, size_t Size>
constexpr Array<Element, Size>::Array(Element const& e1) {
  static_assert(Size == 1, "Incorrect size in Array constructor");
  set(e1);
}

template <typename Element, size_t Size>
template <typename... T>
constexpr Array<Element, Size>::Array(Element const& e1, T const&... rest) {
  static_assert(sizeof...(rest) == Size - 1, "Incorrect size in Array constructor");
  set(e1, rest...);
}

template <typename Element, size_t Size>
template <typename Element2>
constexpr Array<Element, Size>::Array(Array<Element2, Size> const& a) {
  std::copy(a.begin(), a.end(), Base::begin());
}

template <typename Element, size_t Size>
template <size_t i>
auto Array<Element, Size>::get() -> reference {
  static_assert(i < Size, "Incorrect size in Array::at");
  return Base::operator[](i);
}

template <typename Element, size_t Size>
template <size_t i>
auto Array<Element, Size>::get() const -> const_reference {
  static_assert(i < Size, "Incorrect size in Array::at");
  return Base::operator[](i);
}

template <typename Element, size_t Size>
template <typename T2>
Array<Element, Size>& Array<Element, Size>::operator=(Array<T2, Size> const& array) {
  std::copy(array.begin(), array.end(), Base::begin());
  return *this;
}

template <typename Element, size_t Size>
constexpr Element* Array<Element, Size>::ptr() {
  return Base::data();
}

template <typename Element, size_t Size>
constexpr Element const* Array<Element, Size>::ptr() const {
  return Base::data();
}

template <typename Element, size_t Size>
constexpr bool Array<Element, Size>::operator==(Array const& a) const {
  for (size_t i = 0; i < Size; ++i)
    if ((*this)[i] != a[i])
      return false;
  return true;
}

template <typename Element, size_t Size>
constexpr bool Array<Element, Size>::operator!=(Array const& a) const {
  return !operator==(a);
}

template <typename Element, size_t Size>
constexpr bool Array<Element, Size>::operator<(Array const& a) const {
  for (size_t i = 0; i < Size; ++i) {
    if ((*this)[i] < a[i])
      return true;
    else if (a[i] < (*this)[i])
      return false;
  }
  return false;
}

template <typename Element, size_t Size>
constexpr bool Array<Element, Size>::operator<=(Array const& a) const {
  for (size_t i = 0; i < Size; ++i) {
    if ((*this)[i] < a[i])
      return true;
    else if (a[i] < (*this)[i])
      return false;
  }
  return true;
}

template <typename Element, size_t Size>
constexpr bool Array<Element, Size>::operator>(Array const& a) const {
  return a < *this;
}

template <typename Element, size_t Size>
constexpr bool Array<Element, Size>::operator>=(Array const& a) const {
  return a <= *this;
}

template <typename Element, size_t Size>
template <size_t Size2>
Array<Element, Size2> Array<Element, Size>::toSize() const {
  Array<Element, Size2> r;
  size_t ns = std::min(Size2, Size);
  for (size_t i = 0; i < ns; ++i)
    r[i] = (*this)[i];
  return r;
}

template <typename Element, size_t Size>
constexpr void Array<Element, Size>::set() {}

template <typename Element, size_t Size>
template <typename T, typename... TL>
constexpr void Array<Element, Size>::set(T const& e, TL const&... rest) {
  Base::operator[](Size - 1 - sizeof...(rest)) = e;
  set(rest...);
}

template <typename Element, size_t Size>
std::ostream& operator<<(std::ostream& os, Array<Element, Size> const& a) {
  os << '[';
  for (size_t i = 0; i < Size; ++i) {
    os << a[i];
    if (i != Size - 1)
      os << ", ";
  }
  os << ']';
  return os;
}

template <typename DataT, size_t SizeT>
size_t hash<Array<DataT, SizeT>>::operator()(Array<DataT, SizeT> const& a) const {
  size_t hashval = 0;
  for (size_t i = 0; i < SizeT; ++i)
    hashCombine(hashval, dataHasher(a[i]));
  return hashval;
}

}

template <typename Element, size_t Size>
struct std::formatter<Star::Array<Element, Size>> : Star::OstreamFormatter {};
