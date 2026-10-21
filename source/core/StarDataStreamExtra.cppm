module;
import std;
import star.list;
import star.map;
import star.set;
import star.data_stream;
import star.multi_array;
import star.color;
import star.poly;
import star.either;
import star.ordered_map;
import star.ordered_set;

import star.array;
import star.vector;
import star.matrix3;
import star.line;
import star.rect;
import star.algorithm;
namespace Star {

// Declare the std-owned optional codec before generic containers instantiate
// dependent stream expressions; its payload need not associate namespace Star.
template <typename T>
DataStream& operator<<(DataStream& ds, std::optional<T> const& value);
template <typename T>
DataStream& operator>>(DataStream& ds, std::optional<T>& value);

template <typename... Types>
DataStream& operator<<(DataStream& ds, std::variant<Types...> const& value);
template <typename... Types>
DataStream& operator>>(DataStream& ds, std::variant<Types...>& value);

struct DataStreamWriteFunctor {
  DataStreamWriteFunctor(DataStream& ds) : ds(ds) {}

  DataStream& ds;
  template <typename T>
  void operator()(T const& t) const {
    ds << t;
  }
};

struct DataStreamReadFunctor {
  DataStreamReadFunctor(DataStream& ds) : ds(ds) {}

  DataStream& ds;
  template <typename T>
  void operator()(T& t) const {
    ds >> t;
  }
};

inline DataStream& operator<<(DataStream& ds, Empty const&) {
  return ds;
}

inline DataStream& operator>>(DataStream& ds, Empty&) {
  return ds;
}

template <typename ElementT, std::size_t SizeN>
DataStream& operator<<(DataStream& ds, Array<ElementT, SizeN> const& array) {
  for (std::size_t i = 0; i < SizeN; ++i)
    ds << array[i];
  return ds;
}

template <typename ElementT, std::size_t SizeN>
DataStream& operator>>(DataStream& ds, Array<ElementT, SizeN>& array) {
  for (std::size_t i = 0; i < SizeN; ++i)
    ds >> array[i];
  return ds;
}

template <typename ElementT, std::size_t RankN>
DataStream& operator<<(DataStream& ds, MultiArray<ElementT, RankN> const& array) {
  auto size = array.size();
  for (std::size_t i = 0; i < RankN; ++i)
    ds.writeVlqU(size[i]);

  std::size_t count = array.count();
  for (std::size_t i = 0; i < count; ++i)
    ds << array.atIndex(i);

  return ds;
}

template <typename ElementT, std::size_t RankN>
DataStream& operator>>(DataStream& ds, MultiArray<ElementT, RankN>& array) {
  typename MultiArray<ElementT, RankN>::SizeList size;
  for (std::size_t i = 0; i < RankN; ++i)
    size[i] = ds.readVlqU();

  array.setSize(size);
  std::size_t count = array.count();
  for (std::size_t i = 0; i < count; ++i)
    ds >> array.atIndex(i);

  return ds;
}

inline DataStream& operator<<(DataStream& ds, Color const& color) {
  ds << color.toRgbaF();
  return ds;
}

inline DataStream& operator>>(DataStream& ds, Color& color) {
  color = Color::rgbaf(ds.read<Vec4F>());
  return ds;
}

template <typename First, typename Second>
DataStream& operator<<(DataStream& ds, std::pair<First, Second> const& pair) {
  ds << pair.first;
  ds << pair.second;
  return ds;
}

template <typename First, typename Second>
DataStream& operator>>(DataStream& ds, std::pair<First, Second>& pair) {
  ds >> pair.first;
  ds >> pair.second;
  return ds;
}

template <typename Element>
DataStream& operator<<(DataStream& ds, std::shared_ptr<Element> const& ptr) {
  ds.pwrite(ptr);
  return ds;
}

template <typename Element>
DataStream& operator>>(DataStream& ds, std::shared_ptr<Element>& ptr) {
  ds.pread(ptr);
  return ds;
}

template <typename BaseList>
DataStream& operator<<(DataStream& ds, ListMixin<BaseList> const& list) {
  ds.writeContainer(list);
  return ds;
}

template <typename BaseList>
DataStream& operator>>(DataStream& ds, ListMixin<BaseList>& list) {
  ds.readContainer(list);
  return ds;
}

template <typename BaseSet>
DataStream& operator<<(DataStream& ds, SetMixin<BaseSet> const& set) {
  ds.writeContainer(set);
  return ds;
}

template <typename BaseSet>
DataStream& operator>>(DataStream& ds, SetMixin<BaseSet>& set) {
  ds.readContainer(set);
  return ds;
}

template <typename BaseMap>
DataStream& operator<<(DataStream& ds, MapMixin<BaseMap> const& map) {
  ds.writeMapContainer(map);
  return ds;
}

template <typename BaseMap>
DataStream& operator>>(DataStream& ds, MapMixin<BaseMap>& map) {
  ds.readMapContainer(map);
  return ds;
}

template <typename Key, typename Value, typename Compare, typename Allocator>
DataStream& operator>>(DataStream& ds, OrderedMap<Key, Value, Compare, Allocator>& map) {
  ds.readMapContainer(map);
  return ds;
}

template <typename Key, typename Value, typename Compare, typename Allocator>
DataStream& operator<<(DataStream& ds, OrderedMap<Key, Value, Compare, Allocator> const& map) {
  ds.writeMapContainer(map);
  return ds;
}

template <typename Key, typename Value, typename Hash, typename Equals, typename Allocator>
DataStream& operator>>(DataStream& ds, OrderedHashMap<Key, Value, Hash, Equals, Allocator>& map) {
  ds.readMapContainer(map);
  return ds;
}

template <typename Key, typename Value, typename Hash, typename Equals, typename Allocator>
DataStream& operator<<(DataStream& ds, OrderedHashMap<Key, Value, Hash, Equals, Allocator> const& map) {
  ds.writeMapContainer(map);
  return ds;
}

template <typename Value, typename Compare, typename Allocator>
DataStream& operator>>(DataStream& ds, OrderedSet<Value, Compare, Allocator>& set) {
  ds.readContainer(set);
  return ds;
}

template <typename Value, typename Compare, typename Allocator>
DataStream& operator<<(DataStream& ds, OrderedSet<Value, Compare, Allocator> const& set) {
  ds.writeContainer(set);
  return ds;
}

template <typename Value, typename Hash, typename Equals, typename Allocator>
DataStream& operator>>(DataStream& ds, OrderedHashSet<Value, Hash, Equals, Allocator>& set) {
  ds.readContainer(set);
  return ds;
}

template <typename Value, typename Hash, typename Equals, typename Allocator>
DataStream& operator<<(DataStream& ds, OrderedHashSet<Value, Hash, Equals, Allocator> const& set) {
  ds.writeContainer(set);
  return ds;
}

template <typename DataT>
DataStream& operator<<(DataStream& ds, Polygon<DataT> const& poly) {
  ds.writeContainer(poly.vertexes());
  return ds;
}

template <typename DataT>
DataStream& operator>>(DataStream& ds, Polygon<DataT>& poly) {
  ds.readContainer(poly.vertexes());
  return ds;
}

template <typename DataT, std::size_t Dimensions>
DataStream& operator<<(DataStream& ds, Box<DataT, Dimensions> const& box) {
  ds.write(box.min());
  ds.write(box.max());
  return ds;
}

template <typename DataT, std::size_t Dimensions>
DataStream& operator>>(DataStream& ds, Box<DataT, Dimensions>& box) {
  ds.read(box.min());
  ds.read(box.max());
  return ds;
}

template <typename DataT>
DataStream& operator<<(DataStream& ds, Matrix3<DataT> const& mat3) {
  ds.write(mat3[0]);
  ds.write(mat3[1]);
  ds.write(mat3[2]);
  return ds;
}

template <typename DataT>
DataStream& operator>>(DataStream& ds, Matrix3<DataT>& mat3) {
  ds.read(mat3[0]);
  ds.read(mat3[1]);
  ds.read(mat3[2]);
  return ds;
}

// Variant payloads are decoded before construction.  Non-default-constructible
// alternatives provide DataStream::read<T>() specializations.

namespace detail {
template <std::size_t Index = 0, typename... Types>
void readVariantPayload(DataStream& ds, std::variant<Types...>& value, std::uint8_t typeIndex) {
  if (typeIndex == Index) {
    using Type = std::variant_alternative_t<Index, std::variant<Types...>>;
    if constexpr (std::is_same_v<Type, std::monostate>)
      value.template emplace<Index>();
    else
      value.template emplace<Index>(ds.read<Type>());
  } else if constexpr (Index + 1 < sizeof...(Types)) {
    readVariantPayload<Index + 1>(ds, value, typeIndex);
  } else {
    throw DataStreamException("Invalid variant type index");
  }
}
}

template <typename... Types>
DataStream& operator<<(DataStream& ds, std::variant<Types...> const& value) {
  static_assert(sizeof...(Types) <= 255);
  if (value.valueless_by_exception())
    throw DataStreamException("Cannot serialize a valueless variant");
  ds.write<std::uint8_t>(static_cast<std::uint8_t>(value.index()));
  std::visit([&ds](auto const& payload) {
    if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(payload)>, std::monostate>)
      ds << payload;
  }, value);
  return ds;
}

template <typename... Types>
DataStream& operator>>(DataStream& ds, std::variant<Types...>& value) {
  static_assert(sizeof...(Types) <= 255);
  detail::readVariantPayload(ds, value, ds.read<std::uint8_t>());
  return ds;
}

// Writes / reads a std::optional type if the underlying type has operator<< /
// operator>> defined for DataStream

template <typename T, typename WriteFunction>
void writeOptional(DataStream& ds, std::optional<T> const& maybe, WriteFunction&& writeFunction) {
  if (maybe) {
    ds.write<bool>(true);
    writeFunction(ds, *maybe);
  } else {
    ds.write<bool>(false);
  }
}

template <typename T, typename ReadFunction>
void readOptional(DataStream& ds, std::optional<T>& maybe, ReadFunction&& readFunction) {
  bool set = ds.read<bool>();
  if (set) {
    T t;
    readFunction(ds, t);
    maybe.emplace(std::move(t));
  } else {
    maybe.reset();
  }
}

template <typename T>
DataStream& operator<<(DataStream& ds, std::optional<T> const& maybe) {
  writeOptional(ds, maybe, [](DataStream& ds, T const& t) { ds << t; });
  return ds;
}

template <typename T>
DataStream& operator>>(DataStream& ds, std::optional<T>& maybe) {
  if (ds.read<bool>())
    maybe.emplace(ds.read<T>());
  else
    maybe.reset();
  return ds;
}

// Writes / reads an Either type, an Either can either have a left or right
// value, or in edge cases, nothing.

template <typename Left, typename Right>
DataStream& operator<<(DataStream& ds, Either<Left, Right> const& either) {
  if (either.isLeft()) {
    ds.write<std::uint8_t>(1);
    ds.write(either.left());
  } else if (either.isRight()) {
    ds.write<std::uint8_t>(2);
    ds.write(either.right());
  } else {
    ds.write<std::uint8_t>(0);
  }
  return ds;
}

template <typename Left, typename Right>
DataStream& operator>>(DataStream& ds, Either<Left, Right>& either) {
  std::uint8_t m = ds.read<std::uint8_t>();
  if (m == 1)
    either = makeLeft(ds.read<Left>());
  else if (m == 2)
    either = makeRight(ds.read<Right>());
  return ds;
}

template <typename DataT, std::size_t Dimensions>
DataStream& operator<<(DataStream& ds, Line<DataT, Dimensions> const& line) {
  ds.write(line.min());
  ds.write(line.max());
  return ds;
}

template <typename DataT, std::size_t Dimensions>
DataStream& operator>>(DataStream& ds, Line<DataT, Dimensions>& line) {
  ds.read(line.min());
  ds.read(line.max());
  return ds;
}

template <typename T>
DataStream& operator<<(DataStream& ds, std::tuple<T> const& t) {
  ds << std::get<0>(t);
  return ds;
}


template <typename... T>
DataStream& operator>>(DataStream& ds, std::tuple<T...>& t) {
  std::apply([&ds](auto&... values) {
    (static_cast<void>(ds >> values), ...);
  }, t);
  return ds;
}

template <typename... T>
DataStream& operator<<(DataStream& ds, std::tuple<T...> const& t) {
  std::apply([&ds](auto const&... values) {
    (static_cast<void>(ds << values), ...);
  }, t);
  return ds;
}

}

export module star.data_stream_extra;

export namespace Star {
  using ::Star::DataStreamWriteFunctor;
  using ::Star::DataStreamReadFunctor;
  using ::Star::operator<<;
  using ::Star::operator>>;
  using ::Star::writeOptional;
  using ::Star::readOptional;
}
