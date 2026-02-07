#pragma once

#include "StarRect.hpp"
#include "StarVector.hpp"
#include "StarColor.hpp"
#include "StarPoly.hpp"
#include "StarLine.hpp"
#include "StarLua.hpp"
#include "StarVariant.hpp"

import std;

namespace Star {

template <typename T>
struct LuaConverter<LuaNullTermWrapper<T>> : LuaConverter<T> {
  static auto from(LuaEngine& engine, LuaNullTermWrapper<T>&& v) -> LuaValue {
    auto enforcer = engine.nullTerminate();
    return LuaConverter<T>::from(engine, std::forward<T>(v));
  }

  static auto from(LuaEngine& engine, LuaNullTermWrapper<T> const& v) -> LuaValue {
    auto enforcer = engine.nullTerminate();
    return LuaConverter<T>::from(engine, v);
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> LuaNullTermWrapper<T> {
    auto enforcer = engine.nullTerminate();
    return LuaConverter<T>::to(engine, v);
  }
};

template <typename T1, typename T2>
struct LuaConverter<std::pair<T1, T2>> {
  static auto from(LuaEngine& engine, std::pair<T1, T2>&& v) -> LuaValue {
    auto t = engine.createTable();
    t.set(1, std::move(v.first));
    t.set(2, std::move(v.second));
    return t;
  }

  static auto from(LuaEngine& engine, std::pair<T1, T2> const& v) -> LuaValue {
    auto t = engine.createTable();
    t.set(1, v.first);
    t.set(2, v.second);
    return t;
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<std::pair<T1, T2>> {
    if (auto table = engine.luaMaybeTo<LuaTable>(std::move(v))) {
      auto p1 = engine.luaMaybeTo<T1>(table->get(1));
      auto p2 = engine.luaMaybeTo<T2>(table->get(2));
      if (p1 && p2)
        return std::pair<T1, T2>{std::move(*p1), std::move(*p2)};
    }
    return std::nullopt;
  }
};

template <typename T, size_t N>
struct LuaConverter<Vector<T, N>> {
  static auto from(LuaEngine& engine, Vector<T, N> const& v) -> LuaValue {
    return engine.createArrayTable(v);
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Vector<T, N>> {
    auto table = v.ptr<LuaTable>();
    if (!table)
      return std::nullopt;

    Vector<T, N> vec;
    for (size_t i = 0; i < N; ++i) {
      auto val = engine.luaMaybeTo<T>(table->get(i + 1));
      if (!val)
        return std::nullopt;
      vec[i] = *val;
    }
    return vec;
  }
};

template <typename T>
struct LuaConverter<Matrix3<T>> {
  static auto from(LuaEngine& engine, Matrix3<T> const& m) -> LuaValue {
    auto table = engine.createTable(3, 0);
    table.set(1, m[0]);
    table.set(2, m[1]);
    table.set(3, m[2]);
    return table;
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Matrix3<T>> {
    if (auto table = v.ptr<LuaTable>()) {
      auto r1 = engine.luaMaybeTo<typename Matrix3<T>::Vec3>(table->get(1));
      auto r2 = engine.luaMaybeTo<typename Matrix3<T>::Vec3>(table->get(2));
      auto r3 = engine.luaMaybeTo<typename Matrix3<T>::Vec3>(table->get(3));
      if (r1 && r2 && r3)
        return Matrix3<T>(*r1, *r2, *r3);
    }
    return std::nullopt;
  }
};

template <typename T>
struct LuaConverter<Rect<T>> {
  static auto from(LuaEngine& engine, Rect<T> const& v) -> LuaValue {
    if (v.isNull())
      return LuaNil;
    auto t = engine.createTable();
    t.set(1, v.xMin());
    t.set(2, v.yMin());
    t.set(3, v.xMax());
    t.set(4, v.yMax());
    return t;
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Rect<T>> {
    if (v == LuaNil)
      return Rect<T>::null();

    if (auto table = v.ptr<LuaTable>()) {
      auto xMin = engine.luaMaybeTo<T>(table->get(1));
      auto yMin = engine.luaMaybeTo<T>(table->get(2));
      auto xMax = engine.luaMaybeTo<T>(table->get(3));
      auto yMax = engine.luaMaybeTo<T>(table->get(4));
      if (xMin && yMin && xMax && yMax)
        return Rect<T>(*xMin, *yMin, *xMax, *yMax);
    }
    return std::nullopt;
  }
};

template <typename T>
struct LuaConverter<Polygon<T>> {
  static auto from(LuaEngine& engine, Polygon<T> const& poly) -> LuaValue {
    return engine.createArrayTable(poly.vertexes());
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Polygon<T>> {
    if (auto points = engine.luaMaybeTo<typename Polygon<T>::VertexList>(v))
      return Polygon<T>(std::move(*points));
    return std::nullopt;
  }
};

template <typename T, size_t N>
struct LuaConverter<Line<T, N>> {
  static auto from(LuaEngine& engine, Line<T, N> const& line) -> LuaValue {
    auto table = engine.createTable();
    table.set(1, line.min());
    table.set(2, line.max());
    return table;
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Line<T, N>> {
    if (auto table = v.ptr<LuaTable>()) {
      auto min = engine.luaMaybeTo<Vector<T, N>>(table->get(1));
      auto max = engine.luaMaybeTo<Vector<T, N>>(table->get(2));
      if (min && max)
        return Line<T, N>(*min, *max);
    }
    return std::nullopt;
  }
};

// Sort of magical converter, tries to convert from all the types in the
// Variant in order, returning the first correct type.  Types should not be
// ambiguous, or the more specific types should come first, which relies on the
// implementation of the converters.
template <typename FirstType, typename... RestTypes>
struct LuaConverter<Variant<FirstType, RestTypes...>> {
  static auto from(LuaEngine& engine, Variant<FirstType, RestTypes...> const& variant) -> LuaValue {
    return variant.call([&engine](auto const& a) -> auto { return engine.luaFrom(a); });
  }

  static auto from(LuaEngine& engine, Variant<FirstType, RestTypes...>&& variant) -> LuaValue {
    return variant.call([&engine](auto& a) -> auto { return engine.luaFrom(std::move(a)); });
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Variant<FirstType, RestTypes...>> {
    return checkTypeTo<FirstType, RestTypes...>(engine, v);
  }

  template <typename CheckType1, typename CheckType2, typename... CheckTypeRest>
  static auto checkTypeTo(LuaEngine& engine, LuaValue const& v) -> std::optional<Variant<FirstType, RestTypes...>> {
    if (auto t1 = engine.luaMaybeTo<CheckType1>(v))
      return t1;
    else
      return checkTypeTo<CheckType2, CheckTypeRest...>(engine, v);
  }

  template <typename Type>
  static auto checkTypeTo(LuaEngine& engine, LuaValue const& v) -> std::optional<Variant<FirstType, RestTypes...>> {
    return engine.luaMaybeTo<Type>(v);
  }

  static auto to(LuaEngine& engine, LuaValue&& v) -> std::optional<Variant<FirstType, RestTypes...>> {
    return checkTypeTo<FirstType, RestTypes...>(engine, std::move(v));
  }

  template <typename CheckType1, typename CheckType2, typename... CheckTypeRest>
  static auto checkTypeTo(LuaEngine& engine, LuaValue&& v) -> std::optional<Variant<FirstType, RestTypes...>> {
    if (auto t1 = engine.luaMaybeTo<CheckType1>(v))
      return t1;
    else
      return checkTypeTo<CheckType2, CheckTypeRest...>(engine, std::move(v));
  }

  template <typename Type>
  static auto checkTypeTo(LuaEngine& engine, LuaValue&& v) -> std::optional<Variant<FirstType, RestTypes...>> {
    return engine.luaMaybeTo<Type>(std::move(v));
  }
};

// Similarly to Variant converter, tries to convert from all types in order.
// An empty MVariant is converted to nil and vice versa.
template <typename... Types>
struct LuaConverter<MVariant<Types...>> {
  static auto from(LuaEngine& engine, MVariant<Types...> const& variant) -> LuaValue {
    LuaValue value;
    variant.call([&value, &engine](auto const& a) -> auto {
        value = engine.luaFrom(a);
      });
    return value;
  }

  static auto from(LuaEngine& engine, MVariant<Types...>&& variant) -> LuaValue {
    LuaValue value;
    variant.call([&value, &engine](auto& a) -> auto {
        value = engine.luaFrom(std::move(a));
      });
    return value;
  }

  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<MVariant<Types...>> {
    if (v == LuaNil)
      return MVariant<Types...>();
    return checkTypeTo<Types...>(engine, v);
  }

  template <typename CheckType1, typename CheckType2, typename... CheckTypeRest>
  static auto checkTypeTo(LuaEngine& engine, LuaValue const& v) -> std::optional<MVariant<Types...>> {
    if (auto t1 = engine.luaMaybeTo<CheckType1>(v))
      return t1;
    else
      return checkTypeTo<CheckType2, CheckTypeRest...>(engine, v);
  }

  template <typename CheckType>
  static auto checkTypeTo(LuaEngine& engine, LuaValue const& v) -> std::optional<MVariant<Types...>> {
    return engine.luaMaybeTo<CheckType>(v);
  }

  static auto to(LuaEngine& engine, LuaValue&& v) -> std::optional<MVariant<Types...>> {
    if (v == LuaNil)
      return MVariant<Types...>();
    return checkTypeTo<Types...>(engine, std::move(v));
  }

  template <typename CheckType1, typename CheckType2, typename... CheckTypeRest>
  static auto checkTypeTo(LuaEngine& engine, LuaValue&& v) -> std::optional<MVariant<Types...>> {
    if (auto t1 = engine.luaMaybeTo<CheckType1>(v))
      return t1;
    else
      return checkTypeTo<CheckType2, CheckTypeRest...>(engine, std::move(v));
  }

  template <typename CheckType>
  static auto checkTypeTo(LuaEngine& engine, LuaValue&& v) -> std::optional<MVariant<Types...>> {
    return engine.luaMaybeTo<CheckType>(std::move(v));
  }

};

template <>
struct LuaConverter<Color> {
  static auto from(LuaEngine& engine, Color const& c) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Color>;
};

template <>
struct LuaConverter<LuaCallbacks> {
  static auto from(LuaEngine& engine, LuaCallbacks const& c) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<LuaCallbacks>;
};

}
