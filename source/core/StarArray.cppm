module;

import star.memory;
import std;
import star.hash;

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

  static Array filled(Element const& e);

  template <typename Iterator>
  static Array copyFrom(Iterator p, std::size_t n = NPos);

  Array();

  explicit Array(Element const& e1);

  template <typename... T>
  Array(Element const& e1, T const&... rest);

  template <typename Element2>
  explicit Array(Array<Element2, SizeN> const& a);

  template <std::size_t i>
  reference get();

  template <std::size_t i>
  const_reference get() const;

  template <typename T2>
  Array& operator=(Array<T2, SizeN> const& array);

  Element* ptr();
  Element const* ptr() const;

  bool operator==(Array const& a) const;
  // Lexicographic, and deliberately weak: the elements only provide <, so an
  // element pair that is neither less nor greater is not necessarily equal
  // (floats with NaN).
  std::weak_ordering operator<=>(Array const& a) const;

  template <std::size_t Size2>
  Array<ElementT, Size2> toSize() const;

private:
  // Instead of {} array initialization, use recursive assignment to mimic old
  // C++ style construction with less strict narrowing rules.
  template <typename T, typename... TL>
  void set(T const& e, TL const&... rest);
  void set();
};

template <typename DataT, std::size_t SizeT>
struct hash<Array<DataT, SizeT>> {
  std::size_t operator()(Array<DataT, SizeT> const& a) const;
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
Array<Element, Size> Array<Element, Size>::filled(Element const& e) {
  Array a;
  a.fill(e);
  return a;
}

template <typename Element, std::size_t Size>
template <typename Iterator>
Array<Element, Size> Array<Element, Size>::copyFrom(Iterator p, std::size_t n) {
  Array a;
  std::ranges::copy_n(std::move(p), std::min(n, Size), a.begin());
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
Array<Element, Size>& Array<Element, Size>::operator=(Array<T2, Size> const& array) {
  std::copy(array.begin(), array.end(), Base::begin());
  return *this;
}

template <typename Element, std::size_t Size>
Element* Array<Element, Size>::ptr() {
  return Base::data();
}

template <typename Element, std::size_t Size>
Element const* Array<Element, Size>::ptr() const {
  return Base::data();
}

template <typename Element, std::size_t Size>
bool Array<Element, Size>::operator==(Array const& a) const {
  for (std::size_t i = 0; i < Size; ++i)
    if ((*this)[i] != a[i])
      return false;
  return true;
}

template <typename Element, std::size_t Size>
std::weak_ordering Array<Element, Size>::operator<=>(Array const& a) const {
  for (std::size_t i = 0; i < Size; ++i) {
    if ((*this)[i] < a[i])
      return std::weak_ordering::less;
    if (a[i] < (*this)[i])
      return std::weak_ordering::greater;
  }
  return std::weak_ordering::equivalent;
}

template <typename Element, std::size_t Size>
template <std::size_t Size2>
Array<Element, Size2> Array<Element, Size>::toSize() const {
  Array<Element, Size2> r;
  std::ranges::copy_n(this->begin(), std::min(Size2, Size), r.begin());
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
std::ostream& operator<<(std::ostream& os, Array<Element, Size> const& a) {
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
std::size_t hash<Array<DataT, SizeT>>::operator()(Array<DataT, SizeT> const& a) const {
  std::size_t hashval = 0;
  for (std::size_t i = 0; i < SizeT; ++i)
    hashCombine(hashval, dataHasher(a[i]));
  return hashval;
}

}

export module star.array;

export namespace Star {
  template <typename DataT, std::size_t SizeT>
  struct hash<Array<DataT, SizeT>>;
}


export namespace Star {
  using ::Star::Array;
  using ::Star::Array2I;
  using ::Star::Array2S;
  using ::Star::Array2U;
  using ::Star::Array2F;
  using ::Star::Array2D;
  using ::Star::Array3I;
  using ::Star::Array3S;
  using ::Star::Array3U;
  using ::Star::Array3F;
  using ::Star::Array3D;
  using ::Star::Array4I;
  using ::Star::Array4S;
  using ::Star::Array4U;
  using ::Star::Array4F;
  using ::Star::Array4D;
  using ::Star::hash;
  using ::Star::operator<<;
}
