#pragma once

#include "StarException.hpp"
#include "StarFormat.hpp"
#include "StarHash.hpp"
#include "StarPythonic.hpp"
#include "StarSmallVector.hpp"
#include "StarStaticVector.hpp"

import std;

namespace Star {

template <typename BaseList>
class ListMixin : public BaseList {
public:
  using Base = BaseList;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;
  using value_type = typename Base::value_type;
  using reference = typename Base::reference;
  using const_reference = typename Base::const_reference;

  ListMixin();
  ListMixin(Base const& list);
  ListMixin(Base&& list);
  ListMixin(value_type const* p, std::size_t count);
  template <typename InputIterator>
  ListMixin(InputIterator beg, InputIterator end);
  explicit ListMixin(std::size_t len, const_reference s1 = value_type());
  ListMixin(std::initializer_list<value_type> list);

  void append(value_type e);

  template <typename Container>
  void appendAll(Container&& list);

  template <class... Args>
  auto emplaceAppend(Args&&... args) -> reference;

  auto first() -> reference;
  auto first() const -> const_reference;

  auto last() -> reference;
  auto last() const -> const_reference;

  auto maybeFirst() -> std::optional<value_type>;
  auto maybeLast() -> std::optional<value_type>;

  void removeLast();
  auto takeLast() -> value_type;

  auto maybeTakeLast() -> std::optional<value_type>;

  // Limit the size of the list by removing elements from the back until the
  // size is the maximumSize or less.
  void limitSizeBack(std::size_t maximumSize);

  [[nodiscard]] auto count() const -> std::size_t;

  auto contains(const_reference e) const -> bool;
  // Remove all equal to element, returns number removed.
  auto remove(const_reference e) -> std::size_t;

  template <typename Filter>
  void filter(Filter&& filter);

  template <typename Comparator>
  void insertSorted(value_type e, Comparator&& comparator);
  void insertSorted(value_type e);

  // Returns true if this *sorted* list contains the given element.
  template <typename Comparator>
  auto containsSorted(value_type const& e, Comparator&& comparator) -> bool;
  auto containsSorted(value_type e) -> bool;

  template <typename Function>
  void exec(Function&& function);

  template <typename Function>
  void exec(Function&& function) const;

  template <typename Function>
  void transform(Function&& function);

  template <typename Function>
  auto any(Function&& function) const -> bool;
  [[nodiscard]] auto any() const -> bool;

  template <typename Function>
  auto all(Function&& function) const -> bool;
  [[nodiscard]] auto all() const -> bool;
};

template <typename List>
class ListHasher {
public:
  auto operator()(List const& l) const -> std::size_t;

private:
  hash<typename List::value_type> elemHasher;
};

template <typename BaseList>
class RandomAccessListMixin : public BaseList {
public:
  using Base = BaseList;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;
  using value_type = typename Base::value_type;
  using reference = typename Base::reference;
  using const_reference = typename Base::const_reference;

  using Base::Base;

  template <typename Comparator>
  void sort(Comparator&& comparator);
  void sort();

  void reverse();

  // Returns first index of given element, std::numeric_limits<std::size_t>::max() if not found.
  auto indexOf(const_reference e, std::size_t from = 0) const -> std::size_t;
  // Returns last index of given element, std::numeric_limits<std::size_t>::max() if not found.
  auto lastIndexOf(const_reference e, std::size_t til = std::numeric_limits<std::size_t>::max()) const -> std::size_t;

  auto at(std::size_t n) const -> const_reference;
  auto at(std::size_t n) -> reference;

  auto operator[](std::size_t n) const -> const_reference;
  auto operator[](std::size_t n) -> reference;

  // Does not throw if n is beyond end of list, instead returns def
  auto get(std::size_t n, value_type def = value_type()) const -> value_type;

  auto takeAt(std::size_t i) -> value_type;

  // Same as at, but wraps around back to the beginning
  // (throws if list is empty)
  auto wrap(std::size_t n) const -> const_reference;
  auto wrap(std::size_t n) -> reference;

  // Does not throw if list is empty
  auto wrap(std::size_t n, value_type def) const -> value_type;

  void eraseAt(std::size_t index);
  // Erases region from begin to end, not including end.
  void eraseAt(std::size_t begin, std::size_t end);

  void insertAt(std::size_t pos, value_type e);

  template <typename Container>
  void insertAllAt(std::size_t pos, Container const& l);

  // Ensures that list is large enough to hold pos elements.
  void set(std::size_t pos, value_type e);

  void swap(std::size_t i, std::size_t j);
  // same as insert(to, takeAt(from))
  void move(std::size_t from, std::size_t to);
};

template <typename BaseList>
class FrontModifyingListMixin : public BaseList {
public:
  using Base = BaseList;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;
  using value_type = typename Base::value_type;
  using reference = typename Base::reference;
  using const_reference = typename Base::const_reference;

  using Base::Base;

  void prepend(value_type e);

  template <typename Container>
  void prependAll(Container&& list);

  template <class... Args>
  auto emplacePrepend(Args&&... args) -> reference;

  void removeFirst();
  auto takeFirst() -> value_type;

  // Limit the size of the list by removing elements from the front until the
  // size is the maximumSize or less.
  void limitSizeFront(std::size_t maximumSize);
};

template <typename Element, typename Allocator = std::allocator<Element>>
class List : public RandomAccessListMixin<ListMixin<std::vector<Element, Allocator>>> {
public:
  using Base = RandomAccessListMixin<ListMixin<std::vector<Element, Allocator>>>;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;
  using value_type = typename Base::value_type;
  using reference = typename Base::reference;
  using const_reference = typename Base::const_reference;

  template <typename Container>
  static auto from(Container const& c) -> List;

  using Base::Base;

  // Pointer to contiguous storage, returns nullptr if empty
  auto ptr() -> value_type*;
  auto ptr() const -> value_type const*;

  auto slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const -> List;

  template <typename Filter>
  auto filtered(Filter&& filter) const -> List;

  template <typename Comparator>
  auto sorted(Comparator&& comparator) const -> List;
  auto sorted() const -> List;

  template <typename Function>
  auto transformed(Function&& function);

  template <typename Function>
  auto transformed(Function&& function) const;
};

template <typename Element, typename Allocator>
struct hash<List<Element, Allocator>> : public ListHasher<List<Element, Allocator>> {};

template <typename Element, std::size_t MaxSize>
class StaticList : public RandomAccessListMixin<ListMixin<StaticVector<Element, MaxSize>>> {
public:
  using Base = RandomAccessListMixin<ListMixin<StaticVector<Element, MaxSize>>>;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;
  using value_type = typename Base::value_type;
  using reference = typename Base::reference;
  using const_reference = typename Base::const_reference;

  template <typename Container>
  static auto from(Container const& c) -> StaticList;

  using Base::Base;

  auto slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const -> StaticList;

  template <typename Filter>
  auto filtered(Filter&& filter) const -> StaticList;

  template <typename Comparator>
  auto sorted(Comparator&& comparator) const -> StaticList;
  auto sorted() const -> StaticList;

  template <typename Function>
  auto transformed(Function&& function);

  template <typename Function>
  auto transformed(Function&& function) const;
};

template <typename Element, std::size_t MaxStackSize>
struct hash<StaticList<Element, MaxStackSize>> : public ListHasher<StaticList<Element, MaxStackSize>> {};

template <typename Element, std::size_t MaxStackSize>
class SmallList : public RandomAccessListMixin<ListMixin<SmallVector<Element, MaxStackSize>>> {
public:
  using Base = RandomAccessListMixin<ListMixin<SmallVector<Element, MaxStackSize>>>;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;
  using value_type = typename Base::value_type;
  using reference = typename Base::reference;
  using const_reference = typename Base::const_reference;

  template <typename Container>
  static auto from(Container const& c) -> SmallList;

  using Base::Base;

  auto slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const -> SmallList;

  template <typename Filter>
  auto filtered(Filter&& filter) const -> SmallList;

  template <typename Comparator>
  auto sorted(Comparator&& comparator) const -> SmallList;
  auto sorted() const -> SmallList;

  template <typename Function>
  auto transformed(Function&& function);

  template <typename Function>
  auto transformed(Function&& function) const;
};

template <typename Element, std::size_t MaxStackSize>
struct hash<SmallList<Element, MaxStackSize>> : public ListHasher<SmallList<Element, MaxStackSize>> {};

template <typename Element, typename Allocator = std::allocator<Element>>
class Deque : public FrontModifyingListMixin<RandomAccessListMixin<ListMixin<std::deque<Element, Allocator>>>> {
public:
  using Base = FrontModifyingListMixin<RandomAccessListMixin<ListMixin<std::deque<Element, Allocator>>>>;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;
  using value_type = typename Base::value_type;
  using reference = typename Base::reference;
  using const_reference = typename Base::const_reference;

  template <typename Container>
  static auto from(Container const& c) -> Deque;

  using Base::Base;

  auto slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const -> Deque;

  template <typename Filter>
  auto filtered(Filter&& filter) const -> Deque;

  template <typename Comparator>
  auto sorted(Comparator&& comparator) const -> Deque;
  auto sorted() const -> Deque;

  template <typename Function>
  auto transformed(Function&& function);

  template <typename Function>
  auto transformed(Function&& function) const;
};

template <typename Element, typename Allocator>
struct hash<Deque<Element, Allocator>> : public ListHasher<Deque<Element, Allocator>> {};

template <typename Element, typename Allocator = std::allocator<Element>>
class LinkedList : public FrontModifyingListMixin<ListMixin<std::list<Element, Allocator>>> {
public:
  using Base = FrontModifyingListMixin<ListMixin<std::list<Element, Allocator>>>;

  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;
  using value_type = typename Base::value_type;
  using reference = typename Base::reference;
  using const_reference = typename Base::const_reference;

  template <typename Container>
  static auto from(Container const& c) -> LinkedList;

  using Base::Base;

  void appendAll(LinkedList list);
  void prependAll(LinkedList list);

  template <typename Container>
  void appendAll(Container&& list);
  template <typename Container>
  void prependAll(Container&& list);

  template <typename Filter>
  auto filtered(Filter&& filter) const -> LinkedList;

  template <typename Comparator>
  auto sorted(Comparator&& comparator) const -> LinkedList;
  auto sorted() const -> LinkedList;

  template <typename Function>
  auto transformed(Function&& function);

  template <typename Function>
  auto transformed(Function&& function) const;
};

template <typename Element, typename Allocator>
struct hash<LinkedList<Element, Allocator>> : public ListHasher<LinkedList<Element, Allocator>> {};

template <typename BaseList>
auto operator<<(std::ostream& os, ListMixin<BaseList> const& list) -> std::ostream&;

template <typename... Containers>
struct ListZipTypes {
  using Tuple = std::tuple<typename std::decay<Containers>::type::value_type...>;
  using Result = List<Tuple>;
};

template <typename... Containers>
auto zip(Containers&&... args) -> typename ListZipTypes<Containers...>::Result;

template <typename Container>
struct ListEnumerateTypes {
  using Pair = std::pair<typename std::decay<Container>::type::value_type, std::size_t>;
  using Result = List<Pair>;
};

template <typename Container>
auto enumerate(Container&& container) -> typename ListEnumerateTypes<Container>::Result;

template <typename BaseList>
ListMixin<BaseList>::ListMixin()
    : Base() {}

template <typename BaseList>
ListMixin<BaseList>::ListMixin(Base const& list)
    : Base(list) {}

template <typename BaseList>
ListMixin<BaseList>::ListMixin(Base&& list)
    : Base(std::move(list)) {}

template <typename BaseList>
ListMixin<BaseList>::ListMixin(std::size_t len, const_reference s1)
    : Base(len, s1) {}

template <typename BaseList>
ListMixin<BaseList>::ListMixin(value_type const* p, std::size_t count)
    : Base(p, p + count) {}

template <typename BaseList>
template <typename InputIterator>
ListMixin<BaseList>::ListMixin(InputIterator beg, InputIterator end)
    : Base(beg, end) {}

template <typename BaseList>
ListMixin<BaseList>::ListMixin(std::initializer_list<value_type> list) {
  // In case underlying class type doesn't support initializer_list
  for (auto& e : list)
    append(std::move(e));
}

template <typename BaseList>
void ListMixin<BaseList>::append(value_type e) {
  Base::push_back(std::move(e));
}

template <typename BaseList>
template <typename Container>
void ListMixin<BaseList>::appendAll(Container&& list) {
  for (auto& e : list) {
    if (std::is_rvalue_reference_v<Container&&>)
      Base::push_back(std::move(e));
    else
      Base::push_back(e);
  }
}

template <typename BaseList>
template <class... Args>
auto ListMixin<BaseList>::emplaceAppend(Args&&... args) -> reference {
  Base::emplace_back(std::forward<Args>(args)...);
  return *prev(Base::end());
}

template <typename BaseList>
auto ListMixin<BaseList>::first() -> reference {
  if (Base::empty())
    throw OutOfRangeException("first() called on empty list");
  return *Base::begin();
}

template <typename BaseList>
auto ListMixin<BaseList>::first() const -> const_reference {
  if (Base::empty())
    throw OutOfRangeException("first() called on empty list");
  return *Base::begin();
}

template <typename BaseList>
auto ListMixin<BaseList>::last() -> reference {
  if (Base::empty())
    throw OutOfRangeException("last() called on empty list");
  return *prev(Base::end());
}

template <typename BaseList>
auto ListMixin<BaseList>::last() const -> const_reference {
  if (Base::empty())
    throw OutOfRangeException("last() called on empty list");
  return *prev(Base::end());
}

template <typename BaseList>
auto ListMixin<BaseList>::maybeFirst() -> std::optional<value_type> {
  if (Base::empty())
    return std::nullopt;
  return Base::front();
}

template <typename BaseList>
auto ListMixin<BaseList>::maybeLast() -> std::optional<value_type> {
  if (Base::empty())
    return std::nullopt;
  return Base::back();
}

template <typename BaseList>
void ListMixin<BaseList>::removeLast() {
  if (Base::empty())
    throw OutOfRangeException("removeLast() called on empty list");
  Base::pop_back();
}

template <typename BaseList>
auto ListMixin<BaseList>::takeLast() -> value_type {
  value_type e = std::move(last());
  Base::pop_back();
  return e;
}

template <typename BaseList>
auto ListMixin<BaseList>::maybeTakeLast() -> std::optional<value_type> {
  if (Base::empty())
    return std::nullopt;
  value_type e = std::move(Base::back());
  Base::pop_back();
  return e;
}

template <typename BaseList>
void ListMixin<BaseList>::limitSizeBack(std::size_t maximumSize) {
  while (Base::size() > maximumSize)
    Base::pop_back();
}

template <typename BaseList>
auto ListMixin<BaseList>::count() const -> std::size_t {
  return Base::size();
}

template <typename BaseList>
auto ListMixin<BaseList>::contains(const_reference e) const -> bool {
  for (auto const& r : *this) {
    if (r == e)
      return true;
  }
  return false;
}

template <typename BaseList>
auto ListMixin<BaseList>::remove(const_reference e) -> std::size_t {
  std::size_t removed = 0;
  auto i = Base::begin();
  while (i != Base::end()) {
    if (*i == e) {
      ++removed;
      i = Base::erase(i);
    } else {
      ++i;
    }
  }
  return removed;
}

template <typename BaseList>
template <typename Filter>
void ListMixin<BaseList>::filter(Filter&& filter) {
  Star::filter(*this, std::forward<Filter>(filter));
}

template <typename BaseList>
template <typename Comparator>
void ListMixin<BaseList>::insertSorted(value_type e, Comparator&& comparator) {
  auto i = std::upper_bound(Base::begin(), Base::end(), e, std::forward<Comparator>(comparator));
  Base::insert(i, std::move(e));
}

template <typename BaseList>
void ListMixin<BaseList>::insertSorted(value_type e) {
  auto i = std::upper_bound(Base::begin(), Base::end(), e);
  Base::insert(i, std::move(e));
}

template <typename BaseList>
template <typename Comparator>
auto ListMixin<BaseList>::containsSorted(value_type const& e, Comparator&& comparator) -> bool {
  auto range = std::equal_range(Base::begin(), Base::end(), e, std::forward<Comparator>(comparator));
  return range.first != range.second;
}

template <typename BaseList>
auto ListMixin<BaseList>::containsSorted(value_type e) -> bool {
  auto range = std::equal_range(Base::begin(), Base::end(), e);
  return range.first != range.second;
}

template <typename BaseList>
template <typename Function>
void ListMixin<BaseList>::exec(Function&& function) {
  for (auto& e : *this)
    function(e);
}

template <typename BaseList>
template <typename Function>
void ListMixin<BaseList>::exec(Function&& function) const {
  for (auto const& e : *this)
    function(e);
}

template <typename BaseList>
template <typename Function>
void ListMixin<BaseList>::transform(Function&& function) {
  for (auto& e : *this)
    e = function(e);
}

template <typename BaseList>
template <typename Function>
auto ListMixin<BaseList>::any(Function&& function) const -> bool {
  return Star::any(*this, std::forward<Function>(function));
}

template <typename BaseList>
auto ListMixin<BaseList>::any() const -> bool {
  return Star::any(*this);
}

template <typename BaseList>
template <typename Function>
auto ListMixin<BaseList>::all(Function&& function) const -> bool {
  return Star::all(*this, std::forward<Function>(function));
}

template <typename BaseList>
auto ListMixin<BaseList>::all() const -> bool {
  return Star::all(*this);
}

template <typename BaseList>
template <typename Comparator>
void RandomAccessListMixin<BaseList>::sort(Comparator&& comparator) {
  Star::sort(*this, std::forward<Comparator>(comparator));
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::sort() {
  Star::sort(*this);
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::reverse() {
  Star::reverse(*this);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::indexOf(const_reference e, std::size_t from) const -> std::size_t {
  for (std::size_t i = from; i < Base::size(); ++i)
    if (operator[](i) == e)
      return i;
  return std::numeric_limits<std::size_t>::max();
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::lastIndexOf(const_reference e, std::size_t til) const -> std::size_t {
  std::size_t index = std::numeric_limits<std::size_t>::max();
  std::size_t end = std::min(Base::size(), til);
  for (std::size_t i = 0; i < end; ++i) {
    if (operator[](i) == e)
      index = i;
  }
  return index;
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::at(std::size_t n) const -> const_reference {
  if (n >= Base::size())
    throw OutOfRangeException(strf(std::string_view("out of range list::at({})"), n));
  return operator[](n);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::at(std::size_t n) -> reference {
  if (n >= Base::size())
    throw OutOfRangeException(strf(std::string_view("out of range list::at({})"), n));
  return operator[](n);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::operator[](std::size_t n) const -> const_reference {
  return Base::operator[](n);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::operator[](std::size_t n) -> reference {
  return Base::operator[](n);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::get(std::size_t n, value_type def) const -> value_type {
  if (n >= BaseList::size())
    return def;
  return operator[](n);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::takeAt(std::size_t i) -> value_type {
  value_type e = at(i);
  Base::erase(Base::begin() + i);
  return e;
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::wrap(std::size_t n) const -> const_reference {
  if (BaseList::empty())
    throw OutOfRangeException();
  else
    return operator[](n % BaseList::size());
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::wrap(std::size_t n) -> reference {
  if (BaseList::empty())
    throw OutOfRangeException();
  else
    return operator[](n % BaseList::size());
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::wrap(std::size_t n, value_type def) const -> value_type {
  if (BaseList::empty())
    return def;
  else
    return operator[](n % BaseList::size());
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::eraseAt(std::size_t i) {
  Base::erase(Base::begin() + i);
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::eraseAt(std::size_t b, std::size_t e) {
  Base::erase(Base::begin() + b, Base::begin() + e);
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::insertAt(std::size_t pos, value_type e) {
  Base::insert(Base::begin() + pos, std::move(e));
}

template <typename BaseList>
template <typename Container>
void RandomAccessListMixin<BaseList>::insertAllAt(std::size_t pos, Container const& l) {
  Base::insert(Base::begin() + pos, l.begin(), l.end());
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::set(std::size_t pos, value_type e) {
  if (pos >= Base::size())
    Base::resize(pos + 1);
  operator[](pos) = std::move(e);
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::swap(std::size_t i, std::size_t j) {
  std::swap(operator[](i), operator[](j));
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::move(std::size_t from, std::size_t to) {
  Base::insert(to, takeAt(from));
}

template <typename BaseList>
void FrontModifyingListMixin<BaseList>::prepend(value_type e) {
  Base::push_front(std::move(e));
}

template <typename BaseList>
template <typename Container>
void FrontModifyingListMixin<BaseList>::prependAll(Container&& list) {
  for (auto i = std::rbegin(list); i != std::rend(list); ++i) {
    if (std::is_rvalue_reference_v<Container&&>)
      Base::push_front(std::move(*i));
    else
      Base::push_front(*i);
  }
}

template <typename BaseList>
template <class... Args>
auto FrontModifyingListMixin<BaseList>::emplacePrepend(Args&&... args) -> reference {
  Base::emplace_front(std::forward<Args>(args)...);
  return *Base::begin();
}

template <typename BaseList>
void FrontModifyingListMixin<BaseList>::removeFirst() {
  if (Base::empty())
    throw OutOfRangeException("removeFirst() called on empty list");
  Base::pop_front();
}

template <typename BaseList>
auto FrontModifyingListMixin<BaseList>::takeFirst() -> value_type {
  value_type e = std::move(Base::first());
  Base::pop_front();
  return e;
}

template <typename BaseList>
void FrontModifyingListMixin<BaseList>::limitSizeFront(std::size_t maximumSize) {
  while (Base::size() > maximumSize)
    Base::pop_front();
}

template <typename Element, typename Allocator>
template <typename Container>
auto List<Element, Allocator>::from(Container const& c) -> List<Element, Allocator> {
  return List(c.begin(), c.end());
}

template <typename Element, typename Allocator>
auto List<Element, Allocator>::ptr() -> value_type* {
  return Base::data();
}

template <typename Element, typename Allocator>
auto List<Element, Allocator>::ptr() const -> value_type const* {
  return Base::data();
}

template <typename Element, typename Allocator>
auto List<Element, Allocator>::slice(SliceIndex a, SliceIndex b, int i) const -> List {
  return Star::slice(*this, a, b, i);
}

template <typename Element, typename Allocator>
template <typename Filter>
auto List<Element, Allocator>::filtered(Filter&& filter) const -> List {
  List list(*this);
  list.filter(std::forward<Filter>(filter));
  return list;
}

template <typename Element, typename Allocator>
template <typename Comparator>
auto List<Element, Allocator>::sorted(Comparator&& comparator) const -> List {
  List list(*this);
  list.sort(std::forward<Comparator>(comparator));
  return list;
}

template <typename Element, typename Allocator>
auto List<Element, Allocator>::sorted() const -> List<Element, Allocator> {
  List list(*this);
  list.sort();
  return list;
}

template <typename Element, typename Allocator>
template <typename Function>
auto List<Element, Allocator>::transformed(Function&& function) {
  List<std::decay_t<decltype(std::declval<Function>()(std::declval<reference>()))>> res;
  res.reserve(Base::size());
  transformInto(res, *this, std::forward<Function>(function));
  return res;
}

template <typename Element, typename Allocator>
template <typename Function>
auto List<Element, Allocator>::transformed(Function&& function) const {
  List<std::decay_t<decltype(std::declval<Function>()(std::declval<const_reference>()))>> res;
  res.reserve(Base::size());
  transformInto(res, *this, std::forward<Function>(function));
  return res;
}

template <typename Element, std::size_t MaxSize>
template <typename Container>
auto StaticList<Element, MaxSize>::from(Container const& c) -> StaticList<Element, MaxSize> {
  return StaticList(c.begin(), c.end());
}

template <typename Element, std::size_t MaxSize>
auto StaticList<Element, MaxSize>::slice(SliceIndex a, SliceIndex b, int i) const -> StaticList {
  return Star::slice(*this, a, b, i);
}

template <typename Element, std::size_t MaxSize>
template <typename Filter>
auto StaticList<Element, MaxSize>::filtered(Filter&& filter) const -> StaticList {
  StaticList list(*this);
  list.filter(forward<Filter>(filter));
  return list;
}

template <typename Element, std::size_t MaxSize>
template <typename Comparator>
auto StaticList<Element, MaxSize>::sorted(Comparator&& comparator) const -> StaticList {
  StaticList list(*this);
  list.sort(std::forward<Comparator>(comparator));
  return list;
}

template <typename Element, std::size_t MaxSize>
auto StaticList<Element, MaxSize>::sorted() const -> StaticList<Element, MaxSize> {
  StaticList list(*this);
  list.sort();
  return list;
}

template <typename Element, std::size_t MaxSize>
template <typename Function>
auto StaticList<Element, MaxSize>::transformed(Function&& function) {
  StaticList<std::decay_t<decltype(std::declval<Function>()(std::declval<reference>()))>, MaxSize> res;
  transformInto(res, *this, std::forward<Function>(function));
  return res;
}

template <typename Element, std::size_t MaxSize>
template <typename Function>
auto StaticList<Element, MaxSize>::transformed(Function&& function) const {
  StaticList<std::decay_t<decltype(std::declval<Function>()(std::declval<const_reference>()))>, MaxSize> res;
  transformInto(res, *this, std::forward<Function>(function));
  return res;
}

template <typename Element, std::size_t MaxStackSize>
template <typename Container>
auto SmallList<Element, MaxStackSize>::from(Container const& c) -> SmallList<Element, MaxStackSize> {
  return SmallList(c.begin(), c.end());
}

template <typename Element, std::size_t MaxStackSize>
auto SmallList<Element, MaxStackSize>::slice(SliceIndex a, SliceIndex b, int i) const -> SmallList {
  return Star::slice(*this, a, b, i);
}

template <typename Element, std::size_t MaxStackSize>
template <typename Filter>
auto SmallList<Element, MaxStackSize>::filtered(Filter&& filter) const -> SmallList {
  SmallList list(*this);
  list.filter(std::forward<Filter>(filter));
  return list;
}

template <typename Element, std::size_t MaxStackSize>
template <typename Comparator>
auto SmallList<Element, MaxStackSize>::sorted(Comparator&& comparator) const -> SmallList {
  SmallList list(*this);
  list.sort(std::forward<Comparator>(comparator));
  return list;
}

template <typename Element, std::size_t MaxStackSize>
auto SmallList<Element, MaxStackSize>::sorted() const -> SmallList<Element, MaxStackSize> {
  SmallList list(*this);
  list.sort();
  return list;
}

template <typename Element, std::size_t MaxStackSize>
template <typename Function>
auto SmallList<Element, MaxStackSize>::transformed(Function&& function) {
  SmallList<std::decay_t<decltype(std::declval<Function>()(std::declval<reference>()))>, MaxStackSize> res;
  transformInto(res, *this, std::forward<Function>(function));
  return res;
}

template <typename Element, std::size_t MaxStackSize>
template <typename Function>
auto SmallList<Element, MaxStackSize>::transformed(Function&& function) const {
  SmallList<std::decay_t<decltype(std::declval<Function>()(std::declval<const_reference>()))>, MaxStackSize> res;
  transformInto(res, *this, std::forward<Function>(function));
  return res;
}

template <typename Element, typename Allocator>
template <typename Container>
auto Deque<Element, Allocator>::from(Container const& c) -> Deque<Element, Allocator> {
  return Deque(c.begin(), c.end());
}

template <typename Element, typename Allocator>
auto Deque<Element, Allocator>::slice(SliceIndex a, SliceIndex b, int i) const -> Deque<Element, Allocator> {
  return Star::slice(*this, a, b, i);
}

template <typename Element, typename Allocator>
template <typename Filter>
auto Deque<Element, Allocator>::filtered(Filter&& filter) const -> Deque<Element, Allocator> {
  Deque l(*this);
  l.filter(std::forward<Filter>(filter));
  return l;
}

template <typename Element, typename Allocator>
template <typename Comparator>
auto Deque<Element, Allocator>::sorted(Comparator&& comparator) const -> Deque<Element, Allocator> {
  Deque l(*this);
  l.sort(std::forward<Comparator>(comparator));
  return l;
}

template <typename Element, typename Allocator>
auto Deque<Element, Allocator>::sorted() const -> Deque<Element, Allocator> {
  Deque l(*this);
  l.sort();
  return l;
}

template <typename Element, typename Allocator>
template <typename Function>
auto Deque<Element, Allocator>::transformed(Function&& function) {
  return Star::transform<Deque<decltype(std::declval<Function>()(std::declval<reference>()))>>(*this, std::forward<Function>(function));
}

template <typename Element, typename Allocator>
template <typename Function>
auto Deque<Element, Allocator>::transformed(Function&& function) const {
  return Star::transform<Deque<decltype(std::declval<Function>()(std::declval<const_reference>()))>>(*this, std::forward<Function>(function));
}

template <typename Element, typename Allocator>
template <typename Container>
auto LinkedList<Element, Allocator>::from(Container const& c) -> LinkedList<Element, Allocator> {
  return LinkedList(c.begin(), c.end());
}

template <typename Element, typename Allocator>
void LinkedList<Element, Allocator>::appendAll(LinkedList list) {
  Base::splice(Base::end(), list);
}

template <typename Element, typename Allocator>
void LinkedList<Element, Allocator>::prependAll(LinkedList list) {
  Base::splice(Base::begin(), list);
}

template <typename Element, typename Allocator>
template <typename Container>
void LinkedList<Element, Allocator>::appendAll(Container&& list) {
  for (auto& e : list) {
    if (std::is_rvalue_reference_v<Container&&>)
      Base::push_back(std::move(e));
    else
      Base::push_back(e);
  }
}

template <typename Element, typename Allocator>
template <typename Container>
void LinkedList<Element, Allocator>::prependAll(Container&& list) {
  for (auto i = std::rbegin(list); i != std::rend(list); ++i) {
    if (std::is_rvalue_reference_v<Container&&>)
      Base::push_front(std::move(*i));
    else
      Base::push_front(*i);
  }
}

template <typename Element, typename Allocator>
template <typename Filter>
auto LinkedList<Element, Allocator>::filtered(Filter&& filter) const -> LinkedList<Element, Allocator> {
  LinkedList list(*this);
  list.filter(std::forward<Filter>(filter));
  return list;
}

template <typename Element, typename Allocator>
template <typename Comparator>
auto LinkedList<Element, Allocator>::sorted(Comparator&& comparator) const -> LinkedList<Element, Allocator> {
  LinkedList l(*this);
  l.sort(std::forward<Comparator>(comparator));
  return l;
}

template <typename Element, typename Allocator>
auto LinkedList<Element, Allocator>::sorted() const -> LinkedList<Element, Allocator> {
  LinkedList l(*this);
  l.sort();
  return l;
}

template <typename Element, typename Allocator>
template <typename Function>
auto LinkedList<Element, Allocator>::transformed(Function&& function) {
  return Star::transform<LinkedList<decltype(std::declval<Function>()(std::declval<reference>()))>>(*this, std::forward<Function>(function));
}

template <typename Element, typename Allocator>
template <typename Function>
auto LinkedList<Element, Allocator>::transformed(Function&& function) const {
  return Star::transform<LinkedList<decltype(std::declval<Function>()(std::declval<const_reference>()))>>(*this, std::forward<Function>(function));
}

template <typename BaseList>
auto operator<<(std::ostream& os, ListMixin<BaseList> const& list) -> std::ostream& {
  os << "(";
  for (auto i = list.begin(); i != list.end(); ++i) {
    if (i != list.begin())
      os << ", ";
    os << *i;
  }
  os << ")";
  return os;
}

template <typename List>
auto ListHasher<List>::operator()(List const& l) const -> std::size_t {
  std::size_t h = 0;
  for (auto const& e : l)
    hashCombine(h, elemHasher(e));
  return h;
}

template <typename... Containers>
auto zip(Containers&&... args) -> typename ListZipTypes<Containers...>::Result {
  typename ListZipTypes<Containers...>::Result res;
  for (auto el : zipIterator(args...))
    res.push_back(std::move(el));

  return res;
}

template <typename Container>
auto enumerate(Container&& container) -> typename ListEnumerateTypes<Container>::Result {
  typename ListEnumerateTypes<Container>::Result res;
  for (auto el : enumerateIterator(container))
    res.push_back(std::move(el));

  return res;
}

}// namespace Star

template <typename BaseList>
struct std::formatter<Star::ListMixin<BaseList>> : Star::ostream_formatter {};
