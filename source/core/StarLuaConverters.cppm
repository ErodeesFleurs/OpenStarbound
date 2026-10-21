module;

import std;
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
  static LuaValue from(LuaEngine& engine, LuaNullTermWrapper<T>&& v) {
    auto enforcer = engine.nullTerminate();
    return LuaConverter<T>::from(engine, std::forward<T>(v));
  }

  static LuaValue from(LuaEngine& engine, LuaNullTermWrapper<T> const& v) {
    auto enforcer = engine.nullTerminate();
    return LuaConverter<T>::from(engine, v);
  }

  static LuaNullTermWrapper<T> to(LuaEngine& engine, LuaValue const& v) {
    auto enforcer = engine.nullTerminate();
    return LuaConverter<T>::to(engine, v);
  }
};

template <typename T1, typename T2>
struct LuaConverter<std::pair<T1, T2>> {
  static LuaValue from(LuaEngine& engine, std::pair<T1, T2>&& v) {
    auto t = engine.createTable();
    t.set(1, std::move(v.first));
    t.set(2, std::move(v.second));
    return t;
  }

  static LuaValue from(LuaEngine& engine, std::pair<T1, T2> const& v) {
    auto t = engine.createTable();
    t.set(1, v.first);
    t.set(2, v.second);
    return t;
  }

  static std::optional<std::pair<T1, T2>> to(LuaEngine& engine, LuaValue const& v) {
    if (auto table = engine.luaMaybeTo<LuaTable>(std::move(v))) {
      auto p1 = engine.luaMaybeTo<T1>(table->get(1));
      auto p2 = engine.luaMaybeTo<T2>(table->get(2));
      if (p1 && p2)
        return std::optional<std::pair<T1, T2>>(std::in_place, takeOptional(p1), takeOptional(p2));
    }
    return {};
  }
};

template <typename T, std::size_t N>
struct LuaConverter<Vector<T, N>> {
  static LuaValue from(LuaEngine& engine, Vector<T, N> const& v) {
    return engine.createArrayTable(v);
  }

  static std::optional<Vector<T, N>> to(LuaEngine& engine, LuaValue const& v) {
    auto table = std::get_if<LuaTable>(&v);
    if (!table)
      return {};

    Vector<T, N> vec;
    for (std::size_t i = 0; i < N; ++i) {
      auto v = engine.luaMaybeTo<T>(table->get(i + 1));
      if (!v)
        return {};
      vec[i] = *v;
    }
    return vec;
  }
};

template <typename T>
struct LuaConverter<Matrix3<T>> {
  static LuaValue from(LuaEngine& engine, Matrix3<T> const& m) {
    auto table = engine.createTable(3, 0);
    table.set(1, m[0]);
    table.set(2, m[1]);
    table.set(3, m[2]);
    return table;
  }

  static std::optional<Matrix3<T>> to(LuaEngine& engine, LuaValue const& v) {
    if (auto table = std::get_if<LuaTable>(&v)) {
      auto r1 = engine.luaMaybeTo<typename Matrix3<T>::Vec3>(table->get(1));
      auto r2 = engine.luaMaybeTo<typename Matrix3<T>::Vec3>(table->get(2));
      auto r3 = engine.luaMaybeTo<typename Matrix3<T>::Vec3>(table->get(3));
      if (r1 && r2 && r3)
        return Matrix3<T>(*r1, *r2, *r3);
    }
    return {};
  }
};

template <typename T>
struct LuaConverter<Rect<T>> {
  static LuaValue from(LuaEngine& engine, Rect<T> const& v) {
    if (v.isNull())
      return LuaNil;
    auto t = engine.createTable();
    t.set(1, v.xMin());
    t.set(2, v.yMin());
    t.set(3, v.xMax());
    t.set(4, v.yMax());
    return t;
  }

  static std::optional<Rect<T>> to(LuaEngine& engine, LuaValue const& v) {
    if (v == LuaNil)
      return Rect<T>::null();

    if (auto table = std::get_if<LuaTable>(&v)) {
      auto xMin = engine.luaMaybeTo<T>(table->get(1));
      auto yMin = engine.luaMaybeTo<T>(table->get(2));
      auto xMax = engine.luaMaybeTo<T>(table->get(3));
      auto yMax = engine.luaMaybeTo<T>(table->get(4));
      if (xMin && yMin && xMax && yMax)
        return Rect<T>(*xMin, *yMin, *xMax, *yMax);
    }
    return {};
  }
};

template <typename T>
struct LuaConverter<Polygon<T>> {
  static LuaValue from(LuaEngine& engine, Polygon<T> const& poly) {
    return engine.createArrayTable(poly.vertexes());
  }

  static std::optional<Polygon<T>> to(LuaEngine& engine, LuaValue const& v) {
    if (auto points = engine.luaMaybeTo<typename Polygon<T>::VertexList>(v))
      return Polygon<T>(takeOptional(points));
    return {};
  }
};

template <typename T, std::size_t N>
struct LuaConverter<Line<T, N>> {
  static LuaValue from(LuaEngine& engine, Line<T, N> const& line) {
    auto table = engine.createTable();
    table.set(1, line.min());
    table.set(2, line.max());
    return table;
  }

  static std::optional<Line<T, N>> to(LuaEngine& engine, LuaValue const& v) {
    if (auto table = std::get_if<LuaTable>(&v)) {
      auto min = engine.luaMaybeTo<Vector<T, N>>(table->get(1));
      auto max = engine.luaMaybeTo<Vector<T, N>>(table->get(2));
      if (min && max)
        return Line<T, N>(*min, *max);
    }
    return {};
  }
};

// Probe alternatives in declaration order; failed probes must not consume v.
// A leading monostate represents successful nil, not conversion failure.
template <typename... Types>
struct LuaConverter<std::variant<Types...>> {
  using Value = std::variant<Types...>;

  static LuaValue from(LuaEngine& engine, Value const& value) {
    return std::visit([&engine](auto const& payload) -> LuaValue {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(payload)>, std::monostate>)
        return LuaNil;
      else
        return engine.luaFrom(payload);
    }, value);
  }

  static LuaValue from(LuaEngine& engine, Value&& value) {
    return std::visit([&engine](auto& payload) -> LuaValue {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(payload)>, std::monostate>)
        return LuaNil;
      else
        return engine.luaFrom(std::move(payload));
    }, value);
  }

  template <std::size_t Index = 0, typename Input>
  static std::optional<Value> checkTypeTo(LuaEngine& engine, Input&& v) {
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
      if (converted)
        return std::optional<Value>(std::in_place, std::in_place_index<Index>, takeOptional(converted));
    }
    if constexpr (Index + 1 < sizeof...(Types))
      return checkTypeTo<Index + 1>(engine, std::forward<Input>(v));
    else
      return {};
  }

  static std::optional<Value> to(LuaEngine& engine, LuaValue const& v) {
    return checkTypeTo(engine, v);
  }

  static std::optional<Value> to(LuaEngine& engine, LuaValue&& v) {
    return checkTypeTo(engine, std::move(v));
  }
};

template <>
struct LuaConverter<Color> {
  static LuaValue from(LuaEngine& engine, Color const& c);
  static std::optional<Color> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<LuaCallbacks> {
  static LuaValue from(LuaEngine& engine, LuaCallbacks const& c);
  static std::optional<LuaCallbacks> to(LuaEngine& engine, LuaValue const& v);
};

template <typename T>
struct LuaConverter<RpcPromise<T>> : LuaUserDataConverter<RpcPromise<T>> {};

template <typename T>
struct LuaUserDataMethods<RpcPromise<T>> {
  static LuaMethods<RpcPromise<T>> make();
};

template <typename T>
LuaMethods<RpcPromise<T>> LuaUserDataMethods<RpcPromise<T>>::make() {
  LuaMethods<RpcPromise<T>> methods;
  methods.template registerMethodWithSignature<bool, RpcPromise<T>&>("finished",        std::mem_fn(&RpcPromise<T>::finished));
  methods.template registerMethodWithSignature<bool, RpcPromise<T>&>("succeeded",       std::mem_fn(&RpcPromise<T>::succeeded));
  methods.template registerMethodWithSignature<std::optional<T>, RpcPromise<T>&>("result",      std::mem_fn(&RpcPromise<T>::result));
  methods.template registerMethodWithSignature<std::optional<String>, RpcPromise<T>&>("error",  std::mem_fn(&RpcPromise<T>::error));
  return methods;
}

// all RpcPromiseKeepers should be wrapped in this before being passed to Lua
template <typename T>
struct LuaConverter<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> : LuaUserDataConverter<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> {};

template <typename T>
struct LuaUserDataMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> {
  static LuaMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> make();
};

template <typename T>
LuaMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> LuaUserDataMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>>::make() {
  LuaMethods<std::shared_ptr<AutoFailRpcPromiseKeeper<T>>> methods;
  methods.template registerMethodWithSignature<void, std::shared_ptr<AutoFailRpcPromiseKeeper<T>>, RpcPromise<T>>("chain",  [&](std::shared_ptr<AutoFailRpcPromiseKeeper<T>> const& keeper, RpcPromise<T> const& promise) {
        keeper->chain(promise);
  });
  methods.template registerMethodWithSignature<void, std::shared_ptr<AutoFailRpcPromiseKeeper<T>>, T>("fulfill",            [&](std::shared_ptr<AutoFailRpcPromiseKeeper<T>> const& keeper, T const& result) {
        keeper->fulfill(result);
  });
  methods.template registerMethodWithSignature<void, std::shared_ptr<AutoFailRpcPromiseKeeper<T>>, String>("fail",          [&](std::shared_ptr<AutoFailRpcPromiseKeeper<T>> const& keeper, String const& error) {
        keeper->fail(error);
  });
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
