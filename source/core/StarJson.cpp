#include "StarJson.hpp"
#include "StarConfig.hpp"
#include "StarFormat.hpp"
#include "StarJsonBuilder.hpp"
#include "StarJsonPath.hpp"

import std;

namespace Star {

auto Json::typeFromName(String const& t) -> Json::Type {
  if (t == "float")
    return Type::Float;
  else if (t == "bool")
    return Type::Bool;
  else if (t == "int")
    return Type::Int;
  else if (t == "string")
    return Type::String;
  else if (t == "array")
    return Type::Array;
  else if (t == "object")
    return Type::Object;
  else if (t == "null")
    return Type::Null;
  else
    throw JsonException(strf("String '{}' is not a valid json type", t));
}

auto Json::typeName(Type t) -> String {
  switch (t) {
  case Type::Float:
    return "float";
  case Type::Bool:
    return "bool";
  case Type::Int:
    return "int";
  case Type::String:
    return "string";
  case Type::Array:
    return "array";
  case Type::Object:
    return "object";
  default:
    return "null";
  }
}

auto Json::operator==(const Json& v) const -> bool {
  if (type() == Type::Null && v.type() == Type::Null) {
    return true;
  } else if (type() != v.type()) {
    if ((type() == Type::Float || type() == Type::Int) && (v.type() == Type::Float || v.type() == Type::Int))
      return toDouble() == v.toDouble() && toInt() == v.toInt();
    return false;
  } else {
    if (type() == Type::Float)
      return m_data.get<double>() == v.m_data.get<double>();
    else if (type() == Type::Bool)
      return m_data.get<bool>() == v.m_data.get<bool>();
    else if (type() == Type::Int)
      return m_data.get<std::int64_t>() == v.m_data.get<std::int64_t>();
    else if (type() == Type::String)
      return *m_data.get<ConstPtr<String>>() == *v.m_data.get<ConstPtr<String>>();
    else if (type() == Type::Array)
      return *m_data.get<ConstPtr<JsonArray>>() == *v.m_data.get<ConstPtr<JsonArray>>();
    else if (type() == Type::Object)
      return *m_data.get<ConstPtr<JsonObject>>() == *v.m_data.get<ConstPtr<JsonObject>>();
  }
  return false;
}

auto Json::operator!=(const Json& v) const -> bool {
  return !(*this == v);
}

auto Json::unique() const -> bool {
  if (m_data.is<ConstPtr<String>>())
    return m_data.get<ConstPtr<String>>().use_count() == 1;
  else if (m_data.is<ConstPtr<JsonArray>>())
    return m_data.get<ConstPtr<JsonArray>>().use_count() == 1;
  else if (m_data.is<ConstPtr<JsonObject>>())
    return m_data.get<ConstPtr<JsonObject>>().use_count() == 1;
  else
    return true;
}

auto Json::ofType(Type t) -> Json {
  switch (t) {
  case Type::Float:
    return {0.0};
  case Type::Bool:
    return {false};
  case Type::Int:
    return {0};
  case Type::String:
    return {""};
  case Type::Array:
    return {JsonArray()};
  case Type::Object:
    return {JsonObject()};
  default:
    return {};
  }
}

auto Json::parse(String const& string) -> Json {
  return inputUtf32Json<String::const_iterator>(string.begin(), string.end(), JsonParseType::Value);
}

auto Json::parseSequence(String const& sequence) -> Json {
  return inputUtf32Json<String::const_iterator>(sequence.begin(), sequence.end(), JsonParseType::Sequence);
}

auto Json::parseJson(String const& json) -> Json {
  return inputUtf32Json<String::const_iterator>(json.begin(), json.end(), JsonParseType::Top);
}

Json::Json() = default;

Json::Json(double d) {
  m_data = d;
}

Json::Json(bool b) {
  m_data = b;
}

Json::Json(int i) {
  m_data = (std::int64_t)i;
}

Json::Json(long i) {
  m_data = (std::int64_t)i;
}

Json::Json(long long i) {
  m_data = (std::int64_t)i;
}

Json::Json(unsigned int i) {
  m_data = (std::int64_t)i;
}

Json::Json(unsigned long i) {
  m_data = (std::int64_t)i;
}

Json::Json(unsigned long long i) {
  m_data = (std::int64_t)i;
}

Json::Json(char const* s) {
  m_data = std::make_shared<String const>(s);
}

Json::Json(String::Char const* s) {
  m_data = std::make_shared<String const>(s);
}

Json::Json(String::Char const* s, size_t len) {
  m_data = std::make_shared<String const>(s, len);
}

Json::Json(String s) {
  m_data = std::make_shared<String const>(std::move(s));
}

Json::Json(std::string s) {
  m_data = std::make_shared<String const>((std::move(s)));
}

Json::Json(JsonArray l) {
  m_data = std::make_shared<JsonArray const>(std::move(l));
}

Json::Json(JsonObject m) {
  m_data = std::make_shared<JsonObject const>(std::move(m));
}

auto Json::toDouble() const -> double {
  if (type() == Type::Float)
    return m_data.get<double>();
  if (type() == Type::Int)
    return (double)m_data.get<std::int64_t>();

  throw JsonException::format("Improper conversion to double from {}", typeName());
}

auto Json::toFloat() const -> float {
  return (float)toDouble();
}

auto Json::toBool() const -> bool {
  if (type() != Type::Bool)
    throw JsonException::format("Improper conversion to bool from {}", typeName());
  return m_data.get<bool>();
}

auto Json::toInt() const -> std::int64_t {
  if (type() == Type::Float) {
    return (std::int64_t)m_data.get<double>();
  } else if (type() == Type::Int) {
    return m_data.get<std::int64_t>();
  } else {
    throw JsonException::format("Improper conversion to int from {}", typeName());
  }
}

auto Json::toUInt() const -> std::uint64_t {
  if (type() == Type::Float) {
    return (std::uint64_t)m_data.get<double>();
  } else if (type() == Type::Int) {
    return (std::uint64_t)m_data.get<std::int64_t>();
  } else {
    throw JsonException::format("Improper conversion to unsigned int from {}", typeName());
  }
}

auto Json::toString() const -> String {
  if (type() != Type::String)
    throw JsonException(strf("Cannot convert from {} to string", typeName()));
  return *m_data.get<ConstPtr<String>>();
}

auto Json::toArray() const -> JsonArray {
  if (type() != Type::Array)
    throw JsonException::format("Improper conversion to JsonArray from {}", typeName());
  return *m_data.get<ConstPtr<JsonArray>>();
}

auto Json::toObject() const -> JsonObject {
  if (type() != Type::Object)
    throw JsonException::format("Improper conversion to JsonObject from {}", typeName());
  return *m_data.get<ConstPtr<JsonObject>>();
}

auto Json::stringPtr() const -> ConstPtr<String> {
  if (type() != Type::String)
    throw JsonException(strf("Cannot convert from {} to string", typeName()));
  return m_data.get<ConstPtr<String>>();
}

auto Json::arrayPtr() const -> ConstPtr<JsonArray> {
  if (type() != Type::Array)
    throw JsonException::format("Improper conversion to JsonArray from {}", typeName());
  return m_data.get<ConstPtr<JsonArray>>();
}

auto Json::objectPtr() const -> ConstPtr<JsonObject> {
  if (type() != Type::Object)
    throw JsonException::format("Improper conversion to JsonObject from {}", typeName());
  return m_data.get<ConstPtr<JsonObject>>();
}

auto Json::iterateArray() const -> Json::IteratorWrapper<JsonArray> {
  return IteratorWrapper<JsonArray>{arrayPtr()};
}

auto Json::iterateObject() const -> Json::IteratorWrapper<JsonObject> {
  return IteratorWrapper<JsonObject>{objectPtr()};
}

auto Json::opt() const -> std::optional<Json> {
  if (isNull())
    return std::nullopt;
  return *this;
}

auto Json::optDouble() const -> std::optional<double> {
  if (isNull())
    return std::nullopt;
  return toDouble();
}

auto Json::optFloat() const -> std::optional<float> {
  if (isNull())
    return std::nullopt;
  return toFloat();
}

auto Json::optBool() const -> std::optional<bool> {
  if (isNull())
    return std::nullopt;
  return toBool();
}

auto Json::optInt() const -> std::optional<std::int64_t> {
  if (isNull())
    return std::nullopt;
  return toInt();
}

auto Json::optUInt() const -> std::optional<std::uint64_t> {
  if (isNull())
    return std::nullopt;
  return toUInt();
}

auto Json::optString() const -> std::optional<String> {
  if (isNull())
    return std::nullopt;
  return toString();
}

auto Json::optArray() const -> std::optional<JsonArray> {
  if (isNull())
    return std::nullopt;
  return toArray();
}

auto Json::optObject() const -> std::optional<JsonObject> {
  if (isNull())
    return std::nullopt;
  return toObject();
}

auto Json::size() const -> size_t {
  if (type() == Type::Array)
    return m_data.get<ConstPtr<JsonArray>>()->size();
  else if (type() == Type::Object)
    return m_data.get<ConstPtr<JsonObject>>()->size();
  else
    throw JsonException("size() called on improper json type");
}

auto Json::contains(String const& key) const -> bool {
  if (type() == Type::Object)
    return m_data.get<ConstPtr<JsonObject>>()->contains(key);
  else
    throw JsonException("contains() called on improper json type");
}

auto Json::get(std::size_t index) const -> Json {
  if (auto p = ptr(index))
    return *p;
  throw JsonException(strf("Json::get({}) out of range", index));
}

auto Json::getDouble(std::size_t index) const -> double {
  return get(index).toDouble();
}

auto Json::getFloat(std::size_t index) const -> float {
  return get(index).toFloat();
}

auto Json::getBool(std::size_t index) const -> bool {
  return get(index).toBool();
}

auto Json::getInt(std::size_t index) const -> std::int64_t {
  return get(index).toInt();
}

auto Json::getUInt(std::size_t index) const -> std::uint64_t {
  return get(index).toUInt();
}

auto Json::getString(std::size_t index) const -> String {
  return get(index).toString();
}

auto Json::getArray(std::size_t index) const -> JsonArray {
  return get(index).toArray();
}

auto Json::getObject(std::size_t index) const -> JsonObject {
  return get(index).toObject();
}

auto Json::get(std::size_t index, Json def) const -> Json {
  if (auto p = ptr(index))
    return *p;
  return def;
}

auto Json::getDouble(std::size_t index, double def) const -> double {
  if (auto p = ptr(index))
    return p->toDouble();
  return def;
}

auto Json::getFloat(std::size_t index, float def) const -> float {
  if (auto p = ptr(index))
    return p->toFloat();
  return def;
}

auto Json::getBool(std::size_t index, bool def) const -> bool {
  if (auto p = ptr(index))
    return p->toBool();
  return def;
}

auto Json::getInt(std::size_t index, std::int64_t def) const -> std::int64_t {
  if (auto p = ptr(index))
    return p->toInt();
  return def;
}

auto Json::getUInt(std::size_t index, std::int64_t def) const -> std::uint64_t {
  if (auto p = ptr(index))
    return p->toUInt();
  return def;
}

auto Json::getString(std::size_t index, String def) const -> String {
  if (auto p = ptr(index))
    return p->toString();
  return def;
}

auto Json::getArray(std::size_t index, JsonArray def) const -> JsonArray {
  if (auto p = ptr(index))
    return p->toArray();
  return def;
}

auto Json::getObject(std::size_t index, JsonObject def) const -> JsonObject {
  if (auto p = ptr(index))
    return p->toObject();
  return def;
}

auto Json::get(String const& key) const -> Json {
  if (auto p = ptr(key))
    return *p;
  throw JsonException(strf("No such key in Json::get(\"{}\")", key));
}

auto Json::getDouble(String const& key) const -> double {
  return get(key).toDouble();
}

auto Json::getFloat(String const& key) const -> float {
  return get(key).toFloat();
}

auto Json::getBool(String const& key) const -> bool {
  return get(key).toBool();
}

auto Json::getInt(String const& key) const -> std::int64_t {
  return get(key).toInt();
}

auto Json::getUInt(String const& key) const -> std::uint64_t {
  return get(key).toUInt();
}

auto Json::getString(String const& key) const -> String {
  return get(key).toString();
}

auto Json::getArray(String const& key) const -> JsonArray {
  return get(key).toArray();
}

auto Json::getObject(String const& key) const -> JsonObject {
  return get(key).toObject();
}

auto Json::get(String const& key, Json def) const -> Json {
  if (auto p = ptr(key))
    return *p;
  return def;
}

auto Json::getDouble(String const& key, double def) const -> double {
  auto p = ptr(key);
  if (p && *p)
    return p->toDouble();
  return def;
}

auto Json::getFloat(String const& key, float def) const -> float {
  auto p = ptr(key);
  if (p && *p)
    return p->toFloat();
  return def;
}

auto Json::getBool(String const& key, bool def) const -> bool {
  auto p = ptr(key);
  if (p && *p)
    return p->toBool();
  return def;
}

auto Json::getInt(String const& key, std::int64_t def) const -> std::int64_t {
  auto p = ptr(key);
  if (p && *p)
    return p->toInt();
  return def;
}

auto Json::getUInt(String const& key, std::int64_t def) const -> std::uint64_t {
  auto p = ptr(key);
  if (p && *p)
    return p->toUInt();
  return def;
}

auto Json::getString(String const& key, String def) const -> String {
  auto p = ptr(key);
  if (p && *p)
    return p->toString();
  return def;
}

auto Json::getArray(String const& key, JsonArray def) const -> JsonArray {
  auto p = ptr(key);
  if (p && *p)
    return p->toArray();
  return def;
}

auto Json::getObject(String const& key, JsonObject def) const -> JsonObject {
  auto p = ptr(key);
  if (p && *p)
    return p->toObject();
  return def;
}

auto Json::opt(String const& key) const -> std::optional<Json> {
  auto p = ptr(key);
  if (p && *p)
    return *p;
  return std::nullopt;
}

auto Json::optDouble(String const& key) const -> std::optional<double> {
  auto p = ptr(key);
  if (p && *p)
    return p->toDouble();
  return std::nullopt;
}

auto Json::optFloat(String const& key) const -> std::optional<float> {
  auto p = ptr(key);
  if (p && *p)
    return p->toFloat();
  return std::nullopt;
}

auto Json::optBool(String const& key) const -> std::optional<bool> {
  auto p = ptr(key);
  if (p && *p)
    return p->toBool();
  return std::nullopt;
}

auto Json::optInt(String const& key) const -> std::optional<std::int64_t> {
  auto p = ptr(key);
  if (p && *p)
    return p->toInt();
  return std::nullopt;
}

auto Json::optUInt(String const& key) const -> std::optional<std::uint64_t> {
  auto p = ptr(key);
  if (p && *p)
    return p->toUInt();
  return std::nullopt;
}

auto Json::optString(String const& key) const -> std::optional<String> {
  auto p = ptr(key);
  if (p && *p)
    return p->toString();
  return std::nullopt;
}

auto Json::optArray(String const& key) const -> std::optional<JsonArray> {
  auto p = ptr(key);
  if (p && *p)
    return p->toArray();
  return std::nullopt;
}

auto Json::optObject(String const& key) const -> std::optional<JsonObject> {
  auto p = ptr(key);
  if (p && *p)
    return p->toObject();
  return std::nullopt;
}

auto Json::query(String const& q) const -> Json {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, q);
}

auto Json::queryDouble(String const& q) const -> double {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, q).toDouble();
}

auto Json::queryFloat(String const& q) const -> float {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, q).toFloat();
}

auto Json::queryBool(String const& q) const -> bool {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, q).toBool();
}

auto Json::queryInt(String const& q) const -> std::int64_t {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, q).toInt();
}

auto Json::queryUInt(String const& q) const -> std::uint64_t {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, q).toUInt();
}

auto Json::queryString(String const& q) const -> String {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, q).toString();
}

auto Json::queryArray(String const& q) const -> JsonArray {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, q).toArray();
}

auto Json::queryObject(String const& q) const -> JsonObject {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, q).toObject();
}

auto Json::query(String const& query, Json def) const -> Json {
  if (auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, query))
    return *json;
  return def;
}

auto Json::queryDouble(String const& query, double def) const -> double {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, query);
  if (json && *json)
    return json->toDouble();
  return def;
}

auto Json::queryFloat(String const& query, float def) const -> float {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, query);
  if (json && *json)
    return json->toFloat();
  return def;
}

auto Json::queryBool(String const& query, bool def) const -> bool {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, query);
  if (json && *json)
    return json->toBool();
  return def;
}

auto Json::queryInt(String const& query, std::int64_t def) const -> std::int64_t {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, query);
  if (json && *json)
    return json->toInt();
  return def;
}

auto Json::queryUInt(String const& query, std::uint64_t def) const -> std::uint64_t {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, query);
  if (json && *json)
    return json->toUInt();
  return def;
}

auto Json::queryString(String const& query, String const& def) const -> String {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, query);
  if (json && *json)
    return json->toString();
  return def;
}

auto Json::queryArray(String const& query, JsonArray def) const -> JsonArray {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, query);
  if (json && *json)
    return json->toArray();
  return def;
}

auto Json::queryObject(String const& query, JsonObject def) const -> JsonObject {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, query);
  if (json && *json)
    return json->toObject();
  return def;
}

auto Json::optQuery(String const& path) const -> std::optional<Json> {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (json && *json)
    return *json;
  return std::nullopt;
}

auto Json::optQueryDouble(String const& path) const -> std::optional<double> {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (json && *json)
    return json->toDouble();
  return std::nullopt;
}

auto Json::optQueryFloat(String const& path) const -> std::optional<float> {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (json && *json)
    return json->toFloat();
  return std::nullopt;
}

auto Json::optQueryBool(String const& path) const -> std::optional<bool> {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (json && *json)
    return json->toBool();
  return std::nullopt;
}

auto Json::optQueryInt(String const& path) const -> std::optional<std::int64_t> {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (json && *json)
    return json->toInt();
  return std::nullopt;
}

auto Json::optQueryUInt(String const& path) const -> std::optional<std::uint64_t> {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (json && *json)
    return json->toUInt();
  return std::nullopt;
}

auto Json::optQueryString(String const& path) const -> std::optional<String> {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (json && *json)
    return json->toString();
  return std::nullopt;
}

auto Json::optQueryArray(String const& path) const -> std::optional<JsonArray> {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (json && *json)
    return json->toArray();
  return std::nullopt;
}

auto Json::optQueryObject(String const& path) const -> std::optional<JsonObject> {
  auto json = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (json && *json)
    return json->toObject();
  return std::nullopt;
}

auto Json::set(String key, Json value) const -> Json {
  auto map = toObject();
  map[std::move(key)] = std::move(value);
  return map;
}

auto Json::setPath(String path, Json value) const -> Json {
  return JsonPath::pathSet(*this, JsonPath::parseQueryPath, path, value);
}

auto Json::erasePath(String path) const -> Json {
  return JsonPath::pathRemove(*this, JsonPath::parseQueryPath, path);
}

auto Json::setAll(JsonObject values) const -> Json {
  auto map = toObject();
  for (auto& p : values)
    map[std::move(p.first)] = std::move(p.second);
  return map;
}

auto Json::eraseKey(String key) const -> Json {
  auto map = toObject();
  map.erase(std::move(key));
  return map;
}

auto Json::set(size_t index, Json value) const -> Json {
  auto array = toArray();
  array[index] = std::move(value);
  return array;
}

auto Json::insert(size_t index, Json value) const -> Json {
  auto array = toArray();
  array.insertAt(index, std::move(value));
  return array;
}

auto Json::append(Json value) const -> Json {
  auto array = toArray();
  array.append(std::move(value));
  return array;
}

auto Json::eraseIndex(size_t index) const -> Json {
  auto array = toArray();
  array.eraseAt(index);
  return array;
}

auto Json::type() const -> Json::Type {
  return (Type)m_data.typeIndex();
}

auto Json::typeName() const -> String {
  return typeName(type());
}

auto Json::convert(Type u) const -> Json {
  if (type() == u)
    return *this;

  switch (u) {
  case Type::Null:
    return {};
  case Type::Float:
    return toDouble();
  case Type::Bool:
    return toBool();
  case Type::Int:
    return toInt();
  case Type::String:
    return toString();
  case Type::Array:
    return toArray();
  case Type::Object:
    return toObject();
  default:
    throw JsonException::format("Improper conversion to type {}", typeName(u));
  }
}

auto Json::isType(Type t) const -> bool {
  return type() == t;
}

auto Json::canConvert(Type t) const -> bool {
  if (type() == t)
    return true;

  if (t == Type::Null)
    return true;

  if ((type() == Type::Float || type() == Type::Int) && (t == Type::Float || t == Type::Int))
    return true;

  return false;
}

auto Json::isNull() const -> bool {
  return type() == Type::Null;
}

Json::operator bool() const {
  return !isNull();
}

auto Json::repr(int pretty, bool sort) const -> String {
  String result;
  outputUtf32Json(*this, std::back_inserter(result), pretty, sort);
  return result;
}

auto Json::printJson(int pretty, bool sort) const -> String {
  if (type() != Type::Object && type() != Type::Array)
    throw JsonException("printJson called on non-top-level JSON type");

  return repr(pretty, sort);
}

auto Json::printString() const -> String {
  if (type() == Type::String)
    return *m_data.get<ConstPtr<String>>();
  return repr();
}

auto operator<<(std::ostream& os, Json const& v) -> std::ostream& {
  outputUtf8Json(v, std::ostream_iterator<char>(os), 0, false);
  return os;
}

auto operator<<(std::ostream& os, JsonObject const& v) -> std::ostream& {
  // Blargh copy
  os << Json(v);
  return os;
}

auto operator<<(DataStream& os, const Json& v) -> DataStream& {
  // Compatibility with old serialization, 0 was INVALID but INVALID is no
  // longer used.
  os.write<std::uint8_t>((std::uint8_t)v.type() + 1);

  if (v.type() == Json::Type::Float) {
    os.write<double>(v.toDouble());
  } else if (v.type() == Json::Type::Bool) {
    os.write<bool>(v.toBool());
  } else if (v.type() == Json::Type::Int) {
    os.writeVlqI(v.toInt());
  } else if (v.type() == Json::Type::String) {
    os.write<String>(v.toString());
  } else if (v.type() == Json::Type::Array) {
    auto const& l = v.toArray();
    os.writeVlqU(l.size());
    for (auto const& v : l)
      os.write<Json>(v);
  } else if (v.type() == Json::Type::Object) {
    auto const& m = v.toObject();
    os.writeVlqU(m.size());
    for (auto const& v : m) {
      os.write<String>(v.first);
      os.write<Json>(v.second);
    }
  }
  return os;
}

auto operator>>(DataStream& os, Json& v) -> DataStream& {
  // Compatibility with old serialization, 0 was INVALID but INVALID is no
  // longer used.
  auto typeIndex = os.read<std::uint8_t>();
  if (typeIndex > 0)
    typeIndex -= 1;

  auto type = (Json::Type)typeIndex;

  if (type == Json::Type::Float) {
    v = Json(os.read<double>());
  } else if (type == Json::Type::Bool) {
    v = Json(os.read<bool>());
  } else if (type == Json::Type::Int) {
    v = Json(os.readVlqI());
  } else if (type == Json::Type::String) {
    v = Json(os.read<String>());
  } else if (type == Json::Type::Array) {
    JsonArray l;
    size_t s = os.readVlqU();
    for (size_t i = 0; i < s; ++i)
      l.append(os.read<Json>());

    v = std::move(l);
  } else if (type == Json::Type::Object) {
    JsonObject m;
    size_t s = os.readVlqU();
    for (size_t i = 0; i < s; ++i) {
      auto k = os.read<String>();
      m[k] = os.read<Json>();
    }

    v = std::move(m);
  }

  return os;
}

auto operator<<(DataStream& ds, JsonArray const& l) -> DataStream& {
  ds.writeContainer(l);
  return ds;
}

auto operator>>(DataStream& ds, JsonArray& l) -> DataStream& {
  ds.readContainer(l);
  return ds;
}

auto operator<<(DataStream& ds, JsonObject const& m) -> DataStream& {
  ds.writeMapContainer(m);
  return ds;
}

auto operator>>(DataStream& ds, JsonObject& m) -> DataStream& {
  ds.readMapContainer(m);
  return ds;
}

void Json::getHash(std::size_t& seed) const {
  auto type = (Json::Type)m_data.typeIndex();
  hashCombine(seed, (int)type);

  if (type == Json::Type::Bool)
    hashCombine(seed, m_data.get<bool>());
  else {
    if (type == Json::Type::Float)
      hashCombine(seed, *m_data.ptr<double>());
    else if (type == Json::Type::Int)
      hashCombine(seed, *m_data.ptr<std::int64_t>());
    else if (type == Json::Type::String) {
      const String& str = *m_data.get<ConstPtr<String>>();
      hashCombine(seed, std::string_view(str.utf8Ptr(), str.utf8Size()));
    } else if (type == Json::Type::Array) {
      for (Json const& json : *m_data.get<ConstPtr<JsonArray>>())
        json.getHash(seed);
    } else if (type == Json::Type::Object) {
      auto& object = *m_data.get<ConstPtr<JsonObject>>();
      List<JsonObject::const_iterator> iterators;
      iterators.reserve(object.size());
      for (auto i = object.begin(); i != object.end(); ++i)
        iterators.append(i);
      iterators.sort([](JsonObject::const_iterator a, JsonObject::const_iterator b) -> bool {
        return a->first < b->first;
      });
      for (auto& iter : iterators) {
        hashCombine(seed, std::string_view(iter->first.utf8Ptr(), iter->first.utf8Size()));
        iter->second.getHash(seed);
      }
    }
  }
}

auto hash<Json>::operator()(Json const& v) const -> size_t {
  size_t seed = 233;
  v.getHash(seed);
  return seed;
}

auto Json::ptr(size_t index) const -> Json const* {
  if (type() != Type::Array)
    throw JsonException::format("Cannot call get with index on Json type {}, must be Array type", typeName());

  auto const& list = *m_data.get<ConstPtr<JsonArray>>();
  if (index >= list.size())
    return nullptr;
  return &list[index];
}

auto Json::ptr(String const& key) const -> Json const* {
  if (type() != Type::Object)
    throw JsonException::format("Cannot call get with key on Json type {}, must be Object type", typeName());
  auto const& map = m_data.get<ConstPtr<JsonObject>>();

  auto i = map->find(key);
  if (i == map->end())
    return nullptr;

  return &i->second;
}

auto jsonMerge(Json const& base, Json const& merger) -> Json {
  if (base.type() == Json::Type::Object && merger.type() == Json::Type::Object) {
    JsonObject merged = base.toObject();
    for (auto const& p : merger.toObject()) {
      auto res = merged.insert(p);
      if (!res.second)
        res.first->second = jsonMerge(res.first->second, p.second);
    }
    return merged;
  }
  return merger.type() == Json::Type::Null ? base : merger;
}

auto jsonMergeNulling(Json const& base, Json const& merger) -> Json {
  if (base.type() == Json::Type::Object && merger.type() == Json::Type::Object) {
    JsonObject merged = base.toObject();
    for (auto const& p : merger.toObject()) {
      if (p.second.isNull())
        merged.erase(p.first);
      else {
        auto res = merged.insert(p);
        if (!res.second)
          res.first->second = jsonMergeNulling(res.first->second, p.second);
      }
    }
    return merged;
  }
  return merger;
}

auto jsonPartialMatch(Json const& base, Json const& compare) -> bool {
  if (base == compare) {
    return true;
  } else {
    if (base.type() == Json::Type::Object && compare.type() == Json::Type::Object) {
      for (auto const& c : compare.toObject()) {
        if (!base.contains(c.first) || !jsonPartialMatch(base.get(c.first), c.second))
          return false;
      }
      return true;
    }
    if (base.type() == Json::Type::Array && compare.type() == Json::Type::Array) {
      for (auto const& c : compare.toArray()) {
        bool similar = false;
        for (auto const& b : base.toArray()) {
          if (jsonPartialMatch(c, b)) {
            similar = true;
            break;
          }
        }
        if (!similar)
          return false;
      }
      return true;
    }

    return false;
  }
}

}// namespace Star
