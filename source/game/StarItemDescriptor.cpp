#include "StarItemDescriptor.hpp"
#include "StarDataStream.hpp"
#include "StarItem.hpp"
#include "StarVersioningDatabase.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

ItemDescriptor::ItemDescriptor(String name, uint64_t count, Json parameters)
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

[[nodiscard]] ItemDescriptor ItemDescriptor::loadStore(Json const& spec, VersioningDatabaseConstPtr versioningDatabase) {
  versioningDatabase = requireServiceValueAs<ItemException>(std::move(versioningDatabase), "ItemDescriptor::loadStore", "versioning database");
  return ItemDescriptor{versioningDatabase->loadVersionedJson(VersionedJson::fromJson(spec), "Item")};
}

[[nodiscard]] String const& ItemDescriptor::name() const {
  return m_name;
}

[[nodiscard]] uint64_t ItemDescriptor::count() const {
  return m_count;
}

[[nodiscard]] Json const& ItemDescriptor::parameters() const {
  return m_parameters;
}

[[nodiscard]] ItemDescriptor ItemDescriptor::singular() const {
  return ItemDescriptor(name(), 1, parameters(), m_parametersHash);
}

[[nodiscard]] ItemDescriptor ItemDescriptor::withCount(uint64_t count) const {
  return ItemDescriptor(name(), count, parameters(), m_parametersHash);
}

[[nodiscard]] ItemDescriptor ItemDescriptor::multiply(uint64_t count) const {
  return ItemDescriptor(name(), this->count() * count, parameters(), m_parametersHash);
}

[[nodiscard]] ItemDescriptor ItemDescriptor::applyParameters(JsonObject const& parameters) const {
  return ItemDescriptor(name(), this->count(), this->parameters().setAll(parameters));
}

[[nodiscard]] bool ItemDescriptor::isNull() const {
  return m_name.empty();
}

[[nodiscard]] ItemDescriptor::operator bool() const {
  return !isNull();
}

[[nodiscard]] bool ItemDescriptor::isEmpty() const {
  return m_name.empty() || m_count == 0;
}

[[nodiscard]] bool ItemDescriptor::operator==(ItemDescriptor const& rhs) const {
  return std::tie(m_name, m_count, m_parameters) == std::tie(rhs.m_name, rhs.m_count, rhs.m_parameters);
}

[[nodiscard]] bool ItemDescriptor::operator!=(ItemDescriptor const& rhs) const {
  return std::tie(m_name, m_count, m_parameters) != std::tie(rhs.m_name, rhs.m_count, rhs.m_parameters);
}

[[nodiscard]] bool ItemDescriptor::matches(ItemDescriptor const& other, bool exactMatch) const {
  return other.name() == m_name && (!exactMatch || other.parameters() == m_parameters);
}

[[nodiscard]] bool ItemDescriptor::matches(ItemConstPtr const& other, bool exactMatch) const {
  return other->name() == m_name && (!exactMatch || other->parameters() == m_parameters);
}

[[nodiscard]] Json ItemDescriptor::diskStore(VersioningDatabaseConstPtr versioningDatabase) const {
  versioningDatabase = requireServiceValueAs<ItemException>(std::move(versioningDatabase), "ItemDescriptor::diskStore", "versioning database");
  auto res = JsonObject{
    {"name", m_name},
    {"count", m_count},
    {"parameters", m_parameters},
  };

  return versioningDatabase->makeCurrentVersionedJson("Item", res).toJson();
}

[[nodiscard]] Json ItemDescriptor::toJson() const {
  if (isNull()) {
    return Json();
  } else {
    return JsonObject{
      {"name", m_name},
      {"count", m_count},
      {"parameters", m_parameters},
    };
  }
}

ItemDescriptor::ItemDescriptor(String name, uint64_t count, Json parameters, Maybe<size_t> parametersHash)
    : m_name(std::move(name)), m_count(count), m_parameters(std::move(parameters)), m_parametersHash(parametersHash) {}

[[nodiscard]] size_t ItemDescriptor::parametersHash() const {
  if (!m_parametersHash)
    m_parametersHash = hash<Json>()(m_parameters);
  return *m_parametersHash;
}

DataStream& operator>>(DataStream& ds, ItemDescriptor& itemDescriptor) {
  ds.read(itemDescriptor.m_name);
  ds.readVlqU(itemDescriptor.m_count);
  ds.read(itemDescriptor.m_parameters);

  itemDescriptor.m_parametersHash.reset();

  return ds;
}

DataStream& operator<<(DataStream& ds, ItemDescriptor const& itemDescriptor) {
  ds.write(itemDescriptor.m_name);
  ds.writeVlqU(itemDescriptor.m_count);
  ds.write(itemDescriptor.m_parameters);

  return ds;
}

std::ostream& operator<<(std::ostream& os, ItemDescriptor const& descriptor) {
  format(os, "[{}, {}, {}]", descriptor.m_name, descriptor.m_count, descriptor.m_parameters);
  return os;
}

[[nodiscard]] size_t hash<ItemDescriptor>::operator()(ItemDescriptor const& v) const {
  return hashOf(v.m_name, v.m_count, v.m_parametersHash);
}

}// namespace Star
