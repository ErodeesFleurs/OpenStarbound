module;
import std;
import star.string;
import star.maybe;
import star.stream_formatter;
import star.hash;
import star.json;

import star.data_stream;
import star.bi_map;

namespace Star {

class Item;
struct VersionedJson;
class VersioningDatabase;

class ItemDescriptor {
public:
  // Loads ItemDescriptor from store format.
  static ItemDescriptor loadStore(VersioningDatabase const& versioningDatabase, Json const& store);

  ItemDescriptor();
  ItemDescriptor(String name, std::uint64_t count, Json parameters = Json());

  // Populate from a configuration JsonArray containing up to 3 elements, the
  // name, count, and then any item parameters.  If the json is a map, looks
  // for keys 'name', 'parameters', and 'count'.
  explicit ItemDescriptor(Json const& spec);

  String const& name() const;
  std::uint64_t count() const;
  Json const& parameters() const;

  ItemDescriptor singular() const;
  ItemDescriptor withCount(std::uint64_t count) const;
  ItemDescriptor multiply(std::uint64_t count) const;
  ItemDescriptor applyParameters(JsonObject const& parameters) const;

  // Descriptor is the default constructed ItemDescriptor()
  bool isNull() const;

  // Descriptor is not null
  explicit operator bool() const;

  // True if descriptor is null OR if descriptor is size 0
  bool isEmpty() const;

  bool operator==(ItemDescriptor const& rhs) const;
  bool operator!=(ItemDescriptor const& rhs) const;

  bool matches(ItemDescriptor const& other, bool exactMatch = false) const;
  bool matches(std::shared_ptr<Item const> const& other, bool exactMatch = false) const;

  // Stores ItemDescriptor to versioned structure not meant for human reading / writing.
  Json diskStore(VersioningDatabase const& versioningDatabase) const;

  // Converts ItemDescriptor to spec format
  Json toJson() const;

  friend DataStream& operator>>(DataStream& ds, ItemDescriptor& itemDescriptor);
  friend DataStream& operator<<(DataStream& ds, ItemDescriptor const& itemDescriptor);

  friend std::ostream& operator<<(std::ostream& os, ItemDescriptor const& descriptor);

  friend struct hash<ItemDescriptor>;

private:
  ItemDescriptor(String name, std::uint64_t count, Json parameters, Maybe<std::size_t> parametersHash);

  std::size_t parametersHash() const;

  String m_name;
  std::uint64_t m_count;
  Json m_parameters;
  mutable Maybe<std::size_t> m_parametersHash;
};

template <>
struct hash<ItemDescriptor> {
  std::size_t operator()(ItemDescriptor const& v) const;
};

}

export module star.item_descriptor;

export namespace Star {
  template <>
  struct hash<ItemDescriptor>;
}


export template <> struct std::formatter<Star::ItemDescriptor> : Star::StreamFormatter {};

export namespace Star {
  using ::Star::ItemDescriptor;
  using ::Star::Item;
  using ::Star::VersionedJson;
}
