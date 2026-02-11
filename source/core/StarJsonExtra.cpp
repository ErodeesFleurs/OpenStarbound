#include "StarJsonExtra.hpp"

#include "StarList.hpp"
#include "StarRandom.hpp"

import std;

namespace Star {

auto jsonToSize(Json const& v) -> std::size_t {
  if (v.isNull()) {
    return std::numeric_limits<std::size_t>::max();
  }

  if (!v.canConvert(Json::Type::Int)) {
    throw JsonException("Json not an int in jsonToSize");
  }

  return v.toUInt();
}

auto jsonFromSize(std::size_t s) -> Json {
  if (s == std::numeric_limits<std::size_t>::max()) {
    return {};
  }
  return {s};
}

auto jsonToVec2D(Json const& v) -> Vec2D {
  if (v.type() != Json::Type::Array || v.size() != 2) {
    throw JsonException("Json not an array of size 2 in jsonToVec2D");
  }

  return Vec2D{v.getDouble(0), v.getDouble(1)};
}

auto jsonToVec2F(Json const& v) -> Vec2F {
  if (v.type() != Json::Type::Array || v.size() != 2) {
    throw JsonException("Json not an array of size 2 in jsonToVec2F");
  }

  return Vec2F{v.getFloat(0), v.getFloat(1)};
}

auto jsonFromVec2F(Vec2F const& v) -> Json {
  return JsonArray{v[0], v[1]};
}

auto jsonToVec2I(Json const& v) -> Vec2I {
  if (v.type() != Json::Type::Array || v.size() != 2) {
    throw JsonException("Json not an array of size 2 in jsonToVec2I");
  }

  return Vec2I{static_cast<std::int32_t>(v.getInt(0)), static_cast<std::int32_t>(v.getInt(1))};
}

auto jsonFromVec2I(Vec2I const& v) -> Json {
  return JsonArray{v[0], v[1]};
}

auto jsonToVec2U(Json const& v) -> Vec2U {
  if (v.type() != Json::Type::Array || v.size() != 2) {
    throw JsonException("Json not an array of size 2 in jsonToVec2I");
  }

  return Vec2U{static_cast<std::uint32_t>(v.getInt(0)), static_cast<std::uint32_t>(v.getInt(1))};
}

auto jsonFromVec2U(Vec2U const& v) -> Json {
  return JsonArray{v[0], v[1]};
}

auto jsonToVec2B(Json const& v) -> Vec2B {
  if (v.type() != Json::Type::Array || v.size() != 2) {
    throw JsonException("Json not an array of size 2 in jsonToVec2B");
  }

  return Vec2B{static_cast<bool>(v.getInt(0)), static_cast<bool>(v.getInt(1))};
}

auto jsonFromVec2B(Vec2B const& v) -> Json {
  return JsonArray{v[0], v[1]};
}

auto jsonToVec3D(Json const& v) -> Vec3D {
  if (v.type() != Json::Type::Array || v.size() != 3) {
    throw JsonException("Json not an array of size size 3 in jsonToVec3D");
  }

  return Vec3D{v.getDouble(0), v.getDouble(1), v.getDouble(2)};
}

auto jsonToVec3F(Json const& v) -> Vec3F {
  if (v.type() != Json::Type::Array || v.size() != 3) {
    throw JsonException("Json not an array of size 3 in jsonToVec3D");
  }

  return Vec3F{v.getFloat(0), v.getFloat(1), v.getFloat(2)};
}

auto jsonFromVec3F(Vec3F const& v) -> Json {
  return JsonArray{v[0], v[1], v[2]};
}

auto jsonToVec3I(Json const& v) -> Vec3I {
  if (v.type() != Json::Type::Array || v.size() != 3) {
    throw JsonException("Json not an array of size 3 in jsonToVec3I");
  }

  return Vec3I{static_cast<std::int32_t>(v.getInt(0)), static_cast<std::int32_t>(v.getInt(1)), static_cast<std::int32_t>(v.getInt(2))};
}

auto jsonFromVec3I(Vec3I const& v) -> Json {
  JsonArray result;
  result.append(v[0]);
  result.append(v[1]);
  result.append(v[2]);
  return result;
}

auto jsonToVec3B(Json const& v) -> Vec3B {
  if (v.type() != Json::Type::Array || v.size() != 3) {
    throw JsonException("Json not an array of size 3 in jsonToVec3B");
  }

  return Vec3B{static_cast<bool>(v.getInt(0)), static_cast<bool>(v.getInt(1)), static_cast<bool>(v.getInt(2))};
}

auto jsonToVec4B(Json const& v) -> Vec4B {
  if (v.type() != Json::Type::Array || v.size() != 4) {
    throw JsonException("Json not an array of size 4 in jsonToVec4B");
  }

  return Vec4B{static_cast<bool>(v.getInt(0)), static_cast<bool>(v.getInt(1)), static_cast<bool>(v.getInt(2)), static_cast<bool>(v.getInt(3))};
}

auto jsonToVec4I(Json const& v) -> Vec4I {
  if (v.type() != Json::Type::Array || v.size() != 4) {
    throw JsonException("Json not an array of size 4 in jsonToVec4B");
  }

  return Vec4I{static_cast<std::int32_t>(v.getInt(0)), static_cast<std::int32_t>(v.getInt(1)), static_cast<std::int32_t>(v.getInt(2)), static_cast<std::int32_t>(v.getInt(3))};
}

auto jsonToVec4F(Json const& v) -> Vec4F {
  if (v.type() != Json::Type::Array || v.size() != 4) {
    throw JsonException("Json not an array of size 4 in jsonToVec4B");
  }

  return Vec4F{v.getFloat(0), v.getFloat(1), v.getFloat(2), v.getFloat(3)};
}

auto jsonToRectD(Json const& v) -> RectD {
  if (v.type() != Json::Type::Array) {
    throw JsonException("Json not an array in jsonToRectD");
  }

  if (v.size() != 4 && v.size() != 2) {
    throw JsonException("Json not an array of proper size in jsonToRectD");
  }

  if (v.size() == 4) {
    return {v.getDouble(0), v.getDouble(1), v.getDouble(2), v.getDouble(3)};
  }

  try {
    auto lowerLeft = jsonToVec2D(v.get(0));
    auto upperRight = jsonToVec2D(v.get(1));
    return {lowerLeft, upperRight};
  } catch (JsonException const& e) {
    throw JsonException(strf("Inner position not well formed in jsonToRectD: {}", outputException(e, true)));
  }
}

auto jsonFromRectD(RectD const& rect) -> Json {
  return JsonArray{rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax()};
}

auto jsonToRectF(Json const& v) -> RectF {
  return RectF(jsonToRectD(v));
}

auto jsonFromRectF(RectF const& rect) -> Json {
  return JsonArray{rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax()};
}

auto jsonToRectI(Json const& v) -> RectI {
  if (v.type() != Json::Type::Array) {
    throw JsonException("Json not an array in jsonToRectI");
  }

  if (v.size() != 4 && v.size() != 2) {
    throw JsonException("Json not an array of proper size in jsonToRectI");
  }

  if (v.size() == 4) {
    return {static_cast<std::int32_t>(v.getInt(0)), static_cast<std::int32_t>(v.getInt(1)), static_cast<std::int32_t>(v.getInt(2)), static_cast<std::int32_t>(v.getInt(3))};
  }

  try {
    auto lowerLeft = jsonToVec2I(v.get(0));
    auto upperRight = jsonToVec2I(v.get(1));
    return {lowerLeft, upperRight};
  } catch (JsonException const& e) {
    throw JsonException(strf("Inner position not well formed in jsonToRectI: {}", outputException(e, true)));
  }
}

auto jsonFromRectI(RectI const& rect) -> Json {
  return JsonArray{rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax()};
}

auto jsonToRectU(Json const& v) -> RectU {
  if (v.type() != Json::Type::Array) {
    throw JsonException("Json not an array in jsonToRectU");
  }

  if (v.size() != 4 && v.size() != 2) {
    throw JsonException("Json not an array of proper size in jsonToRectU");
  }

  if (v.size() == 4) {
    return {static_cast<std::uint32_t>(v.getInt(0)), static_cast<std::uint32_t>(v.getUInt(1)), static_cast<std::uint32_t>(v.getUInt(2)), static_cast<std::uint32_t>(v.getUInt(3))};
  }

  try {
    auto lowerLeft = jsonToVec2U(v.get(0));
    auto upperRight = jsonToVec2U(v.get(1));
    return {lowerLeft, upperRight};
  } catch (JsonException const& e) {
    throw JsonException(strf("Inner position not well formed in jsonToRectU: {}", outputException(e, true)));
  }
}

auto jsonFromRectU(RectU const& rect) -> Json {
  return JsonArray{rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax()};
}

auto jsonToColor(Json const& v) -> Color {
  if (v.type() == Json::Type::Array) {
    if (v.type() != Json::Type::Array || (v.size() != 3 && v.size() != 4)) {
      throw JsonException("Json not an array of size 3 or 4 in jsonToColor");
    }
    Color c = Color::rgba(0, 0, 0, 255);
    c.setRed(v.getInt(0));
    c.setGreen(v.getInt(1));
    c.setBlue(v.getInt(2));

    if (v.size() == 4) {
      c.setAlpha(v.getInt(3));
    }

    return c;
  } else if (v.type() == Json::Type::String) {
    return Color(v.toString());
  } else {
    throw JsonException(strf("Json of type {} cannot be converted to color", v.typeName()));
  }
}

auto jsonFromColor(Color const& color) -> Json {
  JsonArray result;
  result.push_back(color.red());
  result.push_back(color.green());
  result.push_back(color.blue());
  if (color.alpha() < 255) {
    result.push_back(color.alpha());
  }
  return result;
}

auto jsonToPolyD(Json const& v) -> PolyD {
  PolyD poly;

  for (Json const& vertex : v.iterateArray()) {
    poly.add(jsonToVec2D(vertex));
  }

  return fixInsideOutPoly(poly);
}

auto jsonToPolyF(Json const& v) -> PolyF {
  PolyF poly;

  for (Json const& vertex : v.iterateArray()) {
    poly.add(jsonToVec2F(vertex));
  }

  return fixInsideOutPoly(poly);
}

auto jsonToPolyI(Json const& v) -> PolyI {
  PolyI poly;

  for (Json const& vertex : v.iterateArray()) {
    poly.add(jsonToVec2I(vertex));
  }

  return fixInsideOutPoly(poly);
}

auto jsonFromPolyF(PolyF const& poly) -> Json {
  JsonArray vertexList;
  for (auto const& vertex : poly.vertexes()) {
    vertexList.append(JsonArray{vertex[0], vertex[1]});
  }

  return vertexList;
}

auto jsonToLine2F(Json const& v) -> Line2F {
  return {jsonToVec2F(v.get(0)), jsonToVec2F(v.get(1))};
}

auto jsonFromLine2F(Line2F const& line) -> Json {
  return JsonArray{jsonFromVec2F(line.min()), jsonFromVec2F(line.max())};
}

auto jsonToMat3F(Json const& v) -> Mat3F {
  return {jsonToVec3F(v.get(0)), jsonToVec3F(v.get(1)), jsonToVec3F(v.get(2))};
}

auto jsonFromMat3F(Mat3F const& v) -> Json {
  return JsonArray{jsonFromVec3F(v[0]), jsonFromVec3F(v[1]), jsonFromVec3F(v[2])};
}

auto jsonToStringList(Json const& v) -> StringList {
  StringList result;
  for (auto const& entry : v.iterateArray()) {
    result.push_back(entry.toString());
  }
  return result;
}

auto jsonFromStringList(List<String> const& v) -> Json {
  JsonArray result;
  for (auto& e : v) {
    result.push_back(e);
  }
  return result;
}

auto jsonToFloatList(Json const& v) -> List<float> {
  List<float> result;
  for (auto const& entry : v.iterateArray()) {
    result.push_back(entry.toFloat());
  }
  return result;
}

auto jsonToStringSet(Json const& v) -> StringSet {
  StringSet result;
  for (auto const& entry : v.iterateArray()) {
    result.add(entry.toString());
  }
  return result;
}

auto jsonFromStringSet(StringSet const& v) -> Json {
  JsonArray result;
  for (auto& e : v) {
    result.push_back(e);
  }
  return result;
}

auto jsonToIntList(Json const& v) -> List<int> {
  List<int> result;
  for (auto const& entry : v.iterateArray()) {
    result.push_back(entry.toInt());
  }
  return result;
}

auto jsonToVec2IList(Json const& v) -> List<Vec2I> {
  List<Vec2I> result;
  for (auto const& entry : v.iterateArray()) {
    result.append(jsonToVec2I(entry));
  }
  return result;
}

auto jsonToVec2UList(Json const& v) -> List<Vec2U> {
  List<Vec2U> result;
  for (auto const& entry : v.iterateArray()) {
    result.append(jsonToVec2U(entry));
  }
  return result;
}

auto jsonToVec2FList(Json const& v) -> List<Vec2F> {
  List<Vec2F> result;
  for (auto const& entry : v.iterateArray()) {
    result.append(jsonToVec2F(entry));
  }
  return result;
}

auto jsonToVec4BList(Json const& v) -> List<Vec4B> {
  List<Vec4B> result;
  for (auto const& entry : v.iterateArray()) {
    result.append(jsonToVec4B(entry));
  }
  return result;
}

auto jsonToColorList(Json const& v) -> List<Color> {
  List<Color> result;
  for (auto const& entry : v.iterateArray()) {
    result.append(jsonToColor(entry));
  }
  return result;
}

auto jsonToDirectivesList(Json const& v) -> List<Directives> {
  List<Directives> result;
  for (auto const& entry : v.iterateArray()) {
    result.append(entry.toString());
  }
  return result;
}

auto jsonFromDirectivesList(List<Directives> const& v) -> Json {
  JsonArray result;
  for (auto& e : v) {
    if (e) {
      result.push_back(*e.stringPtr());
    }
  }
  return result;
}

auto weightedChoiceFromJson(Json const& source, Json const& default_) -> Json {
  if (source.isNull()) {
    return default_;
  }
  if (source.type() != Json::Type::Array) {
    throw StarException("Json of array type expected.");
  }
  List<std::pair<float, Json>> options;
  float sum = 0;
  std::size_t idx = 0;
  while (idx < source.size()) {
    float weight = 1;
    Json entry = source.get(idx);
    if (entry.type() == Json::Type::Int || entry.type() == Json::Type::Float) {
      weight = entry.toDouble();
      idx++;
      if (idx >= source.size()) {
        throw StarException("Weighted companion cube cannot cry.");
      }
      sum += weight;
      options.append(std::pair<float, Json>{weight, source.get(idx)});
    } else {
      sum += weight;
      options.append(std::pair<float, Json>{weight, entry});
    }
    idx++;
  }
  if (!options.size()) {
    return default_;
  }
  float choice = Random::randf() * sum;
  idx = 0;
  while (idx < options.size()) {
    auto const& entry = options[idx];
    if (entry.first >= choice) {
      return entry.second;
    }
    choice -= entry.first;
    idx++;
  }
  return options[options.size() - 1].second;
}

auto binnedChoiceFromJson(Json const& bins, float target, Json const& def) -> Json {
  JsonArray binList = bins.toArray();
  sortByComputedValue(binList, [](Json const& pair) -> float { return -pair.getFloat(0); });
  Json result = def;
  for (auto const& pair : binList) {
    if (pair.getFloat(0) <= target) {
      result = pair.get(1);
      break;
    }
  }
  return result;
}

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<int> {
  return jsonToWeightedPool<int>(source, [](Json const& v) -> std::int64_t { return v.toInt(); });
}

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<unsigned> {
  return jsonToWeightedPool<unsigned>(source, [](Json const& v) -> std::uint64_t { return v.toUInt(); });
}

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<float> {
  return jsonToWeightedPool<float>(source, [](Json const& v) -> float { return v.toFloat(); });
}

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<double> {
  return jsonToWeightedPool<double>(source, [](Json const& v) -> double { return v.toDouble(); });
}

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<String> {
  return jsonToWeightedPool<String>(source, [](Json const& v) -> String { return v.toString(); });
}

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<JsonArray> {
  return jsonToWeightedPool<JsonArray>(source, [](Json const& v) -> JsonArray { return v.toArray(); });
}

template <>
auto jsonToWeightedPool(Json const& source) -> WeightedPool<JsonObject> {
  return jsonToWeightedPool<JsonObject>(source, [](Json const& v) -> JsonObject { return v.toObject(); });
}

}// namespace Star
