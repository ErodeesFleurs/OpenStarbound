module;
import std;
import star.format;

namespace Star {

struct StaticVectorSizeExceptionTag {
  static constexpr char const* name() { return "StaticVectorSizeException"; }
};
using StaticVectorSizeException = StarError<StaticVectorSizeExceptionTag, StarException>;

// Stack allocated vector of elements with a dynamic size which must be less
// than a given maximum.  Acts like a vector with a built-in allocator of a
// maximum size, throws bad_alloc on attempting to resize beyond the maximum
// size.
template <typename Element, std::size_t MaxSize>
class StaticVector {
public:
  typedef Element* iterator;
  typedef Element const* const_iterator;

  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  typedef Element value_type;

  typedef Element& reference;
  typedef Element const& const_reference;

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

  StaticVector& operator=(StaticVector const& other);
  StaticVector& operator=(StaticVector&& other);
  StaticVector& operator=(std::initializer_list<Element> list);

  std::size_t size() const;
  bool empty() const;
  void resize(std::size_t size, Element const& e = Element());

  reference at(std::size_t i);
  const_reference at(std::size_t i) const;

  reference operator[](std::size_t i);
  const_reference operator[](std::size_t i) const;

  const_iterator begin() const;
  const_iterator end() const;

  iterator begin();
  iterator end();

  const_reverse_iterator rbegin() const;
  const_reverse_iterator rend() const;

  reverse_iterator rbegin();
  reverse_iterator rend();

  // Pointer to internal data, always valid even if empty.
  Element const* ptr() const;
  Element* ptr();

  void push_back(Element e);
  void pop_back();

  iterator insert(iterator pos, Element e);
  template <typename Iterator>
  iterator insert(iterator pos, Iterator begin, Iterator end);
  iterator insert(iterator pos, std::initializer_list<Element> list);

  template <typename... Args>
  void emplace(iterator pos, Args&&... args);

  template <typename... Args>
  void emplace_back(Args&&... args);

  void clear();

  iterator erase(iterator pos);
  iterator erase(iterator begin, iterator end);

  bool operator==(StaticVector const& other) const;
  bool operator!=(StaticVector const& other) const;
  bool operator<(StaticVector const& other) const;

private:
  std::size_t m_size;
  alignas(Element) std::byte m_elements[MaxSize ? MaxSize * sizeof(Element) : alignof(Element)];
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
std::size_t StaticVector<Element, MaxSize>::size() const {
  return m_size;
}

template <typename Element, std::size_t MaxSize>
bool StaticVector<Element, MaxSize>::empty() const {
  return m_size == 0;
}

template <typename Element, std::size_t MaxSize>
void StaticVector<Element, MaxSize>::resize(std::size_t size, Element const& e) {
  if (size > MaxSize)
    throw StaticVectorSizeException::format("StaticVector::resize({}) out of range {}", m_size + size, MaxSize);

  for (std::size_t i = m_size; i > size; --i)
    pop_back();
  for (std::size_t i = m_size; i < size; ++i)
    emplace_back(e);
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::at(std::size_t i) -> reference {
  if (i >= m_size)
    throw OutOfRangeException::format("out of range in StaticVector::at({})", i);
  return ptr()[i];
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::at(std::size_t i) const -> const_reference {
  if (i >= m_size)
    throw OutOfRangeException::format("out of range in StaticVector::at({})", i);
  return ptr()[i];
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator[](std::size_t i) -> reference {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((i < m_size));
  }
  return ptr()[i];
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::operator[](std::size_t i) const -> const_reference {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((i < m_size));
  }
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
Element const* StaticVector<Element, MaxSize>::ptr() const {
  return reinterpret_cast<Element const*>(m_elements);
}

template <typename Element, std::size_t MaxSize>
Element* StaticVector<Element, MaxSize>::ptr() {
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
  std::destroy_at(ptr() + m_size);
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::insert(iterator pos, Element e) -> iterator {
  auto index = static_cast<std::size_t>(pos - ptr());
  emplace(pos, std::move(e));
  return begin() + index;
}

template <typename Element, std::size_t MaxSize>
template <typename Iterator>
auto StaticVector<Element, MaxSize>::insert(iterator pos, Iterator first, Iterator last) -> iterator {
  auto index = static_cast<std::size_t>(pos - ptr());
  auto previousSize = size();
  if constexpr (std::forward_iterator<Iterator>) {
    if (static_cast<std::size_t>(std::distance(first, last)) > MaxSize - previousSize)
      throw StaticVectorSizeException::format("StaticVector::insert would extend StaticVector beyond size {}", MaxSize);
  }
  try {
    for (; first != last; ++first)
      emplace_back(*first);
  } catch (...) {
    while (size() > previousSize)
      pop_back();
    throw;
  }
  std::rotate(begin() + index, begin() + previousSize, end());
  return begin() + index;
}

template <typename Element, std::size_t MaxSize>
auto StaticVector<Element, MaxSize>::insert(iterator pos, std::initializer_list<Element> list) -> iterator {
  return insert(pos, list.begin(), list.end());
}

template <typename Element, std::size_t MaxSize>
template <typename... Args>
void StaticVector<Element, MaxSize>::emplace(iterator pos, Args&&... args) {
  auto index = static_cast<std::size_t>(pos - ptr());
  if (m_size == MaxSize)
    throw StaticVectorSizeException::format("StaticVector::emplace would extend StaticVector beyond size {}", MaxSize);
  Element value(std::forward<Args>(args)...);
  if (index == size()) {
    emplace_back(std::move(value));
    return;
  }
  emplace_back(std::move(*(end() - 1)));
  std::move_backward(begin() + index, end() - 2, end() - 1);
  operator[](index) = std::move(value);
}

template <typename Element, std::size_t MaxSize>
template <typename... Args>
void StaticVector<Element, MaxSize>::emplace_back(Args&&... args) {
  if (m_size + 1 > MaxSize)
    throw StaticVectorSizeException::format("StaticVector::emplace_back would extend StaticVector beyond size {}", MaxSize);

  std::construct_at(ptr() + m_size, std::forward<Args>(args)...);
  ++m_size;
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
bool StaticVector<Element, MaxSize>::operator==(StaticVector const& other) const {
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
bool StaticVector<Element, MaxSize>::operator!=(StaticVector const& other) const {
  return !operator==(other);
}

template <typename Element, std::size_t MaxSize>
bool StaticVector<Element, MaxSize>::operator<(StaticVector const& other) const {
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

export module star.static_vector;

export namespace Star {
  using ::Star::StaticVectorSizeExceptionTag;
  using ::Star::StaticVectorSizeException;
  using ::Star::StaticVector;
}
