#pragma once

#include "StarBiMap.hpp"
#include "StarJson.hpp"

namespace Star {
class DataStream;
class VersioningDatabase;
using VersioningDatabaseConstPtr = SharedPtr<VersioningDatabase const>;

class Item;
using ItemConstPtr = SharedPtr<Item const>;

class ItemDescriptor {
public:
  // Loads ItemDescriptor from store format.
  [[nodiscard]] static ItemDescriptor loadStore(Json const& store, VersioningDatabaseConstPtr versioningDatabase);

  ItemDescriptor() = default;
  ItemDescriptor(String name, uint64_t count, Json parameters = Json());

  // Populate from a configuration JsonArray containing up to 3 elements, the
  // name, count, and then any item parameters.  If the json is a map, looks
  // for keys 'name', 'parameters', and 'count'.
  explicit ItemDescriptor(Json const& spec);

  [[nodiscard]] String const& name() const;
  [[nodiscard]] uint64_t count() const;
  [[nodiscard]] Json const& parameters() const;

  [[nodiscard]] ItemDescriptor singular() const;
  [[nodiscard]] ItemDescriptor withCount(uint64_t count) const;
  [[nodiscard]] ItemDescriptor multiply(uint64_t count) const;
  [[nodiscard]] ItemDescriptor applyParameters(JsonObject const& parameters) const;

  // Descriptor is the default constructed ItemDescriptor()
  [[nodiscard]] bool isNull() const;

  // Descriptor is not null
  [[nodiscard]] explicit operator bool() const;

  // True if descriptor is null OR if descriptor is size 0
  [[nodiscard]] bool isEmpty() const;

  [[nodiscard]] bool operator==(ItemDescriptor const& rhs) const;
  [[nodiscard]] bool operator!=(ItemDescriptor const& rhs) const;

  [[nodiscard]] bool matches(ItemDescriptor const& other, bool exactMatch = false) const;
  [[nodiscard]] bool matches(ItemConstPtr const& other, bool exactMatch = false) const;

  // Stores ItemDescriptor to versioned structure not meant for human reading / writing.
  [[nodiscard]] Json diskStore(VersioningDatabaseConstPtr versioningDatabase) const;

  // Converts ItemDescriptor to spec format
  [[nodiscard]] Json toJson() const;

  friend DataStream& operator>>(DataStream& ds, ItemDescriptor& itemDescriptor);
  friend DataStream& operator<<(DataStream& ds, ItemDescriptor const& itemDescriptor);

  friend std::ostream& operator<<(std::ostream& os, ItemDescriptor const& descriptor);

  friend struct hash<ItemDescriptor>;

private:
  ItemDescriptor(String name, uint64_t count, Json parameters, Maybe<size_t> parametersHash);

  [[nodiscard]] size_t parametersHash() const;

  String m_name;
  uint64_t m_count = 0;
  Json m_parameters = JsonObject();
  mutable Maybe<size_t> m_parametersHash;
};

template <>
struct hash<ItemDescriptor> {
  [[nodiscard]] size_t operator()(ItemDescriptor const& v) const;
};

}// namespace Star

template <>
struct std::formatter<Star::ItemDescriptor> : Star::OstreamFormatter {};
