module;
import star.memory;
import star.stream_formatter;
import star.hash;

import std;

import star.static_vector;
import star.small_vector;
import star.pythonic;
import star.optional;
import star.format;

import star.algorithm;
namespace Star {

template <typename BaseList>
class ListMixin : public BaseList {
public:
  typedef BaseList Base;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::value_type value_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;
  using AppendResult = decltype(std::declval<Base&>().push_back(std::declval<value_type>()));

  ListMixin();
  ListMixin(Base const& list);
  ListMixin(Base&& list);
  ListMixin(value_type const* p, std::size_t count) requires (!requires { Base::MaximumSize; });
  template <typename InputIterator>
  ListMixin(InputIterator beg, InputIterator end) requires (!requires { Base::MaximumSize; });
  explicit ListMixin(std::size_t len, const_reference s1 = value_type()) requires (!requires { Base::MaximumSize; });
  ListMixin(std::initializer_list<value_type> list) requires (!requires { Base::MaximumSize; });

  [[nodiscard]]
  AppendResult append(value_type e);

  template <typename Container>
  [[nodiscard]]
  AppendResult appendAll(Container&& list);

  template <class... Args>
  reference emplaceAppend(Args&&... args) requires std::is_void_v<AppendResult>;
  template <class... Args>
  [[nodiscard]]
  std::expected<value_type*, StarException> emplaceAppend(Args&&... args) requires (!std::is_void_v<AppendResult>);

  [[nodiscard]]
  std::expected<value_type*, StarException> first();
  [[nodiscard]]
  std::expected<value_type const*, StarException> first() const;

  [[nodiscard]]
  std::expected<value_type*, StarException> last();
  [[nodiscard]]
  std::expected<value_type const*, StarException> last() const;

  std::optional<value_type> maybeFirst();
  std::optional<value_type> maybeLast();

  [[nodiscard]]
  std::expected<void, StarException> removeLast();
  [[nodiscard]]
  std::expected<value_type, StarException> takeLast();

  std::optional<value_type> maybeTakeLast();

  // Limit the size of the list by removing elements from the back until the
  // size is the maximumSize or less.
  void limitSizeBack(std::size_t maximumSize);

  std::size_t count() const;

  bool contains(const_reference e) const;
  // Remove all equal to element, returns number removed.
  std::size_t remove(const_reference e);

  template <typename Filter>
  [[nodiscard]]
  auto filter(Filter&& filter);

  template <typename Self, typename Filter>
  [[nodiscard]]
  auto filtered(this Self const& self, Filter&& filter);

  template <typename Comparator>
  [[nodiscard]]
  AppendResult insertSorted(value_type e, Comparator&& comparator);
  [[nodiscard]]
  AppendResult insertSorted(value_type e);

  // Returns true if this *sorted* list contains the given element.
  template <typename Comparator>
  bool containsSorted(value_type const& e, Comparator&& comparator);
  bool containsSorted(value_type e);

  template <typename Function>
  [[nodiscard]]
  auto exec(Function&& function);

  template <typename Function>
  [[nodiscard]]
  auto exec(Function&& function) const;

  template <typename Function>
  [[nodiscard]]
  auto transform(Function&& function);

  template <typename Function>
  [[nodiscard]]
  auto any(Function&& function) const;
  [[nodiscard]] auto any() const;

  template <typename Function>
  [[nodiscard]]
  auto all(Function&& function) const;
  [[nodiscard]] auto all() const;
};

template <typename List>
class ListHasher {
public:
  std::size_t operator()(List const& l) const;

private:
  hash<typename List::value_type> elemHasher;
};

template <typename BaseList>
class RandomAccessListMixin : public BaseList {
public:
  typedef BaseList Base;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::value_type value_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;
  using AppendResult = typename Base::AppendResult;

  using Base::Base;

  template <typename Comparator>
  void sort(Comparator&& comparator);
  void sort();

  void reverse();

  // Returns first index of given element, NPos if not found.
  std::size_t indexOf(const_reference e, std::size_t from = 0) const;
  // Returns last index of given element, NPos if not found.
  std::size_t lastIndexOf(const_reference e, std::size_t til = NPos) const;

  [[nodiscard]]
  std::expected<value_type const*, StarException> at(std::size_t n) const;
  [[nodiscard]]
  std::expected<value_type*, StarException> at(std::size_t n);

  const_reference operator[](std::size_t n) const;
  reference operator[](std::size_t n);

  // Does not throw if n is beyond end of list, instead returns def
  value_type get(std::size_t n, value_type def = value_type()) const;

  [[nodiscard]]
  std::expected<value_type, StarException> takeAt(std::size_t i);

  // Same as at, but wraps around back to the beginning.
  [[nodiscard]]
  std::expected<value_type const*, StarException> wrap(std::size_t n) const;
  [[nodiscard]]
  std::expected<value_type*, StarException> wrap(std::size_t n);

  // Does not throw if list is empty
  value_type wrap(std::size_t n, value_type def) const;

  void eraseAt(std::size_t index);
  // Erases region from begin to end, not including end.
  void eraseAt(std::size_t begin, std::size_t end);

  [[nodiscard]]
  AppendResult insertAt(std::size_t pos, value_type e);

  template <typename Container>
  [[nodiscard]]
  AppendResult insertAllAt(std::size_t pos, Container const& l);

  // Ensures that list is large enough to hold pos elements.
  [[nodiscard]]
  AppendResult set(std::size_t pos, value_type e);

  void swap(std::size_t i, std::size_t j);
  // same as insert(to, takeAt(from))
  [[nodiscard]]
  std::expected<void, StarException> move(std::size_t from, std::size_t to);
};

template <typename BaseList>
class FrontModifyingListMixin : public BaseList {
public:
  typedef BaseList Base;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::value_type value_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;

  using Base::Base;

  void prepend(value_type e);

  template <typename Container>
  void prependAll(Container&& list);

  template <class... Args>
  reference emplacePrepend(Args&&... args);

  [[nodiscard]]
  std::expected<void, StarException> removeFirst();
  [[nodiscard]]
  std::expected<value_type, StarException> takeFirst();

  // Limit the size of the list by removing elements from the front until the
  // size is the maximumSize or less.
  void limitSizeFront(std::size_t maximumSize);
};

template <typename Element, typename Allocator = std::allocator<Element>>
class List : public RandomAccessListMixin<ListMixin<std::vector<Element, Allocator>>> {
public:
  typedef RandomAccessListMixin<ListMixin<std::vector<Element, Allocator>>> Base;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::value_type value_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;

  template <typename Container>
  static List from(Container const& c);

  using Base::Base;

  // Pointer to contiguous storage, returns nullptr if empty
  value_type* ptr();
  value_type const* ptr() const;

  List slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const;

  template <typename Comparator>
  List sorted(Comparator&& comparator) const;
  List sorted() const;

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function);

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function) const;
};

template <typename Element, typename Allocator>
struct hash<List<Element, Allocator>> : public ListHasher<List<Element, Allocator>> {};

template <typename Element, std::size_t MaxSize>
class StaticList : public RandomAccessListMixin<ListMixin<StaticVector<Element, MaxSize>>> {
public:
  typedef RandomAccessListMixin<ListMixin<StaticVector<Element, MaxSize>>> Base;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::value_type value_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;

  template <typename Container>
  [[nodiscard]]
  static std::expected<StaticList, StarException> from(Container const& c);
  template <typename Iterator>
  [[nodiscard]]
  static std::expected<StaticList, StarException> create(Iterator first, Iterator last);
  [[nodiscard]]
  static std::expected<StaticList, StarException> create(std::initializer_list<Element> list);
  [[nodiscard]]
  static std::expected<StaticList, StarException> create(std::size_t size, Element const& value = Element());

  using Base::Base;

  [[nodiscard]]
  std::expected<StaticList, StarException> slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const;

  template <typename Comparator>
  StaticList sorted(Comparator&& comparator) const;
  StaticList sorted() const;

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function);

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function) const;
};

template <typename Element, std::size_t MaxStackSize>
struct hash<StaticList<Element, MaxStackSize>> : public ListHasher<StaticList<Element, MaxStackSize>> {};

template <typename Element, std::size_t MaxStackSize>
class SmallList : public RandomAccessListMixin<ListMixin<SmallVector<Element, MaxStackSize>>> {
public:
  typedef RandomAccessListMixin<ListMixin<SmallVector<Element, MaxStackSize>>> Base;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::value_type value_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;

  template <typename Container>
  static SmallList from(Container const& c);

  using Base::Base;

  SmallList slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const;

  template <typename Comparator>
  SmallList sorted(Comparator&& comparator) const;
  SmallList sorted() const;

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function);

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function) const;
};

template <typename Element, std::size_t MaxStackSize>
struct hash<SmallList<Element, MaxStackSize>> : public ListHasher<SmallList<Element, MaxStackSize>> {};

template <typename Element, typename Allocator = std::allocator<Element>>
class Deque : public FrontModifyingListMixin<RandomAccessListMixin<ListMixin<std::deque<Element, Allocator>>>> {
public:
  typedef FrontModifyingListMixin<RandomAccessListMixin<ListMixin<std::deque<Element, Allocator>>>> Base;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::value_type value_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;

  template <typename Container>
  static Deque from(Container const& c);

  using Base::Base;

  Deque slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const;

  template <typename Comparator>
  Deque sorted(Comparator&& comparator) const;
  Deque sorted() const;

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function);

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function) const;
};

template <typename Element, typename Allocator>
struct hash<Deque<Element, Allocator>> : public ListHasher<Deque<Element, Allocator>> {};

template <typename Element, typename Allocator = std::allocator<Element>>
class LinkedList : public FrontModifyingListMixin<ListMixin<std::list<Element, Allocator>>> {
public:
  typedef FrontModifyingListMixin<ListMixin<std::list<Element, Allocator>>> Base;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::value_type value_type;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;

  template <typename Container>
  static LinkedList from(Container const& c);

  using Base::Base;

  void appendAll(LinkedList list);
  void prependAll(LinkedList list);

  template <typename Container>
  void appendAll(Container&& list);
  template <typename Container>
  void prependAll(Container&& list);

  template <typename Comparator>
  LinkedList sorted(Comparator&& comparator) const;
  LinkedList sorted() const;

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function);

  template <typename Function>
  [[nodiscard]]
  auto transformed(Function&& function) const;
};

template <typename Element, typename Allocator>
struct hash<LinkedList<Element, Allocator>> : public ListHasher<LinkedList<Element, Allocator>> {};

template <typename T>
decltype(auto) listDiagnosticValue(T const& value) {
  if constexpr (requires { outputOptional(value); })
    return outputOptional(value);
  else
    return (value);
}

template <typename BaseList>
using ListDiagnosticResult = decltype(std::declval<std::ostream&>() << listDiagnosticValue(std::declval<typename BaseList::value_type const&>()));

template <typename BaseList>
std::ostream& operator<<(std::ostream& os, ListMixin<BaseList> const& list)
    requires std::same_as<ListDiagnosticResult<BaseList>, std::ostream&>;

template <typename BaseList>
[[nodiscard]]
std::expected<std::ostream*, StarException> operator<<(std::ostream& os, ListMixin<BaseList> const& list)
    requires std::same_as<ListDiagnosticResult<BaseList>, std::expected<std::ostream*, StarException>>;

template <typename... Containers>
struct ListZipTypes {
  typedef std::tuple<typename std::decay<Containers>::type::value_type...> Tuple;
  typedef List<Tuple> Result;
};

template <typename... Containers>
typename ListZipTypes<Containers...>::Result zip(Containers&&... args);

template <typename Container>
struct ListEnumerateTypes {
  typedef std::pair<typename std::decay<Container>::type::value_type, std::size_t> Pair;
  typedef List<Pair> Result;
};

template <typename Container>
typename ListEnumerateTypes<Container>::Result enumerate(Container&& container);

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
ListMixin<BaseList>::ListMixin(std::size_t len, const_reference s1) requires (!requires { Base::MaximumSize; })
  : Base(len, s1) {}

template <typename BaseList>
ListMixin<BaseList>::ListMixin(value_type const* p, std::size_t count) requires (!requires { Base::MaximumSize; })
  : Base(p, p + count) {}

template <typename BaseList>
template <typename InputIterator>
ListMixin<BaseList>::ListMixin(InputIterator beg, InputIterator end) requires (!requires { Base::MaximumSize; })
  : Base(beg, end) {}

template <typename BaseList>
ListMixin<BaseList>::ListMixin(std::initializer_list<value_type> list) requires (!requires { Base::MaximumSize; }) {
  // In case underlying class type doesn't support initializer_list
  for (auto& e : list)
    append(std::move(e));
}

template <typename BaseList>
auto ListMixin<BaseList>::append(value_type e) -> AppendResult {
  return Base::push_back(std::move(e));
}

template <typename BaseList>
template <typename Container>
auto ListMixin<BaseList>::appendAll(Container&& list) -> AppendResult {
  for (auto& e : list) {
    if constexpr (std::is_void_v<AppendResult>) {
      if constexpr (std::is_rvalue_reference_v<Container&&>)
        Base::push_back(std::move(e));
      else
        Base::push_back(e);
    } else {
      auto added = [&] {
        if constexpr (std::is_rvalue_reference_v<Container&&>)
          return Base::push_back(std::move(e));
        else
          return Base::push_back(e);
      }();
      if (!added)
        return std::unexpected(std::move(added.error()));
    }
  }
  if constexpr (!std::is_void_v<AppendResult>)
    return {};
}

template <typename BaseList>
template <class... Args>
auto ListMixin<BaseList>::emplaceAppend(Args&&... args) -> reference
    requires std::is_void_v<AppendResult> {
  Base::emplace_back(std::forward<Args>(args)...);
  return *std::prev(Base::end());
}

template <typename BaseList>
template <class... Args>
auto ListMixin<BaseList>::emplaceAppend(Args&&... args) -> std::expected<value_type*, StarException>
    requires (!std::is_void_v<AppendResult>) {
  auto added = Base::emplace_back(std::forward<Args>(args)...);
  if (!added)
    return std::unexpected(std::move(added.error()));
  return std::addressof(*std::prev(Base::end()));
}

template <typename BaseList>
auto ListMixin<BaseList>::first() -> std::expected<value_type*, StarException> {
  if (Base::empty())
    return std::unexpected(OutOfRangeException("first() called on empty list"));
  return std::addressof(*Base::begin());
}

template <typename BaseList>
auto ListMixin<BaseList>::first() const -> std::expected<value_type const*, StarException> {
  if (Base::empty())
    return std::unexpected(OutOfRangeException("first() called on empty list"));
  return std::addressof(*Base::begin());
}

template <typename BaseList>
auto ListMixin<BaseList>::last() -> std::expected<value_type*, StarException> {
  if (Base::empty())
    return std::unexpected(OutOfRangeException("last() called on empty list"));
  return std::addressof(*std::prev(Base::end()));
}

template <typename BaseList>
auto ListMixin<BaseList>::last() const -> std::expected<value_type const*, StarException> {
  if (Base::empty())
    return std::unexpected(OutOfRangeException("last() called on empty list"));
  return std::addressof(*std::prev(Base::end()));
}

template <typename BaseList>
auto ListMixin<BaseList>::maybeFirst() -> std::optional<value_type> {
  if (Base::empty())
    return {};
  return std::optional<value_type>(std::in_place, *Base::begin());
}

template <typename BaseList>
auto ListMixin<BaseList>::maybeLast() -> std::optional<value_type> {
  if (Base::empty())
    return {};
  return std::optional<value_type>(std::in_place, *std::prev(Base::end()));
}

template <typename BaseList>
std::expected<void, StarException> ListMixin<BaseList>::removeLast() {
  if (Base::empty())
    return std::unexpected(OutOfRangeException("removeLast() called on empty list"));
  Base::erase(std::prev(Base::end()));
  return {};
}

template <typename BaseList>
auto ListMixin<BaseList>::takeLast() -> std::expected<value_type, StarException> {
  if (Base::empty())
    return std::unexpected(OutOfRangeException("takeLast() called on empty list"));
  value_type e = std::move(*std::prev(Base::end()));
  Base::erase(std::prev(Base::end()));
  return e;
}

template <typename BaseList>
auto ListMixin<BaseList>::maybeTakeLast() -> std::optional<value_type> {
  if (Base::empty())
    return {};
  value_type e = std::move(*std::prev(Base::end()));
  Base::erase(std::prev(Base::end()));
  return std::optional<value_type>(std::in_place, std::move(e));
}

template <typename BaseList>
void ListMixin<BaseList>::limitSizeBack(std::size_t maximumSize) {
  while (Base::size() > maximumSize)
    Base::erase(std::prev(Base::end()));
}

template <typename BaseList>
std::size_t ListMixin<BaseList>::count() const {
  return Base::size();
}

template <typename BaseList>
bool ListMixin<BaseList>::contains(const_reference e) const {
  for (auto const& r : *this) {
    if (r == e)
      return true;
  }
  return false;
}

template <typename BaseList>
std::size_t ListMixin<BaseList>::remove(const_reference e) {
  // Erase immediately: e may alias an element, and subsequent comparisons can
  // observe each erase's moves and destructions, even if a later operation throws.
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
auto ListMixin<BaseList>::filter(Filter&& filter) {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Filter&, reference>>;
  using Status = std::conditional_t<AlgorithmResultInfo<Invocation>::Fallible, std::expected<void, StarException>, void>;
  auto destination = Base::begin();
  for (auto source = Base::begin(); source != Base::end(); ++source) {
    bool keep;
    if constexpr (AlgorithmResultInfo<Invocation>::Fallible) {
      auto accepted = std::invoke(filter, *source);
      if (!accepted)
        return Status(std::unexpected(std::move(accepted.error())));
      keep = static_cast<bool>(*accepted);
    } else {
      keep = static_cast<bool>(std::invoke(filter, *source));
    }
    if (keep) {
      if (destination != source)
        *destination = std::move(*source);
      ++destination;
    }
  }
  Base::erase(destination, Base::end());
  if constexpr (AlgorithmResultInfo<Invocation>::Fallible)
    return Status();
}

template <typename BaseList>
template <typename Self, typename Filter>
auto ListMixin<BaseList>::filtered(this Self const& self, Filter&& filter) {
  Self list(self);
  if constexpr (std::is_void_v<decltype(list.filter(std::forward<Filter>(filter)))>) {
    list.filter(std::forward<Filter>(filter));
    return list;
  } else {
    auto filtered = list.filter(std::forward<Filter>(filter));
    if (!filtered)
      return std::expected<Self, StarException>(std::unexpected(std::move(filtered.error())));
    return std::expected<Self, StarException>(std::move(list));
  }
}

template <typename BaseList>
template <typename Comparator>
auto ListMixin<BaseList>::insertSorted(value_type e, Comparator&& comparator) -> AppendResult {
  static_assert(std::is_invocable_r_v<bool, Comparator&, const_reference, const_reference>,
      "Sorted insertion requires a nonfallible bool comparator");
  auto i = std::upper_bound(Base::begin(), Base::end(), e, std::forward<Comparator>(comparator));
  if constexpr (std::is_void_v<AppendResult>) {
    Base::insert(i, std::move(e));
  } else {
    auto added = Base::insert(i, std::move(e));
    if (!added)
      return std::unexpected(std::move(added.error()));
    return {};
  }
}

template <typename BaseList>
auto ListMixin<BaseList>::insertSorted(value_type e) -> AppendResult {
  return insertSorted(std::move(e), std::less<value_type>{});
}

template <typename BaseList>
template <typename Comparator>
bool ListMixin<BaseList>::containsSorted(value_type const& e, Comparator&& comparator) {
  static_assert(std::is_invocable_r_v<bool, Comparator&, const_reference, const_reference>,
      "Sorted lookup requires a nonfallible bool comparator");
  auto range = std::equal_range(Base::begin(), Base::end(), e, std::forward<Comparator>(comparator));
  return range.first != range.second;
}

template <typename BaseList>
bool ListMixin<BaseList>::containsSorted(value_type e) {
  auto range = std::equal_range(Base::begin(), Base::end(), e);
  return range.first != range.second;
}

template <typename BaseList>
template <typename Function>
auto ListMixin<BaseList>::exec(Function&& function) {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, reference>>;
  using Status = std::conditional_t<AlgorithmResultInfo<Invocation>::Fallible, std::expected<void, StarException>, void>;
  for (auto& e : *this) {
    if constexpr (AlgorithmResultInfo<Invocation>::Fallible) {
      auto result = std::invoke(function, e);
      if (!result)
        return Status(std::unexpected(std::move(result.error())));
    } else {
      std::invoke(function, e);
    }
  }
  if constexpr (AlgorithmResultInfo<Invocation>::Fallible)
    return Status();
}

template <typename BaseList>
template <typename Function>
auto ListMixin<BaseList>::exec(Function&& function) const {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, const_reference>>;
  using Status = std::conditional_t<AlgorithmResultInfo<Invocation>::Fallible, std::expected<void, StarException>, void>;
  for (auto const& e : *this) {
    if constexpr (AlgorithmResultInfo<Invocation>::Fallible) {
      auto result = std::invoke(function, e);
      if (!result)
        return Status(std::unexpected(std::move(result.error())));
    } else {
      std::invoke(function, e);
    }
  }
  if constexpr (AlgorithmResultInfo<Invocation>::Fallible)
    return Status();
}

template <typename BaseList>
template <typename Function>
auto ListMixin<BaseList>::transform(Function&& function) {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, reference>>;
  using Status = std::conditional_t<AlgorithmResultInfo<Invocation>::Fallible, std::expected<void, StarException>, void>;
  for (auto& e : *this) {
    if constexpr (AlgorithmResultInfo<Invocation>::Fallible) {
      auto result = std::invoke(function, e);
      if (!result)
        return Status(std::unexpected(std::move(result.error())));
      e = std::move(*result);
    } else {
      e = std::invoke(function, e);
    }
  }
  if constexpr (AlgorithmResultInfo<Invocation>::Fallible)
    return Status();
}

template <typename BaseList>
template <typename Function>
auto ListMixin<BaseList>::any(Function&& function) const {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function const&, const_reference>>;
  using Result = std::conditional_t<AlgorithmResultInfo<Invocation>::Fallible, std::expected<bool, StarException>, bool>;
  for (auto const& e : *this) {
    if constexpr (AlgorithmResultInfo<Invocation>::Fallible) {
      decltype(auto) accepted = std::invoke(std::as_const(function), e);
      if (!accepted)
        return Result(std::unexpected(std::move(accepted.error())));
      if (static_cast<bool>(*accepted))
        return Result(true);
    } else if (static_cast<bool>(std::invoke(std::as_const(function), e))) {
      return Result(true);
    }
  }
  return Result(false);
}

template <typename BaseList>
auto ListMixin<BaseList>::any() const {
  return any([](auto const& value) -> auto const& { return value; });
}

template <typename BaseList>
template <typename Function>
auto ListMixin<BaseList>::all(Function&& function) const {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function const&, const_reference>>;
  using Result = std::conditional_t<AlgorithmResultInfo<Invocation>::Fallible, std::expected<bool, StarException>, bool>;
  for (auto const& e : *this) {
    if constexpr (AlgorithmResultInfo<Invocation>::Fallible) {
      decltype(auto) accepted = std::invoke(std::as_const(function), e);
      if (!accepted)
        return Result(std::unexpected(std::move(accepted.error())));
      if (!static_cast<bool>(*accepted))
        return Result(false);
    } else if (!static_cast<bool>(std::invoke(std::as_const(function), e))) {
      return Result(false);
    }
  }
  return Result(true);
}

template <typename BaseList>
auto ListMixin<BaseList>::all() const {
  return all([](auto const& value) -> auto const& { return value; });
}

template <typename BaseList>
template <typename Comparator>
void RandomAccessListMixin<BaseList>::sort(Comparator&& comparator) {
  static_assert(std::is_invocable_r_v<bool, Comparator&, reference, reference>,
      "Sorting requires a nonfallible bool comparator; precompute fallible keys first");
  std::sort(Base::begin(), Base::end(), std::forward<Comparator>(comparator));
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::sort() {
  std::sort(Base::begin(), Base::end(), std::less<typename Base::value_type>());
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::reverse() {
  std::reverse(Base::begin(), Base::end());
}

template <typename BaseList>
std::size_t RandomAccessListMixin<BaseList>::indexOf(const_reference e, std::size_t from) const {
  for (std::size_t i = from; i < Base::size(); ++i)
    if (operator[](i) == e)
      return i;
  return NPos;
}

template <typename BaseList>
std::size_t RandomAccessListMixin<BaseList>::lastIndexOf(const_reference e, std::size_t til) const {
  std::size_t index = NPos;
  std::size_t end = std::min(Base::size(), til);
  for (std::size_t i = 0; i < end; ++i) {
    if (operator[](i) == e)
      index = i;
  }
  return index;
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::at(std::size_t n) const -> std::expected<value_type const*, StarException> {
  if (n >= Base::size())
    return std::unexpected(OutOfRangeException(strf("out of range list::at({})", n)));
  return std::addressof(operator[](n));
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::at(std::size_t n) -> std::expected<value_type*, StarException> {
  if (n >= Base::size())
    return std::unexpected(OutOfRangeException(strf("out of range list::at({})", n)));
  return std::addressof(operator[](n));
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::operator[](std::size_t n) const -> const_reference {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((n < Base::size()));
  }
  return Base::operator[](n);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::operator[](std::size_t n) -> reference {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((n < Base::size()));
  }
  return Base::operator[](n);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::get(std::size_t n, value_type def) const -> value_type {
  if (n >= BaseList::size())
    return def;
  return operator[](n);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::takeAt(std::size_t i) -> std::expected<value_type, StarException> {
  auto element = at(i);
  if (!element)
    return std::unexpected(std::move(element.error()));
  value_type e = std::move(**element);
  Base::erase(Base::begin() + i);
  return e;
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::wrap(std::size_t n) const -> std::expected<value_type const*, StarException> {
  if (BaseList::empty())
    return std::unexpected(OutOfRangeException("wrap() called on empty list"));
  return std::addressof(operator[](n % BaseList::size()));
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::wrap(std::size_t n) -> std::expected<value_type*, StarException> {
  if (BaseList::empty())
    return std::unexpected(OutOfRangeException("wrap() called on empty list"));
  return std::addressof(operator[](n % BaseList::size()));
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
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((i < Base::size()));
  }
  Base::erase(Base::begin() + i);
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::eraseAt(std::size_t b, std::size_t e) {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((b < Base::size() && e <= Base::size()));
  }
  Base::erase(Base::begin() + b, Base::begin() + e);
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::insertAt(std::size_t pos, value_type e) -> AppendResult {
  if constexpr (Star::DebugEnabled)
    Star::starAssertFailed((pos <= Base::size()));
  if constexpr (std::is_void_v<AppendResult>) {
    Base::insert(Base::begin() + pos, std::move(e));
  } else {
    auto added = Base::insert(Base::begin() + pos, std::move(e));
    if (!added)
      return std::unexpected(std::move(added.error()));
    return {};
  }
}

template <typename BaseList>
template <typename Container>
auto RandomAccessListMixin<BaseList>::insertAllAt(std::size_t pos, Container const& l) -> AppendResult {
  if constexpr (Star::DebugEnabled)
    Star::starAssertFailed((pos <= Base::size()));
  if constexpr (std::is_void_v<AppendResult>) {
    Base::insert(Base::begin() + pos, l.begin(), l.end());
  } else {
    auto added = Base::insert(Base::begin() + pos, l.begin(), l.end());
    if (!added)
      return std::unexpected(std::move(added.error()));
    return {};
  }
}

template <typename BaseList>
auto RandomAccessListMixin<BaseList>::set(std::size_t pos, value_type e) -> AppendResult {
  if constexpr (std::is_void_v<AppendResult>) {
    if (pos >= Base::size())
      Base::resize(pos + 1);
    operator[](pos) = std::move(e);
  } else {
    if (pos == std::numeric_limits<std::size_t>::max())
      return std::unexpected(OutOfRangeException("List::set index overflows size"));
    if (pos >= Base::size()) {
      auto resized = Base::resize(pos + 1);
      if (!resized)
        return std::unexpected(std::move(resized.error()));
    }
    operator[](pos) = std::move(e);
    return {};
  }
}

template <typename BaseList>
void RandomAccessListMixin<BaseList>::swap(std::size_t i, std::size_t j) {
  std::swap(operator[](i), operator[](j));
}

template <typename BaseList>
std::expected<void, StarException> RandomAccessListMixin<BaseList>::move(std::size_t from, std::size_t to) {
  if (from >= Base::size() || to >= Base::size())
    return std::unexpected(OutOfRangeException("List::move index out of range"));
  if (from < to)
    std::rotate(Base::begin() + from, Base::begin() + from + 1, Base::begin() + to + 1);
  else if (to < from)
    std::rotate(Base::begin() + to, Base::begin() + from, Base::begin() + from + 1);
  return {};
}

template <typename BaseList>
void FrontModifyingListMixin<BaseList>::prepend(value_type e) {
  Base::push_front(std::move(e));
}

template <typename BaseList>
template <typename Container>
void FrontModifyingListMixin<BaseList>::prependAll(Container&& list) {
  for (auto i = std::rbegin(list); i != std::rend(list); ++i) {
    if (std::is_rvalue_reference<Container&&>::value)
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
std::expected<void, StarException> FrontModifyingListMixin<BaseList>::removeFirst() {
  if (Base::empty())
    return std::unexpected(OutOfRangeException("removeFirst() called on empty list"));
  Base::pop_front();
  return {};
}

template <typename BaseList>
auto FrontModifyingListMixin<BaseList>::takeFirst() -> std::expected<value_type, StarException> {
  if (Base::empty())
    return std::unexpected(OutOfRangeException("takeFirst() called on empty list"));
  value_type e = std::move(*Base::begin());
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
List<Element, Allocator> List<Element, Allocator>::from(Container const& c) {
  return List(c.begin(), c.end());
}

template <typename Element, typename Allocator>
auto List<Element, Allocator>::ptr() -> value_type * {
  return Base::data();
}

template <typename Element, typename Allocator>
auto List<Element, Allocator>::ptr() const -> value_type const * {
  return Base::data();
}

template <typename Element, typename Allocator>
auto List<Element, Allocator>::slice(SliceIndex a, SliceIndex b, int i) const -> List {
  return Star::slice(*this, a, b, i);
}

template <typename Element, typename Allocator>
template <typename Comparator>
auto List<Element, Allocator>::sorted(Comparator&& comparator) const -> List {
  static_assert(std::is_invocable_r_v<bool, Comparator&, reference, reference>,
      "Sorting requires a nonfallible bool comparator; precompute fallible keys first");
  List list(*this);
  list.sort(std::forward<Comparator>(comparator));
  return list;
}

template <typename Element, typename Allocator>
List<Element, Allocator> List<Element, Allocator>::sorted() const {
  List list(*this);
  list.sort();
  return list;
}

template <typename Element, typename Allocator>
template <typename Function>
auto List<Element, Allocator>::transformed(Function&& function) {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, reference>>;
  using Output = List<typename AlgorithmResultInfo<Invocation>::Value>;
  Output res;
  res.reserve(Base::size());
  if constexpr (AlgorithmResultInfo<Invocation>::Fallible) {
    auto transformed = transformInto(res, *this, std::forward<Function>(function));
    if (!transformed)
      return std::expected<Output, StarException>(std::unexpected(std::move(transformed.error())));
    return std::expected<Output, StarException>(std::move(res));
  } else {
    transformInto(res, *this, std::forward<Function>(function));
    return res;
  }
}

template <typename Element, typename Allocator>
template <typename Function>
auto List<Element, Allocator>::transformed(Function&& function) const {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, const_reference>>;
  using Output = List<typename AlgorithmResultInfo<Invocation>::Value>;
  Output res;
  res.reserve(Base::size());
  if constexpr (AlgorithmResultInfo<Invocation>::Fallible) {
    auto transformed = transformInto(res, *this, std::forward<Function>(function));
    if (!transformed)
      return std::expected<Output, StarException>(std::unexpected(std::move(transformed.error())));
    return std::expected<Output, StarException>(std::move(res));
  } else {
    transformInto(res, *this, std::forward<Function>(function));
    return res;
  }
}

template <typename Element, std::size_t MaxSize>
template <typename Container>
auto StaticList<Element, MaxSize>::from(Container const& c) -> std::expected<StaticList, StarException> {
  return create(c.begin(), c.end());
}

template <typename Element, std::size_t MaxSize>
template <typename Iterator>
auto StaticList<Element, MaxSize>::create(Iterator first, Iterator last) -> std::expected<StaticList, StarException> {
  StaticList result;
  auto added = result.insert(result.end(), first, last);
  if (!added)
    return std::unexpected(std::move(added.error()));
  return result;
}

template <typename Element, std::size_t MaxSize>
auto StaticList<Element, MaxSize>::create(std::initializer_list<Element> list) -> std::expected<StaticList, StarException> {
  return create(list.begin(), list.end());
}

template <typename Element, std::size_t MaxSize>
auto StaticList<Element, MaxSize>::create(std::size_t size, Element const& value) -> std::expected<StaticList, StarException> {
  StaticList result;
  auto resized = result.resize(size, value);
  if (!resized)
    return std::unexpected(std::move(resized.error()));
  return result;
}

template <typename Element, std::size_t MaxSize>
auto StaticList<Element, MaxSize>::slice(SliceIndex a, SliceIndex b, int i) const -> std::expected<StaticList, StarException> {
  return Star::slice(*this, a, b, i);
}

template <typename Element, std::size_t MaxSize>
template <typename Comparator>
auto StaticList<Element, MaxSize>::sorted(Comparator&& comparator) const -> StaticList {
  static_assert(std::is_invocable_r_v<bool, Comparator&, reference, reference>,
      "Sorting requires a nonfallible bool comparator; precompute fallible keys first");
  StaticList list(*this);
  list.sort(std::forward<Comparator>(comparator));
  return list;
}

template <typename Element, std::size_t MaxSize>
StaticList<Element, MaxSize> StaticList<Element, MaxSize>::sorted() const {
  StaticList list(*this);
  list.sort();
  return list;
}

template <typename Element, std::size_t MaxSize>
template <typename Function>
auto StaticList<Element, MaxSize>::transformed(Function&& function) {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, reference>>;
  using Output = StaticList<typename AlgorithmResultInfo<Invocation>::Value, MaxSize>;
  return Star::transform<Output>(*this, std::forward<Function>(function));
}

template <typename Element, std::size_t MaxSize>
template <typename Function>
auto StaticList<Element, MaxSize>::transformed(Function&& function) const {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, const_reference>>;
  using Output = StaticList<typename AlgorithmResultInfo<Invocation>::Value, MaxSize>;
  return Star::transform<Output>(*this, std::forward<Function>(function));
}

template <typename Element, std::size_t MaxStackSize>
template <typename Container>
SmallList<Element, MaxStackSize> SmallList<Element, MaxStackSize>::from(Container const& c) {
  return SmallList(c.begin(), c.end());
}

template <typename Element, std::size_t MaxStackSize>
auto SmallList<Element, MaxStackSize>::slice(SliceIndex a, SliceIndex b, int i) const -> SmallList {
  return Star::slice(*this, a, b, i);
}

template <typename Element, std::size_t MaxStackSize>
template <typename Comparator>
auto SmallList<Element, MaxStackSize>::sorted(Comparator&& comparator) const -> SmallList {
  static_assert(std::is_invocable_r_v<bool, Comparator&, reference, reference>,
      "Sorting requires a nonfallible bool comparator; precompute fallible keys first");
  SmallList list(*this);
  list.sort(std::forward<Comparator>(comparator));
  return list;
}

template <typename Element, std::size_t MaxStackSize>
SmallList<Element, MaxStackSize> SmallList<Element, MaxStackSize>::sorted() const {
  SmallList list(*this);
  list.sort();
  return list;
}

template <typename Element, std::size_t MaxStackSize>
template <typename Function>
auto SmallList<Element, MaxStackSize>::transformed(Function&& function) {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, reference>>;
  using Output = SmallList<typename AlgorithmResultInfo<Invocation>::Value, MaxStackSize>;
  return Star::transform<Output>(*this, std::forward<Function>(function));
}

template <typename Element, std::size_t MaxStackSize>
template <typename Function>
auto SmallList<Element, MaxStackSize>::transformed(Function&& function) const {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, const_reference>>;
  using Output = SmallList<typename AlgorithmResultInfo<Invocation>::Value, MaxStackSize>;
  return Star::transform<Output>(*this, std::forward<Function>(function));
}

template <typename Element, typename Allocator>
template <typename Container>
Deque<Element, Allocator> Deque<Element, Allocator>::from(Container const& c) {
  return Deque(c.begin(), c.end());
}

template <typename Element, typename Allocator>
Deque<Element, Allocator> Deque<Element, Allocator>::slice(SliceIndex a, SliceIndex b, int i) const {
  return Star::slice(*this, a, b, i);
}

template <typename Element, typename Allocator>
template <typename Comparator>
Deque<Element, Allocator> Deque<Element, Allocator>::sorted(Comparator&& comparator) const {
  static_assert(std::is_invocable_r_v<bool, Comparator&, reference, reference>,
      "Sorting requires a nonfallible bool comparator; precompute fallible keys first");
  Deque l(*this);
  l.sort(std::forward<Comparator>(comparator));
  return l;
}

template <typename Element, typename Allocator>
Deque<Element, Allocator> Deque<Element, Allocator>::sorted() const {
  Deque l(*this);
  l.sort();
  return l;
}

template <typename Element, typename Allocator>
template <typename Function>
auto Deque<Element, Allocator>::transformed(Function&& function) {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, reference>>;
  return Star::transform<Deque<typename AlgorithmResultInfo<Invocation>::Value>>(*this, std::forward<Function>(function));
}

template <typename Element, typename Allocator>
template <typename Function>
auto Deque<Element, Allocator>::transformed(Function&& function) const {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, const_reference>>;
  return Star::transform<Deque<typename AlgorithmResultInfo<Invocation>::Value>>(*this, std::forward<Function>(function));
}

template <typename Element, typename Allocator>
template <typename Container>
LinkedList<Element, Allocator> LinkedList<Element, Allocator>::from(Container const& c) {
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
    if (std::is_rvalue_reference<Container&&>::value)
      Base::push_back(std::move(e));
    else
      Base::push_back(e);
  }
}

template <typename Element, typename Allocator>
template <typename Container>
void LinkedList<Element, Allocator>::prependAll(Container&& list) {
  for (auto i = std::rbegin(list); i != std::rend(list); ++i) {
    if (std::is_rvalue_reference<Container&&>::value)
      Base::push_front(std::move(*i));
    else
      Base::push_front(*i);
  }
}

template <typename Element, typename Allocator>
template <typename Comparator>
LinkedList<Element, Allocator> LinkedList<Element, Allocator>::sorted(Comparator&& comparator) const {
  static_assert(std::is_invocable_r_v<bool, Comparator&, reference, reference>,
      "Sorting requires a nonfallible bool comparator; precompute fallible keys first");
  LinkedList l(*this);
  l.sort(std::forward<Comparator>(comparator));
  return l;
}

template <typename Element, typename Allocator>
LinkedList<Element, Allocator> LinkedList<Element, Allocator>::sorted() const {
  LinkedList l(*this);
  l.sort();
  return l;
}

template <typename Element, typename Allocator>
template <typename Function>
auto LinkedList<Element, Allocator>::transformed(Function&& function) {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, reference>>;
  return Star::transform<LinkedList<typename AlgorithmResultInfo<Invocation>::Value>>(*this, std::forward<Function>(function));
}

template <typename Element, typename Allocator>
template <typename Function>
auto LinkedList<Element, Allocator>::transformed(Function&& function) const {
  using Invocation = std::remove_cvref_t<std::invoke_result_t<Function&, const_reference>>;
  return Star::transform<LinkedList<typename AlgorithmResultInfo<Invocation>::Value>>(*this, std::forward<Function>(function));
}


template <typename BaseList>
std::ostream& operator<<(std::ostream& os, ListMixin<BaseList> const& list)
    requires std::same_as<ListDiagnosticResult<BaseList>, std::ostream&> {
  os << "(";
  for (auto i = list.begin(); i != list.end(); ++i) {
    if (i != list.begin())
      os << ", ";
    os << listDiagnosticValue(*i);
  }
  return os << ")";
}

template <typename BaseList>
[[nodiscard]]
std::expected<std::ostream*, StarException> operator<<(std::ostream& os, ListMixin<BaseList> const& list)
    requires std::same_as<ListDiagnosticResult<BaseList>, std::expected<std::ostream*, StarException>> {
  os << "(";
  for (auto i = list.begin(); i != list.end(); ++i) {
    if (i != list.begin())
      os << ", ";
    auto written = os << listDiagnosticValue(*i);
    if (!written)
      return std::unexpected(std::move(written.error()));
  }
  os << ")";
  return &os;
}

template <typename List>
std::size_t ListHasher<List>::operator()(List const& l) const {
  std::size_t h = 0;
  for (auto const& e : l)
    hashCombine(h, elemHasher(e));
  return h;
}

template <typename... Containers>
typename ListZipTypes<Containers...>::Result zip(Containers&&... args) {
  typename ListZipTypes<Containers...>::Result res;
  for (auto el : std::views::zip(args...))
    res.push_back(std::move(el));

  return res;
}

template <typename Container>
typename ListEnumerateTypes<Container>::Result enumerate(Container&& container) {
  typename ListEnumerateTypes<Container>::Result res;
  for (auto [index, value] : std::views::enumerate(container))
    res.emplace_back(value, static_cast<std::size_t>(index));

  return res;
}

}

export module star.list;

export namespace Star {
  template <typename Element, typename Allocator>
  struct hash<List<Element, Allocator>>;
  template <typename Element, std::size_t MaxStackSize>
  struct hash<StaticList<Element, MaxStackSize>>;
  template <typename Element, std::size_t MaxStackSize>
  struct hash<SmallList<Element, MaxStackSize>>;
  template <typename Element, typename Allocator>
  struct hash<Deque<Element, Allocator>>;
  template <typename Element, typename Allocator>
  struct hash<LinkedList<Element, Allocator>>;
}


export template <typename BaseList>
  requires std::same_as<decltype(std::declval<std::ostream&>() << std::declval<Star::ListMixin<BaseList> const&>()), std::ostream&>
struct std::formatter<Star::ListMixin<BaseList>> : Star::StreamFormatter {};

export namespace Star {
  using ::Star::ListMixin;
  using ::Star::ListHasher;
  using ::Star::RandomAccessListMixin;
  using ::Star::FrontModifyingListMixin;
  using ::Star::List;
  using ::Star::StaticList;
  using ::Star::SmallList;
  using ::Star::Deque;
  using ::Star::LinkedList;
  using ::Star::ListZipTypes;
  using ::Star::ListEnumerateTypes;
  using ::Star::zip;
  using ::Star::enumerate;
  using ::Star::operator<<;
  using ::Star::hash;
}
