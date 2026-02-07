#pragma once

#include "StarArray.hpp"
#include "StarException.hpp"
#include "StarList.hpp"

import std;

namespace Star {

using MultiArrayException = ExceptionDerived<"MultiArrayException">;

// Multidimensional array class that wraps a vector as a simple contiguous N
// dimensional array.  Values are stored so that the highest dimension is the
// dimension with stride 0, and the lowest dimension has the largest stride.
//
// Due to usage of std::vector, ElementT = bool means that the user must use
// set() and get() rather than operator()
template <typename ElementT, size_t RankN>
class MultiArray {
public:
  using Storage = List<ElementT>;

  using Element = ElementT;
  static size_t const Rank = RankN;

  using IndexArray = Array<size_t, Rank>;
  using SizeArray = Array<size_t, Rank>;

  using iterator = typename Storage::iterator;
  using const_iterator = typename Storage::const_iterator;
  using value_type = Element;

  MultiArray();
  template <typename... T>
  explicit MultiArray(size_t i, T... rest);
  explicit MultiArray(SizeArray const& shape);
  explicit MultiArray(SizeArray const& shape, Element const& c);

  auto size() const -> SizeArray const&;
  [[nodiscard]] auto size(size_t dimension) const -> size_t;

  void clear();

  void resize(SizeArray const& shape);
  void resize(SizeArray const& shape, Element const& c);

  template <typename... T>
  void resize(size_t i, T... rest);

  void fill(Element const& element);

  // Does not preserve previous element position, array contents will be
  // invalid.
  void setSize(SizeArray const& shape);
  void setSize(SizeArray const& shape, Element const& c);

  template <typename... T>
  void setSize(size_t i, T... rest);

  auto operator()(IndexArray const& index) -> Element&;
  auto operator()(IndexArray const& index) const -> Element const&;

  template <typename... T>
  auto operator()(size_t i1, T... rest) -> Element&;
  template <typename... T>
  auto operator()(size_t i1, T... rest) const -> Element const&;

  // Throws exception if out of bounds
  auto at(IndexArray const& index) -> Element&;
  auto at(IndexArray const& index) const -> Element const&;

  template <typename... T>
  auto at(size_t i1, T... rest) -> Element&;
  template <typename... T>
  auto at(size_t i1, T... rest) const -> Element const&;

  // Throws an exception of out of bounds
  void set(IndexArray const& index, Element element);

  // Returns default element if out of bounds.
  auto get(IndexArray const& index, Element def = Element()) -> Element;

  // Auto-resizes array if out of bounds
  void setResize(IndexArray const& index, Element element);

  // Copy the given array element for element into this array.  The shape of
  // this array must be at least as large in every dimension as the source
  // array
  void copy(MultiArray const& source);
  void copy(MultiArray const& source, IndexArray const& sourceMin, IndexArray const& sourceMax, IndexArray const& targetMin);

  // op will be called with IndexArray and Element parameters.
  template <typename OpType>
  void forEach(IndexArray const& min, SizeArray const& size, OpType&& op);
  template <typename OpType>
  void forEach(IndexArray const& min, SizeArray const& size, OpType&& op) const;

  // Shortcut for calling forEach on the entire array
  template <typename OpType>
  void forEach(OpType&& op);
  template <typename OpType>
  void forEach(OpType&& op) const;

  template <typename OStream>
  void print(OStream& os) const;

  // Api for more direct access to elements.

  [[nodiscard]] auto count() const -> size_t;

  auto atIndex(size_t index) const -> Element const&;
  auto atIndex(size_t index) -> Element&;

  auto data() const -> Element const*;
  auto data() -> Element*;

private:
  auto storageIndex(IndexArray const& index) const -> size_t;

  template <typename OStream>
  void subPrint(OStream& os, IndexArray index, size_t dim) const;

  template <typename OpType>
  void subForEach(IndexArray const& min, SizeArray const& size, OpType&& op, IndexArray& index, size_t offset, size_t dim) const;

  template <typename OpType>
  void subForEach(IndexArray const& min, SizeArray const& size, OpType&& op, IndexArray& index, size_t offset, size_t dim);

  void subCopy(MultiArray const& source, IndexArray const& sourceMin, IndexArray const& sourceMax,
               IndexArray const& targetMin, IndexArray& sourceIndex, IndexArray& targetIndex, size_t dim);

  Storage m_data;
  SizeArray m_shape;
};

using MultiArray2I = MultiArray<int, 2>;
using MultiArray2S = MultiArray<size_t, 2>;
using MultiArray2U = MultiArray<unsigned, 2>;
using MultiArray2F = MultiArray<float, 2>;
using MultiArray2D = MultiArray<double, 2>;

using MultiArray3I = MultiArray<int, 3>;
using MultiArray3S = MultiArray<size_t, 3>;
using MultiArray3U = MultiArray<unsigned, 3>;
using MultiArray3F = MultiArray<float, 3>;
using MultiArray3D = MultiArray<double, 3>;

using MultiArray4I = MultiArray<int, 4>;
using MultiArray4S = MultiArray<size_t, 4>;
using MultiArray4U = MultiArray<unsigned, 4>;
using MultiArray4F = MultiArray<float, 4>;
using MultiArray4D = MultiArray<double, 4>;

template <typename Element, size_t Rank>
auto operator<<(std::ostream& os, MultiArray<Element, Rank> const& array) -> std::ostream&;

template <typename Element, size_t Rank>
MultiArray<Element, Rank>::MultiArray() {
  m_shape = SizeArray::filled(0);
}

template <typename Element, size_t Rank>
MultiArray<Element, Rank>::MultiArray(SizeArray const& shape) {
  setSize(shape);
}

template <typename Element, size_t Rank>
MultiArray<Element, Rank>::MultiArray(SizeArray const& shape, Element const& c) {
  setSize(shape, c);
}

template <typename Element, size_t Rank>
template <typename... T>
MultiArray<Element, Rank>::MultiArray(size_t i, T... rest) {
  setSize(SizeArray{i, rest...});
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::size() const -> SizeArray const& {
  return m_shape;
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::size(size_t dimension) const -> size_t {
  return m_shape[dimension];
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::clear() {
  setSize(SizeArray::filled(0));
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::resize(SizeArray const& shape) {
  if (m_data.empty()) {
    setSize(shape);
    return;
  }

  bool equal = true;
  for (size_t i = 0; i < Rank; ++i)
    equal = equal && (m_shape[i] == shape[i]);

  if (equal)
    return;

  MultiArray newArray(shape);
  newArray.copy(*this);
  std::swap(*this, newArray);
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::resize(SizeArray const& shape, Element const& c) {
  if (m_data.empty()) {
    setSize(shape, c);
    return;
  }

  bool equal = true;
  for (size_t i = 0; i < Rank; ++i)
    equal = equal && (m_shape[i] == shape[i]);

  if (equal)
    return;

  MultiArray newArray(shape, c);
  newArray.copy(*this);
  *this = std::move(newArray);
}

template <typename Element, size_t Rank>
template <typename... T>
void MultiArray<Element, Rank>::resize(size_t i, T... rest) {
  resize(SizeArray{i, rest...});
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::fill(Element const& element) {
  std::fill(m_data.begin(), m_data.end(), element);
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::setSize(SizeArray const& shape) {
  size_t storageSize = 1;
  for (size_t i = 0; i < Rank; ++i) {
    m_shape[i] = shape[i];
    storageSize *= shape[i];
  }

  m_data.resize(storageSize);
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::setSize(SizeArray const& shape, Element const& c) {
  size_t storageSize = 1;
  for (size_t i = 0; i < Rank; ++i) {
    m_shape[i] = shape[i];
    storageSize *= shape[i];
  }
  m_data.resize(storageSize, c);
}

template <typename Element, size_t Rank>
template <typename... T>
void MultiArray<Element, Rank>::setSize(size_t i, T... rest) {
  setSize({i, rest...});
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::operator()(IndexArray const& index) -> Element& {
  return m_data[storageIndex(index)];
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::operator()(IndexArray const& index) const -> Element const& {
  return m_data[storageIndex(index)];
}

template <typename Element, size_t Rank>
template <typename... T>
auto MultiArray<Element, Rank>::operator()(size_t i1, T... rest) -> Element& {
  return m_data[storageIndex(IndexArray(i1, rest...))];
}

template <typename Element, size_t Rank>
template <typename... T>
auto MultiArray<Element, Rank>::operator()(size_t i1, T... rest) const -> Element const& {
  return m_data[storageIndex(IndexArray(i1, rest...))];
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::at(IndexArray const& index) const -> Element const& {
  for (size_t i = Rank; i != 0; --i) {
    if (index[i - 1] >= m_shape[i - 1])
      throw MultiArrayException(strf("Out of bounds on MultiArray::at({})", index));
  }

  return m_data[storageIndex(index)];
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::at(IndexArray const& index) -> Element& {
  for (size_t i = Rank; i != 0; --i) {
    if (index[i - 1] >= m_shape[i - 1])
      throw MultiArrayException(strf("Out of bounds on MultiArray::at({})", index));
  }

  return m_data[storageIndex(index)];
}

template <typename Element, size_t Rank>
template <typename... T>
auto MultiArray<Element, Rank>::at(size_t i1, T... rest) -> Element& {
  return at(IndexArray(i1, rest...));
}

template <typename Element, size_t Rank>
template <typename... T>
auto MultiArray<Element, Rank>::at(size_t i1, T... rest) const -> Element const& {
  return at(IndexArray(i1, rest...));
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::set(IndexArray const& index, Element element) {
  for (size_t i = Rank; i != 0; --i) {
    if (index[i - 1] >= m_shape[i - 1])
      throw MultiArrayException(strf("Out of bounds on MultiArray::set({})", index));
  }

  m_data[storageIndex(index)] = std::move(element);
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::get(IndexArray const& index, Element def) -> Element {
  for (size_t i = Rank; i != 0; --i) {
    if (index[i - 1] >= m_shape[i - 1])
      return std::move(def);
  }

  return m_data[storageIndex(index)];
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::setResize(IndexArray const& index, Element element) {
  SizeArray newShape;
  for (size_t i = 0; i < Rank; ++i)
    newShape[i] = std::max(m_shape[i], index[i] + 1);
  resize(newShape);

  m_data[storageIndex(index)] = std::move(element);
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::copy(MultiArray const& source) {
  IndexArray max;
  for (size_t i = 0; i < Rank; ++i)
    max[i] = std::min(size(i), source.size(i));

  copy(source, IndexArray::filled(0), max, IndexArray::filled(0));
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::copy(MultiArray const& source, IndexArray const& sourceMin, IndexArray const& sourceMax, IndexArray const& targetMin) {
  IndexArray sourceIndex;
  IndexArray targetIndex;
  subCopy(source, sourceMin, sourceMax, targetMin, sourceIndex, targetIndex, 0);
}

template <typename Element, size_t Rank>
template <typename OpType>
void MultiArray<Element, Rank>::forEach(IndexArray const& min, SizeArray const& size, OpType&& op) {
  IndexArray index;
  subForEach(min, size, std::forward<OpType>(op), index, 0, 0);
}

template <typename Element, size_t Rank>
template <typename OpType>
void MultiArray<Element, Rank>::forEach(IndexArray const& min, SizeArray const& size, OpType&& op) const {
  IndexArray index;
  subForEach(min, size, std::forward<OpType>(op), index, 0, 0);
}

template <typename Element, size_t Rank>
template <typename OpType>
void MultiArray<Element, Rank>::forEach(OpType&& op) {
  forEach(IndexArray::filled(0), size(), std::forward<OpType>(op));
}

template <typename Element, size_t Rank>
template <typename OpType>
void MultiArray<Element, Rank>::forEach(OpType&& op) const {
  forEach(IndexArray::filled(0), size(), std::forward<OpType>(op));
}

template <typename Element, size_t Rank>
template <typename OStream>
void MultiArray<Element, Rank>::print(OStream& os) const {
  subPrint(os, IndexArray(), 0);
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::count() const -> size_t {
  return m_data.size();
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::atIndex(size_t index) const -> Element const& {
  return m_data[index];
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::atIndex(size_t index) -> Element& {
  return m_data[index];
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::data() const -> Element const* {
  return m_data.ptr();
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::data() -> Element* {
  return m_data.ptr();
}

template <typename Element, size_t Rank>
auto MultiArray<Element, Rank>::storageIndex(IndexArray const& index) const -> size_t {
  size_t loc = index[0];
  for (size_t i = 1; i < Rank; ++i) {
    loc = loc * m_shape[i] + index[i];
  }
  return loc;
}

template <typename Element, size_t Rank>
template <typename OStream>
void MultiArray<Element, Rank>::subPrint(OStream& os, IndexArray index, size_t dim) const {
  if (dim == Rank - 1) {
    for (size_t i = 0; i < m_shape[dim]; ++i) {
      index[dim] = i;
      os << m_data[storageIndex(index)] << ' ';
    }
    os << std::endl;
  } else {
    for (size_t i = 0; i < m_shape[dim]; ++i) {
      index[dim] = i;
      subPrint(os, index, dim + 1);
    }
    os << std::endl;
  }
}

template <typename Element, size_t Rank>
template <typename OpType>
void MultiArray<Element, Rank>::subForEach(IndexArray const& min, SizeArray const& size, OpType&& op, IndexArray& index, size_t offset, size_t dim) {
  size_t minIndex = min[dim];
  size_t maxIndex = minIndex + size[dim];
  for (size_t i = minIndex; i < maxIndex; ++i) {
    index[dim] = i;
    if (dim == Rank - 1)
      op(index, m_data[offset + i]);
    else
      subForEach(min, size, std::forward<OpType>(op), index, (offset + i) * m_shape[dim + 1], dim + 1);
  }
}

template <typename Element, size_t Rank>
template <typename OpType>
void MultiArray<Element, Rank>::subForEach(IndexArray const& min, SizeArray const& size, OpType&& op, IndexArray& index, size_t offset, size_t dim) const {
  size_t minIndex = min[dim];
  size_t maxIndex = minIndex + size[dim];
  for (size_t i = minIndex; i < maxIndex; ++i) {
    index[dim] = i;
    if (dim == Rank - 1)
      op(index, m_data[offset + i]);
    else
      subForEach(min, size, std::forward<OpType>(op), index, (offset + i) * m_shape[dim + 1], dim + 1);
  }
}

template <typename Element, size_t Rank>
void MultiArray<Element, Rank>::subCopy(MultiArray const& source, IndexArray const& sourceMin, IndexArray const& sourceMax,
                                        IndexArray const& targetMin, IndexArray& sourceIndex, IndexArray& targetIndex, size_t dim) {
  size_t w = sourceMax[dim] - sourceMin[dim];
  if (dim < Rank - 1) {
    for (size_t i = 0; i < w; ++i) {
      sourceIndex[dim] = i + sourceMin[dim];
      targetIndex[dim] = i + targetMin[dim];
      subCopy(source, sourceMin, sourceMax, targetMin, sourceIndex, targetIndex, dim + 1);
    }
  } else {
    sourceIndex[dim] = sourceMin[dim];
    targetIndex[dim] = targetMin[dim];
    size_t sourceStorageStart = source.storageIndex(sourceIndex);
    size_t targetStorageStart = storageIndex(targetIndex);
    for (size_t i = 0; i < w; ++i)
      m_data[targetStorageStart + i] = source.m_data[sourceStorageStart + i];
  }
}

template <typename Element, size_t Rank>
auto operator<<(std::ostream& os, MultiArray<Element, Rank> const& array) -> std::ostream& {
  array.print(os);
  return os;
}

}// namespace Star

template <typename Element, size_t Rank>
struct std::formatter<Star::MultiArray<Element, Rank>> : Star::ostream_formatter {};
