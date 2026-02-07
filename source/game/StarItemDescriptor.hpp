#pragma once

#include "StarConfig.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

class Item;

class ItemDescriptor {
public:
  // Loads ItemDescriptor from store format.
  static auto loadStore(Json const& store) -> ItemDescriptor;

  ItemDescriptor();
  ItemDescriptor(String name, std::uint64_t count, Json parameters = Json());

  // Populate from a configuration JsonArray containing up to 3 elements, the
  // name, count, and then any item parameters.  If the json is a map, looks
  // for keys 'name', 'parameters', and 'count'.
  explicit ItemDescriptor(Json const& spec);

  auto name() const -> String const&;
  auto count() const -> std::uint64_t;
  auto parameters() const -> Json const&;

  auto singular() const -> ItemDescriptor;
  auto withCount(std::uint64_t count) const -> ItemDescriptor;
  auto multiply(std::uint64_t count) const -> ItemDescriptor;
  auto applyParameters(JsonObject const& parameters) const -> ItemDescriptor;

  // Descriptor is the default constructed ItemDescriptor()
  auto isNull() const -> bool;

  // Descriptor is not null
  explicit operator bool() const;

  // True if descriptor is null OR if descriptor is size 0
  auto isEmpty() const -> bool;

  auto operator==(ItemDescriptor const& rhs) const -> bool;
  auto operator!=(ItemDescriptor const& rhs) const -> bool;

  auto matches(ItemDescriptor const& other, bool exactMatch = false) const -> bool;
  auto matches(ConstPtr<Item> const& other, bool exactMatch = false) const -> bool;

  // Stores ItemDescriptor to versioned structure not meant for human reading / writing.
  auto diskStore() const -> Json;

  // Converts ItemDescriptor to spec format
  auto toJson() const -> Json;

  friend auto operator>>(DataStream& ds, ItemDescriptor& itemDescriptor) -> DataStream&;
  friend auto operator<<(DataStream& ds, ItemDescriptor const& itemDescriptor) -> DataStream&;

  friend auto operator<<(std::ostream& os, ItemDescriptor const& descriptor) -> std::ostream&;

  friend struct hash<ItemDescriptor>;

private:
  ItemDescriptor(String name, std::uint64_t count, Json parameters, std::optional<size_t> parametersHash);

  auto parametersHash() const -> size_t;

  String m_name;
  std::uint64_t m_count;
  Json m_parameters;
  mutable std::optional<size_t> m_parametersHash;
};

template <>
struct hash<ItemDescriptor> {
  auto operator()(ItemDescriptor const& v) const -> size_t;
};

}// namespace Star

template <>
struct std::formatter<Star::ItemDescriptor> : Star::ostream_formatter {};
