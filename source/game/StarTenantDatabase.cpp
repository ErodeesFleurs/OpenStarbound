#include "StarTenantDatabase.hpp"
#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

bool Tenant::criteriaSatisfied(StringMap<unsigned> const& colonyTags) const {
  // Check whether colonyTags is a supermultiset of colonyTagCriteria
  for (auto const& [tagName, requiredCount] : colonyTagCriteria.pairs()) {
    if (colonyTags.value(tagName, 0) < requiredCount)
      return false;
  }
  return true;
}

TenantDatabase::TenantDatabase(AssetsConstPtr assets)
  : m_assets(requireServiceValueAs<TenantException>(std::move(assets), "TenantDatabase", "assets")) {
  auto& files = m_assets->scanExtension("tenant");
  m_assets->queueJsons(files);
  for (auto& file : files) {
    try {
      String name = m_assets->json(file).getString("name");
      if (m_paths.contains(name))
        Logger::error("Tenant {} defined twice, second time from {}", name, file);
      else
        m_paths[name] = file;
    } catch (std::exception const& e) {
      Logger::error("Error loading tenant file {}: {}", file, outputException(e, true));
    }
  }
}

void TenantDatabase::cleanup() {
  MutexLocker locker(m_cacheMutex);
  m_tenantCache.cleanup();
}

TenantPtr TenantDatabase::getTenant(String const& name) const {
  MutexLocker locker(m_cacheMutex);
  return m_tenantCache.get(name,
      [this](String const& name) -> TenantPtr {
        if (auto path = m_paths.maybe(name))
          return readTenant(*path);
        throw TenantException::format("No such tenant named '{}'", name);
      });
}

List<TenantPtr> TenantDatabase::getMatchingTenants(StringMap<unsigned> const& colonyTags) const {
  // This implementation loops over every tenant. Smarter implementations could
  // be written if it becomes a bottleneck, depending on how colonyTags end up
  // being
  // used, how many there are, how many tenants have similar criteria, etc.
  List<TenantPtr> matchingTenants;
  for (String const& name : m_paths.keys()) {
    TenantPtr tenant = getTenant(name);
    if (tenant->criteriaSatisfied(colonyTags))
      matchingTenants.append(tenant);
  }
  return matchingTenants;
}

TenantPtr TenantDatabase::readTenant(String const& path) const {
  try {
    Json config = m_assets->json(path);

    String name = config.getString("name");
    float priority = config.getFloat("priority");

    StringMap<unsigned> colonyTagCriteria;
    for (auto const& [tagName, tagConfig] : config.getObject("colonyTagCriteria").pairs()) {
      colonyTagCriteria[tagName] = tagConfig.toUInt();
    }

    List<TenantSpawnable> tenants = config.getArray("tenants").transformed([](Json const& json) -> TenantSpawnable {
      String spawn = json.getString("spawn");
      if (spawn == "monster") {
        return TenantMonsterSpawnable{json.getString("type"), json.optFloat("level"), json.opt("overrides")};
      } else {
        starAssert(json.getString("spawn") == "npc");

        Json speciesJson = json.get("species");
        List<String> species;
        if (speciesJson.isType(Json::Type::Array))
          species = speciesJson.toArray().transformed(mem_fn(&Json::toString));
        else
          species.append(speciesJson.toString());

        return TenantNpcSpawnable{species, json.getString("type"), json.optFloat("level"), json.opt("overrides")};
      }
    });

    Maybe<TenantRent> rent = config.opt("rent").apply([](Json const& json) {
      return TenantRent{jsonToVec2F(json.get("periodRange")), json.getString("pool")};
    });

    return make_shared<Tenant>(Tenant{name, priority, colonyTagCriteria, tenants, rent, config});
  } catch (std::exception const& e) {
    throw TenantException::format("Error loading tenant '{}': {}", path, outputException(e, false));
  }
}

}
