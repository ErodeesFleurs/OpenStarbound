module;
import std;
import star.stream_formatter;
import star.hash;
import star.xx_hash;

import star.data_stream;
import star.string;
import star.list;
import star.format;

import star.algorithm;
namespace Star {

struct JsonExceptionTag {
  static constexpr char const* name() { return "JsonException"; }
};
using JsonException = StarError<JsonExceptionTag, StarException>;
struct JsonParsingExceptionTag {
  static constexpr char const* name() { return "JsonParsingException"; }
};
using JsonParsingException = StarError<JsonParsingExceptionTag, StarException>;

class Json;

typedef List<Json> JsonArray;

typedef StringMap<Json> JsonObject;

// Class for holding representation of JSON data.  Immutable and implicitly
// shared.
class Json {
public:
  template <typename Container>
  struct IteratorWrapper {
    typedef typename Container::const_iterator const_iterator;
    typedef const_iterator iterator;

    const_iterator begin() const;
    const_iterator end() const;

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

  static String typeName(Type t);
  static Type typeFromName(String const& t);

  static Json ofType(Type t);

  // Parses JSON or JSON sub-type
  static Json parse(String const& string);

  // Parses JSON sequence
  static Json parseSequence(String const& sequence);

  // Parses JSON object or array only (the only top level types allowed by
  // JSON)
  static Json parseJson(String const& json);

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

  double toDouble() const;
  float toFloat() const;
  bool toBool() const;
  std::int64_t toInt() const;
  std::uint64_t toUInt() const;
  String toString() const;
  JsonArray toArray() const;
  JsonObject toObject() const;

  // Internally, String, JsonArray, and JsonObject are shared via shared_ptr
  // since this class is immutable.  Use these methods to get at this pointer
  // without causing a copy.
  std::shared_ptr<String const> stringPtr() const;
  std::shared_ptr<JsonArray const> arrayPtr() const;
  std::shared_ptr<JsonObject const> objectPtr() const;

  // As a convenience, make it easy to safely and quickly iterate over a
  // JsonArray or JsonObject contents by holding the container pointer.
  IteratorWrapper<JsonArray> iterateArray() const;
  IteratorWrapper<JsonObject> iterateObject() const;

  // opt* methods work like this, if the json is null, it returns none.  If the
  // json is convertible, it returns the converted type, otherwise an exception
  // occurrs.
  std::optional<Json> opt() const;
  std::optional<double> optDouble() const;
  std::optional<float> optFloat() const;
  std::optional<bool> optBool() const;
  std::optional<std::int64_t> optInt() const;
  std::optional<std::uint64_t> optUInt() const;
  std::optional<String> optString() const;
  std::optional<JsonArray> optArray() const;
  std::optional<JsonObject> optObject() const;

  // Size of array / object type json
  std::size_t size() const;

  // If this json is array type, get the value at the given index
  Json get(std::size_t index) const;
  double getDouble(std::size_t index) const;
  float getFloat(std::size_t index) const;
  bool getBool(std::size_t index) const;
  std::int64_t getInt(std::size_t index) const;
  std::uint64_t getUInt(std::size_t index) const;
  String getString(std::size_t index) const;
  JsonArray getArray(std::size_t index) const;
  JsonObject getObject(std::size_t index) const;

  // These versions of get* return default value if the index is out of range,
  // or if the value pointed to is null.
  Json get(std::size_t index, Json def) const;
  double getDouble(std::size_t index, double def) const;
  float getFloat(std::size_t index, float def) const;
  bool getBool(std::size_t index, bool def) const;
  std::int64_t getInt(std::size_t index, std::int64_t def) const;
  std::uint64_t getUInt(std::size_t index, std::int64_t def) const;
  String getString(std::size_t index, String def) const;
  JsonArray getArray(std::size_t index, JsonArray def) const;
  JsonObject getObject(std::size_t index, JsonObject def) const;

  // If object type, whether object contains key
  bool contains(String const& key) const;

  // If this json is object type, get the value for the given key
  Json get(String const& key) const;
  double getDouble(String const& key) const;
  float getFloat(String const& key) const;
  bool getBool(String const& key) const;
  std::int64_t getInt(String const& key) const;
  std::uint64_t getUInt(String const& key) const;
  String getString(String const& key) const;
  JsonArray getArray(String const& key) const;
  JsonObject getObject(String const& key) const;

  // These versions of get* return the default if the key is missing or the
  // value is null.
  Json get(String const& key, Json def) const;
  double getDouble(String const& key, double def) const;
  float getFloat(String const& key, float def) const;
  bool getBool(String const& key, bool def) const;
  std::int64_t getInt(String const& key, std::int64_t def) const;
  std::uint64_t getUInt(String const& key, std::int64_t def) const;
  String getString(String const& key, String def) const;
  JsonArray getArray(String const& key, JsonArray def) const;
  JsonObject getObject(String const& key, JsonObject def) const;

  // Works the same way as opt methods above.  Will never return a null value,
  // if there is a null entry it will just return an empty std::optional.
  std::optional<Json> opt(String const& key) const;
  std::optional<double> optDouble(String const& key) const;
  std::optional<float> optFloat(String const& key) const;
  std::optional<bool> optBool(String const& key) const;
  std::optional<std::int64_t> optInt(String const& key) const;
  std::optional<std::uint64_t> optUInt(String const& key) const;
  std::optional<String> optString(String const& key) const;
  std::optional<JsonArray> optArray(String const& key) const;
  std::optional<JsonObject> optObject(String const& key) const;

  // Combines gets recursively in friendly expressions.  For
  // example, call like this: json.query("path.to.array[3][4]")
  Json query(String const& path) const;
  double queryDouble(String const& path) const;
  float queryFloat(String const& path) const;
  bool queryBool(String const& path) const;
  std::int64_t queryInt(String const& path) const;
  std::uint64_t queryUInt(String const& path) const;
  String queryString(String const& path) const;
  JsonArray queryArray(String const& path) const;
  JsonObject queryObject(String const& path) const;

  // These versions of get* do not throw on missing / null keys anywhere in the
  // query path.
  Json query(String const& path, Json def) const;
  double queryDouble(String const& path, double def) const;
  float queryFloat(String const& path, float def) const;
  bool queryBool(String const& path, bool def) const;
  std::int64_t queryInt(String const& path, std::int64_t def) const;
  std::uint64_t queryUInt(String const& path, std::uint64_t def) const;
  String queryString(String const& path, String const& def) const;
  JsonArray queryArray(String const& path, JsonArray def) const;
  JsonObject queryObject(String const& path, JsonObject def) const;

  // Returns none on on missing / null keys anywhere in the query path.  Will
  // never return a null value, just an empty std::optional.
  std::optional<Json> optQuery(String const& path) const;
  std::optional<double> optQueryDouble(String const& path) const;
  std::optional<float> optQueryFloat(String const& path) const;
  std::optional<bool> optQueryBool(String const& path) const;
  std::optional<std::int64_t> optQueryInt(String const& path) const;
  std::optional<std::uint64_t> optQueryUInt(String const& path) const;
  std::optional<String> optQueryString(String const& path) const;
  std::optional<JsonArray> optQueryArray(String const& path) const;
  std::optional<JsonObject> optQueryObject(String const& path) const;

  // Returns a *new* object with the given values set/erased.  Throws if not an
  // object.
  Json set(String key, Json value) const;
  Json setPath(String path, Json value) const;
  Json setAll(JsonObject values) const;
  Json eraseKey(String key) const;
  Json erasePath(String path) const;

  // Returns a *new* array with the given values set/inserted/appended/erased.
  // Throws if not an array.
  Json set(std::size_t index, Json value) const;
  Json insert(std::size_t index, Json value) const;
  Json append(Json value) const;
  Json eraseIndex(std::size_t index) const;

  Type type() const;
  String typeName() const;
  Json convert(Type u) const;

  bool isType(Type type) const;
  bool canConvert(Type type) const;

  // isNull returns true when the type of the Json is null.  operator bool() is
  // the opposite of isNull().
  bool isNull() const;
  explicit operator bool() const;

  // Prints JSON or JSON sub-type.  If sort is true, then any object anywhere
  // inside this value will be sorted alphanumerically before being written,
  // resulting in a known *unique* textual representation of the Json that is
  // cross-platform.
  String repr(int pretty = 0, bool sort = false) const;
  // Prints JSON object or array only (only top level types allowed by JSON)
  String printJson(int pretty = 0, bool sort = false) const;
  // Same but avoids quotation marks if this is a string
  String printString() const;

  // operator== and operator!= compare for exact equality with all types, and
  // additionally equality with numeric conversion with Int <-> Float
  bool operator==(Json const& v) const;
  bool operator!=(Json const& v) const;

  // Does this Json not share its storage with any other Json?
  bool unique() const;

  void getHash(XXHash3& hasher) const;

private:
  Json const* ptr(std::size_t index) const;
  Json const* ptr(String const& key) const;

  std::variant<Empty, double, bool, std::int64_t, std::shared_ptr<String const>, std::shared_ptr<JsonArray const>, std::shared_ptr<JsonObject const>> m_data;
};

std::ostream& operator<<(std::ostream& os, Json const& v);

// Fixes ambiguity with OrderedHashMap operator<<
std::ostream& operator<<(std::ostream& os, JsonObject const& v);

// Serialize json to DataStream.  Strings are stored as UTF-8, ints are stored
// as VLQ, doubles as 64 bit.
DataStream& operator<<(DataStream& ds, Json const& v);
DataStream& operator>>(DataStream& ds, Json& v);

// Convenience methods for Json containers
DataStream& operator<<(DataStream& ds, JsonArray const& l);
DataStream& operator>>(DataStream& ds, JsonArray& l);
DataStream& operator<<(DataStream& ds, JsonObject const& m);
DataStream& operator>>(DataStream& ds, JsonObject& m);

// Merges the two given json values and returns the result, by the following
// rules (applied in order):  If the base value is null, returns the merger.
// If the merger value is null, returns base.  For any two non-objects types,
// returns the merger.  If both values are objects, then the resulting object
// is the combination of both objects, but for each repeated key jsonMerge is
// called recursively on both values to determine the result.
Json jsonMerge(Json const& base, Json const& merger);
// Same as above, but applies null mergers.
Json jsonMergeNulling(Json const& base, Json const& merger);

template <typename... T>
Json jsonMerge(Json const& base, Json const& merger, T const&... rest);

// Similar to jsonMerge, but query only for a single key.  Gets a value equal
// to jsonMerge(jsons...).query(key, Json()), but much faster than doing an
// entire merge operation.
template <typename... T>
Json jsonMergeQuery(String const& key, Json const& first, T const&... rest);

// jsonMergeQuery with a default.
template <typename... T>
Json jsonMergeQueryDef(String const& key, Json def, Json const& first, T const&... rest);

template <>
struct hash<Json> {
  std::size_t operator()(Json const& v) const;
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
Json jsonMerge(Json const& base, Json const& merger, T const&... rest) {
  return jsonMerge(jsonMerge(base, merger), rest...);
}

template <typename... T>
Json jsonMergeQuery(String const&, Json def) {
  return def;
}

template <typename... T>
Json jsonMergeQueryImpl(String const& key, Json const& json) {
  return json.query(key, {});
}

template <typename... T>
Json jsonMergeQueryImpl(String const& key, Json const& base, Json const& first, T const&... rest) {
  Json value = jsonMergeQueryImpl(key, first, rest...);
  if (value && !value.isType(Json::Type::Object))
    return value;
  return jsonMerge(base.query(key, {}), value);
}

template <typename... T>
Json jsonMergeQuery(String const& key, Json const& first, T const&... rest) {
  return jsonMergeQueryImpl(key, first, rest...);
}

template <typename... T>
Json jsonMergeQueryDef(String const& key, Json def, Json const& first, T const&... rest) {
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
bool jsonPartialMatch(Json const& base, Json const& compare);

}

export module star.json;

export namespace Star {
  template <>
  struct hash<Json>;
}


export namespace Star {
  using ::Star::JsonExceptionTag;
  using ::Star::JsonException;
  using ::Star::JsonParsingExceptionTag;
  using ::Star::JsonParsingException;
  using ::Star::Json;
  using ::Star::JsonArray;
  using ::Star::JsonObject;
  using ::Star::jsonMerge;
  using ::Star::jsonMergeNulling;
  using ::Star::jsonMergeQuery;
  using ::Star::jsonMergeQueryImpl;
  using ::Star::jsonMergeQueryDef;
  using ::Star::jsonPartialMatch;
  using ::Star::hash;
  using ::Star::operator<<;
  using ::Star::operator>>;
}

export template <> struct std::formatter<Star::Json> : Star::StreamFormatter {};
export template <> struct std::formatter<Star::JsonObject> : Star::StreamFormatter {};
