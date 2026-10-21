module;
import star.format;
import std;
import star.output_proxy;

namespace Star {

// Function that does nothing and takes any number of arguments
template <typename... T>
void nothing(T&&...) {}

// Functional constructor call / casting.
template <typename ToType>
struct construct {
  template <typename... FromTypes>
  ToType operator()(FromTypes&&... fromTypes) const {
    return ToType(std::forward<FromTypes>(fromTypes)...);
  }
};

template <typename Func>
struct SwallowReturn {
  template <typename... T>
  void operator()(T&&... args) {
    func(std::forward<T>(args)...);
  }

  Func func;
};

template <typename Func>
SwallowReturn<Func> swallow(Func f) {
  return SwallowReturn<Func>{std::move(f)};
}

struct Empty {
  bool operator==(Empty const) const {
    return true;
  }

  bool operator<(Empty const) const {
    return false;
  }
};

// Return intersection of sorted containers.
template <typename Container>
Container intersect(Container const& a, Container const& b) {
  Container r;
  std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(r, r.end()));
  return r;
}

template <typename MapType1, typename MapType2>
bool mapMerge(MapType1& targetMap, MapType2 const& sourceMap, bool overwrite = false) {
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
bool mapsEqual(MapType1 const& m1, MapType2 const& m2) {
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
OutContainer filtered(InContainer const& input, Filter&& filter) {
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
Container sorted(Container const& c, Compare comp) {
  auto c2 = c;
  std::sort(c2.begin(), c2.end(), comp);
  return c2;
}

template <typename Container, typename Compare>
Container stableSorted(Container const& c, Compare comp) {
  auto c2 = c;
  std::sort(c2.begin(), c2.end(), comp);
  return c2;
}

template <typename Container>
Container sorted(Container const& c) {
  auto c2 = c;
  std::sort(c2.begin(), c2.end(), std::less<typename Container::value_type>());
  return c2;
}

template <typename Container>
Container stableSorted(Container const& c) {
  auto c2 = c;
  std::sort(c2.begin(), c2.end(), std::less<typename Container::value_type>());
  return c2;
}

// Sort a container by the output of a computed value. The computed value is
// only computed *once* per item in the container, which is useful both for
// when the computed value is costly, and to avoid sorting instability with
// floating point values.  Container must have size() and operator[], and also
// must be constructable with Container(size_t).
template <typename Container, typename Getter>
void sortByComputedValue(Container& container, Getter&& valueGetter, bool stable = false) {
  typedef typename Container::value_type ContainerValue;
  typedef decltype(valueGetter(ContainerValue())) ComputedValue;
  typedef std::pair<ComputedValue, std::size_t> ComputedPair;

  std::size_t containerSize = container.size();

  if (containerSize <= 1)
    return;

  std::vector<ComputedPair> work(containerSize);
  for (std::size_t i = 0; i < containerSize; ++i)
    work[i] = {valueGetter(container[i]), i};

  auto compare = [](ComputedPair const& a, ComputedPair const& b) { return a.first < b.first; };

  // Sort the comptued values and the associated indexes
  if (stable)
    std::stable_sort(work.begin(), work.end(), compare);
  else
    std::sort(work.begin(), work.end(), compare);

  Container result(containerSize);
  for (std::size_t i = 0; i < containerSize; ++i) {
    if constexpr (requires { swap(result[i], container[work[i].second]); })
      swap(result[i], container[work[i].second]);
    else
      std::swap(result[i], container[work[i].second]);
  }

  if constexpr (requires { swap(container, result); })
    swap(container, result);
  else
    std::swap(container, result);
}

template <typename Container, typename Getter>
void stableSortByComputedValue(Container& container, Getter&& valueGetter) {
  return sortByComputedValue(container, std::forward<Getter>(valueGetter), true);
}

template <typename Container>
Container reverseCopy(Container c) {
  std::reverse(c.begin(), c.end());
  return c;
}

template <typename T>
T copy(T c) {
  return c;
}

template <typename OutContainer, typename InContainer, typename Function>
void transformInto(OutContainer& outContainer, InContainer&& inContainer, Function&& function) {
  for (auto&& elem : inContainer) {
    if (std::is_rvalue_reference<InContainer&&>::value)
      outContainer.insert(outContainer.end(), function(std::move(elem)));
    else
      outContainer.insert(outContainer.end(), function(elem));
  }
}

template <typename OutContainer, typename InContainer, typename Function>
OutContainer transform(InContainer&& container, Function&& function) {
  OutContainer res;
  transformInto(res, std::forward<InContainer>(container), std::forward<Function>(function));
  return res;
}

// Moves the given value and into an rvalue.  Works whether or not the type has
// a valid move constructor or not.  Always leaves the given value in its
// default constructed state.
// Default construction follows the move; std::exchange constructs its
// replacement first, which differs if default construction throws.
template <typename T>
T take(T& t) {
  T t2 = std::move(t);
  t = T();
  return t2;
}

// Wraps a unary function to produce an output iterator
template <typename UnaryFunction>
class FunctionOutputIterator {
public:
  typedef std::output_iterator_tag iterator_category;
  typedef void value_type;
  typedef void difference_type;
  typedef void pointer;
  typedef void reference;

  class OutputProxy {
  public:
    OutputProxy(UnaryFunction& f)
      : m_function(f) {}

    template <typename T>
    OutputProxy& operator=(T&& value) {
      m_function(std::forward<T>(value));
      return *this;
    }

  private:
    UnaryFunction& m_function;
  };

  explicit FunctionOutputIterator(UnaryFunction f = UnaryFunction())
    : m_function(std::move(f)) {}

  OutputProxy operator*() {
    return OutputProxy(m_function);
  }

  FunctionOutputIterator& operator++() {
    return *this;
  }

  FunctionOutputIterator operator++(int) {
    return *this;
  }

private:
  UnaryFunction m_function;
};

template <typename UnaryFunction>
FunctionOutputIterator<UnaryFunction> makeFunctionOutputIterator(UnaryFunction f) {
  return FunctionOutputIterator<UnaryFunction>(std::move(f));
}

// Wraps a nullary function to produce an input iterator
template <typename NullaryFunction>
class FunctionInputIterator {
public:
  typedef std::output_iterator_tag iterator_category;
  typedef void value_type;
  typedef void difference_type;
  typedef void pointer;
  typedef void reference;

  using FunctionReturnType = std::invoke_result_t<NullaryFunction&>;

  explicit FunctionInputIterator(NullaryFunction f = {})
    : m_function(std::move(f)) {}

  FunctionReturnType operator*() {
    return m_function();
  }

  FunctionInputIterator& operator++() {
    return *this;
  }

  FunctionInputIterator operator++(int) {
    return *this;
  }

private:
  NullaryFunction m_function;
};

template <typename NullaryFunction>
FunctionInputIterator<NullaryFunction> makeFunctionInputIterator(NullaryFunction f) {
  return FunctionInputIterator<NullaryFunction>(std::move(f));
}

template <typename Functor>
class FinallyGuard {
public:
  FinallyGuard(Functor functor) : functor(std::move(functor)), dismiss(false) {}

  FinallyGuard(FinallyGuard&& o) : functor(std::move(o.functor)), dismiss(o.dismiss) {
    o.cancel();
  }

  FinallyGuard& operator=(FinallyGuard&& o) {
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
  bool dismiss;
};

template <typename Functor>
FinallyGuard<typename std::decay<Functor>::type> finally(Functor&& f) {
  return FinallyGuard<Functor>(std::forward<Functor>(f));
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
Container unpackVariadic(T&&... t) {
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
    using type = std::tuple_element_t<i, ArgTuple>;
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
  using OwnerType = ClassType const&;
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

export module star.algorithm;

export namespace Star {
  using ::Star::nothing;
  using ::Star::construct;
  using ::Star::SwallowReturn;
  using ::Star::swallow;
  using ::Star::Empty;
  using ::Star::intersect;
  using ::Star::mapMerge;
  using ::Star::mapsEqual;
  using ::Star::filter;
  using ::Star::filtered;
  using ::Star::eraseWhere;
  using ::Star::sorted;
  using ::Star::stableSorted;
  using ::Star::sortByComputedValue;
  using ::Star::stableSortByComputedValue;
  using ::Star::reverseCopy;
  using ::Star::copy;
  using ::Star::transformInto;
  using ::Star::transform;
  using ::Star::take;
  using ::Star::FunctionOutputIterator;
  using ::Star::makeFunctionOutputIterator;
  using ::Star::FunctionInputIterator;
  using ::Star::makeFunctionInputIterator;
  using ::Star::FinallyGuard;
  using ::Star::finally;
  using ::Star::unpackVariadicImpl;
  using ::Star::unpackVariadic;
  using ::Star::callFunctionVariadic;
  using ::Star::VariadicTypedef;
  using ::Star::FunctionTraits;
}
