#pragma once

#include "StarException.hpp"

import std;

namespace Star {

using IteratorException = ExceptionDerived<"IteratorException">;

// Provides java style iterators for bidirectional list-like containers
// (SIterator and SMutableIterator) and forward only map-like containers
// (SMapIterator and SMutableMapIterator)
template <typename Container>
class SIterator {
public:
  using iterator = typename Container::const_iterator;
  using value_ref = decltype(*iterator());

  SIterator(Container const& c) : cont(c) {
    toFront();
  }

  void toFront() {
    curr = cont.begin();
    direction = 0;
  }

  void toBack() {
    curr = cont.end();
    direction = 0;
  }

  [[nodiscard]] auto hasNext() const -> bool {
    return curr != cont.end();
  }

  [[nodiscard]] auto hasPrevious() const -> bool {
    return curr != cont.begin();
  }

  auto value() const -> value_ref {
    if (direction == 1) {
      if (curr != cont.end() && cont.size() != 0)
        return *curr;
      else
        throw IteratorException("value() called on end()");
    } else if (direction == -1) {
      if (curr != cont.begin() && cont.size() != 0) {
        iterator back = curr;
        return *(--back);
      } else {
        throw IteratorException("value() called on begin()");
      }
    } else {
      throw IteratorException("value() called without previous next() or previous()");
    }
  }

  auto next() -> value_ref {
    if (hasNext()) {
      direction = -1;
      return *(curr++);
    }
    throw IteratorException("next() called on end");
  }

  auto previous() -> value_ref {
    if (hasPrevious()) {
      direction = 1;
      return *(--curr);
    }
    throw IteratorException("prev() called on beginning");
  }

  auto peekNext() const -> value_ref {
    SIterator t = *this;
    return t.next();
  }

  auto peekPrevious() const -> value_ref {
    SIterator t = *this;
    return t.previous();
  }

  [[nodiscard]] auto distFront() const -> std::size_t {
    return std::distance(cont.begin(), curr);
  }

  [[nodiscard]] auto distBack() const -> std::size_t {
    return std::distance(curr, cont.end());
  }

private:
  auto operator=(iterator const& i) -> SIterator& {
    return iterator::operator=(i);
  }
  Container const& cont;
  iterator curr;

  int direction;
};

template <typename Container>
auto makeSIterator(Container const& c) -> SIterator<Container> {
  return SIterator<Container>(c);
}

template <typename Container>
class SMutableIterator {
public:
  using value_type = typename Container::value_type;
  using iterator = typename Container::iterator;
  using value_ref = decltype(*iterator());

  SMutableIterator(Container& c) : cont(c) {
    toFront();
  }

  void toFront() {
    curr = cont.begin();
    direction = 0;
  }

  void toBack() {
    curr = cont.end();
    direction = 0;
  }

  [[nodiscard]] auto hasNext() const -> bool {
    return curr != cont.end();
  }

  [[nodiscard]] auto hasPrevious() const -> bool {
    return curr != cont.begin();
  }

  void insert(value_type v) {
    curr = ++cont.insert(curr, std::move(v));
    direction = -1;
  }

  void remove() {
    if (direction == 1) {
      direction = 0;
      if (curr != cont.end() && cont.size() != 0)
        curr = cont.erase(curr);
      else
        throw IteratorException("remove() called on end()");
    } else if (direction == -1) {
      direction = 0;
      if (curr != cont.begin() && cont.size() != 0)
        curr = cont.erase(--curr);
      else
        throw IteratorException("remove() called on begin()");
    } else {
      throw IteratorException("remove() called without previous next() or previous()");
    }
  }

  auto value() const -> value_ref {
    if (direction == 1) {
      if (curr != cont.end() && cont.size() != 0)
        return *curr;
      else
        throw IteratorException("value() called on end()");
    } else if (direction == -1) {
      if (curr != cont.begin() && cont.size() != 0) {
        iterator back = curr;
        return *(--back);
      } else {
        throw IteratorException("value() called on begin()");
      }
    } else {
      throw IteratorException("value() called without previous next() or previous()");
    }
  }

  void setValue(value_type v) const {
    value() = std::move(v);
  }

  auto next() -> value_ref {
    if (curr == cont.end())
      throw IteratorException("next() called on end");
    direction = -1;
    return *curr++;
  }

  auto previous() -> value_ref {
    if (curr == cont.begin())
      throw IteratorException("previous() called on begin");
    direction = 1;
    return *--curr;
  }

  auto peekNext() const -> value_ref {
    SMutableIterator n = *this;
    return n.next();
  }

  auto peekPrevious() const -> value_ref {
    SMutableIterator n = *this;
    return n.previous();
  }

  [[nodiscard]] auto distFront() const -> std::size_t {
    return std::distance(cont.begin(), curr);
  }

  [[nodiscard]] auto distBack() const -> std::size_t {
    return std::distance(curr, cont.end());
  }

private:
  auto operator=(iterator const& i) -> SMutableIterator& {
    return iterator::operator=(i);
  }

  Container& cont;
  iterator curr;

  // -1 means remove() will remove --cur, +1 means ++cur, 0 means remove() is
  // invalid.
  int direction;
};

template <typename Container>
auto makeSMutableIterator(Container& c) -> SMutableIterator<Container> {
  return SMutableIterator<Container>(c);
}

template <typename Container>
class SMapIterator {
public:
  using key_type = typename Container::key_type;
  using mapped_type = typename Container::mapped_type;

  using iterator = typename Container::const_iterator;
  using value_ref = decltype(*iterator());

  SMapIterator(Container const& c) : cont(c) {
    toFront();
  }

  void toFront() {
    curr = cont.end();
  }

  void toBack() {
    curr = cont.end();
    if (curr != cont.begin())
      --curr;
  }

  [[nodiscard]] auto hasNext() const -> bool {
    iterator end = cont.end();
    if (curr == end)
      return cont.begin() != end;
    else
      return ++iterator(curr) != end;
  }

  auto key() const -> key_type const& {
    if (curr != cont.end()) {
      return curr->first;
    } else {
      throw IteratorException("key() called on begin()");
    }
  }

  auto value() const -> mapped_type const& {
    if (curr != cont.end()) {
      return curr->second;
    } else {
      throw IteratorException("value() called on begin()");
    }
  }

  auto next() -> value_ref const& {
    if (hasNext()) {
      if (curr == cont.end())
        curr = cont.begin();
      else
        ++curr;
      return *curr;
    }
    throw IteratorException("next() called on end");
  }

  auto peekNext() const -> value_ref {
    SMapIterator t = *this;
    return t.next();
  }

  [[nodiscard]] auto distFront() const -> std::size_t {
    return std::distance(cont.begin(), curr);
  }

  [[nodiscard]] auto distBack() const -> std::size_t {
    return std::distance(curr, cont.end()) - 1;
  }

protected:
  auto operator=(iterator const& i) -> SMapIterator& {
    return iterator::operator=(i);
  }
  Container const& cont;
  iterator curr;
};

template <typename Container>
auto makeSMapIterator(Container const& c) -> SMapIterator<Container> {
  return SMapIterator<Container>(c);
}

template <typename Container>
class SMutableMapIterator {
public:
  using key_type = typename Container::key_type;
  using mapped_type = typename Container::mapped_type;

  using iterator = typename Container::iterator;
  using value_ref = decltype(*iterator());

  SMutableMapIterator(Container& c) : cont(c) {
    toFront();
  }

  void toFront() {
    curr = cont.end();
    remCalled = false;
  }

  void toBack() {
    curr = cont.end();
    if (curr != cont.begin())
      --curr;
  }

  [[nodiscard]] auto hasNext() const -> bool {
    iterator end = cont.end();
    if (curr == end)
      return cont.begin() != end && !remCalled;
    else if (remCalled)
      return curr != end;
    else
      return ++iterator(curr) != end;
  }

  auto key() const -> key_type const& {
    if (remCalled)
      throw IteratorException("key() called after remove()");
    else if (curr != cont.end())
      return curr->first;
    else
      throw IteratorException("key() called on begin()");
  }

  auto value() const -> mapped_type& {
    if (remCalled)
      throw IteratorException("value() called after remove()");
    else if (curr != cont.end())
      return curr->second;
    else
      throw IteratorException("value() called on begin()");
  }

  auto next() -> value_ref {
    if (hasNext()) {
      if (curr == cont.end())
        curr = cont.begin();
      else if (remCalled)
        remCalled = false;
      else
        ++curr;

      return *curr;
    } else {
      throw IteratorException("next() called on end");
    }
  }

  auto peekNext() const -> value_ref {
    SMutableMapIterator t = *this;
    return t.next();
  }

  void remove() {
    if (remCalled) {
      throw IteratorException("remove() called twice");
    } else if (curr == cont.end()) {
      throw IteratorException("remove() called at front");
    } else {
      if (curr == cont.begin()) {
        cont.erase(curr);
        curr = cont.end();
      } else {
        curr = cont.erase(curr);
        remCalled = true;
      }
    }
  }

  [[nodiscard]] auto distFront() const -> std::size_t {
    if (curr == cont.end())
      return 0;
    else
      return std::distance(cont.begin(), curr) - (remCalled ? 1 : 0);
  }

  [[nodiscard]] auto distBack() const -> std::size_t {
    if (curr == cont.end())
      return cont.size();
    else
      return std::distance(curr, cont.end()) - 1 + (remCalled ? 1 : 0);
  }

private:
  auto operator=(iterator const& i) -> SMutableMapIterator& {
    return iterator::operator=(i);
  }

  Container& cont;
  iterator curr;
  bool remCalled;
};

template <typename Container>
auto makeSMutableMapIterator(Container& c) -> SMutableMapIterator<Container> {
  return SMutableMapIterator<Container>(c);
}

}
