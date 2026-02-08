#pragma once

#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarWeightedPool.hpp"

import std;

namespace Star {

// Extra methods to parse a variety of types out of pure JSON.  Throws
// JsonException if json is not of correct type or size.

auto jsonToSize(Json const& v) -> std::size_t;
auto jsonFromSize(std::size_t s) -> Json;

// Must be array of appropriate size.

auto jsonToVec2D(Json const& v) -> Vec2D;
auto jsonToVec2F(Json const& v) -> Vec2F;
auto jsonFromVec2F(Vec2F const& v) -> Json;
auto jsonToVec2I(Json const& v) -> Vec2I;
auto jsonFromVec2I(Vec2I const& v) -> Json;
auto jsonToVec2U(Json const& v) -> Vec2U;
auto jsonFromVec2U(Vec2U const& v) -> Json;
auto jsonToVec2B(Json const& v) -> Vec2B;
auto jsonFromVec2B(Vec2B const& v) -> Json;

auto jsonToVec3D(Json const& v) -> Vec3D;
auto jsonToVec3F(Json const& v) -> Vec3F;
auto jsonFromVec3F(Vec3F const& v) -> Json;
auto jsonToVec3I(Json const& v) -> Vec3I;
auto jsonFromVec3I(Vec3I const& v) -> Json;
auto jsonToVec3B(Json const& v) -> Vec3B;

auto jsonToVec4B(Json const& v) -> Vec4B;
auto jsonToVec4I(Json const& v) -> Vec4I;
auto jsonToVec4F(Json const& v) -> Vec4F;

// Must be array of size 4 or 2 arrays of size 2 in an array.
auto jsonToRectD(Json const& v) -> RectD;
auto jsonFromRectD(RectD const& rect) -> Json;
auto jsonToRectF(Json const& v) -> RectF;
auto jsonFromRectF(RectF const& rect) -> Json;
auto jsonToRectI(Json const& v) -> RectI;
auto jsonFromRectI(RectI const& rect) -> Json;
auto jsonToRectU(Json const& v) -> RectU;
auto jsonFromRectU(RectU const& rect) -> Json;

// Can be a string, array of size 3 or 4 of doubles or ints.  If double, range
// is 0.0 to 1.0, if int range is 0-255
auto jsonToColor(Json const& v) -> Color;
auto jsonFromColor(Color const& color) -> Json;

// HACK: Fix clockwise specified polygons in coming from JSON
template <typename Float>
auto fixInsideOutPoly(Polygon<Float> p) -> Polygon<Float>;

// Array of size 2 arrays
auto jsonToPolyD(Json const& v) -> PolyD;
auto jsonToPolyF(Json const& v) -> PolyF;
auto jsonToPolyI(Json const& v) -> PolyI;
auto jsonFromPolyF(PolyF const& poly) -> Json;

// Expects a size 2 array of size 2 arrays
auto jsonToLine2F(Json const& v) -> Line2F;
auto jsonFromLine2F(Line2F const& line) -> Json;

auto jsonToMat3F(Json const& v) -> Mat3F;
auto jsonFromMat3F(Mat3F const& v) -> Json;

auto jsonToStringList(Json const& v) -> StringList;
auto jsonFromStringList(List<String> const& v) -> Json;
auto jsonToStringSet(Json const& v) -> StringSet;
auto jsonFromStringSet(StringSet const& v) -> Json;
auto jsonToFloatList(Json const& v) -> List<float>;
auto jsonToIntList(Json const& v) -> List<int>;
auto jsonToVec2IList(Json const& v) -> List<Vec2I>;
auto jsonToVec2UList(Json const& v) -> List<Vec2U>;
auto jsonToVec2FList(Json const& v) -> List<Vec2F>;
auto jsonToVec4BList(Json const& v) -> List<Vec4B>;
auto jsonToColorList(Json const& v) -> List<Color>;
auto jsonToDirectivesList(Json const& v) -> List<Directives>;
auto jsonFromDirectivesList(List<Directives> const& v) -> Json;

auto weightedChoiceFromJson(Json const& source, Json const& default_) -> Json;

// Assumes that the bins parameter is an array of pairs (arrays), where the
// first element is a minimum value and the second element is the actual
// important value.  Finds the pair with the highest value that is less than or
// equal to the given target, and returns the second element.
auto binnedChoiceFromJson(Json const& bins, float target, Json const& def = Json()) -> Json;

template <typename T>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<T>;
template <typename T, typename Converter>
auto jsonToWeightedPool(Json const& source, Converter&& converter) -> WeightedPool<T>;

template <typename T>
auto jsonFromWeightedPool(WeightedPool<T> const& pool) -> Json;
template <typename T, typename Converter>
auto jsonFromWeightedPool(WeightedPool<T> const& pool, Converter&& converter) -> Json;

template <std::size_t Size>
auto jsonToArrayU(Json const& v) -> Array<unsigned, Size> {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayU", Size).c_str());

  Array<unsigned, Size> res;
  for (std::size_t i = 0; i < Size; i++) {
    res[i] = v.getUInt(i);
  }

  return res;
}

template <std::size_t Size>
auto jsonToArrayS(Json const& v) -> Array<std::size_t, Size> {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayS", Size).c_str());

  Array<std::size_t, Size> res;
  for (std::size_t i = 0; i < Size; i++) {
    res[i] = v.getUInt(i);
  }

  return res;
}

template <std::size_t Size>
auto jsonToArrayI(Json const& v) -> Array<int, Size> {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayI", Size).c_str());

  Array<int, Size> res;
  for (std::size_t i = 0; i < Size; i++) {
    res[i] = v.getInt(i);
  }

  return res;
}

template <std::size_t Size>
auto jsonToArrayF(Json const& v) -> Array<float, Size> {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayF", Size).c_str());

  Array<float, Size> res;
  for (std::size_t i = 0; i < Size; i++) {
    res[i] = v.getFloat(i);
  }

  return res;
}

template <std::size_t Size>
auto jsonToArrayD(Json const& v) -> Array<double, Size> {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayD", Size).c_str());

  Array<double, Size> res;
  for (std::size_t i = 0; i < Size; i++) {
    res[i] = v.getDouble(i);
  }

  return res;
}

template <std::size_t Size>
auto jsonToStringArray(Json const& v) -> Array<String, Size> {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToStringArray", Size).c_str());

  Array<String, Size> res;
  for (std::size_t i = 0; i < Size; i++) {
    res[i] = v.getString(i);
  }

  return res;
}

template <typename Value>
auto jsonToList(Json const& v) -> List<Value> {
  return jsonToList<Value>(v, construct<Value>());
}

template <typename Value, typename Converter>
auto jsonToList(Json const& v, Converter&& valueConvert) -> List<Value> {
  if (v.type() != Json::Type::Array)
    throw JsonException("Json type is not a array in jsonToList");

  List<Value> res;
  for (auto const& entry : v.iterateArray())
    res.push_back(valueConvert(entry));

  return res;
}

template <typename Value>
auto jsonFromList(List<Value> const& list) -> Json {
  return jsonFromList<Value>(list, construct<Json>());
}

template <typename Value, typename Converter>
auto jsonFromList(List<Value> const& list, Converter&& valueConvert) -> Json {
  JsonArray res;
  for (auto const& entry : list)
    res.push_back(valueConvert(entry));

  return res;
}

template <typename SetType>
auto jsonToSet(Json const& v) -> SetType {
  return jsonToSet<SetType>(v, construct<typename SetType::value_type>());
}

template <typename SetType, typename Converter>
auto jsonToSet(Json const& v, Converter&& valueConvert) -> SetType {
  if (v.type() != Json::Type::Array)
    throw JsonException("Json type is not an array in jsonToSet");

  SetType res;
  for (auto const& entry : v.iterateArray())
    res.add(valueConvert(entry));

  return res;
}

template <typename SetType>
auto jsonFromSet(SetType const& Set) -> Json {
  return jsonFromSet<SetType>(Set, construct<Json>());
}

template <typename SetType, typename Converter>
auto jsonFromSet(SetType const& Set, Converter&& valueConvert) -> Json {
  JsonArray res;
  for (auto& entry : Set)
    res.push_back(valueConvert(entry));

  return res;
}

template <typename MapType, typename KeyConverter, typename ValueConverter>
auto jsonToMapKV(Json const& v, KeyConverter&& keyConvert, ValueConverter&& valueConvert) -> MapType {
  if (v.type() != Json::Type::Object)
    throw JsonException("Json type is not an object in jsonToMap");

  MapType res;
  for (auto const& pair : v.iterateObject())
    res.add(keyConvert(pair.first), valueConvert(pair.second));

  return res;
}

template <typename MapType, typename KeyConverter>
auto jsonToMapK(Json const& v, KeyConverter&& keyConvert) -> MapType {
  return jsonToMapKV<MapType>(v, forward<KeyConverter>(keyConvert), construct<typename MapType::mapped_type>());
}

template <typename MapType, typename ValueConverter>
auto jsonToMapV(Json const& v, ValueConverter&& valueConvert) -> MapType {
  return jsonToMapKV<MapType>(v, construct<typename MapType::key_type>(), std::forward<ValueConverter>(valueConvert));
}

template <typename MapType>
auto jsonToMap(Json const& v) -> MapType {
  return jsonToMapKV<MapType>(v, construct<typename MapType::key_type>(), construct<typename MapType::mapped_type>());
}

template <typename MapType, typename KeyConverter, typename ValueConverter>
auto jsonFromMapKV(MapType const& map, KeyConverter&& keyConvert, ValueConverter&& valueConvert) -> Json {
  JsonObject res;
  for (auto pair : map)
    res[keyConvert(pair.first)] = valueConvert(pair.second);

  return res;
}

template <typename MapType, typename KeyConverter>
auto jsonFromMapK(MapType const& map, KeyConverter&& keyConvert) -> Json {
  return jsonFromMapKV<MapType>(map, std::forward<KeyConverter>(keyConvert), construct<Json>());
}

template <typename MapType, typename ValueConverter>
auto jsonFromMapV(MapType const& map, ValueConverter&& valueConvert) -> Json {
  return jsonFromMapKV<MapType>(map, construct<String>(), std::forward<ValueConverter>(valueConvert));
}

template <typename MapType>
auto jsonFromMap(MapType const& map) -> Json {
  return jsonFromMapKV<MapType>(map, construct<String>(), construct<Json>());
}

template <typename T, typename Converter>
auto jsonFromMaybe(std::optional<T> const& m, Converter&& converter) -> Json {
  return m.transform(converter).value_or(Json());
}

template <typename T>
auto jsonFromMaybe(std::optional<T> const& m) -> Json {
  return jsonFromMaybe(m, construct<Json>());
}

template <typename T, typename Converter>
auto jsonToMaybe(Json v, Converter&& converter) -> std::optional<T> {
  if (v.isNull())
    return std::nullopt;
  return converter(v);
}

template <typename T>
auto jsonToMaybe(Json const& v) -> std::optional<T> {
  return jsonToMaybe<T>(v, construct<T>());
}

template <typename T>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<T> {
  return jsonToWeightedPool<T>(source, construct<T>());
}

template <typename T, typename Converter>
auto jsonToWeightedPool(Json const& source, Converter&& converter) -> WeightedPool<T> {
  WeightedPool<T> res;
  if (source.isNull())
    return res;
  for (auto entry : source.iterateArray()) {
    if (entry.isType(Json::Type::Array))
      res.add(entry.get(0).toDouble(), converter(entry.get(1)));
    else
      res.add(entry.getDouble("weight"), converter(entry.get("item")));
  }

  return res;
}

template <typename T>
auto jsonFromWeightedPool(WeightedPool<T> const& pool) -> Json {
  return jsonFromWeightedPool<T>(pool, construct<Json>());
}

template <typename T, typename Converter>
auto jsonFromWeightedPool(WeightedPool<T> const& pool, Converter&& converter) -> Json {
  JsonArray res;
  for (auto const& pair : pool.items()) {
    res.append(JsonObject{
      {"weight", pair.first},
      {"item", converter(pair.second)},
    });
  }
  return res;
}

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<int>;

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<unsigned>;

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<float>;

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<double>;

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<String>;

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<JsonArray>;

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<JsonObject>;

template <typename Float>
auto fixInsideOutPoly(Polygon<Float> p) -> Polygon<Float> {
  if (p.sides() > 2) {
    if ((p.side(1).diff() ^ p.side(0).diff()) > 0)
      reverse(p.vertexes());
  }
  return p;
}

}// namespace Star
