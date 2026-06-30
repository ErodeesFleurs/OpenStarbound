#pragma once

#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarSet.hpp"
#include "StarWeightedPool.hpp"
#include "StarDirectives.hpp"

namespace Star {

// Extra methods to parse a variety of types out of pure JSON.  Throws
// JsonException if json is not of correct type or size.

[[nodiscard]] size_t jsonToSize(Json const& v);
[[nodiscard]] Json jsonFromSize(size_t s);

// Must be array of appropriate size.

[[nodiscard]] Vec2D jsonToVec2D(Json const& v);
[[nodiscard]] Vec2F jsonToVec2F(Json const& v);
[[nodiscard]] Json jsonFromVec2F(Vec2F const& v);
[[nodiscard]] Vec2I jsonToVec2I(Json const& v);
[[nodiscard]] Json jsonFromVec2I(Vec2I const& v);
[[nodiscard]] Vec2U jsonToVec2U(Json const& v);
[[nodiscard]] Json jsonFromVec2U(Vec2U const& v);
[[nodiscard]] Vec2B jsonToVec2B(Json const& v);
[[nodiscard]] Json jsonFromVec2B(Vec2B const& v);

[[nodiscard]] Vec3D jsonToVec3D(Json const& v);
[[nodiscard]] Vec3F jsonToVec3F(Json const& v);
[[nodiscard]] Json jsonFromVec3F(Vec3F const& v);
[[nodiscard]] Vec3I jsonToVec3I(Json const& v);
[[nodiscard]] Json jsonFromVec3I(Vec3I const& v);
[[nodiscard]] Vec3B jsonToVec3B(Json const& v);

[[nodiscard]] Vec4B jsonToVec4B(Json const& v);
[[nodiscard]] Vec4I jsonToVec4I(Json const& v);
[[nodiscard]] Vec4F jsonToVec4F(Json const& v);

// Must be array of size 4 or 2 arrays of size 2 in an array.
[[nodiscard]] RectD jsonToRectD(Json const& v);
[[nodiscard]] Json jsonFromRectD(RectD const& rect);
[[nodiscard]] RectF jsonToRectF(Json const& v);
[[nodiscard]] Json jsonFromRectF(RectF const& rect);
[[nodiscard]] RectI jsonToRectI(Json const& v);
[[nodiscard]] Json jsonFromRectI(RectI const& rect);
[[nodiscard]] RectU jsonToRectU(Json const& v);
[[nodiscard]] Json jsonFromRectU(RectU const& rect);

// Can be a string, array of size 3 or 4 of doubles or ints.  If double, range
// is 0.0 to 1.0, if int range is 0-255
[[nodiscard]] Color jsonToColor(Json const& v);
[[nodiscard]] Json jsonFromColor(Color const& color);

// HACK: Fix clockwise specified polygons in coming from JSON
template <typename Float>
[[nodiscard]] Polygon<Float> fixInsideOutPoly(Polygon<Float> p);

// Array of size 2 arrays
[[nodiscard]] PolyD jsonToPolyD(Json const& v);
[[nodiscard]] PolyF jsonToPolyF(Json const& v);
[[nodiscard]] PolyI jsonToPolyI(Json const& v);
[[nodiscard]] Json jsonFromPolyF(PolyF const& poly);

// Expects a size 2 array of size 2 arrays
[[nodiscard]] Line2F jsonToLine2F(Json const& v);
[[nodiscard]] Json jsonFromLine2F(Line2F const& line);

[[nodiscard]] Mat3F jsonToMat3F(Json const& v);
[[nodiscard]] Json jsonFromMat3F(Mat3F const& v);

[[nodiscard]] StringList jsonToStringList(Json const& v);
[[nodiscard]] Json jsonFromStringList(List<String> const& v);
[[nodiscard]] StringSet jsonToStringSet(Json const& v);
[[nodiscard]] Json jsonFromStringSet(StringSet const& v);
[[nodiscard]] List<float> jsonToFloatList(Json const& v);
[[nodiscard]] List<int> jsonToIntList(Json const& v);
[[nodiscard]] List<Vec2I> jsonToVec2IList(Json const& v);
[[nodiscard]] List<Vec2U> jsonToVec2UList(Json const& v);
[[nodiscard]] List<Vec2F> jsonToVec2FList(Json const& v);
[[nodiscard]] List<Vec4B> jsonToVec4BList(Json const& v);
[[nodiscard]] List<Color> jsonToColorList(Json const& v);
[[nodiscard]] List<Directives> jsonToDirectivesList(Json const& v);
[[nodiscard]] Json jsonFromDirectivesList(List<Directives> const& v);

[[nodiscard]] Json weightedChoiceFromJson(Json const& source, Json const& default_);

// Assumes that the bins parameter is an array of pairs (arrays), where the
// first element is a minimum value and the second element is the actual
// important value.  Finds the pair with the highest value that is less than or
// equal to the given target, and returns the second element.
[[nodiscard]] Json binnedChoiceFromJson(Json const& bins, float target, Json const& def = Json());

template <typename T>
[[nodiscard]] WeightedPool<T> jsonToWeightedPool(Json const& source);
template <typename T, typename Converter>
[[nodiscard]] WeightedPool<T> jsonToWeightedPool(Json const& source, Converter&& converter);

template <typename T>
[[nodiscard]] Json jsonFromWeightedPool(WeightedPool<T> const& pool);
template <typename T, typename Converter>
[[nodiscard]] Json jsonFromWeightedPool(WeightedPool<T> const& pool, Converter&& converter);

template <size_t Size>
Array<unsigned, Size> jsonToArrayU(Json const& v) {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayU", Size).c_str());

  Array<unsigned, Size> res;
  for (size_t i = 0; i < Size; i++) {
    res[i] = v.getUInt(i);
  }

  return res;
}

template <size_t Size>
Array<size_t, Size> jsonToArrayS(Json const& v) {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayS", Size).c_str());

  Array<size_t, Size> res;
  for (size_t i = 0; i < Size; i++) {
    res[i] = v.getUInt(i);
  }

  return res;
}

template <size_t Size>
Array<int, Size> jsonToArrayI(Json const& v) {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayI", Size).c_str());

  Array<int, Size> res;
  for (size_t i = 0; i < Size; i++) {
    res[i] = v.getInt(i);
  }

  return res;
}

template <size_t Size>
Array<float, Size> jsonToArrayF(Json const& v) {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayF", Size).c_str());

  Array<float, Size> res;
  for (size_t i = 0; i < Size; i++) {
    res[i] = v.getFloat(i);
  }

  return res;
}

template <size_t Size>
Array<double, Size> jsonToArrayD(Json const& v) {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToArrayD", Size).c_str());

  Array<double, Size> res;
  for (size_t i = 0; i < Size; i++) {
    res[i] = v.getDouble(i);
  }

  return res;
}

template <size_t Size>
Array<String, Size> jsonToStringArray(Json const& v) {
  if (v.size() != Size)
    throw JsonException(strf("Json array not of size {} in jsonToStringArray", Size).c_str());

  Array<String, Size> res;
  for (size_t i = 0; i < Size; i++) {
    res[i] = v.getString(i);
  }

  return res;
}

template <typename Value>
List<Value> jsonToList(Json const& v) {
  return jsonToList<Value>(v, construct<Value>());
}

template <typename Value, typename Converter>
List<Value> jsonToList(Json const& v, Converter&& valueConvert) {
  if (v.type() != Json::Type::Array)
    throw JsonException("Json type is not a array in jsonToList");

  List<Value> res;
  for (auto const& entry : v.iterateArray())
    res.push_back(valueConvert(entry));

  return res;
}

template <typename Value>
Json jsonFromList(List<Value> const& list) {
  return jsonFromList<Value>(list, construct<Json>());
}

template <typename Value, typename Converter>
Json jsonFromList(List<Value> const& list, Converter&& valueConvert) {
  JsonArray res;
  for (auto const& entry : list)
    res.push_back(valueConvert(entry));

  return res;
}

template <typename SetType>
SetType jsonToSet(Json const& v) {
  return jsonToSet<SetType>(v, construct<typename SetType::value_type>());
}

template <typename SetType, typename Converter>
SetType jsonToSet(Json const& v, Converter&& valueConvert) {
  if (v.type() != Json::Type::Array)
    throw JsonException("Json type is not an array in jsonToSet");

  SetType res;
  for (auto const& entry : v.iterateArray())
    res.add(valueConvert(entry));

  return res;
}

template <typename SetType>
Json jsonFromSet(SetType const& Set) {
  return jsonFromSet<SetType>(Set, construct<Json>());
}

template <typename SetType, typename Converter>
Json jsonFromSet(SetType const& Set, Converter&& valueConvert) {
  JsonArray res;
  for (auto& entry : Set)
    res.push_back(valueConvert(entry));

  return res;
}

template <typename MapType, typename KeyConverter, typename ValueConverter>
MapType jsonToMapKV(Json const& v, KeyConverter&& keyConvert, ValueConverter&& valueConvert) {
  if (v.type() != Json::Type::Object)
    throw JsonException("Json type is not an object in jsonToMap");

  MapType res;
  for (auto const& [key, value] : v.iterateObject())
    res.add(keyConvert(key), valueConvert(value));

  return res;
}

template <typename MapType, typename KeyConverter>
MapType jsonToMapK(Json const& v, KeyConverter&& keyConvert) {
  return jsonToMapKV<MapType>(v, forward<KeyConverter>(keyConvert), construct<typename MapType::mapped_type>());
}

template <typename MapType, typename ValueConverter>
MapType jsonToMapV(Json const& v, ValueConverter&& valueConvert) {
  return jsonToMapKV<MapType>(v, construct<typename MapType::key_type>(), std::forward<ValueConverter>(valueConvert));
}

template <typename MapType>
MapType jsonToMap(Json const& v) {
  return jsonToMapKV<MapType>(v, construct<typename MapType::key_type>(), construct<typename MapType::mapped_type>());
}

template <typename MapType, typename KeyConverter, typename ValueConverter>
Json jsonFromMapKV(MapType const& map, KeyConverter&& keyConvert, ValueConverter&& valueConvert) {
  JsonObject res;
  for (auto [key, value] : map)
    res[keyConvert(key)] = valueConvert(value);

  return res;
}

template <typename MapType, typename KeyConverter>
Json jsonFromMapK(MapType const& map, KeyConverter&& keyConvert) {
  return jsonFromMapKV<MapType>(map, std::forward<KeyConverter>(keyConvert), construct<Json>());
}

template <typename MapType, typename ValueConverter>
Json jsonFromMapV(MapType const& map, ValueConverter&& valueConvert) {
  return jsonFromMapKV<MapType>(map, construct<String>(), std::forward<ValueConverter>(valueConvert));
}

template <typename MapType>
Json jsonFromMap(MapType const& map) {
  return jsonFromMapKV<MapType>(map, construct<String>(), construct<Json>());
}

template <typename T, typename Converter>
Json jsonFromMaybe(Maybe<T> const& m, Converter&& converter) {
  return m.apply(converter).value();
}

template <typename T>
Json jsonFromMaybe(Maybe<T> const& m) {
  return jsonFromMaybe(m, construct<Json>());
}

template <typename T, typename Converter>
Maybe<T> jsonToMaybe(Json v, Converter&& converter) {
  if (v.isNull())
    return {};
  return converter(v);
}

template <typename T>
Maybe<T> jsonToMaybe(Json const& v) {
  return jsonToMaybe<T>(v, construct<T>());
}

template <typename T>
WeightedPool<T> jsonToWeightedPool(Json const& source) {
  return jsonToWeightedPool<T>(source, construct<T>());
}

template <typename T, typename Converter>
WeightedPool<T> jsonToWeightedPool(Json const& source, Converter&& converter) {
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
Json jsonFromWeightedPool(WeightedPool<T> const& pool) {
  return jsonFromWeightedPool<T>(pool, construct<Json>());
}

template <typename T, typename Converter>
Json jsonFromWeightedPool(WeightedPool<T> const& pool, Converter&& converter) {
  JsonArray res;
  for (auto const& [weight, item] : pool.items()) {
    res.append(JsonObject{
        {"weight", weight}, {"item", converter(item)},
    });
  }
  return res;
}

template <>
[[nodiscard]] WeightedPool<int> jsonToWeightedPool(Json const& source);

template <>
[[nodiscard]] WeightedPool<unsigned> jsonToWeightedPool(Json const& source);

template <>
[[nodiscard]] WeightedPool<float> jsonToWeightedPool(Json const& source);

template <>
[[nodiscard]] WeightedPool<double> jsonToWeightedPool(Json const& source);

template <>
[[nodiscard]] WeightedPool<String> jsonToWeightedPool(Json const& source);

template <>
[[nodiscard]] WeightedPool<JsonArray> jsonToWeightedPool(Json const& source);

template <>
[[nodiscard]] WeightedPool<JsonObject> jsonToWeightedPool(Json const& source);

template <typename Float>
Polygon<Float> fixInsideOutPoly(Polygon<Float> p) {
  if (p.sides() > 2) {
    if ((p.side(1).diff() ^ p.side(0).diff()) > 0)
      reverse(p.vertexes());
  }
  return p;
}

}
