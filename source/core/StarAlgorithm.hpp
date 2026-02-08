#pragma once

import std;

namespace Star {

// Function that does nothing and takes any number of arguments
template <typename... T>
void nothing(T&&...) {}

// Functional constructor call / casting.
template <typename ToType>
struct construct {
  template <typename... FromTypes>
  auto operator()(FromTypes&&... fromTypes) const -> ToType {
    return ToType(std::forward<FromTypes>(fromTypes)...);
  }
};

// Use std::identity from C++20 instead of custom implementation
// The custom identity struct has been removed in favor of std::identity

template <typename Func>
struct SwallowReturn {
  template <typename... T>
  void operator()(T&&... args) {
    func(std::forward<T>(args)...);
  }

  Func func;
};

template <typename Func>
auto swallow(Func f) -> SwallowReturn<Func> {
  return SwallowReturn<Func>{std::move(f)};
}

struct Empty {
  auto operator==(Empty const) const -> bool {
    return true;
  }

  auto operator<(Empty const) const -> bool {
    return false;
  }
};

// Compose arbitrary functions
template <typename FirstFunction, typename SecondFunction>
struct FunctionComposer {
  FirstFunction f1;
  SecondFunction f2;

  template <typename... T>
  auto operator()(T&&... args) -> decltype(auto) {
    return f1(f2(std::forward<T>(args)...));
  }
};

template <typename FirstFunction, typename SecondFunction>
auto compose(FirstFunction&& firstFunction, SecondFunction&& secondFunction) -> decltype(auto) {
  return FunctionComposer<FirstFunction, SecondFunction>{std::move(std::forward<FirstFunction>(firstFunction)), std::move(std::forward<SecondFunction>(secondFunction))};
}

template <typename FirstFunction, typename SecondFunction, typename ThirdFunction, typename... RestFunctions>
auto compose(FirstFunction firstFunction, SecondFunction secondFunction, ThirdFunction thirdFunction, RestFunctions... restFunctions) -> decltype(auto) {
  return compose(std::forward<FirstFunction>(firstFunction), compose(std::forward<SecondFunction>(secondFunction), compose(std::forward<ThirdFunction>(thirdFunction), std::forward<RestFunctions>(restFunctions)...)));
}

// fold and fold1 are convenience wrappers around std::accumulate
// They provide a more functional programming style interface
// For direct replacements, consider: std::accumulate(l.begin(), l.end(), v, f)
template <typename Container, typename Value, typename Function>
auto fold(Container const& l, Value v, Function f) -> Value {
  auto i = l.begin();
  auto e = l.end();
  while (i != e) {
    v = f(v, *i);
    ++i;
  }
  return v;
}

// Like fold, but returns default value when container is empty.
template <typename Container, typename Function>
auto fold1(Container const& l, Function f) -> typename Container::value_type {
  typename Container::value_type res = {};
  typename Container::const_iterator i = l.begin();
  typename Container::const_iterator e = l.end();

  if (i == e)
    return res;

  res = *i;
  ++i;
  while (i != e) {
    res = f(res, *i);
    ++i;
  }
  return res;
}

// Return intersection of sorted containers.
template <typename Container>
auto intersect(Container const& a, Container const& b) -> Container {
  Container r;
  std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(r, r.end()));
  return r;
}

template <typename MapType1, typename MapType2>
auto mapMerge(MapType1& targetMap, MapType2 const& sourceMap, bool overwrite = false) -> bool {
  bool noCommonKeys = true;
  for (auto i = sourceMap.begin(); i != sourceMap.end(); ++i) {
    auto res = targetMap.insert(*i);
    if (!res.second) {
      noCommonKeys = false;
      if (overwrite)
        res.first->second = i->second;
    }
  }
  return noCommonKeys;
}

template <typename MapType1, typename MapType2>
auto mapsEqual(MapType1 const& m1, MapType2 const& m2) -> bool {
  if (&m1 == &m2)
    return true;

  if (m1.size() != m2.size())
    return false;

  for (auto const& m1pair : m1) {
    auto m2it = m2.find(m1pair.first);
    if (m2it == m2.end() || !(m2it->second == m1pair.second))
      return false;
  }

  return true;
}

template <typename Container, typename Filter>
void filter(Container& container, Filter&& filter) {
  auto p = std::begin(container);
  while (p != std::end(container)) {
    if (!filter(*p))
      p = container.erase(p);
    else
      ++p;
  }
}

template <typename OutContainer, typename InContainer, typename Filter>
auto filtered(InContainer const& input, Filter&& filter) -> OutContainer {
  OutContainer out;
  auto p = std::begin(input);
  while (p != std::end(input)) {
    if (filter(*p))
      out.insert(out.end(), *p);
    ++p;
  }
  return out;
}

template <typename Container, typename Cond>
void eraseWhere(Container& container, Cond&& cond) {
  auto p = std::begin(container);
  while (p != std::end(container)) {
    if (cond(*p))
      p = container.erase(p);
    else
      ++p;
  }
}

template <typename Container, typename Compare>
void sort(Container& c, Compare comp) {
  std::sort(c.begin(), c.end(), comp);
}

template <typename Container, typename Compare>
void stableSort(Container& c, Compare comp) {
  std::stable_sort(c.begin(), c.end(), comp);
}

template <typename Container>
void sort(Container& c) {
  std::sort(c.begin(), c.end(), std::less<typename Container::value_type>());
}

template <typename Container>
void stableSort(Container& c) {
  std::stable_sort(c.begin(), c.end(), std::less<typename Container::value_type>());
}

template <typename Container, typename Compare>
auto sorted(Container const& c, Compare comp) -> Container {
  auto c2 = c;
  sort(c2, comp);
  return c2;
}

template <typename Container, typename Compare>
auto stableSorted(Container const& c, Compare comp) -> Container {
  auto c2 = c;
  sort(c2, comp);
  return c2;
}

template <typename Container>
auto sorted(Container const& c) -> Container {
  auto c2 = c;
  sort(c2);
  return c2;
}

template <typename Container>
auto stableSorted(Container const& c) -> Container {
  auto c2 = c;
  sort(c2);
  return c2;
}

// Sort a container by the output of a computed value. The computed value is
// only computed *once* per item in the container, which is useful both for
// when the computed value is costly, and to avoid sorting instability with
// floating point values.  Container must have size() and operator[], and also
// must be constructable with Container(size_t).
template <typename Container, typename Getter>
void sortByComputedValue(Container& container, Getter&& valueGetter, bool stable = false) {
  using ContainerValue = typename Container::value_type;
  using ComputedValue = decltype(valueGetter(ContainerValue()));
  using ComputedPair = std::pair<ComputedValue, std::size_t>;

  std::size_t containerSize = container.size();

  if (containerSize <= 1)
    return;

  std::vector<ComputedPair> work(containerSize);
  for (std::size_t i = 0; i < containerSize; ++i)
    work[i] = {valueGetter(container[i]), i};

  auto compare = [](ComputedPair const& a, ComputedPair const& b) -> auto { return a.first < b.first; };

  // Sort the comptued values and the associated indexes
  if (stable)
    stableSort(work, compare);
  else
    sort(work, compare);

  Container result(containerSize);
  for (std::size_t i = 0; i < containerSize; ++i)
    std::swap(result[i], container[work[i].second]);

  std::swap(container, result);
}

template <typename Container, typename Getter>
void stableSortByComputedValue(Container& container, Getter&& valueGetter) {
  return sortByComputedValue(container, std::forward<Getter>(valueGetter), true);
}

template <typename Container>
void reverse(Container& c) {
  std::reverse(c.begin(), c.end());
}

template <typename Container>
auto reverseCopy(Container c) -> Container {
  reverse(c);
  return c;
}

template <typename T>
auto copy(T c) -> T {
  return c;
}

template <typename Container>
auto sum(Container const& cont) -> typename Container::value_type {
  return fold1(cont, std::plus<typename Container::value_type>());
}

template <typename Container>
auto product(Container const& cont) -> typename Container::value_type {
  return fold1(cont, std::multiplies<typename Container::value_type>());
}

template <typename OutContainer, typename InContainer, typename Function>
void transformInto(OutContainer& outContainer, InContainer&& inContainer, Function&& function) {
  for (auto&& elem : inContainer) {
    if (std::is_rvalue_reference_v<InContainer&&>)
      outContainer.insert(outContainer.end(), function(std::move(elem)));
    else
      outContainer.insert(outContainer.end(), function(elem));
  }
}

template <typename OutContainer, typename InContainer, typename Function>
auto transform(InContainer&& container, Function&& function) -> OutContainer {
  OutContainer res;
  transformInto(res, std::forward<InContainer>(container), std::forward<Function>(function));
  return res;
}

template <typename OutputContainer, typename Function, typename Container1, typename Container2>
auto zipWith(Function&& function, Container1 const& cont1, Container2 const& cont2) -> OutputContainer {
  auto it1 = cont1.begin();
  auto it2 = cont2.begin();

  OutputContainer out;
  while (it1 != cont1.end() && it2 != cont2.end()) {
    out.insert(out.end(), function(*it1, *it2));
    ++it1;
    ++it2;
  }

  return out;
}

// Moves the given value and into an rvalue.  Works whether or not the type has
// a valid move constructor or not.  Always leaves the given value in its
// default constructed state.
template <typename T>
auto take(T& t) -> T {
  T t2 = std::move(t);
  t = T();
  return t2;
}

template <typename Container1, typename Container2>
auto containersEqual(Container1 const& cont1, Container2 const& cont2) -> bool {
  if (cont1.size() != cont2.size())
    return false;
  else
    return std::equal(cont1.begin(), cont1.end(), cont2.begin());
}

// Wraps a unary function to produce an output iterator
template <typename UnaryFunction>
class FunctionOutputIterator {
public:
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using difference_type = void;
  using pointer = void;
  using reference = void;

  class OutputProxy {
  public:
    OutputProxy(UnaryFunction& f)
      : m_function(f) {}

    template <typename T>
    auto operator=(T&& value) -> OutputProxy& {
      m_function(std::forward<T>(value));
      return *this;
    }

  private:
    UnaryFunction& m_function;
  };

  explicit FunctionOutputIterator(UnaryFunction f = UnaryFunction())
    : m_function(std::move(f)) {}

  auto operator*() -> OutputProxy {
    return OutputProxy(m_function);
  }

  auto operator++() -> FunctionOutputIterator& {
    return *this;
  }

  auto operator++(int) -> FunctionOutputIterator {
    return *this;
  }

private:
  UnaryFunction m_function;
};

template <typename UnaryFunction>
auto makeFunctionOutputIterator(UnaryFunction f) -> FunctionOutputIterator<UnaryFunction> {
  return FunctionOutputIterator<UnaryFunction>(std::move(f));
}

// Wraps a nullary function to produce an input iterator
template <typename NullaryFunction>
class FunctionInputIterator {
public:
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using difference_type = void;
  using pointer = void;
  using reference = void;

  using FunctionReturnType = std::invoke_result_t<NullaryFunction>;

  explicit FunctionInputIterator(NullaryFunction f = {})
    : m_function(std::move(f)) {}

  auto operator*() -> FunctionReturnType {
    return m_function();
  }

  auto operator++() -> FunctionInputIterator& {
    return *this;
  }

  auto operator++(int) -> FunctionInputIterator {
    return *this;
  }

private:
  NullaryFunction m_function;
};

template <typename NullaryFunction>
auto makeFunctionInputIterator(NullaryFunction f) -> FunctionInputIterator<NullaryFunction> {
  return FunctionInputIterator<NullaryFunction>(std::move(f));
}

template <typename Iterable>
struct ReverseWrapper {
private:
  Iterable& m_iterable;

public:
  ReverseWrapper(Iterable& iterable) : m_iterable(iterable) {}

  auto begin() const -> decltype(auto) {
    return std::rbegin(m_iterable);
  }

  auto end() const -> decltype(auto) {
    return std::rend(m_iterable);
  }
};

template <typename Iterable>
auto reverseIterate(Iterable& list) -> ReverseWrapper<Iterable> {
  return ReverseWrapper<Iterable>(list);
}

template <typename Functor>
class FinallyGuard {
public:
  FinallyGuard(Functor functor) : functor(std::move(functor)) {}

  FinallyGuard(FinallyGuard&& o) : functor(std::move(o.functor)), dismiss(o.dismiss) {
    o.cancel();
  }

  auto operator=(FinallyGuard&& o) -> FinallyGuard& {
    functor = std::move(o.functor);
    dismiss = o.dismiss;
    o.cancel();
    return *this;
  }

  ~FinallyGuard() {
    if (!dismiss)
      functor();
  }

  void cancel() {
    dismiss = true;
  }

private:
  Functor functor;
  bool dismiss{};
};

template <typename Functor>
auto finally(Functor&& f) -> FinallyGuard< std::decay_t<Functor>> {
  return FinallyGuard<Functor>(std::forward<Functor>(f));
}

// Generates compile time sequences of indexes from MinIndex to MaxIndex

template <std::size_t...>
struct IndexSequence {};

template <std::size_t Min, std::size_t N, std::size_t... S>
struct GenIndexSequence : GenIndexSequence<Min, N - 1, N - 1, S...> {};

template <std::size_t Min, std::size_t... S>
struct GenIndexSequence<Min, Min, S...> {
  using type = IndexSequence<S...>;
};

// Apply a tuple as individual arguments to a function

template <typename Function, typename Tuple, std::size_t... Indexes>
auto tupleUnpackFunctionIndexes(Function&& function, Tuple&& args, IndexSequence<Indexes...> const&) -> decltype(auto) {
  return function(get<Indexes>(std::forward<Tuple>(args))...);
}

template <typename Function, typename Tuple>
auto tupleUnpackFunction(Function&& function, Tuple&& args) -> decltype(auto) {
  return tupleUnpackFunctionIndexes<Function, Tuple>(std::forward<Function>(function), std::forward<Tuple>(args),
      typename GenIndexSequence<0, std::tuple_size_v< std::decay_t<Tuple>>>::type());
}

// Apply a function to every element of a tuple.  This will NOT happen in a
// predictable order!

template <typename Function, typename Tuple, std::size_t... Indexes>
auto tupleApplyFunctionIndexes(Function&& function, Tuple&& args, IndexSequence<Indexes...> const&) -> decltype(auto) {
  return make_tuple(function(get<Indexes>(std::forward<Tuple>(args)))...);
}

template <typename Function, typename Tuple>
auto tupleApplyFunction(Function&& function, Tuple&& args) -> decltype(auto) {
  return tupleApplyFunctionIndexes<Function, Tuple>(std::forward<Function>(function), std::forward<Tuple>(args),
      typename GenIndexSequence<0, std::tuple_size_v< std::decay_t<Tuple>>>::type());
}

// Use this version if you do not care about the return value of the function
// or your function returns void.  This version DOES happen in a predictable
// order, first argument first, last argument last.

template <typename Function, typename Tuple>
void tupleCallFunctionCaller(Function&&, Tuple&&) {}

template <typename Tuple, typename Function, typename First, typename... Rest>
void tupleCallFunctionCaller(Tuple&& t, Function&& function) {
  tupleCallFunctionCaller<Tuple, Function, Rest...>(std::forward<Tuple>(t), std::forward<Function>(function));
  function(get<sizeof...(Rest)>(std::forward<Tuple>(t)));
}

template <typename Tuple, typename Function, typename... T>
void tupleCallFunctionExpander(Tuple&& t, Function&& function, std::tuple<T...> const&) {
  tupleCallFunctionCaller<Tuple, Function, T...>(std::forward<Tuple>(t), std::forward<Function>(function));
}

template <typename Tuple, typename Function>
void tupleCallFunction(Tuple&& t, Function&& function) {
  tupleCallFunctionExpander<Tuple, Function>(std::forward<Tuple>(t), std::forward<Function>(function), std::forward<Tuple>(t));
}

// Get a subset of a tuple

template <typename Tuple, std::size_t... Indexes>
auto subTupleIndexes(Tuple&& t, IndexSequence<Indexes...> const&) -> decltype(auto) {
  return make_tuple(get<Indexes>(std::forward<Tuple>(t))...);
}

template <std::size_t Min, std::size_t Size, typename Tuple>
auto subTuple(Tuple&& t) -> decltype(auto) {
  return subTupleIndexes(std::forward<Tuple>(t), GenIndexSequence<Min, Size>::type());
}

template <std::size_t Trim, typename Tuple>
auto trimTuple(Tuple&& t) -> decltype(auto) {
  return subTupleIndexes(std::forward<Tuple>(t), typename GenIndexSequence<Trim, std::tuple_size_v< std::decay_t<Tuple>>>::type());
}

// Unpack a parameter expansion into a container

template <typename Container>
void unpackVariadicImpl(Container&) {}

template <typename Container, typename TFirst, typename... TRest>
void unpackVariadicImpl(Container& container, TFirst&& tfirst, TRest&&... trest) {
  container.insert(container.end(), std::forward<TFirst>(tfirst));
  unpackVariadicImpl(container, std::forward<TRest>(trest)...);
}

template <typename Container, typename... T>
auto unpackVariadic(T&&... t) -> Container {
  Container c;
  unpackVariadicImpl(c, std::forward<T>(t)...);
  return c;
}

// Call a function on each entry in a variadic parameter set

template <typename Function>
void callFunctionVariadic(Function&&) {}

template <typename Function, typename Arg1, typename... ArgRest>
void callFunctionVariadic(Function&& function, Arg1&& arg1, ArgRest&&... argRest) {
  function(arg1);
  callFunctionVariadic(std::forward<Function>(function), std::forward<ArgRest>(argRest)...);
}

template <typename... Rest>
struct VariadicTypedef;

template <>
struct VariadicTypedef<> {};

template <typename FirstT, typename... RestT>
struct VariadicTypedef<FirstT, RestT...> {
  using First = FirstT;
  using Rest = VariadicTypedef<RestT...>;
};

// For generic types, directly use the result of the signature of its
// 'operator()'
template <typename T>
struct FunctionTraits : public FunctionTraits<decltype(&T::operator())> {};

template <typename ReturnType, typename... ArgsTypes>
struct FunctionTraits<ReturnType(ArgsTypes...)> {
  // arity is the number of arguments.
  static constexpr std::size_t Arity = sizeof...(ArgsTypes);

  using Return = ReturnType;

  using Args = VariadicTypedef<ArgsTypes...>;
  using ArgTuple = std::tuple<ArgsTypes...>;

  template <std::size_t i>
  struct Arg {
    // the i-th argument is equivalent to the i-th tuple element of a tuple
    // composed of those arguments.
    using type =  std::tuple_element_t<i, ArgTuple>;
  };
};

template <typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (*)(Args...)> : public FunctionTraits<ReturnType(Args...)> {};

template <typename FunctionType>
struct FunctionTraits<std::function<FunctionType>> : public FunctionTraits<FunctionType> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (ClassType::*)(Args...)> : public FunctionTraits<ReturnType(Args...)> {
  using OwnerType = ClassType&;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (ClassType::*)(Args...) const> : public FunctionTraits<ReturnType(Args...)> {
  using OwnerType = ClassType&;
};

template <typename T>
struct FunctionTraits<T&> : public FunctionTraits<T> {};

template <typename T>
struct FunctionTraits<T const&> : public FunctionTraits<T> {};

template <typename T>
struct FunctionTraits<T&&> : public FunctionTraits<T> {};

template <typename T>
struct FunctionTraits<T const&&> : public FunctionTraits<T> {};

}
