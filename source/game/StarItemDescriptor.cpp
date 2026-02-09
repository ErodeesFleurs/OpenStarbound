#include "StarItemDescriptor.hpp"

#include "StarConfig.hpp"
#include "StarItem.hpp"
#include "StarRoot.hpp"
#include "StarVersioningDatabase.hpp"

import std;

namespace Star {

ItemDescriptor::ItemDescriptor() : m_count(0), m_parameters(JsonObject()) {}

ItemDescriptor::ItemDescriptor(String name, std::uint64_t count, Json parameters)
    : m_name(std::move(name)), m_count(count), m_parameters(std::move(parameters)) {
  if (m_parameters.isNull())
    m_parameters = JsonObject();
  if (!m_parameters.isType(Json::Type::Object))
    throw StarException("Item parameters not map in ItemDescriptor constructor");
}

ItemDescriptor::ItemDescriptor(Json const& spec) {
  if (spec.type() == Json::Type::Array) {
    m_name = spec.getString(0);
    m_count = spec.getInt(1, 1);
    m_parameters = spec.getObject(2, {});
  } else if (spec.type() == Json::Type::Object) {
    if (spec.contains("name"))
      m_name = spec.getString("name");
    else if (spec.contains("item"))
      m_name = spec.getString("item");
    else
      throw StarException("Item name missing.");
    m_count = spec.getInt("count", 1);
    m_parameters = spec.getObject("parameters", spec.getObject("data", JsonObject{}));
  } else if (spec.type() == Json::Type::String) {
    m_name = spec.toString();
    m_count = 1;
    m_parameters = JsonObject();
  } else if (spec.isNull()) {
    m_parameters = JsonObject();
    m_count = 0;
  } else {
    throw ItemException("ItemDescriptor spec variant not list, map, string, or null");
  }
}

auto ItemDescriptor::loadStore(Json const& spec) -> ItemDescriptor {
  auto versioningDatabase = Root::singleton().versioningDatabase();
  return ItemDescriptor{versioningDatabase->loadVersionedJson(VersionedJson::fromJson(spec), "Item")};
}

auto ItemDescriptor::name() const -> String const& {
  return m_name;
}

auto ItemDescriptor::count() const -> std::uint64_t {
  return m_count;
}

auto ItemDescriptor::parameters() const -> Json const& {
  return m_parameters;
}

auto ItemDescriptor::singular() const -> ItemDescriptor {
  return {name(), 1, parameters(), m_parametersHash};
}

auto ItemDescriptor::withCount(std::uint64_t count) const -> ItemDescriptor {
  return {name(), count, parameters(), m_parametersHash};
}

auto ItemDescriptor::multiply(std::uint64_t count) const -> ItemDescriptor {
  return {name(), this->count() * count, parameters(), m_parametersHash};
}

auto ItemDescriptor::applyParameters(JsonObject const& parameters) const -> ItemDescriptor {
  return {name(), this->count(), this->parameters().setAll(parameters)};
}

auto ItemDescriptor::isNull() const -> bool {
  return m_name.empty();
}

ItemDescriptor::operator bool() const {
  return !isNull();
}

auto ItemDescriptor::isEmpty() const -> bool {
  return m_name.empty() || m_count == 0;
}

auto ItemDescriptor::operator==(ItemDescriptor const& rhs) const -> bool {
  return std::tie(m_name, m_count, m_parameters) == std::tie(rhs.m_name, rhs.m_count, rhs.m_parameters);
}

auto ItemDescriptor::operator!=(ItemDescriptor const& rhs) const -> bool {
  return std::tie(m_name, m_count, m_parameters) != std::tie(rhs.m_name, rhs.m_count, rhs.m_parameters);
}

auto ItemDescriptor::matches(ItemDescriptor const& other, bool exactMatch) const -> bool {
  return other.name() == m_name && (!exactMatch || other.parameters() == m_parameters);
}

auto ItemDescriptor::matches(ConstPtr<Item> const& other, bool exactMatch) const -> bool {
  return other->name() == m_name && (!exactMatch || other->parameters() == m_parameters);
}

auto ItemDescriptor::diskStore() const -> Json {
  auto versioningDatabase = Root::singleton().versioningDatabase();
  auto res = JsonObject{
    {"name", m_name},
    {"count", m_count},
    {"parameters", m_parameters},
  };

  return versioningDatabase->makeCurrentVersionedJson("Item", res).toJson();
}

auto ItemDescriptor::toJson() const -> Json {
  if (isNull()) {
    return {};
  } else {
    return JsonObject{
      {"name", m_name},
      {"count", m_count},
      {"parameters", m_parameters},
    };
  }
}

ItemDescriptor::ItemDescriptor(String name, std::uint64_t count, Json parameters, std::optional<size_t> parametersHash)
    : m_name(std::move(name)), m_count(count), m_parameters(std::move(parameters)), m_parametersHash(parametersHash) {}

auto ItemDescriptor::parametersHash() const -> size_t {
  if (!m_parametersHash)
    m_parametersHash = hash<Json>()(m_parameters);
  return *m_parametersHash;
}

auto operator>>(DataStream& ds, ItemDescriptor& itemDescriptor) -> DataStream& {
  ds.read(itemDescriptor.m_name);
  ds.readVlqU(itemDescriptor.m_count);
  ds.read(itemDescriptor.m_parameters);

  itemDescriptor.m_parametersHash.reset();

  return ds;
}

auto operator<<(DataStream& ds, ItemDescriptor const& itemDescriptor) -> DataStream& {
  ds.write(itemDescriptor.m_name);
  ds.writeVlqU(itemDescriptor.m_count);
  ds.write(itemDescriptor.m_parameters);

  return ds;
}

auto operator<<(std::ostream& os, ItemDescriptor const& descriptor) -> std::ostream& {
  format(os, "[{}, {}, {}]", descriptor.m_name, descriptor.m_count, descriptor.m_parameters);
  return os;
}

auto hash<ItemDescriptor>::operator()(ItemDescriptor const& v) const -> size_t {
  return hashOf(v.m_name, v.m_count, v.m_parametersHash);
}

}// namespace Star
