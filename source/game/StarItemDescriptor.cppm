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

class ItemDescriptor;
using ItemDescriptorPtr = std::shared_ptr<ItemDescriptor>;
using ItemDescriptorConstPtr = std::shared_ptr<const ItemDescriptor>;
using ItemDescriptorWeakPtr = std::weak_ptr<ItemDescriptor>;
using ItemDescriptorConstWeakPtr = std::weak_ptr<const ItemDescriptor>;
using ItemDescriptorUPtr = std::unique_ptr<ItemDescriptor>;
using ItemDescriptorConstUPtr = std::unique_ptr<const ItemDescriptor>;
class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<const Item>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<const Item>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<const Item>;
struct VersionedJson;
using VersionedJsonPtr = std::shared_ptr<VersionedJson>;
using VersionedJsonConstPtr = std::shared_ptr<const VersionedJson>;
using VersionedJsonWeakPtr = std::weak_ptr<VersionedJson>;
using VersionedJsonConstWeakPtr = std::weak_ptr<const VersionedJson>;
using VersionedJsonUPtr = std::unique_ptr<VersionedJson>;
using VersionedJsonConstUPtr = std::unique_ptr<const VersionedJson>;

class ItemDescriptor {
public:
  // Loads ItemDescriptor from store format.
  static ItemDescriptor loadStore(Json const& store);

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
  bool matches(ItemConstPtr const& other, bool exactMatch = false) const;

  // Stores ItemDescriptor to versioned structure not meant for human reading / writing.
  Json diskStore() const;

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
  using ::Star::ItemDescriptorPtr;
  using ::Star::ItemDescriptorConstPtr;
  using ::Star::ItemDescriptorWeakPtr;
  using ::Star::ItemDescriptorConstWeakPtr;
  using ::Star::ItemDescriptorUPtr;
  using ::Star::ItemDescriptorConstUPtr;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::VersionedJson;
  using ::Star::VersionedJsonPtr;
  using ::Star::VersionedJsonConstPtr;
  using ::Star::VersionedJsonWeakPtr;
  using ::Star::VersionedJsonConstWeakPtr;
  using ::Star::VersionedJsonUPtr;
  using ::Star::VersionedJsonConstUPtr;
}
