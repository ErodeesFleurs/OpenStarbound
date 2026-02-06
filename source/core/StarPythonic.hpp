#pragma once

#include "StarException.hpp"
import std;

namespace Star {

// any and all

template <typename Iterator, typename Functor>
auto any(Iterator iterBegin, Iterator iterEnd, Functor const& f) -> bool {
  for (; iterBegin != iterEnd; iterBegin++)
    if (f(*iterBegin))
      return true;
  return false;
}

template <typename Iterator>
auto any(Iterator const& iterBegin, Iterator const& iterEnd) -> bool {
  using IteratorValue = typename std::iterator_traits<Iterator>::value_type;
  std::function<bool(IteratorValue)> compare = [](IteratorValue const& i) { return (bool)i; };
  return any(iterBegin, iterEnd, compare);
}

template <typename Iterable, typename Functor>
auto any(Iterable const& iter, Functor const& f) -> bool {
  return any(std::begin(iter), std::end(iter), f);
}

template <typename Iterable>
auto any(Iterable const& iter) -> bool {
  using IteratorValue = decltype(*std::begin(iter));
  std::function<bool(IteratorValue)> compare = [](IteratorValue const& i) -> auto { return (bool)i; };
  return any(std::begin(iter), std::end(iter), compare);
}

template <typename Iterator, typename Functor>
auto all(Iterator iterBegin, Iterator iterEnd, Functor const& f) -> bool {
  for (; iterBegin != iterEnd; iterBegin++)
    if (!f(*iterBegin))
      return false;
  return true;
}

template <typename Iterator>
auto all(Iterator const& iterBegin, Iterator const& iterEnd) -> bool {
  using IteratorValue = typename std::iterator_traits<Iterator>::value_type;
  std::function<bool(IteratorValue)> compare = [](IteratorValue const& i) { return (bool)i; };
  return all(iterBegin, iterEnd, compare);
}

template <typename Iterable, typename Functor>
auto all(Iterable const& iter, Functor const& f) -> bool {
  return all(std::begin(iter), std::end(iter), f);
}

template <typename Iterable>
auto all(Iterable const& iter) -> bool {
  using IteratorValue = decltype(*std::begin(iter));
  std::function<bool(IteratorValue)> compare = [](IteratorValue const& i) { return (bool)i; };
  return all(std::begin(iter), std::end(iter), compare);
}

// Python style container slicing

struct SliceIndex {
  SliceIndex() : index(0), given(false) {}
  SliceIndex(int i) : index(i), given(true) {}

  int index;
  bool given;
};

SliceIndex const SliceNil = SliceIndex();

// T must have operator[](int), size(), and
// push_back(typeof T::operator[](int()))
template <typename Res, typename In>
auto slice(In const& r, SliceIndex a, SliceIndex b = SliceIndex(), int j = 1) -> Res {
  int size = (int)r.size();
  int start, end;

  // Throw exception on j == 0?
  if (j == 0 || size == 0)
    return Res();

  if (!a.given) {
    if (j > 0)
      start = 0;
    else
      start = size - 1;
  } else if (a.index < 0) {
    if (-a.index > size - 1)
      start = 0;
    else
      start = size - -a.index;
  } else {
    if (a.index > size)
      start = size;
    else
      start = a.index;
  }

  if (!b.given) {
    if (j > 0)
      end = size;
    else
      end = -1;
  } else if (b.index < 0) {
    if (-b.index > size - 1) {
      end = -1;
    } else {
      end = size - -b.index;
    }
  } else {
    if (b.index > size - 1) {
      end = size;
    } else {
      end = b.index;
    }
  }

  if (start < end && j < 0)
    return Res();
  if (start > end && j > 0)
    return Res();

  Res returnSlice;
  int i;
  for (i = start; i < end; i += j)
    returnSlice.push_back(r[i]);

  return returnSlice;
}

template <typename T>
auto slice(T const& r, SliceIndex a, SliceIndex b = SliceIndex(), int j = 1) -> T {
  return slice<T, T>(r, a, b, j);
}

// ZIP

// Wraps a regular iterator and returns a singleton tuple, as well as
// supporting the iterator protocol that the zip iterator code expects.
template <typename IteratorT>
class ZipWrapperIterator {
private:
  IteratorT current;
  IteratorT last;
  bool atEnd;

public:
  using Iterator = IteratorT;
  using IteratorValue = decltype(*std::declval<Iterator>());
  using value_type = std::tuple<IteratorValue>;

  ZipWrapperIterator() : atEnd(true) {}

  ZipWrapperIterator(Iterator current, Iterator last) : current(current), last(last) {
    atEnd = current == last;
  }

  auto operator++() -> ZipWrapperIterator {
    if (!atEnd) {
      ++current;
      atEnd = current == last;
    }

    return *this;
  }

  auto operator*() const -> value_type {
    return std::tuple<IteratorValue>(*current);
  }

  auto operator==(ZipWrapperIterator const& rhs) const -> bool {
    return (atEnd && rhs.atEnd) || (!atEnd && !rhs.atEnd && current == rhs.current && last == rhs.last);
  }

  auto operator!=(ZipWrapperIterator const& rhs) const -> bool {
    return !(*this == rhs);
  }

  explicit operator bool() const {
    return !atEnd;
  }

  auto begin() const -> ZipWrapperIterator {
    return *this;
  }

  auto end() const -> ZipWrapperIterator {
    return ZipWrapperIterator();
  }
};
template <typename IteratorT>
auto makeZipWrapperIterator(IteratorT current, IteratorT end) -> ZipWrapperIterator<IteratorT> {
  return ZipWrapperIterator<IteratorT>(current, end);
}

// Takes two ZipIterators / ZipTupleIterators and concatenates them into a
// single iterator that returns the concatenated tuple.
template <typename TailIteratorT, typename HeadIteratorT>
class ZipTupleIterator {
private:
  TailIteratorT tailIterator;
  HeadIteratorT headIterator;
  bool atEnd;

public:
  using TailIterator = TailIteratorT;
  using HeadIterator = HeadIteratorT;

  using TailType = decltype(*TailIterator());
  using HeadType = decltype(*HeadIterator());

  using value_type = decltype(std::tuple_cat(std::declval<TailType>(), std::declval<HeadType>()));

  ZipTupleIterator() : atEnd(true) {}

  ZipTupleIterator(TailIterator tailIterator, HeadIterator headIterator)
    : tailIterator(tailIterator), headIterator(headIterator) {
    atEnd = tailIterator == TailIterator() || headIterator == HeadIterator();
  }

  auto operator++() -> ZipTupleIterator {
    if (!atEnd) {
      ++tailIterator;
      ++headIterator;
      atEnd = tailIterator == TailIterator() || headIterator == HeadIterator();
    }

    return *this;
  }

  auto operator*() const -> value_type {
    return std::tuple_cat(*tailIterator, *headIterator);
  }

  auto operator==(ZipTupleIterator const& rhs) const -> bool {
    return (atEnd && rhs.atEnd)
        || (!atEnd && !rhs.atEnd && tailIterator == rhs.tailIterator && headIterator == rhs.headIterator);
  }

  auto operator!=(ZipTupleIterator const& rhs) const -> bool {
    return !(*this == rhs);
  }

  explicit operator bool() const {
    return !atEnd;
  }

  auto begin() const -> ZipTupleIterator {
    return *this;
  }

  auto end() const -> ZipTupleIterator {
    return ZipTupleIterator();
  }
};
template <typename HeadIteratorT, typename TailIteratorT>
auto makeZipTupleIterator(HeadIteratorT head, TailIteratorT tail) -> ZipTupleIterator<HeadIteratorT, TailIteratorT> {
  return ZipTupleIterator<HeadIteratorT, TailIteratorT>(head, tail);
}

template <typename Container, typename... Rest>
struct zipIteratorReturn {
  using type = ZipTupleIterator<typename zipIteratorReturn<Container>::type, typename zipIteratorReturn<Rest...>::type>;
};

template <typename Container>
struct zipIteratorReturn<Container> {
  using type = ZipWrapperIterator<decltype(std::declval<Container>().begin())>;
};

template <typename Container>
auto zipIterator(Container& container) -> typename zipIteratorReturn<Container>::type {
  return makeZipWrapperIterator(container.begin(), container.end());
}

template <typename Container, typename... Rest>
auto zipIterator(Container& container, Rest&... rest) -> typename zipIteratorReturn<Container, Rest...>::type {
  return makeZipTupleIterator(makeZipWrapperIterator(container.begin(), container.end()), zipIterator(rest...));
}

// END ZIP

// RANGE

namespace RangeHelper {

  template <typename Diff>
  auto checkIfDiffLessThanZero(Diff) -> bool requires std::is_unsigned_v<Diff> {
    return false;
  }

  template <typename Diff>
  auto checkIfDiffLessThanZero(Diff diff) -> bool requires (!std::is_unsigned_v<Diff>) {
    return diff < 0;
  }
}

using RangeException = ExceptionDerived<"RangeException">;

template <typename Value, typename Diff = int>
class RangeIterator {
  using iterator_category = std::random_access_iterator_tag;
  using value_type = Value;
  using difference_type = Diff;
  using pointer = Value*;
  using reference = Value&;

public:
  RangeIterator() : m_start(), m_end(), m_diff(1), m_current(), m_stop(true) {}

  RangeIterator(Value min, Value max, Diff diff)
    : m_start(min), m_end(max), m_diff(diff), m_current(min), m_stop(false) {
    sanity();
  }

  RangeIterator(Value min, Value max) : m_start(min), m_end(max), m_diff(1), m_current(min), m_stop(false) {
    sanity();
  }

  RangeIterator(Value max) : m_start(), m_end(max), m_diff(1), m_current(), m_stop(false) {
    sanity();
  }

  RangeIterator(RangeIterator const& rhs) {
    copy(rhs);
  }

  auto operator=(RangeIterator const& rhs) -> RangeIterator& {
    copy(rhs);
    return *this;
  }

  auto operator+=(Diff steps) -> RangeIterator& {
    if ((applySteps(m_current, m_diff * steps) >= m_end) != (RangeHelper::checkIfDiffLessThanZero<Diff>(m_diff))) {
      if (!m_stop) {
        Diff stepsLeft = stepsBetween(m_current, m_end);
        m_current = applySteps(m_current, stepsLeft * m_diff);
        m_stop = true;
      }
    } else {
      m_current = applySteps(m_current, steps * m_diff);
    }
    return *this;
  }

  auto operator-=(Diff steps) -> RangeIterator {
    m_stop = false;
    sanity();

    if (applySteps(m_current, -(m_diff * steps)) < m_start)
      m_current = m_start;
    else
      m_current = applySteps(m_current, -(m_diff * steps));

    return *this;
  }

  auto operator*() const -> Value {
    return m_current;
  }

  auto operator->() const -> Value const* {
    return &m_current;
  }

  auto operator[](unsigned rhs) const -> Value {
    // Should return at maximum, the value that this iterator will normally
    // reach when at end().
    rhs = std::min(rhs, stepsBetween(m_start, m_end) + 1);
    return m_start + rhs * m_diff;
  }

  auto operator++() -> RangeIterator& {
    return operator+=(1);
  }

  auto operator--() -> RangeIterator& {
    return operator-=(1);
  }

  auto operator++(int) -> RangeIterator {
    RangeIterator tmp(*this);
    ++(*this);
    return tmp;
  }

  auto operator--(int) -> RangeIterator {
    RangeIterator tmp(*this);
    --(*this);
    return tmp;
  }

  auto operator+(Diff steps) const -> RangeIterator {
    RangeIterator copy(*this);
    copy += steps;
    return copy;
  }

  auto operator-(Diff steps) const -> RangeIterator {
    RangeIterator copy(*this);
    copy -= steps;
    return copy;
  }

  auto operator-(RangeIterator const& rhs) const -> int {
    if (!sameClass(rhs))
      throw RangeException("Attempted to subtract incompatible ranges.");

    return stepsBetween(rhs.m_current, m_current);
  }

  friend auto operator+(Diff lhs, RangeIterator const& rhs) -> RangeIterator {
    return rhs + lhs;
  }

  friend auto operator-(Diff lhs, RangeIterator const& rhs) -> RangeIterator {
    return rhs - lhs;
  }

  auto operator==(RangeIterator const& rhs) const -> bool {
    return (sameClass(rhs) && m_current == rhs.m_current && m_stop == rhs.m_stop);
  }

  auto operator!=(RangeIterator const& rhs) const -> bool {
    return !(*this == rhs);
  }

  auto operator<(RangeIterator const& rhs) const -> bool {
    return std::tie(m_start, m_end, m_diff, m_current) < std::tie(rhs.m_start, rhs.m_end, rhs.m_diff, rhs.m_current);
  }

  auto operator<=(RangeIterator const& rhs) const -> bool {
    return (*this == rhs) || (*this < rhs);
  }

  auto operator>=(RangeIterator const& rhs) const -> bool {
    return !(*this < rhs);
  }

  auto operator>(RangeIterator const& rhs) const -> bool {
    return !(*this <= rhs);
  }

  auto begin() const -> RangeIterator {
    return RangeIterator(m_start, m_end, m_diff);
  }

  auto end() const -> RangeIterator {
    Diff steps = stepsBetween(m_start, m_end);
    RangeIterator res(m_start, m_end, m_diff);
    res += steps;
    return res;
  }

private:
  void copy(RangeIterator const& copy) {
    m_start = copy.m_start;
    m_end = copy.m_end;
    m_diff = copy.m_diff;
    m_current = copy.m_current;
    m_stop = copy.m_stop;
    sanity();
  }

  void sanity() {
    if (m_diff == 0)
      throw RangeException("Invalid difference in range function.");

    if ((m_end < m_start) != (RangeHelper::checkIfDiffLessThanZero<Diff>(m_diff))) {
      if (RangeHelper::checkIfDiffLessThanZero<Diff>(m_diff))
        throw RangeException("Start cannot be less than end if diff is negative.");
      throw RangeException("Max cannot be less than min.");
    }

    if (m_end == m_start)
      m_stop = true;
  }

  auto sameClass(RangeIterator const& rhs) const -> bool {
    return m_start == rhs.m_start && m_end == rhs.m_end && m_diff == rhs.m_diff;
  }

  auto stepsBetween(Value start, Value end) const -> Diff {
    return ((Diff)end - (Diff)start) / m_diff;
  }

  auto applySteps(Value start, Diff travel) const -> Value {
    return (Value)((Diff)start + travel);
  }

  Value m_start;
  Value m_end;
  Diff m_diff;

  Value m_current;

  bool m_stop;
};

template <typename Numeric, typename Diff>
auto range(Numeric min, Numeric max, Diff diff) -> RangeIterator<Numeric, Diff> {
  return RangeIterator<Numeric, Diff>(min, max, diff);
}

template <typename Numeric, typename Diff = int>
auto range(Numeric max) -> RangeIterator<Numeric, Diff> {
  return RangeIterator<Numeric, Diff>(max);
}

template <typename Numeric, typename Diff = int>
auto range(Numeric min, Numeric max) -> RangeIterator<Numeric, Diff> {
  return RangeIterator<Numeric, Diff>(min, max);
}

template <typename Numeric, typename Diff>
auto rangeInclusive(Numeric min, Numeric max, Diff diff) -> RangeIterator<Numeric, Diff> {
  return RangeIterator<Numeric, Diff>(min, (Numeric)((Diff)max + 1), diff);
}

template <typename Numeric, typename Diff = int>
auto rangeInclusive(Numeric max) -> RangeIterator<Numeric, Diff> {
  return RangeIterator<Numeric, Diff>((Numeric)((Diff)max + 1));
}

template <typename Numeric, typename Diff = int>
auto rangeInclusive(Numeric min, Numeric max) -> RangeIterator<Numeric, Diff> {
  return RangeIterator<Numeric, Diff>(min, (Numeric)((Diff)max + 1));
}

// END RANGE

// Wraps a forward-iterator to produce {value, index} pairs, similar to
// python's enumerate()
template <typename Iterator>
struct EnumerateIterator {
private:
  Iterator current;
  Iterator last;
  std::size_t index;
  bool atEnd;

public:
  using IteratorValue = decltype(*std::declval<Iterator>());
  using value_type = std::pair<IteratorValue&, std::size_t>;

  EnumerateIterator() : index(0), atEnd(true) {}

  EnumerateIterator(Iterator begin, Iterator end) : current(begin), last(end), index(0) {
    atEnd = current == last;
  }

  auto begin() const -> EnumerateIterator {
    return *this;
  }

  auto end() const -> EnumerateIterator {
    return EnumerateIterator();
  }

  auto operator++() -> EnumerateIterator {
    if (!atEnd) {
      ++current;
      ++index;

      atEnd = current == last;
    }

    return *this;
  }

  auto operator*() const -> value_type {
    return {*current, index};
  }

  auto operator==(EnumerateIterator const& rhs) const -> bool {
    return (atEnd && rhs.atEnd) || (!atEnd && !rhs.atEnd && current == rhs.current && last == rhs.last);
  }

  auto operator!=(EnumerateIterator const& rhs) const -> bool {
    return !(*this == rhs);
  }

  explicit operator bool() const {
    return !atEnd;
  }
};

template <typename Iterable>
auto enumerateIterator(Iterable& list) -> EnumerateIterator<decltype(std::declval<Iterable>().begin())> {
  return EnumerateIterator<decltype(std::declval<Iterable>().begin())>(list.begin(), list.end());
}

template <typename ResultContainer, typename Iterable>
auto enumerateConstruct(Iterable&& list) -> ResultContainer {
  ResultContainer res;
  for (auto el : enumerateIterator(list))
    res.push_back(std::move(el));

  return res;
}

}
