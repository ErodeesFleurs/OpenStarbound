#pragma once

#include "StarConfig.hpp"
#include "StarDataStreamExtra.hpp" // IWYU pragma: export
#include "StarException.hpp"
#include "StarString.hpp"
#include "StarVariant.hpp"

import std;
import star.data_stream;

namespace Star {

class Json;

using JsonException = ExceptionDerived<"JsonException">;
using JsonTypeException = ExceptionDerived<"JsonTypeException">;
using JsonParsingException = ExceptionDerived<"JsonParsingException", JsonException>;

using JsonArray = List<Json>;
using JsonObject = StringMap<Json>;
// Class for holding representation of JSON data.  Immutable and implicitly
// shared.
class Json {
public:
  template <typename Container>
  struct IteratorWrapper {
    using const_iterator = typename Container::const_iterator;
    using iterator = const_iterator;

    auto begin() const -> const_iterator;
    auto end() const -> const_iterator;

    std::shared_ptr<Container const> ptr;
  };

  enum class Type : std::uint8_t {
    Null = 0,
    Float = 1,
    Bool = 2,
    Int = 3,
    String = 4,
    Array = 5,
    Object = 6
  };

  static auto typeName(Type t) -> String;
  static auto typeFromName(String const& t) -> Type;

  static auto ofType(Type t) -> Json;

  // Parses JSON or JSON sub-type
  static auto parse(String const& string) -> Json;

  // Parses JSON sequence
  static auto parseSequence(String const& sequence) -> Json;

  // Parses JSON object or array only (the only top level types allowed by
  // JSON)
  static auto parseJson(String const& json) -> Json;

  // Constructs type Null
  Json();

  Json(double);
  Json(bool);
  Json(int);
  Json(long);
  Json(long long);
  Json(unsigned int);
  Json(unsigned long);
  Json(unsigned long long);
  Json(char const*);
  Json(String::Char const*);
  Json(String::Char const*, std::size_t);
  Json(String);
  Json(std::string);
  Json(JsonArray);
  Json(JsonObject);

  // Float and Int types are convertible between each other.  toDouble,
  // toFloat, toInt, toUInt may be called on either an Int or a Float.  For a
  // Float this is simply a C style cast from double, and for an Int it is
  // simply a C style cast from int64_t.
  //
  // Bools, Strings, Arrays, Objects, and Null are not automatically
  // convertible to any other type.

  [[nodiscard]] auto toDouble() const -> double;
  [[nodiscard]] auto toFloat() const -> float;
  [[nodiscard]] auto toBool() const -> bool;
  [[nodiscard]] auto toInt() const -> std::int64_t;
  [[nodiscard]] auto toUInt() const -> std::uint64_t;
  [[nodiscard]] auto toString() const -> String;
  [[nodiscard]] auto toArray() const -> JsonArray;
  [[nodiscard]] auto toObject() const -> JsonObject;

  // Internally, String, JsonArray, and JsonObject are shared via shared_ptr
  // since this class is immutable.  Use these methods to get at this pointer
  // without causing a copy.
  [[nodiscard]] auto stringPtr() const -> ConstPtr<String>;
  [[nodiscard]] auto arrayPtr() const -> ConstPtr<JsonArray>;
  [[nodiscard]] auto objectPtr() const -> ConstPtr<JsonObject>;

  // As a convenience, make it easy to safely and quickly iterate over a
  // JsonArray or JsonObject contents by holding the container pointer.
  [[nodiscard]] auto iterateArray() const -> IteratorWrapper<JsonArray>;
  [[nodiscard]] auto iterateObject() const -> IteratorWrapper<JsonObject>;

  // opt* methods work like this, if the json is null, it returns none.  If the
  // json is convertible, it returns the converted type, otherwise an exception
  // occurrs.
  [[nodiscard]] auto opt() const -> std::optional<Json>;
  [[nodiscard]] auto optDouble() const -> std::optional<double>;
  [[nodiscard]] auto optFloat() const -> std::optional<float>;
  [[nodiscard]] auto optBool() const -> std::optional<bool>;
  [[nodiscard]] auto optInt() const -> std::optional<std::int64_t>;
  [[nodiscard]] auto optUInt() const -> std::optional<std::uint64_t>;
  [[nodiscard]] auto optString() const -> std::optional<String>;
  [[nodiscard]] auto optArray() const -> std::optional<JsonArray>;
  [[nodiscard]] auto optObject() const -> std::optional<JsonObject>;

  // Size of array / object type json
  [[nodiscard]] auto size() const -> std::size_t;

  // If this json is array type, get the value at the given index
  [[nodiscard]] auto get(std::size_t index) const -> Json;
  [[nodiscard]] auto getDouble(std::size_t index) const -> double;
  [[nodiscard]] auto getFloat(std::size_t index) const -> float;
  [[nodiscard]] auto getBool(std::size_t index) const -> bool;
  [[nodiscard]] auto getInt(std::size_t index) const -> std::int64_t;
  [[nodiscard]] auto getUInt(std::size_t index) const -> std::uint64_t;
  [[nodiscard]] auto getString(std::size_t index) const -> String;
  [[nodiscard]] auto getArray(std::size_t index) const -> JsonArray;
  [[nodiscard]] auto getObject(std::size_t index) const -> JsonObject;

  // These versions of get* return default value if the index is out of range,
  // or if the value pointed to is null.
  [[nodiscard]] auto get(std::size_t index, Json def) const -> Json;
  [[nodiscard]] auto getDouble(std::size_t index, double def) const -> double;
  [[nodiscard]] auto getFloat(std::size_t index, float def) const -> float;
  [[nodiscard]] auto getBool(std::size_t index, bool def) const -> bool;
  [[nodiscard]] auto getInt(std::size_t index, std::int64_t def) const -> std::int64_t;
  [[nodiscard]] auto getUInt(std::size_t index, std::int64_t def) const -> std::uint64_t;
  [[nodiscard]] auto getString(std::size_t index, String def) const -> String;
  [[nodiscard]] auto getArray(std::size_t index, JsonArray def) const -> JsonArray;
  [[nodiscard]] auto getObject(std::size_t index, JsonObject def) const -> JsonObject;

  // If object type, whether object contains key
  [[nodiscard]] auto contains(String const& key) const -> bool;

  // If this json is object type, get the value for the given key
  [[nodiscard]] auto get(String const& key) const -> Json;
  [[nodiscard]] auto getDouble(String const& key) const -> double;
  [[nodiscard]] auto getFloat(String const& key) const -> float;
  [[nodiscard]] auto getBool(String const& key) const -> bool;
  [[nodiscard]] auto getInt(String const& key) const -> std::int64_t;
  [[nodiscard]] auto getUInt(String const& key) const -> std::uint64_t;
  [[nodiscard]] auto getString(String const& key) const -> String;
  [[nodiscard]] auto getArray(String const& key) const -> JsonArray;
  [[nodiscard]] auto getObject(String const& key) const -> JsonObject;

  // These versions of get* return the default if the key is missing or the
  // value is null.
  [[nodiscard]] auto get(String const& key, Json def) const -> Json;
  [[nodiscard]] auto getDouble(String const& key, double def) const -> double;
  [[nodiscard]] auto getFloat(String const& key, float def) const -> float;
  [[nodiscard]] auto getBool(String const& key, bool def) const -> bool;
  [[nodiscard]] auto getInt(String const& key, std::int64_t def) const -> std::int64_t;
  [[nodiscard]] auto getUInt(String const& key, std::int64_t def) const -> std::uint64_t;
  [[nodiscard]] auto getString(String const& key, String def) const -> String;
  [[nodiscard]] auto getArray(String const& key, JsonArray def) const -> JsonArray;
  [[nodiscard]] auto getObject(String const& key, JsonObject def) const -> JsonObject;

  // Works the same way as opt methods above.  Will never return a null value,
  // if there is a null entry it will just return an empty optional.
  [[nodiscard]] auto opt(String const& key) const -> std::optional<Json>;
  [[nodiscard]] auto optDouble(String const& key) const -> std::optional<double>;
  [[nodiscard]] auto optFloat(String const& key) const -> std::optional<float>;
  [[nodiscard]] auto optBool(String const& key) const -> std::optional<bool>;
  [[nodiscard]] auto optInt(String const& key) const -> std::optional<std::int64_t>;
  [[nodiscard]] auto optUInt(String const& key) const -> std::optional<std::uint64_t>;
  [[nodiscard]] auto optString(String const& key) const -> std::optional<String>;
  [[nodiscard]] auto optArray(String const& key) const -> std::optional<JsonArray>;
  [[nodiscard]] auto optObject(String const& key) const -> std::optional<JsonObject>;

  // Combines gets recursively in friendly expressions.  For
  // example, call like this: json.query("path.to.array[3][4]")
  [[nodiscard]] auto query(String const& path) const -> Json;
  [[nodiscard]] auto queryDouble(String const& path) const -> double;
  [[nodiscard]] auto queryFloat(String const& path) const -> float;
  [[nodiscard]] auto queryBool(String const& path) const -> bool;
  [[nodiscard]] auto queryInt(String const& path) const -> std::int64_t;
  [[nodiscard]] auto queryUInt(String const& path) const -> std::uint64_t;
  [[nodiscard]] auto queryString(String const& path) const -> String;
  [[nodiscard]] auto queryArray(String const& path) const -> JsonArray;
  [[nodiscard]] auto queryObject(String const& path) const -> JsonObject;

  // These versions of get* do not throw on missing / null keys anywhere in the
  // query path.
  [[nodiscard]] auto query(String const& path, Json def) const -> Json;
  [[nodiscard]] auto queryDouble(String const& path, double def) const -> double;
  [[nodiscard]] auto queryFloat(String const& path, float def) const -> float;
  [[nodiscard]] auto queryBool(String const& path, bool def) const -> bool;
  [[nodiscard]] auto queryInt(String const& path, std::int64_t def) const -> std::int64_t;
  [[nodiscard]] auto queryUInt(String const& path, std::uint64_t def) const -> std::uint64_t;
  [[nodiscard]] auto queryString(String const& path, String const& def) const -> String;
  [[nodiscard]] auto queryArray(String const& path, JsonArray def) const -> JsonArray;
  [[nodiscard]] auto queryObject(String const& path, JsonObject def) const -> JsonObject;

  // Returns none on on missing / null keys anywhere in the query path.  Will
  // never return a null value, just an empty optional.
  [[nodiscard]] auto optQuery(String const& path) const -> std::optional<Json>;
  [[nodiscard]] auto optQueryDouble(String const& path) const -> std::optional<double>;
  [[nodiscard]] auto optQueryFloat(String const& path) const -> std::optional<float>;
  [[nodiscard]] auto optQueryBool(String const& path) const -> std::optional<bool>;
  [[nodiscard]] auto optQueryInt(String const& path) const -> std::optional<std::int64_t>;
  [[nodiscard]] auto optQueryUInt(String const& path) const -> std::optional<std::uint64_t>;
  [[nodiscard]] auto optQueryString(String const& path) const -> std::optional<String>;
  [[nodiscard]] auto optQueryArray(String const& path) const -> std::optional<JsonArray>;
  [[nodiscard]] auto optQueryObject(String const& path) const -> std::optional<JsonObject>;

  // Returns a *new* object with the given values set/erased.  Throws if not an
  // object.
  [[nodiscard]] auto set(String key, Json value) const -> Json;
  [[nodiscard]] auto setPath(String path, Json value) const -> Json;
  [[nodiscard]] auto setAll(JsonObject values) const -> Json;
  [[nodiscard]] auto eraseKey(String key) const -> Json;
  [[nodiscard]] auto erasePath(String path) const -> Json;

  // Returns a *new* array with the given values set/inserted/appended/erased.
  // Throws if not an array.
  [[nodiscard]] auto set(std::size_t index, Json value) const -> Json;
  [[nodiscard]] auto insert(std::size_t index, Json value) const -> Json;
  [[nodiscard]] auto append(Json value) const -> Json;
  [[nodiscard]] auto eraseIndex(std::size_t index) const -> Json;

  [[nodiscard]] auto type() const -> Type;
  [[nodiscard]] auto typeName() const -> String;
  [[nodiscard]] auto convert(Type u) const -> Json;

  [[nodiscard]] auto isType(Type type) const -> bool;
  [[nodiscard]] auto canConvert(Type type) const -> bool;

  // isNull returns true when the type of the Json is null.  operator bool() is
  // the opposite of isNull().
  [[nodiscard]] auto isNull() const -> bool;
  explicit operator bool() const;

  // Prints JSON or JSON sub-type.  If sort is true, then any object anywhere
  // inside this value will be sorted alphanumerically before being written,
  // resulting in a known *unique* textual representation of the Json that is
  // cross-platform.
  [[nodiscard]] auto repr(int pretty = 0, bool sort = false) const -> String;
  // Prints JSON object or array only (only top level types allowed by JSON)
  [[nodiscard]] auto printJson(int pretty = 0, bool sort = false) const -> String;
  // Same but avoids quotation marks if this is a string
  [[nodiscard]] auto printString() const -> String;

  // operator== and operator!= compare for exact equality with all types, and
  // additionally equality with numeric conversion with Int <-> Float
  auto operator==(Json const& v) const -> bool;
  auto operator!=(Json const& v) const -> bool;

  // Does this Json not share its storage with any other Json?
  [[nodiscard]] auto unique() const -> bool;

  auto getHash(std::size_t& seed) const -> void;

private:
  [[nodiscard]] auto ptr(std::size_t index) const -> Json const*;
  [[nodiscard]] auto ptr(String const& key) const -> Json const*;

  Variant<Empty, double, bool, std::int64_t, ConstPtr<String>, ConstPtr<JsonArray>, ConstPtr<JsonObject>> m_data;
};

auto operator<<(std::ostream& os, Json const& v) -> std::ostream&;

// Fixes ambiguity with OrderedHashMap operator<<
auto operator<<(std::ostream& os, JsonObject const& v) -> std::ostream&;

// Serialize json to DataStream.  Strings are stored as UTF-8, ints are stored
// as VLQ, doubles as 64 bit.
auto operator<<(data_stream& ds, Json const& v) -> data_stream&;
auto operator>>(data_stream& ds, Json& v) -> data_stream&;

// Convenience methods for Json containers
auto operator<<(data_stream& ds, JsonArray const& l) -> data_stream&;
auto operator>>(data_stream& ds, JsonArray& l) -> data_stream&;
auto operator<<(data_stream& ds, JsonObject const& m) -> data_stream&;
auto operator>>(data_stream& ds, JsonObject& m) -> data_stream&;

// Merges the two given json values and returns the result, by the following
// rules (applied in order):  If the base value is null, returns the merger.
// If the merger value is null, returns base.  For any two non-objects types,
// returns the merger.  If both values are objects, then the resulting object
// is the combination of both objects, but for each repeated key jsonMerge is
// called recursively on both values to determine the result.
auto jsonMerge(Json const& base, Json const& merger) -> Json;
// Same as above, but applies null mergers.
auto jsonMergeNulling(Json const& base, Json const& merger) -> Json;

template <typename... T>
auto jsonMerge(Json const& base, Json const& merger, T const&... rest) -> Json;

// Similar to jsonMerge, but query only for a single key.  Gets a value equal
// to jsonMerge(jsons...).query(key, Json()), but much faster than doing an
// entire merge operation.
template <typename... T>
auto jsonMergeQuery(String const& key, Json const& first, T const&... rest) -> Json;

// jsonMergeQuery with a default.
template <typename... T>
auto jsonMergeQueryDef(String const& key, Json def, Json const& first, T const&... rest) -> Json;

template <>
struct hash<Json> {
  auto operator()(Json const& v) const -> std::size_t;
};

template <typename Container>
auto Json::IteratorWrapper<Container>::begin() const -> const_iterator {
  return ptr->begin();
}

template <typename Container>
auto Json::IteratorWrapper<Container>::end() const -> const_iterator {
  return ptr->end();
}

template <typename... T>
auto jsonMerge(Json const& base, Json const& merger, T const&... rest) -> Json {
  return jsonMerge(jsonMerge(base, merger), rest...);
}

template <typename... T>
auto jsonMergeQuery(String const&, Json def) -> Json {
  return def;
}

template <typename... T>
auto jsonMergeQueryImpl(String const& key, Json const& json) -> Json {
  return json.query(key, {});
}

template <typename... T>
auto jsonMergeQueryImpl(String const& key, Json const& base, Json const& first, T const&... rest) -> Json {
  Json value = jsonMergeQueryImpl(key, first, rest...);
  if (value && !value.isType(Json::Type::Object))
    return value;
  return jsonMerge(base.query(key, {}), value);
}

template <typename... T>
auto jsonMergeQuery(String const& key, Json const& first, T const&... rest) -> Json {
  return jsonMergeQueryImpl(key, first, rest...);
}

template <typename... T>
auto jsonMergeQueryDef(String const& key, Json def, Json const& first, T const&... rest) -> Json {
  if (auto v = jsonMergeQueryImpl(key, first, rest...))
    return v;
  return def;
}

// Compares two JSON values to see if the second is a subset of the first.
// For objects, each key in the second object must exist in the first
// object and the values are recursively compared the same way. For arrays,
// each element in the second array must successfully compare with some
// element of the first array, regardless of order or duplication.
// For all other types, the values must be equal.
auto jsonPartialMatch(Json const& base, Json const& compare) -> bool;

}// namespace Star

template <>
struct std::formatter<Star::Json> : Star::ostream_formatter {};
template <>
struct std::formatter<Star::JsonObject> : Star::ostream_formatter {};
