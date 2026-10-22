module;
import std;
import star.format;
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
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, std::optional<T> const& value);
template <typename T>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, std::optional<T>& value);

template <typename... Types>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, std::variant<Types...> const& value);
template <typename... Types>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, std::variant<Types...>& value);

struct DataStreamWriteFunctor {
  DataStreamWriteFunctor(DataStream& ds) : ds(ds) {}

  DataStream& ds;
  template <typename T>
  [[nodiscard]] std::expected<void, StarException> operator()(T const& t) const {
    {
      auto result = ds << t;
      if (!result) return std::unexpected(std::move(result.error()));
    }
    return {};
  }
};

struct DataStreamReadFunctor {
  DataStreamReadFunctor(DataStream& ds) : ds(ds) {}

  DataStream& ds;
  template <typename T>
  [[nodiscard]] std::expected<void, StarException> operator()(T& t) const {
    {
      auto result = ds >> t;
      if (!result) return std::unexpected(std::move(result.error()));
    }
    return {};
  }
};

[[nodiscard]] inline std::expected<DataStream*, StarException> operator<<(DataStream& ds, Empty const&) {
  return &ds;
}

[[nodiscard]] inline std::expected<DataStream*, StarException> operator>>(DataStream& ds, Empty&) {
  return &ds;
}

template <typename ElementT, std::size_t SizeN>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, Array<ElementT, SizeN> const& array) {
  for (std::size_t i = 0; i < SizeN; ++i)
    {
      auto result = ds << array[i];
      if (!result) return std::unexpected(std::move(result.error()));
    }
  return &ds;
}

template <typename ElementT, std::size_t SizeN>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, Array<ElementT, SizeN>& array) {
  for (std::size_t i = 0; i < SizeN; ++i)
    {
      auto result = ds >> array[i];
      if (!result) return std::unexpected(std::move(result.error()));
    }
  return &ds;
}

template <typename ElementT, std::size_t RankN>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, MultiArray<ElementT, RankN> const& array) {
  auto size = array.size();
  for (std::size_t i = 0; i < RankN; ++i)
    {
      auto result = ds.writeVlqU(size[i]);
      if (!result) return std::unexpected(std::move(result.error()));
    }

  std::size_t count = array.count();
  for (std::size_t i = 0; i < count; ++i)
    {
      auto result = ds << array.atIndex(i);
      if (!result) return std::unexpected(std::move(result.error()));
    }

  return &ds;
}

template <typename ElementT, std::size_t RankN>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, MultiArray<ElementT, RankN>& array) {
  typename MultiArray<ElementT, RankN>::SizeList size;
  for (std::size_t i = 0; i < RankN; ++i)
    {
      auto dimension = ds.readVlqU();
      if (!dimension) return std::unexpected(std::move(dimension.error()));
      size[i] = *dimension;
    }

  array.setSize(size);
  std::size_t count = array.count();
  for (std::size_t i = 0; i < count; ++i)
    {
      auto result = ds >> array.atIndex(i);
      if (!result) return std::unexpected(std::move(result.error()));
    }

  return &ds;
}

[[nodiscard]] inline std::expected<DataStream*, StarException> operator<<(DataStream& ds, Color const& color) {
  {
    auto result = ds << color.toRgbaF();
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

[[nodiscard]] inline std::expected<DataStream*, StarException> operator>>(DataStream& ds, Color& color) {
  auto rgba = ds.read<Vec4F>();
  if (!rgba) return std::unexpected(std::move(rgba.error()));
  color = Color::rgbaf(*rgba);
  return &ds;
}

template <typename First, typename Second>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, std::pair<First, Second> const& pair) {
  {
    auto result = ds << pair.first;
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds << pair.second;
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename First, typename Second>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, std::pair<First, Second>& pair) {
  {
    auto result = ds >> pair.first;
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds >> pair.second;
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Element>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, std::shared_ptr<Element> const& ptr) {
  {
    auto result = ds.pwrite(ptr);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Element>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, std::shared_ptr<Element>& ptr) {
  {
    auto result = ds.pread(ptr);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename BaseList>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, ListMixin<BaseList> const& list) {
  {
    auto result = ds.writeContainer(list);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename BaseList>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, ListMixin<BaseList>& list) {
  {
    auto result = ds.readContainer(list);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename BaseSet>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, SetMixin<BaseSet> const& set) {
  {
    auto result = ds.writeContainer(set);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename BaseSet>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, SetMixin<BaseSet>& set) {
  {
    auto result = ds.readContainer(set);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename BaseMap>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, MapMixin<BaseMap> const& map) {
  {
    auto result = ds.writeMapContainer(map);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename BaseMap>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, MapMixin<BaseMap>& map) {
  {
    auto result = ds.readMapContainer(map);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Key, typename Value, typename Compare, typename Allocator>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, OrderedMap<Key, Value, Compare, Allocator>& map) {
  {
    auto result = ds.readMapContainer(map);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Key, typename Value, typename Compare, typename Allocator>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, OrderedMap<Key, Value, Compare, Allocator> const& map) {
  {
    auto result = ds.writeMapContainer(map);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Key, typename Value, typename Hash, typename Equals, typename Allocator>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, OrderedHashMap<Key, Value, Hash, Equals, Allocator>& map) {
  {
    auto result = ds.readMapContainer(map);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Key, typename Value, typename Hash, typename Equals, typename Allocator>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, OrderedHashMap<Key, Value, Hash, Equals, Allocator> const& map) {
  {
    auto result = ds.writeMapContainer(map);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Value, typename Compare, typename Allocator>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, OrderedSet<Value, Compare, Allocator>& set) {
  {
    auto result = ds.readContainer(set);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Value, typename Compare, typename Allocator>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, OrderedSet<Value, Compare, Allocator> const& set) {
  {
    auto result = ds.writeContainer(set);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Value, typename Hash, typename Equals, typename Allocator>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, OrderedHashSet<Value, Hash, Equals, Allocator>& set) {
  {
    auto result = ds.readContainer(set);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename Value, typename Hash, typename Equals, typename Allocator>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, OrderedHashSet<Value, Hash, Equals, Allocator> const& set) {
  {
    auto result = ds.writeContainer(set);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename DataT>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, Polygon<DataT> const& poly) {
  {
    auto result = ds.writeContainer(poly.vertexes());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename DataT>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, Polygon<DataT>& poly) {
  {
    auto result = ds.readContainer(poly.vertexes());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename DataT, std::size_t Dimensions>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, Box<DataT, Dimensions> const& box) {
  {
    auto result = ds.write(box.min());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(box.max());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename DataT, std::size_t Dimensions>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, Box<DataT, Dimensions>& box) {
  {
    auto result = ds.read(box.min());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(box.max());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename DataT>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, Matrix3<DataT> const& mat3) {
  {
    auto result = ds.write(mat3[0]);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(mat3[1]);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(mat3[2]);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename DataT>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, Matrix3<DataT>& mat3) {
  {
    auto result = ds.read(mat3[0]);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(mat3[1]);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(mat3[2]);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

// Variant payloads are decoded before construction.  Non-default-constructible
// alternatives provide DataStream::read<T>() specializations.

namespace detail {
template <std::size_t Index = 0, typename... Types>
[[nodiscard]] std::expected<void, StarException> readVariantPayload(DataStream& ds, std::variant<Types...>& value, std::uint8_t typeIndex) {
  if (typeIndex == Index) {
    using Type = std::variant_alternative_t<Index, std::variant<Types...>>;
    if constexpr (std::is_same_v<Type, std::monostate>) {
      value.template emplace<Index>();
    } else {
      auto payload = ds.read<Type>();
      if (!payload) return std::unexpected(std::move(payload.error()));
      value.template emplace<Index>(std::move(*payload));
    }
    return {};
  } else if constexpr (Index + 1 < sizeof...(Types)) {
    return readVariantPayload<Index + 1>(ds, value, typeIndex);
  } else {
    return std::unexpected(DataStreamException("Invalid variant type index"));
  }
}
}

template <typename... Types>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, std::variant<Types...> const& value) {
  static_assert(sizeof...(Types) <= 255);
  if (value.valueless_by_exception())
    return std::unexpected(DataStreamException("Cannot serialize a valueless variant"));
  auto tag = ds.write<std::uint8_t>(static_cast<std::uint8_t>(value.index()));
  if (!tag) return std::unexpected(std::move(tag.error()));
  auto result = std::visit([&ds] [[nodiscard]] (auto const& payload) -> std::expected<void, StarException> {
    if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(payload)>, std::monostate>) {
      auto written = ds << payload;
      if (!written) return std::unexpected(std::move(written.error()));
    }
    return {};
  }, value);
  if (!result) return std::unexpected(std::move(result.error()));
  return &ds;
}

template <typename... Types>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, std::variant<Types...>& value) {
  static_assert(sizeof...(Types) <= 255);
  auto tag = ds.read<std::uint8_t>();
  if (!tag) return std::unexpected(std::move(tag.error()));
  auto result = detail::readVariantPayload(ds, value, *tag);
  if (!result) return std::unexpected(std::move(result.error()));
  return &ds;
}

// Optional payloads are committed only after decoding succeeds.
template <typename T, typename WriteFunction>
[[nodiscard]] std::expected<void, StarException> writeOptional(DataStream& ds, std::optional<T> const& maybe, WriteFunction&& writeFunction) {
  auto tag = ds.write<bool>(maybe.has_value());
  if (!tag) return std::unexpected(std::move(tag.error()));
  if (maybe) {
    auto result = writeFunction(ds, *maybe);
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return {};
}

template <typename T, typename ReadFunction>
[[nodiscard]] std::expected<void, StarException> readOptional(DataStream& ds, std::optional<T>& maybe, ReadFunction&& readFunction) {
  auto set = ds.read<bool>();
  if (!set) return std::unexpected(std::move(set.error()));
  if (*set) {
    T t;
    auto result = readFunction(ds, t);
    if (!result) return std::unexpected(std::move(result.error()));
    maybe.emplace(std::move(t));
  } else {
    maybe.reset();
  }
  return {};
}

template <typename T>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, std::optional<T> const& maybe) {
  auto result = writeOptional(ds, maybe, [] [[nodiscard]] (DataStream& ds, T const& t) { return ds << t; });
  if (!result) return std::unexpected(std::move(result.error()));
  return &ds;
}

template <typename T>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, std::optional<T>& maybe) {
  auto set = ds.read<bool>();
  if (!set) return std::unexpected(std::move(set.error()));
  if (*set) {
    auto payload = ds.read<T>();
    if (!payload) return std::unexpected(std::move(payload.error()));
    maybe.emplace(std::move(*payload));
  } else {
    maybe.reset();
  }
  return &ds;
}

// Writes / reads an Either type, an Either can either have a left or right
// value, or in edge cases, nothing.

template <typename Left, typename Right>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, Either<Left, Right> const& either) {
  if (either.isLeft()) {
    {
      auto result = ds.write<std::uint8_t>(1);
      if (!result) return std::unexpected(std::move(result.error()));
    }
    {
      auto result = ds.write(*either.leftPtr());
      if (!result) return std::unexpected(std::move(result.error()));
    }
  } else if (either.isRight()) {
    {
      auto result = ds.write<std::uint8_t>(2);
      if (!result) return std::unexpected(std::move(result.error()));
    }
    {
      auto result = ds.write(*either.rightPtr());
      if (!result) return std::unexpected(std::move(result.error()));
    }
  } else {
    {
      auto result = ds.write<std::uint8_t>(0);
      if (!result) return std::unexpected(std::move(result.error()));
    }
  }
  return &ds;
}

template <typename Left, typename Right>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, Either<Left, Right>& either) {
  auto tag = ds.read<std::uint8_t>();
  if (!tag) return std::unexpected(std::move(tag.error()));
  if (*tag == 1) {
    auto payload = ds.read<Left>();
    if (!payload) return std::unexpected(std::move(payload.error()));
    either = makeLeft(std::move(*payload));
  } else if (*tag == 2) {
    auto payload = ds.read<Right>();
    if (!payload) return std::unexpected(std::move(payload.error()));
    either = makeRight(std::move(*payload));
  }
  return &ds;
}

template <typename DataT, std::size_t Dimensions>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, Line<DataT, Dimensions> const& line) {
  {
    auto result = ds.write(line.min());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(line.max());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename DataT, std::size_t Dimensions>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, Line<DataT, Dimensions>& line) {
  {
    auto result = ds.read(line.min());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(line.max());
    if (!result) return std::unexpected(std::move(result.error()));
  }
  return &ds;
}

template <typename... T>
[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, std::tuple<T...>& t) {
  auto result = std::apply([&ds] [[nodiscard]] (auto&... values) -> std::expected<void, StarException> {
    std::expected<void, StarException> status;
    auto readOne = [&] [[nodiscard]] (auto& value) -> std::expected<void, StarException> {
      auto read = ds >> value;
      if (!read) return std::unexpected(std::move(read.error()));
      return {};
    };
    ((status = std::move(status).and_then([&] [[nodiscard]] () { return readOne(values); })), ...);
    return status;
  }, t);
  if (!result) return std::unexpected(std::move(result.error()));
  return &ds;
}

template <typename... T>
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, std::tuple<T...> const& t) {
  auto result = std::apply([&ds] [[nodiscard]] (auto const&... values) -> std::expected<void, StarException> {
    std::expected<void, StarException> status;
    auto writeOne = [&] [[nodiscard]] (auto const& value) -> std::expected<void, StarException> {
      auto written = ds << value;
      if (!written) return std::unexpected(std::move(written.error()));
      return {};
    };
    ((status = std::move(status).and_then([&] [[nodiscard]] () { return writeOne(values); })), ...);
    return status;
  }, t);
  if (!result) return std::unexpected(std::move(result.error()));
  return &ds;
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
