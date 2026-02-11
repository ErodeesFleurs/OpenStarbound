module;

#include "StarColor.hpp"
#include "StarMultiArray.hpp"

export module star.data_stream_extra;

import std;
import star.data_stream;
import star.poly;

namespace Star {

struct DataStreamWriteFunctor {
  explicit DataStreamWriteFunctor(DataStream& ds) : ds(ds) {}

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

inline auto operator<<(DataStream& ds, Empty const&) -> DataStream& {
  return ds;
}

inline auto operator>>(DataStream& ds, Empty&) -> DataStream& {
  return ds;
}

template <typename ElementT, std::size_t SizeN>
auto operator<<(DataStream& ds, Array<ElementT, SizeN> const& array) -> DataStream& {
  for (std::size_t i = 0; i < SizeN; ++i)
    ds << array[i];
  return ds;
}

template <typename ElementT, std::size_t SizeN>
auto operator>>(DataStream& ds, Array<ElementT, SizeN>& array) -> DataStream& {
  for (std::size_t i = 0; i < SizeN; ++i)
    ds >> array[i];
  return ds;
}

template <typename ElementT, std::size_t RankN>
auto operator<<(DataStream& ds, MultiArray<ElementT, RankN> const& array) -> DataStream& {
  auto size = array.size();
  for (std::size_t i = 0; i < RankN; ++i)
    ds.writeVlqU(size[i]);

  std::size_t count = array.count();
  for (std::size_t i = 0; i < count; ++i)
    ds << array.atIndex(i);

  return ds;
}

template <typename ElementT, std::size_t RankN>
auto operator>>(DataStream& ds, MultiArray<ElementT, RankN>& array) -> DataStream& {
  typename MultiArray<ElementT, RankN>::SizeList size;
  for (std::size_t i = 0; i < RankN; ++i)
    size[i] = ds.readVlqU();

  array.setSize(size);
  std::size_t count = array.count();
  for (std::size_t i = 0; i < count; ++i)
    ds >> array.atIndex(i);

  return ds;
}

template <typename T, std::size_t N>
auto operator<<(DataStream& ds, Vector<T, N> const& vector) -> DataStream& {
  for (std::size_t i = 0; i < N; ++i)
    ds << vector[i];
  return ds;
}

template <typename T, std::size_t N>
auto operator>>(DataStream& ds, Vector<T, N>& vector) -> DataStream& {
  for (std::size_t i = 0; i < N; ++i)
    ds >> vector[i];
  return ds;
}

inline auto operator<<(DataStream& ds, Color const& color) -> DataStream& {
  ds << color.to_rgbaf();
  return ds;
}

inline auto operator>>(DataStream& ds, Color& color) -> DataStream& {
  color = Color::rgbaf(ds.read<Vec4F>());
  return ds;
}

template <typename First, typename Second>
auto operator<<(DataStream& ds, std::pair<First, Second> const& pair) -> DataStream& {
  ds << pair.first;
  ds << pair.second;
  return ds;
}

template <typename First, typename Second>
auto operator>>(DataStream& ds, std::pair<First, Second>& pair) -> DataStream& {
  ds >> pair.first;
  ds >> pair.second;
  return ds;
}

template <typename Element>
auto operator<<(DataStream& ds, std::shared_ptr<Element> const& ptr) -> DataStream& {
  ds.pwrite(ptr);
  return ds;
}

template <typename Element>
auto operator>>(DataStream& ds, std::shared_ptr<Element>& ptr) -> DataStream& {
  ds.pread(ptr);
  return ds;
}

template <typename BaseList>
auto operator<<(DataStream& ds, ListMixin<BaseList> const& list) -> DataStream& {
  ds.writeContainer(list);
  return ds;
}

template <typename BaseList>
auto operator>>(DataStream& ds, ListMixin<BaseList>& list) -> DataStream& {
  ds.readContainer(list);
  return ds;
}

template <typename Key, typename Value, typename Compare, typename Allocator>
auto operator>>(DataStream& ds, std::flat_map<Key, Value, Compare, Allocator>& map) -> DataStream& {
  ds.readMapContainer(map);
  return ds;
}

template <typename Key, typename Value, typename Compare, typename Allocator>
auto operator<<(DataStream& ds, std::flat_map<Key, Value, Compare, Allocator> const& map) -> DataStream& {
  ds.writeMapContainer(map);
  return ds;
}

template <typename Key, typename Value, typename Hash, typename Equals, typename Allocator>
auto operator>>(DataStream& ds,  std::flat_map<Key, Value, Hash, Equals, Allocator>& map) -> DataStream& {
  ds.readMapContainer(map);
  return ds;
}

template <typename Key, typename Value, typename Hash, typename Equals, typename Allocator>
auto operator<<(DataStream& ds, std::flat_map<Key, Value, Hash, Equals, Allocator> const& map) -> DataStream& {
  ds.writeMapContainer(map);
  return ds;
}

template <typename Value, typename Compare, typename Allocator>
auto operator>>(DataStream& ds, std::flat_set<Value, Compare, Allocator>& set) -> DataStream& {
  ds.readContainer(set);
  return ds;
}

template <typename Value, typename Compare, typename Allocator>
auto operator<<(DataStream& ds, std::flat_set<Value, Compare, Allocator> const& set) -> DataStream& {
  ds.writeContainer(set);
  return ds;
}

template <typename Value, typename Hash, typename Equals, typename Allocator>
auto operator>>(DataStream& ds, std::flat_set<Value, Equals, Allocator>& set) -> DataStream& {
  ds.readContainer(set);
  return ds;
}

template <typename Value, typename Hash, typename Equals, typename Allocator>
auto operator<<(DataStream& ds, std::flat_set<Value, Equals, Allocator> const& set) -> DataStream& {
  ds.writeContainer(set);
  return ds;
}

template <typename DataT>
auto operator<<(DataStream& ds, Polygon<DataT> const& poly) -> DataStream& {
  ds.writeContainer(poly.vertexes());
  return ds;
}

template <typename DataT>
auto operator>>(DataStream& ds, Polygon<DataT>& poly) -> DataStream& {
  ds.readContainer(poly.vertexes());
  return ds;
}

template <typename DataT, std::size_t Dimensions>
auto operator<<(DataStream& ds, Box<DataT, Dimensions> const& box) -> DataStream& {
  ds.write(box.min());
  ds.write(box.max());
  return ds;
}

template <typename DataT, std::size_t Dimensions>
auto operator>>(DataStream& ds, Box<DataT, Dimensions>& box) -> DataStream& {
  ds.read(box.min());
  ds.read(box.max());
  return ds;
}

template <typename DataT>
auto operator<<(DataStream& ds, Matrix3<DataT> const& mat3) -> DataStream& {
  ds.write(mat3[0]);
  ds.write(mat3[1]);
  ds.write(mat3[2]);
  return ds;
}

template <typename DataT>
auto operator>>(DataStream& ds, Matrix3<DataT>& mat3) -> DataStream& {
  ds.read(mat3[0]);
  ds.read(mat3[1]);
  ds.read(mat3[2]);
  return ds;
}

// Writes / reads a Variant type if every type has operator<< / operator>>
// defined for DataStream and if it is default constructible.

template <typename FirstType, typename... RestTypes>
auto operator<<(DataStream& ds, Variant<FirstType, RestTypes...> const& variant) -> DataStream& {
  ds.write<VariantTypeIndex>(variant.typeIndex());
  variant.call(DataStreamWriteFunctor{ds});
  return ds;
}

template <typename FirstType, typename... RestTypes>
auto operator>>(DataStream& ds, Variant<FirstType, RestTypes...>& variant) -> DataStream& {
  variant.makeType(ds.read<VariantTypeIndex>());
  variant.call(DataStreamReadFunctor{ds});
  return ds;
}

template <typename... AllowedTypes>
auto operator<<(DataStream& ds, MVariant<AllowedTypes...> const& mvariant) -> DataStream& {
  ds.write<VariantTypeIndex>(mvariant.typeIndex());
  mvariant.call(DataStreamWriteFunctor{ds});
  return ds;
}

template <typename... AllowedTypes>
auto operator>>(DataStream& ds, MVariant<AllowedTypes...>& mvariant) -> DataStream& {
  mvariant.makeType(ds.read<VariantTypeIndex>());
  mvariant.call(DataStreamReadFunctor{ds});
  return ds;
}

// Writes / reads a std::optional type if the underlying type has
// operator<< / operator>> defined for DataStream

template <typename T, typename WriteFunction>
void writeMaybe(DataStream& ds, T const& maybe, WriteFunction&& writeFunction) {
  if (maybe) {
    ds.write<bool>(true);
    writeFunction(ds, *maybe);
  } else {
    ds.write<bool>(false);
  }
}

template <typename T, typename ReadFunction>
void readMaybe(DataStream& ds, T& maybe, ReadFunction&& readFunction) {
  bool set = ds.read<bool>();
  if (set) {
    using ValueType = std::decay_t<decltype(*maybe)>;
    ValueType t;
    readFunction(ds, t);
    maybe = std::move(t);
  } else {
    maybe.reset();
  }
}

template <typename T>
auto operator<<(DataStream& ds, std::optional<T> const& maybe) -> DataStream& {
  writeMaybe(ds, maybe, [](DataStream& ds, T const& t) -> auto { ds << t; });
  return ds;
}

template <typename T>
auto operator>>(DataStream& ds, std::optional<T>& maybe) -> DataStream& {
  readMaybe(ds, maybe, [](DataStream& ds, T& t) -> auto { ds >> t; });
  return ds;
}

// Writes / reads an Either type, an Either can either have a left or right
// value, or in edge cases, nothing.

template <typename Left, typename Right>
auto operator<<(DataStream& ds, Either<Left, Right> const& either) -> DataStream& {
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
auto operator>>(DataStream& ds, Either<Left, Right>& either) -> DataStream& {
  auto m = ds.read<std::uint8_t>();
  if (m == 1) {
    either = makeLeft(ds.read<Left>());
  } else if (m == 2) {
    either = makeRight(ds.read<Right>());
}
  return ds;
}

template <typename DataT, std::size_t Dimensions>
auto operator<<(DataStream& ds, Line<DataT, Dimensions> const& line) -> DataStream& {
  ds.write(line.min());
  ds.write(line.max());
  return ds;
}

template <typename DataT, std::size_t Dimensions>
auto operator>>(DataStream& ds, Line<DataT, Dimensions>& line) -> DataStream& {
  ds.read(line.min());
  ds.read(line.max());
  return ds;
}

template <typename T>
auto operator<<(DataStream& ds, std::tuple<T> const& t) -> DataStream& {
  ds << get<0>(t);
  return ds;
}

struct DataStreamReader {
  DataStream& ds;

  template <typename RT>
  void operator()(RT& t) {
    ds >> t;
  }
};

struct DataStreamWriter {
  DataStream& ds;

  template <typename RT>
  void operator()(RT const& t) {
    ds << t;
  }
};

template <typename... T>
auto operator>>(DataStream& ds, std::tuple<T...>& t) -> DataStream& {
  tupleCallFunction(t, DataStreamReader{ds});
  return ds;
}

template <typename... T>
auto operator<<(DataStream& ds, std::tuple<T...> const& t) -> DataStream& {
  tupleCallFunction(t, DataStreamWriter{ds});
  return ds;
}

}// namespace Star
