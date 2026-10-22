module;
import star.stream_formatter;

import std;

import star.flat_hash_map;
import star.list;
import star.optional;
import star.hash;
import star.format;

import star.output_proxy;
import star.algorithm;
namespace Star {

struct MapExceptionTag {
  static constexpr char const* name() { return "MapException"; }
};
using MapException = StarError<MapExceptionTag, StarException>;

template <typename T>
[[nodiscard]] std::expected<std::string, StarException> mapDiagnosticValue(T const& value) {
  if constexpr (std::is_enum_v<T>) {
    return strf("{}", std::to_underlying(value));
  } else {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    using PrintResult = decltype(stream << outputAny(value));
    if constexpr (std::same_as<PrintResult, std::expected<std::ostream*, StarException>>) {
      auto printed = stream << outputAny(value);
      if (!printed)
        return std::unexpected(std::move(printed.error()));
    } else {
      stream << outputAny(value);
    }
    return std::move(stream).str();
  }
}

template <typename Error = MapException, typename T>
[[nodiscard]] StarException mapDiagnosticError(std::format_string<std::string> message,
    std::string_view context, T const& value) {
  auto rendered = mapDiagnosticValue(value);
  if (!rendered)
    return std::move(rendered.error()).withContext(std::string(context));
  return Error(strf(message, std::move(*rendered)));
}

template <typename BaseMap>
class MapMixin : public BaseMap {
public:
  typedef BaseMap Base;

  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;

  typedef typename Base::key_type key_type;
  typedef typename Base::mapped_type mapped_type;
  typedef typename Base::value_type value_type;

  typedef typename std::decay<mapped_type>::type* mapped_ptr;
  typedef typename std::decay<mapped_type>::type const* mapped_const_ptr;

  template <typename MapType>
  static MapMixin from(MapType const& m);

  using Base::Base;

  List<key_type> keys() const;
  List<mapped_type> values() const;
  List<std::pair<key_type, mapped_type>> pairs() const;

  bool contains(key_type const& k) const;

  // Removes the item with key k and returns true if contains(k) is true,
  // false otherwise.
  bool remove(key_type const& k);

  // Removes *all* items that have a value matching the given one.  Returns
  // true if any elements were removed.
  bool removeValues(mapped_type const& v);

  // Returns an error if the key is not found.
  [[nodiscard]] std::expected<mapped_type, StarException> take(key_type const& k);

  std::optional<mapped_type> maybeTake(key_type const& k);

  // Returns an error if the key is not found.
  [[nodiscard]] std::expected<mapped_type*, StarException> get(key_type const& k);
  [[nodiscard]] std::expected<mapped_type const*, StarException> get(key_type const& k) const;
  [[nodiscard]] std::expected<mapped_type*, StarException> at(key_type const& k) { return get(k); }
  [[nodiscard]] std::expected<mapped_type const*, StarException> at(key_type const& k) const { return get(k); }

  // Return d if key not found
  mapped_type value(key_type const& k, mapped_type d = mapped_type()) const;

  std::optional<mapped_type> maybe(key_type const& k) const;

  mapped_const_ptr ptr(key_type const& k) const;
  mapped_ptr ptr(key_type const& k);

  // Finds first value matching the given value and returns its key.
  [[nodiscard]] std::expected<key_type, StarException> keyOf(mapped_type const& v) const;

  // Finds all of the values matching the given value and returns their keys.
  List<key_type> keysOf(mapped_type const& v) const;

  bool hasValue(mapped_type const& v) const;

  using Base::insert;
  using Base::try_emplace;
  using Base::insert_or_assign;

  template <typename LookupKey>
    requires requires(Base const& base, LookupKey const& key) { base.find(key); }
  bool contains(LookupKey const& key) const { return Base::find(key) != Base::end(); }

  template <typename LookupKey>
    requires requires(Base& base, LookupKey const& key) { base.find(key); }
  [[nodiscard]] std::expected<mapped_type*, StarException> get(LookupKey const& key) {
    auto position = Base::find(key);
    if (position == Base::end())
      return std::unexpected(mapDiagnosticError("Key '{}' not found in Map::get()", "Map::get: key not found while rendering its diagnostic", key));
    return &position->second;
  }

  template <typename LookupKey>
    requires requires(Base const& base, LookupKey const& key) { base.find(key); }
  [[nodiscard]] std::expected<mapped_type const*, StarException> get(LookupKey const& key) const {
    auto position = Base::find(key);
    if (position == Base::end())
      return std::unexpected(mapDiagnosticError("Key '{}' not found in Map::get()", "Map::get: key not found while rendering its diagnostic", key));
    return &position->second;
  }

  template <typename LookupKey>
    requires requires(Base const& base, LookupKey const& key) { base.find(key); }
  mapped_type value(LookupKey const& key, mapped_type fallback = mapped_type()) const {
    auto position = Base::find(key);
    return position == Base::end() ? std::move(fallback) : position->second;
  }

  template <typename LookupKey>
    requires requires(Base const& base, LookupKey const& key) { base.find(key); }
  std::optional<mapped_type> maybe(LookupKey const& key) const {
    auto position = Base::find(key);
    if (position == Base::end())
      return {};
    return std::optional<mapped_type>(std::in_place, position->second);
  }

  template <typename LookupKey>
    requires requires(Base const& base, LookupKey const& key) { base.find(key); }
  mapped_const_ptr ptr(LookupKey const& key) const {
    auto position = Base::find(key);
    return position == Base::end() ? nullptr : &position->second;
  }

  template <typename LookupKey>
    requires requires(Base& base, LookupKey const& key) { base.find(key); }
  mapped_ptr ptr(LookupKey const& key) {
    auto position = Base::find(key);
    return position == Base::end() ? nullptr : &position->second;
  }

  // Same as insert(value_type), returns the iterator to either the newly
  // inserted value or the existing value, and then a bool that is true if the
  // new element was inserted.
  std::pair<iterator, bool> insert(key_type k, mapped_type v);

  // Add a key / value pair, returning an error if the key already exists.
  [[nodiscard]] std::expected<mapped_type*, StarException> add(key_type k, mapped_type v);

  // Set a key to a value, always override if it already exists
  mapped_type& set(key_type k, mapped_type v);

  // Appends all values of given map into this map.  If overwite is false, then
  // skips values that already exist in this map.  Returns false if any keys
  // previously existed.
  template <typename MapType>
  bool merge(MapType const& m, bool overwrite = false);

  bool operator==(MapMixin const& m) const;
};

template <typename BaseMap>
std::ostream& operator<<(std::ostream& os, MapMixin<BaseMap> const& m);

template <typename Key, typename Value, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<Key const, Value>>>
using Map = MapMixin<std::map<Key, Value, Compare, Allocator>>;

template <typename Key, typename Value, typename Hash = hash<Key>, typename Equals = std::equal_to<>, typename Allocator = std::allocator<std::pair<Key const, Value>>>
using HashMap = MapMixin<FlatHashMap<Key, Value, Hash, Equals, Allocator>>;

template <typename Key, typename Value, typename Hash = hash<Key>, typename Equals = std::equal_to<>, typename Allocator = std::allocator<std::pair<Key const, Value>>>
using StableHashMap = MapMixin<std::unordered_map<Key, Value, Hash, Equals, Allocator>>;

template <typename BaseMap>
template <typename MapType>
auto MapMixin<BaseMap>::from(MapType const& m) -> MapMixin {
  return MapMixin(m.begin(), m.end());
}

template <typename BaseMap>
auto MapMixin<BaseMap>::keys() const -> List<key_type> {
  List<key_type> klist;
  for (const_iterator i = Base::begin(); i != Base::end(); ++i)
    klist.push_back(i->first);
  return klist;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::values() const -> List<mapped_type> {
  List<mapped_type> vlist;
  for (const_iterator i = Base::begin(); i != Base::end(); ++i)
    vlist.push_back(i->second);
  return vlist;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::pairs() const -> List<std::pair<key_type, mapped_type>> {
  List<std::pair<key_type, mapped_type>> plist;
  for (const_iterator i = Base::begin(); i != Base::end(); ++i)
    plist.push_back(*i);
  return plist;
}

template <typename BaseMap>
bool MapMixin<BaseMap>::contains(key_type const& k) const {
  return Base::find(k) != Base::end();
}

template <typename BaseMap>
bool MapMixin<BaseMap>::remove(key_type const& k) {
  return Base::erase(k) != 0;
}

template <typename BaseMap>
bool MapMixin<BaseMap>::removeValues(mapped_type const& v) {
  bool removed = false;
  const_iterator i = Base::begin();
  while (i != Base::end()) {
    if (i->second == v) {
      i = Base::erase(i);
      removed = true;
    } else {
      ++i;
    }
  }
  return removed;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::take(key_type const& k) -> std::expected<mapped_type, StarException> {
  if (auto v = maybeTake(k))
    return std::move(*v);
  return std::unexpected(mapDiagnosticError("Key '{}' not found in Map::take()", "Map::take: key not found while rendering its diagnostic", k));
}

template <typename BaseMap>
auto MapMixin<BaseMap>::maybeTake(key_type const& k) -> std::optional<mapped_type> {
  const_iterator i = Base::find(k);
  if (i != Base::end()) {
    mapped_type v = std::move(i->second);
    Base::erase(i);
    return std::optional<mapped_type>(std::in_place, std::move(v));
  }

  return {};
}

template <typename BaseMap>
auto MapMixin<BaseMap>::get(key_type const& k) -> std::expected<mapped_type*, StarException> {
  iterator i = Base::find(k);
  if (i == Base::end())
    return std::unexpected(mapDiagnosticError("Key '{}' not found in Map::get()", "Map::get: key not found while rendering its diagnostic", k));
  return &i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::get(key_type const& k) const -> std::expected<mapped_type const*, StarException> {
  const_iterator i = Base::find(k);
  if (i == Base::end())
    return std::unexpected(mapDiagnosticError("Key '{}' not found in Map::get()", "Map::get: key not found while rendering its diagnostic", k));
  return &i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::value(key_type const& k, mapped_type d) const -> mapped_type {
  const_iterator i = Base::find(k);
  if (i == Base::end())
    return d;
  else
    return i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::maybe(key_type const& k) const -> std::optional<mapped_type> {
  auto i = Base::find(k);
  if (i == Base::end())
    return {};
  else
    return std::optional<mapped_type>(std::in_place, i->second);
}

template <typename BaseMap>
auto MapMixin<BaseMap>::ptr(key_type const& k) const -> mapped_const_ptr {
  auto i = Base::find(k);
  if (i == Base::end())
    return nullptr;
  else
    return &i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::ptr(key_type const& k) -> mapped_ptr {
  auto i = Base::find(k);
  if (i == Base::end())
    return nullptr;
  else
    return &i->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::keyOf(mapped_type const& v) const -> std::expected<key_type, StarException> {
  for (const_iterator i = Base::begin(); i != Base::end(); ++i) {
    if (i->second == v)
      return i->first;
  }
  return std::unexpected(mapDiagnosticError("Value '{}' not found in Map::keyOf()", "Map::keyOf: value not found while rendering its diagnostic", v));
}

template <typename BaseMap>
auto MapMixin<BaseMap>::keysOf(mapped_type const& v) const -> List<key_type> {
  List<key_type> keys;
  for (const_iterator i = Base::begin(); i != Base::end(); ++i) {
    if (i->second == v)
      keys.append(i->first);
  }
  return keys;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::hasValue(mapped_type const& v) const -> bool {
  for (const_iterator i = Base::begin(); i != Base::end(); ++i) {
    if (i->second == v)
      return true;
  }
  return false;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::insert(key_type k, mapped_type v) -> std::pair<iterator, bool> {
  return Base::try_emplace(std::move(k), std::move(v));
}

template <typename BaseMap>
auto MapMixin<BaseMap>::add(key_type k, mapped_type v) -> std::expected<mapped_type*, StarException> {
  auto pair = Base::try_emplace(std::move(k), std::move(v));
  if (!pair.second)
    return std::unexpected(mapDiagnosticError("Entry with key '{}' already present.", "Map::add: key already present while rendering its diagnostic", pair.first->first));
  return &pair.first->second;
}

template <typename BaseMap>
auto MapMixin<BaseMap>::set(key_type k, mapped_type v) -> mapped_type& {
  return Base::insert_or_assign(std::move(k), std::move(v)).first->second;
}

template <typename BaseMap>
template <typename OtherMapType>
bool MapMixin<BaseMap>::merge(OtherMapType const& m, bool overwrite) {
  return mapMerge(*this, m, overwrite);
}

template <typename BaseMap>
bool MapMixin<BaseMap>::operator==(MapMixin const& m) const {
  return this == &m || mapsEqual(*this, m);
}

template <typename MapType>
void printMap(std::ostream& os, MapType const& m) {
  os << "{ ";
  for (auto i = m.begin(); i != m.end(); ++i) {
    if (m.begin() == i)
      os << "\"";
    else
      os << ", \"";
    if constexpr (requires { outputOptional(i->first); })
      os << outputOptional(i->first);
    else
      os << i->first;
    os << "\" : \"";
    if constexpr (requires { outputOptional(i->second); })
      os << outputOptional(i->second);
    else
      os << i->second;
    os << "\"";
  }
  os << " }";
}

template <typename BaseMap>
std::ostream& operator<<(std::ostream& os, MapMixin<BaseMap> const& m) {
  printMap(os, m);
  return os;
}

}

export module star.map;

export template <typename BaseMap>
struct std::formatter<Star::MapMixin<BaseMap>> : Star::StreamFormatter {};

export namespace Star {
  using ::Star::MapExceptionTag;
  using ::Star::MapException;
  using ::Star::mapDiagnosticValue;
  using ::Star::mapDiagnosticError;
  using ::Star::MapMixin;
  using ::Star::Map;
  using ::Star::HashMap;
  using ::Star::StableHashMap;
  using ::Star::printMap;
  using ::Star::operator<<;
}
