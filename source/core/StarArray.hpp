#pragma once

#include "StarHash.hpp"
#include "StarOstreamFormatter.hpp"

import std;

namespace Star {

// Somewhat nicer form of std::array, always initializes values, uses nicer
// constructor pattern.
template <typename ElementT, std::size_t SizeN>
class Array : public std::array<ElementT, SizeN> {
public:
  using Base = std::array<ElementT, SizeN>;

  using Element = ElementT;
  static std::size_t const ArraySize = SizeN;

  using iterator = Element*;
  using const_iterator = Element const*;

  using reference = Element&;
  using const_reference = Element const&;

  using value_type = Element;

  static auto filled(Element const& e) -> Array;

  template <typename Iterator>
  static auto copyFrom(Iterator p, std::size_t n = std::numeric_limits<std::size_t>::max()) -> Array;

  Array();

  explicit Array(Element const& e1);

  template <typename... T>
  Array(Element const& e1, T const&... rest);

  template <typename Element2>
  explicit Array(Array<Element2, SizeN> const& a);

  template <std::size_t i>
  auto get() -> reference;

  template <std::size_t i>
  auto get() const -> const_reference;

  template <typename T2>
  auto operator=(Array<T2, SizeN> const& array) -> Array&;

  auto ptr() -> Element*;
  auto ptr() const -> Element const*;

  auto operator==(Array const& a) const -> bool;
  auto operator!=(Array const& a) const -> bool;
  auto operator<(Array const& a) const -> bool;
  auto operator<=(Array const& a) const -> bool;
  auto operator>(Array const& a) const -> bool;
  auto operator>=(Array const& a) const -> bool;

  template <std::size_t Size2>
  auto toSize() const -> Array<ElementT, Size2>;

private:
  // Instead of {} array initialization, use recursive assignment to mimic old
  // C++ style construction with less strict narrowing rules.
  template <typename T, typename... TL>
  void set(T const& e, TL const&... rest);
  void set();
};

template <typename DataT, std::size_t SizeT>
struct hash<Array<DataT, SizeT>> {
  auto operator()(Array<DataT, SizeT> const& a) const -> std::size_t;
  Star::hash<DataT> dataHasher;
};

using Array2I = Array<int, 2>;
using Array2S = Array<std::size_t, 2>;
using Array2U = Array<unsigned, 2>;
using Array2F = Array<float, 2>;
using Array2D = Array<double, 2>;

using Array3I = Array<int, 3>;
using Array3S = Array<std::size_t, 3>;
using Array3U = Array<unsigned, 3>;
using Array3F = Array<float, 3>;
using Array3D = Array<double, 3>;

using Array4I = Array<int, 4>;
using Array4S = Array<std::size_t, 4>;
using Array4U = Array<unsigned, 4>;
using Array4F = Array<float, 4>;
using Array4D = Array<double, 4>;

template <typename Element, std::size_t Size>
auto Array<Element, Size>::filled(Element const& e) -> Array<Element, Size> {
  Array a;
  a.fill(e);
  return a;
}

template <typename Element, std::size_t Size>
template <typename Iterator>
auto Array<Element, Size>::copyFrom(Iterator p, std::size_t n) -> Array<Element, Size> {
  Array a;
  for (std::size_t i = 0; i < n && i < Size; ++i)
    a[i] = *(p++);
  return a;
}

template <typename Element, std::size_t Size>
Array<Element, Size>::Array()
    : Base() {}

template <typename Element, std::size_t Size>
Array<Element, Size>::Array(Element const& e1) {
  static_assert(Size == 1, "Incorrect size in Array constructor");
  set(e1);
}

template <typename Element, std::size_t Size>
template <typename... T>
Array<Element, Size>::Array(Element const& e1, T const&... rest) {
  static_assert(sizeof...(rest) == Size - 1, "Incorrect size in Array constructor");
  set(e1, rest...);
}

template <typename Element, std::size_t Size>
template <typename Element2>
Array<Element, Size>::Array(Array<Element2, Size> const& a) {
  std::copy(a.begin(), a.end(), Base::begin());
}

template <typename Element, std::size_t Size>
template <std::size_t i>
auto Array<Element, Size>::get() -> reference {
  static_assert(i < Size, "Incorrect size in Array::at");
  return Base::operator[](i);
}

template <typename Element, std::size_t Size>
template <std::size_t i>
auto Array<Element, Size>::get() const -> const_reference {
  static_assert(i < Size, "Incorrect size in Array::at");
  return Base::operator[](i);
}

template <typename Element, std::size_t Size>
template <typename T2>
auto Array<Element, Size>::operator=(Array<T2, Size> const& array) -> Array<Element, Size>& {
  std::copy(array.begin(), array.end(), Base::begin());
  return *this;
}

template <typename Element, std::size_t Size>
auto Array<Element, Size>::ptr() -> Element* {
  return Base::data();
}

template <typename Element, std::size_t Size>
auto Array<Element, Size>::ptr() const -> Element const* {
  return Base::data();
}

template <typename Element, std::size_t Size>
auto Array<Element, Size>::operator==(Array const& a) const -> bool {
  for (std::size_t i = 0; i < Size; ++i)
    if ((*this)[i] != a[i])
      return false;
  return true;
}

template <typename Element, std::size_t Size>
auto Array<Element, Size>::operator!=(Array const& a) const -> bool {
  return !operator==(a);
}

template <typename Element, std::size_t Size>
auto Array<Element, Size>::operator<(Array const& a) const -> bool {
  for (std::size_t i = 0; i < Size; ++i) {
    if ((*this)[i] < a[i])
      return true;
    else if (a[i] < (*this)[i])
      return false;
  }
  return false;
}

template <typename Element, std::size_t Size>
auto Array<Element, Size>::operator<=(Array const& a) const -> bool {
  for (std::size_t i = 0; i < Size; ++i) {
    if ((*this)[i] < a[i])
      return true;
    else if (a[i] < (*this)[i])
      return false;
  }
  return true;
}

template <typename Element, std::size_t Size>
auto Array<Element, Size>::operator>(Array const& a) const -> bool {
  return a < *this;
}

template <typename Element, std::size_t Size>
auto Array<Element, Size>::operator>=(Array const& a) const -> bool {
  return a <= *this;
}

template <typename Element, std::size_t Size>
template <std::size_t Size2>
auto Array<Element, Size>::toSize() const -> Array<Element, Size2> {
  Array<Element, Size2> r;
  std::size_t ns = std::min(Size2, Size);
  for (std::size_t i = 0; i < ns; ++i)
    r[i] = (*this)[i];
  return r;
}

template <typename Element, std::size_t Size>
void Array<Element, Size>::set() {}

template <typename Element, std::size_t Size>
template <typename T, typename... TL>
void Array<Element, Size>::set(T const& e, TL const&... rest) {
  Base::operator[](Size - 1 - sizeof...(rest)) = e;
  set(rest...);
}

template <typename Element, std::size_t Size>
auto operator<<(std::ostream& os, Array<Element, Size> const& a) -> std::ostream& {
  os << '[';
  for (std::size_t i = 0; i < Size; ++i) {
    os << a[i];
    if (i != Size - 1)
      os << ", ";
  }
  os << ']';
  return os;
}

template <typename DataT, std::size_t SizeT>
auto hash<Array<DataT, SizeT>>::operator()(Array<DataT, SizeT> const& a) const -> std::size_t {
  std::size_t hashval = 0;
  for (std::size_t i = 0; i < SizeT; ++i)
    hashCombine(hashval, dataHasher(a[i]));
  return hashval;
}

}// namespace Star

template <typename Element, std::size_t Size>
struct std::formatter<Star::Array<Element, Size>> : Star::ostream_formatter {};
