import std;
import star.json;
import star.hash;
import star.xx_hash;
import star.string;
import star.list;
import star.map;
import star.data_stream;
import star.algorithm;
import star.lexical_cast;
import star.format;

import star.json_parser;
import star.json_builder;
import star.json_path;
import star.file;

namespace Star {

std::expected<Json::Type, StarException> Json::typeFromName(String const& t) {
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
    return std::unexpected(JsonException(strf("String '{}' is not a valid json type", t)));
}

String Json::typeName(Type t) {
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

bool Json::operator==(const Json& v) const {
  if (type() == Type::Null && v.type() == Type::Null) {
    return true;
  } else if (type() != v.type()) {
    if ((type() == Type::Float || type() == Type::Int) && (v.type() == Type::Float || v.type() == Type::Int))
      {
        auto const& integer = type() == Type::Int ? *this : v;
        auto const& floating = type() == Type::Float ? *this : v;
        auto i = std::get<std::int64_t>(integer.m_data);
        auto d = std::get<double>(floating.m_data);
        return d >= -0x1p63 && d < 0x1p63 && d == (double)i && (std::int64_t)d == i;
      }
    return false;
  } else {
    if (type() == Type::Float)
      return std::get<double>(m_data) == std::get<double>(v.m_data);
    else if (type() == Type::Bool)
      return std::get<bool>(m_data) == std::get<bool>(v.m_data);
    else if (type() == Type::Int)
      return std::get<std::int64_t>(m_data) == std::get<std::int64_t>(v.m_data);
    else if (type() == Type::String)
      return *std::get<std::shared_ptr<String const>>(m_data) == *std::get<std::shared_ptr<String const>>(v.m_data);
    else if (type() == Type::Array)
      return *std::get<std::shared_ptr<JsonArray const>>(m_data) == *std::get<std::shared_ptr<JsonArray const>>(v.m_data);
    else if (type() == Type::Object)
      return *std::get<std::shared_ptr<JsonObject const>>(m_data) == *std::get<std::shared_ptr<JsonObject const>>(v.m_data);
  }
  return false;
}

bool Json::operator!=(const Json& v) const {
  return !(*this == v);
}

bool Json::unique() const {
  if (std::holds_alternative<std::shared_ptr<String const>>(m_data))
    return std::get<std::shared_ptr<String const>>(m_data).use_count() == 1;
  else if (std::holds_alternative<std::shared_ptr<JsonArray const>>(m_data))
    return std::get<std::shared_ptr<JsonArray const>>(m_data).use_count() == 1;
  else if (std::holds_alternative<std::shared_ptr<JsonObject const>>(m_data))
    return std::get<std::shared_ptr<JsonObject const>>(m_data).use_count() == 1;
  else
    return true;
}

Json Json::ofType(Type t) {
  switch (t) {
    case Type::Float:
      return Json(0.0);
    case Type::Bool:
      return Json(false);
    case Type::Int:
      return Json(0);
    case Type::String:
      return Json("");
    case Type::Array:
      return Json(JsonArray());
    case Type::Object:
      return Json(JsonObject());
    default:
      return Json();
  }
}

std::expected<Json, StarException> Json::parse(String const& string) {
  auto const& bytes = string.utf8();
  return inputUtf8Json(bytes.begin(), bytes.end(), JsonParseType::Value);
}

std::expected<Json, StarException> Json::parseSequence(String const& sequence) {
  auto const& bytes = sequence.utf8();
  return inputUtf8Json(bytes.begin(), bytes.end(), JsonParseType::Sequence);
}

std::expected<Json, StarException> Json::parseJson(String const& json) {
  auto const& bytes = json.utf8();
  return inputUtf8Json(bytes.begin(), bytes.end(), JsonParseType::Top);
}

Json::Json() {}

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

std::expected<Json, StarException> Json::create(String::Char const* s) {
  auto string = String::create(s);
  if (!string)
    return std::unexpected(std::move(string.error()));
  return Json(std::move(*string));
}

std::expected<Json, StarException> Json::create(String::Char const* s, std::size_t len) {
  auto string = String::create(s, len);
  if (!string)
    return std::unexpected(std::move(string.error()));
  return Json(std::move(*string));
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

std::expected<double, StarException> Json::toDouble() const {
  if (type() == Type::Float)
    return std::get<double>(m_data);
  if (type() == Type::Int)
    return (double)std::get<std::int64_t>(m_data);
  return std::unexpected(JsonException::format("Improper conversion to double from {}", typeName()));
}

std::expected<float, StarException> Json::toFloat() const {
  auto value = toDouble();
  if (!value)
    return std::unexpected(std::move(value.error()));
  return (float)*value;
}

std::expected<bool, StarException> Json::toBool() const {
  if (type() != Type::Bool)
    return std::unexpected(JsonException::format("Improper conversion to bool from {}", typeName()));
  return std::get<bool>(m_data);
}

std::expected<std::int64_t, StarException> Json::toInt() const {
  if (type() == Type::Float) {
    auto value = std::get<double>(m_data);
    if (!(value >= -0x1p63 && value < 0x1p63))
      return std::unexpected(JsonException::format("Cannot convert out-of-range float {} to int", value));
    return (std::int64_t)value;
  }
  if (type() == Type::Int)
    return (std::int64_t)std::get<std::int64_t>(m_data);
  return std::unexpected(JsonException::format("Improper conversion to int from {}", typeName()));
}

std::expected<std::uint64_t, StarException> Json::toUInt() const {
  if (type() == Type::Float) {
    auto value = std::get<double>(m_data);
    if (!(value > -1.0 && value < 0x1p64))
      return std::unexpected(JsonException::format("Cannot convert out-of-range float {} to unsigned int", value));
    return (std::uint64_t)value;
  }
  if (type() == Type::Int)
    return (std::uint64_t)std::get<std::int64_t>(m_data);
  return std::unexpected(JsonException::format("Improper conversion to unsigned int from {}", typeName()));
}

std::expected<String, StarException> Json::toString() const {
  if (type() != Type::String)
    return std::unexpected(JsonException::format("Cannot convert from {} to string", typeName()));
  return *std::get<std::shared_ptr<String const>>(m_data);
}

std::expected<std::shared_ptr<String const>, StarException> Json::stringPtr() const {
  if (type() != Type::String)
    return std::unexpected(JsonException::format("Cannot convert from {} to string", typeName()));
  return std::get<std::shared_ptr<String const>>(m_data);
}

std::expected<JsonArray, StarException> Json::toArray() const {
  if (type() != Type::Array)
    return std::unexpected(JsonException::format("Improper conversion to JsonArray from {}", typeName()));
  return *std::get<std::shared_ptr<JsonArray const>>(m_data);
}

std::expected<std::shared_ptr<JsonArray const>, StarException> Json::arrayPtr() const {
  if (type() != Type::Array)
    return std::unexpected(JsonException::format("Improper conversion to JsonArray from {}", typeName()));
  return std::get<std::shared_ptr<JsonArray const>>(m_data);
}

std::expected<Json::IteratorWrapper<JsonArray>, StarException> Json::iterateArray() const {
  auto pointer = arrayPtr();
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  return IteratorWrapper<JsonArray>{std::move(*pointer)};
}

std::expected<JsonObject, StarException> Json::toObject() const {
  if (type() != Type::Object)
    return std::unexpected(JsonException::format("Improper conversion to JsonObject from {}", typeName()));
  return *std::get<std::shared_ptr<JsonObject const>>(m_data);
}

std::expected<std::shared_ptr<JsonObject const>, StarException> Json::objectPtr() const {
  if (type() != Type::Object)
    return std::unexpected(JsonException::format("Improper conversion to JsonObject from {}", typeName()));
  return std::get<std::shared_ptr<JsonObject const>>(m_data);
}

std::expected<Json::IteratorWrapper<JsonObject>, StarException> Json::iterateObject() const {
  auto pointer = objectPtr();
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  return IteratorWrapper<JsonObject>{std::move(*pointer)};
}

std::optional<Json> Json::opt() const {
  if (isNull())
    return {};
  return *this;
}

std::expected<std::optional<double>, StarException> Json::optDouble() const {
  if (isNull())
    return std::optional<double>{};
  auto value = toDouble();
  if (!value)
    return std::unexpected(std::move(value.error()));
  return std::optional<double>(std::move(*value));
}

std::expected<std::optional<float>, StarException> Json::optFloat() const {
  if (isNull())
    return std::optional<float>{};
  auto value = toFloat();
  if (!value)
    return std::unexpected(std::move(value.error()));
  return std::optional<float>(std::move(*value));
}

std::expected<std::optional<bool>, StarException> Json::optBool() const {
  if (isNull())
    return std::optional<bool>{};
  auto value = toBool();
  if (!value)
    return std::unexpected(std::move(value.error()));
  return std::optional<bool>(std::move(*value));
}

std::expected<std::optional<std::int64_t>, StarException> Json::optInt() const {
  if (isNull())
    return std::optional<std::int64_t>{};
  auto value = toInt();
  if (!value)
    return std::unexpected(std::move(value.error()));
  return std::optional<std::int64_t>(std::move(*value));
}

std::expected<std::optional<std::uint64_t>, StarException> Json::optUInt() const {
  if (isNull())
    return std::optional<std::uint64_t>{};
  auto value = toUInt();
  if (!value)
    return std::unexpected(std::move(value.error()));
  return std::optional<std::uint64_t>(std::move(*value));
}

std::expected<std::optional<String>, StarException> Json::optString() const {
  if (isNull())
    return std::optional<String>{};
  auto value = toString();
  if (!value)
    return std::unexpected(std::move(value.error()));
  return std::optional<String>(std::move(*value));
}

std::expected<std::optional<JsonArray>, StarException> Json::optArray() const {
  if (isNull())
    return std::optional<JsonArray>{};
  auto value = toArray();
  if (!value)
    return std::unexpected(std::move(value.error()));
  return std::optional<JsonArray>(std::move(*value));
}

std::expected<std::optional<JsonObject>, StarException> Json::optObject() const {
  if (isNull())
    return std::optional<JsonObject>{};
  auto value = toObject();
  if (!value)
    return std::unexpected(std::move(value.error()));
  return std::optional<JsonObject>(std::move(*value));
}

std::expected<std::size_t, StarException> Json::size() const {
  if (type() == Type::Array)
    return std::get<std::shared_ptr<JsonArray const>>(m_data)->size();
  if (type() == Type::Object)
    return std::get<std::shared_ptr<JsonObject const>>(m_data)->size();
  return std::unexpected(JsonException("size() called on improper json type"));
}

std::expected<bool, StarException> Json::contains(String const& key) const {
  if (type() == Type::Object)
    return std::get<std::shared_ptr<JsonObject const>>(m_data)->contains(key);
  return std::unexpected(JsonException("contains() called on improper json type"));
}

std::expected<Json, StarException> Json::get(std::size_t index) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return **pointer;
  return std::unexpected(JsonException::format("Json::get({}) out of range", index));
}

std::expected<Json, StarException> Json::get(std::size_t index, Json def) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return **pointer;
  return def;
}

std::expected<double, StarException> Json::getDouble(std::size_t index) const {
  auto value = get(index);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toDouble();
}

std::expected<double, StarException> Json::getDouble(std::size_t index, double def) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return (*pointer)->toDouble();
  return def;
}

std::expected<float, StarException> Json::getFloat(std::size_t index) const {
  auto value = get(index);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toFloat();
}

std::expected<float, StarException> Json::getFloat(std::size_t index, float def) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return (*pointer)->toFloat();
  return def;
}

std::expected<bool, StarException> Json::getBool(std::size_t index) const {
  auto value = get(index);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toBool();
}

std::expected<bool, StarException> Json::getBool(std::size_t index, bool def) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return (*pointer)->toBool();
  return def;
}

std::expected<std::int64_t, StarException> Json::getInt(std::size_t index) const {
  auto value = get(index);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toInt();
}

std::expected<std::int64_t, StarException> Json::getInt(std::size_t index, std::int64_t def) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return (*pointer)->toInt();
  return def;
}

std::expected<std::uint64_t, StarException> Json::getUInt(std::size_t index) const {
  auto value = get(index);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toUInt();
}

std::expected<std::uint64_t, StarException> Json::getUInt(std::size_t index, std::int64_t def) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return (*pointer)->toUInt();
  return def;
}

std::expected<String, StarException> Json::getString(std::size_t index) const {
  auto value = get(index);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toString();
}

std::expected<String, StarException> Json::getString(std::size_t index, String def) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return (*pointer)->toString();
  return def;
}

std::expected<JsonArray, StarException> Json::getArray(std::size_t index) const {
  auto value = get(index);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toArray();
}

std::expected<JsonArray, StarException> Json::getArray(std::size_t index, JsonArray def) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return (*pointer)->toArray();
  return def;
}

std::expected<JsonObject, StarException> Json::getObject(std::size_t index) const {
  auto value = get(index);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toObject();
}

std::expected<JsonObject, StarException> Json::getObject(std::size_t index, JsonObject def) const {
  auto pointer = ptr(index);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return (*pointer)->toObject();
  return def;
}

std::expected<Json, StarException> Json::get(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return **pointer;
  return std::unexpected(JsonException::format("No such key in Json::get(\"{}\")", key));
}

std::expected<Json, StarException> Json::get(String const& key, Json def) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer)
    return **pointer;
  return def;
}

std::expected<double, StarException> Json::getDouble(String const& key) const {
  auto value = get(key);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toDouble();
}

std::expected<double, StarException> Json::getDouble(String const& key, double def) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer && **pointer)
    return (*pointer)->toDouble();
  return def;
}

std::expected<float, StarException> Json::getFloat(String const& key) const {
  auto value = get(key);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toFloat();
}

std::expected<float, StarException> Json::getFloat(String const& key, float def) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer && **pointer)
    return (*pointer)->toFloat();
  return def;
}

std::expected<bool, StarException> Json::getBool(String const& key) const {
  auto value = get(key);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toBool();
}

std::expected<bool, StarException> Json::getBool(String const& key, bool def) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer && **pointer)
    return (*pointer)->toBool();
  return def;
}

std::expected<std::int64_t, StarException> Json::getInt(String const& key) const {
  auto value = get(key);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toInt();
}

std::expected<std::int64_t, StarException> Json::getInt(String const& key, std::int64_t def) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer && **pointer)
    return (*pointer)->toInt();
  return def;
}

std::expected<std::uint64_t, StarException> Json::getUInt(String const& key) const {
  auto value = get(key);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toUInt();
}

std::expected<std::uint64_t, StarException> Json::getUInt(String const& key, std::int64_t def) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer && **pointer)
    return (*pointer)->toUInt();
  return def;
}

std::expected<String, StarException> Json::getString(String const& key) const {
  auto value = get(key);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toString();
}

std::expected<String, StarException> Json::getString(String const& key, String def) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer && **pointer)
    return (*pointer)->toString();
  return def;
}

std::expected<JsonArray, StarException> Json::getArray(String const& key) const {
  auto value = get(key);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toArray();
}

std::expected<JsonArray, StarException> Json::getArray(String const& key, JsonArray def) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer && **pointer)
    return (*pointer)->toArray();
  return def;
}

std::expected<JsonObject, StarException> Json::getObject(String const& key) const {
  auto value = get(key);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toObject();
}

std::expected<JsonObject, StarException> Json::getObject(String const& key, JsonObject def) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer && **pointer)
    return (*pointer)->toObject();
  return def;
}

std::expected<std::optional<Json>, StarException> Json::opt(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (*pointer && **pointer)
    return std::optional<Json>(**pointer);
  return std::optional<Json>{};
}

std::expected<std::optional<double>, StarException> Json::optDouble(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (!*pointer || !(**pointer))
    return std::optional<double>{};
  return (*pointer)->optDouble();
}

std::expected<std::optional<float>, StarException> Json::optFloat(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (!*pointer || !(**pointer))
    return std::optional<float>{};
  return (*pointer)->optFloat();
}

std::expected<std::optional<bool>, StarException> Json::optBool(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (!*pointer || !(**pointer))
    return std::optional<bool>{};
  return (*pointer)->optBool();
}

std::expected<std::optional<std::int64_t>, StarException> Json::optInt(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (!*pointer || !(**pointer))
    return std::optional<std::int64_t>{};
  return (*pointer)->optInt();
}

std::expected<std::optional<std::uint64_t>, StarException> Json::optUInt(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (!*pointer || !(**pointer))
    return std::optional<std::uint64_t>{};
  return (*pointer)->optUInt();
}

std::expected<std::optional<String>, StarException> Json::optString(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (!*pointer || !(**pointer))
    return std::optional<String>{};
  return (*pointer)->optString();
}

std::expected<std::optional<JsonArray>, StarException> Json::optArray(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (!*pointer || !(**pointer))
    return std::optional<JsonArray>{};
  return (*pointer)->optArray();
}

std::expected<std::optional<JsonObject>, StarException> Json::optObject(String const& key) const {
  auto pointer = ptr(key);
  if (!pointer)
    return std::unexpected(std::move(pointer.error()));
  if (!*pointer || !(**pointer))
    return std::optional<JsonObject>{};
  return (*pointer)->optObject();
}

std::expected<Json, StarException> Json::query(String const& path) const {
  return JsonPath::pathGet(*this, JsonPath::parseQueryPath, path);
}

std::expected<double, StarException> Json::queryDouble(String const& path) const {
  auto value = query(path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toDouble();
}

std::expected<float, StarException> Json::queryFloat(String const& path) const {
  auto value = query(path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toFloat();
}

std::expected<bool, StarException> Json::queryBool(String const& path) const {
  auto value = query(path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toBool();
}

std::expected<std::int64_t, StarException> Json::queryInt(String const& path) const {
  auto value = query(path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toInt();
}

std::expected<std::uint64_t, StarException> Json::queryUInt(String const& path) const {
  auto value = query(path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toUInt();
}

std::expected<String, StarException> Json::queryString(String const& path) const {
  auto value = query(path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toString();
}

std::expected<JsonArray, StarException> Json::queryArray(String const& path) const {
  auto value = query(path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toArray();
}

std::expected<JsonObject, StarException> Json::queryObject(String const& path) const {
  auto value = query(path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->toObject();
}

std::expected<Json, StarException> Json::query(String const& path, Json def) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value)
    return std::move(**value);
  return def;
}

std::expected<double, StarException> Json::queryDouble(String const& path, double def) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).toDouble();
  return def;
}

std::expected<float, StarException> Json::queryFloat(String const& path, float def) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).toFloat();
  return def;
}

std::expected<bool, StarException> Json::queryBool(String const& path, bool def) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).toBool();
  return def;
}

std::expected<std::int64_t, StarException> Json::queryInt(String const& path, std::int64_t def) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).toInt();
  return def;
}

std::expected<std::uint64_t, StarException> Json::queryUInt(String const& path, std::uint64_t def) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).toUInt();
  return def;
}

std::expected<String, StarException> Json::queryString(String const& path, String const& def) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).toString();
  return def;
}

std::expected<JsonArray, StarException> Json::queryArray(String const& path, JsonArray def) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).toArray();
  return def;
}

std::expected<JsonObject, StarException> Json::queryObject(String const& path, JsonObject def) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).toObject();
  return def;
}

std::expected<std::optional<Json>, StarException> Json::optQuery(String const& path) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return std::move(*value);
  return std::optional<Json>{};
}

std::expected<std::optional<double>, StarException> Json::optQueryDouble(String const& path) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).optDouble();
  return std::optional<double>{};
}

std::expected<std::optional<float>, StarException> Json::optQueryFloat(String const& path) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).optFloat();
  return std::optional<float>{};
}

std::expected<std::optional<bool>, StarException> Json::optQueryBool(String const& path) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).optBool();
  return std::optional<bool>{};
}

std::expected<std::optional<std::int64_t>, StarException> Json::optQueryInt(String const& path) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).optInt();
  return std::optional<std::int64_t>{};
}

std::expected<std::optional<std::uint64_t>, StarException> Json::optQueryUInt(String const& path) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).optUInt();
  return std::optional<std::uint64_t>{};
}

std::expected<std::optional<String>, StarException> Json::optQueryString(String const& path) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).optString();
  return std::optional<String>{};
}

std::expected<std::optional<JsonArray>, StarException> Json::optQueryArray(String const& path) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).optArray();
  return std::optional<JsonArray>{};
}

std::expected<std::optional<JsonObject>, StarException> Json::optQueryObject(String const& path) const {
  auto value = JsonPath::pathFind(*this, JsonPath::parseQueryPath, path);
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (*value && **value)
    return (**value).optObject();
  return std::optional<JsonObject>{};
}

std::expected<Json, StarException> Json::set(String key, Json value) const {
  auto map = toObject();
  if (!map)
    return std::unexpected(std::move(map.error()));
  (*map)[std::move(key)] = std::move(value);
  return Json(std::move(*map));
}

std::expected<Json, StarException> Json::setPath(String path, Json value) const {
  return JsonPath::pathSet(*this, JsonPath::parseQueryPath, path, value);
}

std::expected<Json, StarException> Json::erasePath(String path) const {
  return JsonPath::pathRemove(*this, JsonPath::parseQueryPath, path);
}

std::expected<Json, StarException> Json::setAll(JsonObject values) const {
  auto map = toObject();
  if (!map)
    return std::unexpected(std::move(map.error()));
  for (auto& p : values)
    (*map)[std::move(p.first)] = std::move(p.second);
  return Json(std::move(*map));
}

std::expected<Json, StarException> Json::eraseKey(String key) const {
  auto map = toObject();
  if (!map)
    return std::unexpected(std::move(map.error()));
  map->erase(key);
  return Json(std::move(*map));
}

std::expected<Json, StarException> Json::set(std::size_t index, Json value) const {
  auto array = toArray();
  if (!array)
    return std::unexpected(std::move(array.error()));
  if (index >= array->size())
    return std::unexpected(JsonException::format("Json::set({}) out of range", index));
  (*array)[index] = std::move(value);
  return Json(std::move(*array));
}

std::expected<Json, StarException> Json::insert(std::size_t index, Json value) const {
  auto array = toArray();
  if (!array)
    return std::unexpected(std::move(array.error()));
  if (index > array->size())
    return std::unexpected(JsonException::format("Json::insert({}) out of range", index));
  array->insert(array->begin() + index, std::move(value));
  return Json(std::move(*array));
}

std::expected<Json, StarException> Json::append(Json value) const {
  auto array = toArray();
  if (!array)
    return std::unexpected(std::move(array.error()));
  array->append(std::move(value));
  return Json(std::move(*array));
}

std::expected<Json, StarException> Json::eraseIndex(std::size_t index) const {
  auto array = toArray();
  if (!array)
    return std::unexpected(std::move(array.error()));
  if (index >= array->size())
    return std::unexpected(JsonException::format("Json::eraseIndex({}) out of range", index));
  array->erase(array->begin() + index);
  return Json(std::move(*array));
}

Json::Type Json::type() const {
  return (Type)m_data.index();
}

String Json::typeName() const {
  return typeName(type());
}

std::expected<Json, StarException> Json::convert(Type target) const {
  if (type() == target)
    return *this;
  switch (target) {
    case Type::Null:
      return Json();
    case Type::Float: {
      auto value = toDouble();
      if (!value)
        return std::unexpected(std::move(value.error()));
      return Json(std::move(*value));
    }
    case Type::Bool: {
      auto value = toBool();
      if (!value)
        return std::unexpected(std::move(value.error()));
      return Json(std::move(*value));
    }
    case Type::Int: {
      auto value = toInt();
      if (!value)
        return std::unexpected(std::move(value.error()));
      return Json(std::move(*value));
    }
    case Type::String: {
      auto value = toString();
      if (!value)
        return std::unexpected(std::move(value.error()));
      return Json(std::move(*value));
    }
    case Type::Array: {
      auto value = toArray();
      if (!value)
        return std::unexpected(std::move(value.error()));
      return Json(std::move(*value));
    }
    case Type::Object: {
      auto value = toObject();
      if (!value)
        return std::unexpected(std::move(value.error()));
      return Json(std::move(*value));
    }
    default:
      return std::unexpected(JsonException::format("Improper conversion to type {}", typeName(target)));
  }
}

bool Json::isType(Type t) const {
  return type() == t;
}

bool Json::canConvert(Type t) const {
  if (type() == t)
    return true;

  if (t == Type::Null)
    return true;

  if ((type() == Type::Float || type() == Type::Int) && (t == Type::Float || t == Type::Int))
    return true;

  return false;
}

bool Json::isNull() const {
  return type() == Type::Null;
}

Json::operator bool() const {
  return !isNull();
}

std::expected<String, StarException> Json::repr(int pretty, bool sort) const {
  std::string result;
  auto written = outputUtf8Json(*this, std::back_inserter(result), pretty, sort);
  if (!written)
    return std::unexpected(std::move(written.error()));
  return String(std::move(result));
}

std::expected<String, StarException> Json::printJson(int pretty, bool sort) const {
  if (type() != Type::Object && type() != Type::Array)
    return std::unexpected(JsonException("printJson called on non-top-level JSON type"));

  return repr(pretty, sort);
}

std::expected<String, StarException> Json::printString() const {
  if (type() == Type::String)
    return *std::get<std::shared_ptr<String const>>(m_data);
  return repr(0, false);
}

std::expected<std::ostream*, StarException> operator<<(std::ostream& os, Json const& value) {
  auto written = outputUtf8Json(value, std::ostream_iterator<char>(os), 0, false);
  if (!written)
    return std::unexpected(std::move(written.error()));
  return &os;
}

std::expected<std::ostream*, StarException> operator<<(std::ostream& os, JsonObject const& value) {
  return os << Json(value);
}

std::expected<DataStream*, StarException> operator<<(DataStream& ds, Json const& value) {
  auto header = ds.write<std::uint8_t>((std::uint8_t)value.type() + 1);
  if (!header)
    return std::unexpected(std::move(header.error()));
  switch (value.type()) {
    case Json::Type::Float: {
      auto converted = value.toDouble();
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      auto written = ds.write<double>(*converted);
      if (!written)
        return std::unexpected(std::move(written.error()));
      break;
    }
    case Json::Type::Bool: {
      auto converted = value.toBool();
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      auto written = ds.write<bool>(*converted);
      if (!written)
        return std::unexpected(std::move(written.error()));
      break;
    }
    case Json::Type::Int: {
      auto converted = value.toInt();
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      auto written = ds.writeVlqI(*converted);
      if (!written)
        return std::unexpected(std::move(written.error()));
      break;
    }
    case Json::Type::String: {
      auto converted = value.toString();
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      auto written = ds.write<String>(*converted);
      if (!written)
        return std::unexpected(std::move(written.error()));
      break;
    }
    case Json::Type::Array: {
      auto converted = value.arrayPtr();
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      auto count = ds.writeVlqU((*converted)->size());
      if (!count)
        return std::unexpected(std::move(count.error()));
      for (auto const& entry : **converted) {
        auto item = ds.write<Json>(entry);
        if (!item)
          return std::unexpected(std::move(item.error()));
      }
      break;
    }
    case Json::Type::Object: {
      auto converted = value.objectPtr();
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      auto count = ds.writeVlqU((*converted)->size());
      if (!count)
        return std::unexpected(std::move(count.error()));
      for (auto const& entry : **converted) {
        auto key = ds.write<String>(entry.first);
        if (!key)
          return std::unexpected(std::move(key.error()));
        auto item = ds.write<Json>(entry.second);
        if (!item)
          return std::unexpected(std::move(item.error()));
      }
      break;
    }
    default:
      break;
  }
  return &ds;
}

std::expected<DataStream*, StarException> operator>>(DataStream& ds, Json& value) {
  auto header = ds.read<std::uint8_t>();
  if (!header)
    return std::unexpected(std::move(header.error()));
  auto typeIndex = *header;
  if (typeIndex > 0)
    --typeIndex;
  auto type = (Json::Type)typeIndex;
  switch (type) {
    case Json::Type::Float: {
      auto item = ds.read<double>();
      if (!item)
        return std::unexpected(std::move(item.error()));
      value = Json(std::move(*item));
      break;
    }
    case Json::Type::Bool: {
      auto item = ds.read<bool>();
      if (!item)
        return std::unexpected(std::move(item.error()));
      value = Json(std::move(*item));
      break;
    }
    case Json::Type::Int: {
      auto item = ds.readVlqI();
      if (!item)
        return std::unexpected(std::move(item.error()));
      value = Json(std::move(*item));
      break;
    }
    case Json::Type::String: {
      auto item = ds.read<String>();
      if (!item)
        return std::unexpected(std::move(item.error()));
      value = Json(std::move(*item));
      break;
    }
    case Json::Type::Array: {
      auto count = ds.readVlqU();
      if (!count)
        return std::unexpected(std::move(count.error()));
      JsonArray items;
      for (std::uint64_t index = 0; index < *count; ++index) {
        auto item = ds.read<Json>();
        if (!item)
          return std::unexpected(std::move(item.error()));
        items.append(std::move(*item));
      }
      value = Json(std::move(items));
      break;
    }
    case Json::Type::Object: {
      auto count = ds.readVlqU();
      if (!count)
        return std::unexpected(std::move(count.error()));
      JsonObject items;
      for (std::uint64_t index = 0; index < *count; ++index) {
        auto key = ds.read<String>();
        if (!key)
          return std::unexpected(std::move(key.error()));
        auto item = ds.read<Json>();
        if (!item)
          return std::unexpected(std::move(item.error()));
        items[std::move(*key)] = std::move(*item);
      }
      value = Json(std::move(items));
      break;
    }
    default:
      break;
  }
  return &ds;
}

std::expected<DataStream*, StarException> operator<<(DataStream& ds, JsonArray const& value) {
  auto result = ds.writeContainer(value);
  if (!result)
    return std::unexpected(std::move(result.error()));
  return &ds;
}

std::expected<DataStream*, StarException> operator>>(DataStream& ds, JsonArray& value) {
  auto result = ds.readContainer(value);
  if (!result)
    return std::unexpected(std::move(result.error()));
  return &ds;
}

std::expected<DataStream*, StarException> operator<<(DataStream& ds, JsonObject const& value) {
  auto result = ds.writeMapContainer(value);
  if (!result)
    return std::unexpected(std::move(result.error()));
  return &ds;
}

std::expected<DataStream*, StarException> operator>>(DataStream& ds, JsonObject& value) {
  auto result = ds.readMapContainer(value);
  if (!result)
    return std::unexpected(std::move(result.error()));
  return &ds;
}

void Json::getHash(XXHash3& hasher) const {
  Json::Type type = (Json::Type)m_data.index();
  if (type == Json::Type::Bool)
    hasher.push(std::get<bool>(m_data) ? "\2\1" : "\2\0", 2);
  else {
    hasher.push((const char*)&type, sizeof(type));
    if (type == Json::Type::Float)
      hasher.push((const char*)std::get_if<double>(&m_data), sizeof(double));
    else if (type == Json::Type::Int)
      hasher.push((const char*)std::get_if<std::int64_t>(&m_data), sizeof(std::int64_t));
    else if (type == Json::Type::String) {
      const String& str = *std::get<std::shared_ptr<String const>>(m_data);
      hasher.push(str.utf8Ptr(), str.utf8Size());
    }
    else if (type == Json::Type::Array) {
      for (Json const& json : *std::get<std::shared_ptr<JsonArray const>>(m_data))
        json.getHash(hasher);
    }
    else if (type == Json::Type::Object) {
      auto& object = *std::get<std::shared_ptr<JsonObject const>>(m_data);
      List<JsonObject::const_iterator> iterators;
      iterators.reserve(object.size());
      for (auto i = object.begin(); i != object.end(); ++i)
        iterators.append(i);
      iterators.sort([](JsonObject::const_iterator a, JsonObject::const_iterator b) {
        return a->first < b->first;
      });
      for (auto& iter : iterators) {
        hasher.push(iter->first.utf8Ptr(), iter->first.utf8Size());
        iter->second.getHash(hasher);
      }
    }
  }
}

std::size_t hash<Json>::operator()(Json const& v) const {
  XXHash3 hasher;
  v.getHash(hasher);
  return hasher.digest();
}

std::expected<Json const*, StarException> Json::ptr(std::size_t index) const {
  if (type() != Type::Array)
    return std::unexpected(JsonException::format("Cannot call get with index on Json type {}, must be Array type", typeName()));

  auto const& list = *std::get<std::shared_ptr<JsonArray const>>(m_data);
  if (index >= list.size())
    return nullptr;
  return &list[index];
}

std::expected<Json const*, StarException> Json::ptr(String const& key) const {
  if (type() != Type::Object)
    return std::unexpected(JsonException::format("Cannot call get with key on Json type {}, must be Object type", typeName()));
  auto const& map = std::get<std::shared_ptr<JsonObject const>>(m_data);

  auto i = map->find(key);
  if (i == map->end())
    return nullptr;

  return &i->second;
}

Json jsonMerge(Json const& base, Json const& merger) {
  if (base.type() == Json::Type::Object && merger.type() == Json::Type::Object) {
    JsonObject merged = *std::get<std::shared_ptr<JsonObject const>>(base.m_data);
    for (auto const& p : *std::get<std::shared_ptr<JsonObject const>>(merger.m_data)) {
      auto res = merged.insert(p);
      if (!res.second)
        res.first->second = jsonMerge(res.first->second, p.second);
    }
    return merged;
  }
  return merger.type() == Json::Type::Null ? base : merger;
}

Json jsonMergeNulling(Json const& base, Json const& merger) {
  if (base.type() == Json::Type::Object && merger.type() == Json::Type::Object) {
    JsonObject merged = *std::get<std::shared_ptr<JsonObject const>>(base.m_data);
    for (auto const& p : *std::get<std::shared_ptr<JsonObject const>>(merger.m_data)) {
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

bool jsonPartialMatch(Json const& base, Json const& compare) {
  if (base == compare) {
    return true;
  } else {
    if (base.type() == Json::Type::Object && compare.type() == Json::Type::Object) {
      for (auto const& c : *std::get<std::shared_ptr<JsonObject const>>(compare.m_data)) {
        auto const& object = *std::get<std::shared_ptr<JsonObject const>>(base.m_data);
        auto entry = object.find(c.first);
        if (entry == object.end() || !jsonPartialMatch(entry->second, c.second))
          return false;
      }
      return true;
    }
    if (base.type() == Json::Type::Array && compare.type() == Json::Type::Array) {
      for (auto const& c : *std::get<std::shared_ptr<JsonArray const>>(compare.m_data)) {
        bool similar = false;
        for (auto const& b : *std::get<std::shared_ptr<JsonArray const>>(base.m_data)) {
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

}