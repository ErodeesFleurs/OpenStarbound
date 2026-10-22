module;

import std;
import star.format;
import star.rect;
import star.vector;
import star.string;
import star.optional;
import star.lua;
import star.color;
import star.poly;
import star.line;
import star.rpc_promise;

import star.matrix3;
namespace Star {

template <typename T>
struct LuaConverter<LuaNullTermWrapper<T>> : LuaConverter<T> {
  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, LuaNullTermWrapper<T>&& v) {
    auto enforcer = engine.nullTerminate();
    return LuaConverter<T>::from(engine, std::forward<T>(v));
  }

  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, LuaNullTermWrapper<T> const& v) {
    auto enforcer = engine.nullTerminate();
    return LuaConverter<T>::from(engine, v);
  }

  [[nodiscard]] static std::expected<std::optional<LuaNullTermWrapper<T>>, StarException> to(LuaEngine& engine, LuaValue const& v) {
    auto enforcer = engine.nullTerminate();
    auto converted = engine.luaMaybeTo<T>(v);
    if (!converted)
      return std::unexpected(std::move(converted.error()));
    if (!*converted)
      return std::nullopt;
    return LuaNullTermWrapper<T>(std::move(**converted));
  }
};

template <typename T1, typename T2>
struct LuaConverter<std::pair<T1, T2>> {
  template <typename Pair>
  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, Pair&& v) {
    auto created = engine.createTable();
    if (!created)
      return std::unexpected(std::move(created.error()));
    auto t = std::move(*created);
    auto first = t.set(1, std::forward<Pair>(v).first);
    if (!first)
      return std::unexpected(std::move(first.error()));
    auto second = t.set(2, std::forward<Pair>(v).second);
    if (!second)
      return std::unexpected(std::move(second.error()));
    return t;
  }

  [[nodiscard]] static std::expected<std::optional<std::pair<T1, T2>>, StarException> to(LuaEngine& engine, LuaValue const& v) {
    auto table = std::get_if<LuaTable>(&v);
    if (!table)
      return std::nullopt;
    auto first = table->get<LuaValue>(1);
    if (!first)
      return std::unexpected(std::move(first.error()));
    auto p1 = engine.luaMaybeTo<T1>(*first);
    if (!p1)
      return std::unexpected(std::move(p1.error()));
    if (!*p1)
      return std::nullopt;
    auto second = table->get<LuaValue>(2);
    if (!second)
      return std::unexpected(std::move(second.error()));
    auto p2 = engine.luaMaybeTo<T2>(*second);
    if (!p2)
      return std::unexpected(std::move(p2.error()));
    if (!*p2)
      return std::nullopt;
    return std::optional<std::pair<T1, T2>>(std::in_place, std::move(**p1), std::move(**p2));
  }
};

template <typename T, std::size_t N>
struct LuaConverter<Vector<T, N>> {
  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, Vector<T, N> const& v) {
    auto table = engine.createArrayTable(v);
    if (!table)
      return std::unexpected(std::move(table.error()));
    return std::move(*table);
  }

  [[nodiscard]] static std::expected<std::optional<Vector<T, N>>, StarException> to(LuaEngine& engine, LuaValue const& v) {
    auto table = std::get_if<LuaTable>(&v);
    if (!table)
      return std::nullopt;
    Vector<T, N> vec;
    for (std::size_t i = 0; i < N; ++i) {
      auto value = table->get<LuaValue>(i + 1);
      if (!value)
        return std::unexpected(std::move(value.error()));
      auto converted = engine.luaMaybeTo<T>(*value);
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      if (!*converted)
        return std::nullopt;
      vec[i] = std::move(**converted);
    }
    return vec;
  }
};

template <typename T>
struct LuaConverter<Matrix3<T>> {
  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, Matrix3<T> const& m) {
    auto created = engine.createTable(3, 0);
    if (!created)
      return std::unexpected(std::move(created.error()));
    auto table = std::move(*created);
    for (std::size_t i = 0; i < 3; ++i) {
      auto result = table.set(i + 1, m[i]);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
    return table;
  }

  [[nodiscard]] static std::expected<std::optional<Matrix3<T>>, StarException> to(LuaEngine& engine, LuaValue const& v) {
    auto rows = LuaConverter<Vector<typename Matrix3<T>::Vec3, 3>>::to(engine, v);
    if (!rows)
      return std::unexpected(std::move(rows.error()));
    if (!*rows)
      return std::nullopt;
    return Matrix3<T>((**rows)[0], (**rows)[1], (**rows)[2]);
  }
};

template <typename T>
struct LuaConverter<Rect<T>> {
  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, Rect<T> const& v) {
    if (v.isNull())
      return LuaNil;
    return LuaConverter<Vector<T, 4>>::from(engine, Vector<T, 4>(v.xMin(), v.yMin(), v.xMax(), v.yMax()));
  }

  [[nodiscard]] static std::expected<std::optional<Rect<T>>, StarException> to(LuaEngine& engine, LuaValue const& v) {
    if (v == LuaNil)
      return Rect<T>::null();
    auto bounds = LuaConverter<Vector<T, 4>>::to(engine, v);
    if (!bounds)
      return std::unexpected(std::move(bounds.error()));
    if (!*bounds)
      return std::nullopt;
    return Rect<T>((**bounds)[0], (**bounds)[1], (**bounds)[2], (**bounds)[3]);
  }
};

template <typename T>
struct LuaConverter<Polygon<T>> {
  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, Polygon<T> const& poly) {
    auto table = engine.createArrayTable(poly.vertexes());
    if (!table)
      return std::unexpected(std::move(table.error()));
    return std::move(*table);
  }

  [[nodiscard]] static std::expected<std::optional<Polygon<T>>, StarException> to(LuaEngine& engine, LuaValue const& v) {
    auto points = engine.luaMaybeTo<typename Polygon<T>::VertexList>(v);
    if (!points)
      return std::unexpected(std::move(points.error()));
    if (!*points)
      return std::nullopt;
    return Polygon<T>(std::move(**points));
  }
};

template <typename T, std::size_t N>
struct LuaConverter<Line<T, N>> {
  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, Line<T, N> const& line) {
    return LuaConverter<std::pair<Vector<T, N>, Vector<T, N>>>::from(engine, std::pair(line.min(), line.max()));
  }

  [[nodiscard]] static std::expected<std::optional<Line<T, N>>, StarException> to(LuaEngine& engine, LuaValue const& v) {
    auto endpoints = LuaConverter<std::pair<Vector<T, N>, Vector<T, N>>>::to(engine, v);
    if (!endpoints)
      return std::unexpected(std::move(endpoints.error()));
    if (!*endpoints)
      return std::nullopt;
    return Line<T, N>((**endpoints).first, (**endpoints).second);
  }
};

// Probe alternatives in declaration order; failed probes must not consume v.
// A leading monostate represents successful nil, not conversion failure.
template <typename... Types>
struct LuaConverter<std::variant<Types...>> {
  using Value = std::variant<Types...>;

  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, Value const& value) {
    return std::visit([&engine](auto const& payload) -> std::expected<LuaValue, StarException> {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(payload)>, std::monostate>)
        return LuaNil;
      else
        return engine.luaFrom(payload);
    }, value);
  }

  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, Value&& value) {
    return std::visit([&engine](auto& payload) -> std::expected<LuaValue, StarException> {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(payload)>, std::monostate>)
        return LuaNil;
      else
        return engine.luaFrom(std::move(payload));
    }, value);
  }

  template <std::size_t Index = 0, typename Input>
  [[nodiscard]] static std::expected<std::optional<Value>, StarException> checkTypeTo(LuaEngine& engine, Input&& v) {
    using Type = std::variant_alternative_t<Index, Value>;
    if constexpr (std::is_same_v<Type, std::monostate>) {
      if (v == LuaNil)
        return std::optional<Value>(std::in_place, std::in_place_index<Index>);
    } else {
      auto converted = [&] {
        if constexpr (Index + 1 == sizeof...(Types))
          return engine.luaMaybeTo<Type>(std::forward<Input>(v));
        else
          return engine.luaMaybeTo<Type>(v);
      }();
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      if (*converted)
        return std::optional<Value>(std::in_place, std::in_place_index<Index>, std::move(**converted));
    }
    if constexpr (Index + 1 < sizeof...(Types))
      return checkTypeTo<Index + 1>(engine, std::forward<Input>(v));
    else
      return std::nullopt;
  }

  [[nodiscard]] static std::expected<std::optional<Value>, StarException> to(LuaEngine& engine, LuaValue const& v) {
    return checkTypeTo(engine, v);
  }

  [[nodiscard]] static std::expected<std::optional<Value>, StarException> to(LuaEngine& engine, LuaValue&& v) {
    return checkTypeTo(engine, std::move(v));
  }
};

template <>
struct LuaConverter<Color> {
  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, Color const& c);
  [[nodiscard]] static std::expected<std::optional<Color>, StarException> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<LuaCallbacks> {
  [[nodiscard]] static std::expected<LuaValue, StarException> from(LuaEngine& engine, LuaCallbacks const& c);
  [[nodiscard]] static std::expected<std::optional<LuaCallbacks>, StarException> to(LuaEngine& engine, LuaValue const& v);
};

template <typename T>
struct LuaConverter<RpcPromise<T>> : LuaUserDataConverter<RpcPromise<T>> {};

template <typename T>
struct LuaUserDataMethods<RpcPromise<T>> {
  [[nodiscard]] static std::expected<LuaMethods<RpcPromise<T>>, StarException> make();
};

template <typename T>
[[nodiscard]] std::expected<LuaMethods<RpcPromise<T>>, StarException> LuaUserDataMethods<RpcPromise<T>>::make() {
  LuaMethods<RpcPromise<T>> methods;
  auto finished = methods.template registerMethodWithSignature<bool, RpcPromise<T>&>("finished", std::mem_fn(&RpcPromise<T>::finished));
  if (!finished)
    return std::unexpected(std::move(finished.error()));
  auto succeeded = methods.template registerMethodWithSignature<bool, RpcPromise<T>&>("succeeded", std::mem_fn(&RpcPromise<T>::succeeded));
  if (!succeeded)
    return std::unexpected(std::move(succeeded.error()));
  auto result = methods.template registerMethodWithSignature<std::optional<T>, RpcPromise<T>&>("result", std::mem_fn(&RpcPromise<T>::result));
  if (!result)
    return std::unexpected(std::move(result.error()));
  auto error = methods.template registerMethodWithSignature<std::optional<String>, RpcPromise<T>&>("error", std::mem_fn(&RpcPromise<T>::error));
  if (!error)
    return std::unexpected(std::move(error.error()));
  return methods;
}

// all RpcPromiseKeepers should be wrapped in this before being passed to Lua
template <typename T>
struct LuaConverter<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> : LuaUserDataConverter<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> {};

template <typename T>
struct LuaUserDataMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> {
  [[nodiscard]] static std::expected<LuaMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>>, StarException> make();
};

template <typename T>
[[nodiscard]] std::expected<LuaMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>>, StarException> LuaUserDataMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>>::make() {
  LuaMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> methods;
  auto chain = methods.template registerMethodWithSignature<void, std::shared_ptr<AutoFailRpcPromiseKeeper<T>>, RpcPromise<T>>("chain", [](std::shared_ptr<AutoFailRpcPromiseKeeper<T>> const& keeper, RpcPromise<T> const& promise) {
    return keeper->chain(promise);
  });
  if (!chain)
    return std::unexpected(std::move(chain.error()));
  auto fulfill = methods.template registerMethodWithSignature<void, std::shared_ptr<AutoFailRpcPromiseKeeper<T>>, T>("fulfill", [](std::shared_ptr<AutoFailRpcPromiseKeeper<T>> const& keeper, T const& result) {
    return keeper->fulfill(result);
  });
  if (!fulfill)
    return std::unexpected(std::move(fulfill.error()));
  auto fail = methods.template registerMethodWithSignature<void, std::shared_ptr<AutoFailRpcPromiseKeeper<T>>, String>("fail", [](std::shared_ptr<AutoFailRpcPromiseKeeper<T>> const& keeper, String const& error) {
    return keeper->fail(error);
  });
  if (!fail)
    return std::unexpected(std::move(fail.error()));
  return methods;
}

}

export module star.lua_converters;

export namespace Star {
  using ::Star::LuaConverter;
  using ::Star::LuaUserDataMethods;

  template <typename T>
  struct LuaConverter<LuaNullTermWrapper<T>>;
  template <typename T1, typename T2>
  struct LuaConverter<std::pair<T1, T2>>;
  template <typename T, std::size_t N>
  struct LuaConverter<Vector<T, N>>;
  template <typename T>
  struct LuaConverter<Matrix3<T>>;
  template <typename T>
  struct LuaConverter<Rect<T>>;
  template <typename T>
  struct LuaConverter<Polygon<T>>;
  template <typename T, std::size_t N>
  struct LuaConverter<Line<T, N>>;
  template <typename... Types>
  struct LuaConverter<std::variant<Types...>>;
  template <>
  struct LuaConverter<Color>;
  template <>
  struct LuaConverter<LuaCallbacks>;
  template <typename T>
  struct LuaConverter<RpcPromise<T>>;
  template <typename T>
  struct LuaUserDataMethods<RpcPromise<T>>;
  template <typename T>
  struct LuaConverter<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>>;
  template <typename T>
  struct LuaUserDataMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>>;
}
