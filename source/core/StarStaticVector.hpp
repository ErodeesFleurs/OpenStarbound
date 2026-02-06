#pragma once

#include "StarException.hpp"

import std;

namespace Star {

using StaticVectorSizeException =  ExceptionDerived<"StaticVectorSizeException">;

// Stack allocated vector of elements with a dynamic size which must be less
// than a given maximum.  Acts like a vector with a built-in allocator of a
// maximum size, throws bad_alloc on attempting to resize beyond the maximum
// size.
template <typename Element, std::size_t MaxSize>
class StaticVector {
public:
  using iterator = Element*;
  using const_iterator = Element const*;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using value_type = Element;

  using reference = Element&;
  using const_reference = Element const&;

  static constexpr std::size_t MaximumSize = MaxSize;

  StaticVector();
  StaticVector(StaticVector const& other);
  StaticVector(StaticVector&& other);
  template <typename OtherElement, std::size_t OtherMaxSize>
  StaticVector(StaticVector<OtherElement, OtherMaxSize> const& other);
  template <class Iterator>
  StaticVector(Iterator first, Iterator last);
  StaticVector(std::size_t size, Element const& value = Element());
  StaticVector(std::initializer_list<Element> list);
  ~StaticVector();

  auto operator=(StaticVector const& other) -> StaticVector&;
  auto operator=(StaticVector&& other) -> StaticVector&;
  auto operator=(std::initializer_list<Element> list) -> StaticVector&;

  [[nodiscard]] auto size() const -> std::size_t;
  [[nodiscard]] auto empty() const -> bool;
  void resize(std::size_t size, Element const& e = Element());

  auto at(std::size_t i) -> reference;
  auto at(std::size_t i) const -> const_reference;

  auto operator[](std::size_t i) -> reference;
  auto operator[](std::size_t i) const -> const_reference;

  auto begin() const -> const_iterator;
  auto end() const -> const_iterator;

  auto begin() -> iterator;
  auto end() -> iterator;

  auto rbegin() const -> const_reverse_iterator;
  auto rend() const -> const_reverse_iterator;

  auto rbegin() -> reverse_iterator;
  auto rend() -> reverse_iterator;

  // Pointer to internal data, always valid even if empty.
  auto ptr() const -> Element const*;
  auto ptr() -> Element*;

  void push_back(Element e);
  void pop_back();

  auto insert(iterator pos, Element e) -> iterator;
  template <typename Iterator>
  auto insert(iterator pos, Iterator begin, Iterator end) -> iterator;
  auto insert(iterator pos, std::initializer_list<Element> list) -> iterator;

  template <typename... Args>
  void emplace(iterator pos, Args&&... args);

  template <typename... Args>
  void emplace_back(Args&&... args);

  void clear();

  auto erase(iterator pos) -> iterator;
  auto erase(iterator begin, iterator end) -> iterator;

  auto operator==(StaticVector const& other) const -> bool;
  auto operator!=(StaticVector const& other) const -> bool;
  auto operator<(StaticVector const& other) const -> bool;

private:
  std::size_t m_size;
  alignas(Element) std::array<std::byte, MaxSize * sizeof(Element)> m_elements;
};

template <typename Element, std::size_t MaxSize>
StaticVector<Element, MaxSize>::StaticVector()
  : m_size(0) {}

template <typename Element, std::size_t MaxSize>
StaticVector<Element, MaxSize>::~StaticVector() {
  clear();
}

template <typename Element, std::size_t MaxSize>
StaticVector<Element, MaxSize>::StaticVector(StaticVector const& other)
  : StaticVector() {
  insert(begin(), other.begin(), other.end());
}

template <typename Element, std::size_t MaxSize>
StaticVector<Element, MaxSize>::StaticVector(StaticVector&& other)
  : StaticVector() {
  for (auto& e : other)
    emplace_back(std::move(e));
}

template <typename Element, std::size_t MaxSize>
template <typename OtherElement, std::size_t OtherMaxSize>
StaticVector<Element, MaxSize>::StaticVector(StaticVector<OtherElement, OtherMaxSize> const& other)
  : StaticVector() {
  for (auto const& e : other)
    emplace_back(e);
}

template <typename Element, std::size_t MaxSize>
template <class Iterator>
StaticVector<Element, MaxSize>::StaticVector(Iterator first, Iterator last)
  : StaticVector() {
  insert(begin(), first, last);
}

template <typename Element, std::size_t MaxSize>
StaticVector<Element, MaxSize>::StaticVector(std::size_t size, Element const& value)
  : StaticVector() {
  resize(size, value);
}

template <typename Element, std::size_t MaxSize>
StaticVector<Element, MaxSize>::StaticVector(std::initializer_list<Element> list)
  : StaticVector() {
  for (auto const& e : list)
    emplace_back(e);
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator=(StaticVector const& other) -> StaticVector& {
  if (this == &other)
    return *this;

  resize(other.size());
  for (std::size_t i = 0; i < m_size; ++i)
    operator[](i) = other[i];

  return *this;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator=(StaticVector&& other) -> StaticVector& {
  resize(other.size());
  for (std::size_t i = 0; i < m_size; ++i)
    operator[](i) = std::move(other[i]);

  return *this;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator=(std::initializer_list<Element> list) -> StaticVector& {
  resize(list.size());
  for (std::size_t i = 0; i < m_size; ++i)
    operator[](i) = std::move(list[i]);
  return *this;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::size() const -> std::size_t {
  return m_size;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::empty() const -> bool {
  return m_size == 0;
}

template <typename Element, std::size_t MaxSize>
void StaticVector<Element, MaxSize>::resize(std::size_t size, Element const& e) {
  if (size > MaxSize)
    throw StaticVectorSizeException::format(std::string_view("StaticVector::resize({}) out of range {}"), m_size + size, MaxSize);

  for (std::size_t i = m_size; i > size; --i)
    pop_back();
  for (std::size_t i = m_size; i < size; ++i)
    emplace_back(e);
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::at(std::size_t i) -> reference {
  if (i >= m_size)
    throw OutOfRangeException::format(std::string_view("out of range in StaticVector::at({})"), i);
  return ptr()[i];
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::at(std::size_t i) const -> const_reference {
  if (i >= m_size)
    throw OutOfRangeException::format(std::string_view("out of range in StaticVector::at({})"), i);
  return ptr()[i];
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator[](std::size_t i) -> reference {
  return ptr()[i];
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator[](std::size_t i) const -> const_reference {
  return ptr()[i];
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::begin() const -> const_iterator {
  return ptr();
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::end() const -> const_iterator {
  return ptr() + m_size;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::begin() -> iterator {
  return ptr();
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::end() -> iterator {
  return ptr() + m_size;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::rbegin() const -> const_reverse_iterator {
  return const_reverse_iterator(end());
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::rend() const -> const_reverse_iterator {
  return const_reverse_iterator(begin());
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::rbegin() -> reverse_iterator {
  return reverse_iterator(end());
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::rend() -> reverse_iterator {
  return reverse_iterator(begin());
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::ptr() const -> Element const* {
  return reinterpret_cast<Element const*>(m_elements);
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::ptr() -> Element* {
  return reinterpret_cast<Element*>(m_elements);
}

template <typename Element, std::size_t MaxSize>
void StaticVector<Element, MaxSize>::push_back(Element e) {
  emplace_back(std::move(e));
}

template <typename Element, std::size_t MaxSize>
void StaticVector<Element, MaxSize>::pop_back() {
  if (m_size == 0)
    throw OutOfRangeException("StaticVector::pop_back called on empty StaticVector");
  --m_size;
  (ptr() + m_size)->~Element();
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::insert(iterator pos, Element e) -> iterator {
  emplace(pos, std::move(e));
  return pos;
}

template <typename Element, std::size_t MaxSize>
template <typename Iterator>
auto StaticVector<Element, MaxSize>::insert(iterator pos, Iterator begin, Iterator end) -> iterator {
  std::size_t toAdd = std::distance(begin, end);
  std::size_t startIndex = pos - ptr();
  std::size_t endIndex = startIndex + toAdd;
  std::size_t toShift = m_size - startIndex;

  resize(m_size + toAdd);

  for (std::size_t i = toShift; i != 0; --i)
    operator[](endIndex + i - 1) = std::move(operator[](startIndex + i - 1));

  for (std::size_t i = 0; i != toAdd; ++i)
    operator[](startIndex + i) = *begin++;

  return pos;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::insert(iterator pos, std::initializer_list<Element> list) -> iterator {
  return insert(pos, list.begin(), list.end());
}

template <typename Element, std::size_t MaxSize>
template <typename... Args>
void StaticVector<Element, MaxSize>::emplace(iterator pos, Args&&... args) {
  std::size_t index = pos - ptr();
  resize(m_size + 1);
  for (std::size_t i = m_size - 1; i != index; --i)
    operator[](i) = std::move(operator[](i - 1));
  operator[](index) = Element(std::forward<Args>(args)...);
}

template <typename Element, std::size_t MaxSize>
template <typename... Args>
void StaticVector<Element, MaxSize>::emplace_back(Args&&... args) {
  if (m_size + 1 > MaxSize)
    throw StaticVectorSizeException::format(std::string_view("StaticVector::emplace_back would extend StaticVector beyond size {}"), MaxSize);

  m_size += 1;
  new (ptr() + m_size - 1) Element(std::forward<Args>(args)...);
}

template <typename Element, std::size_t MaxSize>
void StaticVector<Element, MaxSize>::clear() {
  while (m_size != 0)
    pop_back();
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::erase(iterator pos) -> iterator {
  std::size_t index = pos - ptr();
  for (std::size_t i = index; i < m_size - 1; ++i)
    operator[](i) = std::move(operator[](i + 1));
  resize(m_size - 1);
  return pos;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::erase(iterator begin, iterator end) -> iterator {
  std::size_t startIndex = begin - ptr();
  std::size_t endIndex = end - ptr();
  std::size_t toRemove = endIndex - startIndex;
  for (std::size_t i = endIndex; i < m_size; ++i)
    operator[](startIndex + (i - endIndex)) = std::move(operator[](i));
  resize(m_size - toRemove);
  return begin;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator==(StaticVector const& other) const -> bool {
  if (this == &other)
    return true;

  if (m_size != other.m_size)
    return false;
  for (std::size_t i = 0; i < m_size; ++i) {
    if (operator[](i) != other[i])
      return false;
  }
  return true;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator!=(StaticVector const& other) const -> bool {
  return !operator==(other);
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator<(StaticVector const& other) const -> bool {
  for (std::size_t i = 0; i < m_size; ++i) {
    if (i >= other.size())
      return false;

    Element const& a = operator[](i);
    Element const& b = other[i];

    if (a < b)
      return true;
    else if (b < a)
      return false;
  }

  return m_size < other.size();
}

}
