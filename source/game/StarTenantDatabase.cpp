import star.json;
import star.json_extra;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.maybe;
import star.format;
import star.version;

import star.asset_source;
import star.assets;
import star.root;

import star.tenant_database;

namespace Star {

bool Tenant::criteriaSatisfied(StringMap<unsigned> const& colonyTags) const {
  // Check whether colonyTags is a supermultiset of colonyTagCriteria
  for (std::pair<String, unsigned> const& entry : colonyTagCriteria.pairs()) {
    if (colonyTags.value(entry.first, 0) < entry.second)
      return false;
  }
  return true;
}

TenantDatabase::TenantDatabase(Root& root)
  : m_root(root), m_tenantCache(root.random()) {
  auto assets = m_root.assets();
  auto& logger = m_root.logger();

  auto& files = assets->scanExtension("tenant");
  assets->queueJsons(files);
  for (auto& file : files) {
    try {
      String name = assets->json(file).getString("name");
      if (m_paths.contains(name))
        logger.error("Tenant {} defined twice, second time from {}", name, file);
      else
        m_paths[name] = file;
    } catch (std::exception const&) {
      ExceptionDiagnostic const e(std::current_exception());
      logger.error("Error loading tenant file {}: {}", file, outputException(e, true));
    }
  }
}

void TenantDatabase::cleanup() {
  MutexLocker locker(m_cacheMutex);
  m_tenantCache.cleanup();
}

std::shared_ptr<Tenant> TenantDatabase::getTenant(String const& name) const {
  MutexLocker locker(m_cacheMutex);
  return m_tenantCache.get(name,
      [this](String const& name) -> std::shared_ptr<Tenant> {
        if (auto path = m_paths.maybe(name))
          return readTenant(*m_root.assets(), *path);
        throw TenantException::format("No such tenant named '{}'", name);
      });
}

List<std::shared_ptr<Tenant>> TenantDatabase::getMatchingTenants(StringMap<unsigned> const& colonyTags) const {
  // This implementation loops over every tenant. Smarter implementations could
  // be written if it becomes a bottleneck, depending on how colonyTags end up
  // being
  // used, how many there are, how many tenants have similar criteria, etc.
  List<std::shared_ptr<Tenant>> matchingTenants;
  for (String const& name : m_paths.keys()) {
    std::shared_ptr<Tenant> tenant = getTenant(name);
    if (tenant->criteriaSatisfied(colonyTags))
      matchingTenants.append(tenant);
  }
  return matchingTenants;
}

std::shared_ptr<Tenant> TenantDatabase::readTenant(Assets const& assets, String const& path) {
  try {
    Json config = assets.json(path);

    String name = config.getString("name");
    float priority = config.getFloat("priority");

    StringMap<unsigned> colonyTagCriteria;
    for (std::pair<String, Json> const& entry : config.getObject("colonyTagCriteria").pairs()) {
      colonyTagCriteria[entry.first] = entry.second.toUInt();
    }

    List<TenantSpawnable> tenants = config.getArray("tenants").transformed([](Json const& json) -> TenantSpawnable {
      String spawn = json.getString("spawn");
      if (spawn == "monster") {
        return TenantMonsterSpawnable{json.getString("type"), json.optFloat("level"), json.opt("overrides")};
      } else {
        if constexpr (Star::DebugEnabled) {
          Star::starAssertFailed((json.getString("spawn") == "npc"));
        }

        Json speciesJson = json.get("species");
        List<String> species;
        if (speciesJson.isType(Json::Type::Array))
          species = speciesJson.toArray().transformed(std::mem_fn(&Json::toString));
        else
          species.append(speciesJson.toString());

        return TenantNpcSpawnable{species, json.getString("type"), json.optFloat("level"), json.opt("overrides")};
      }
    });

    Maybe<TenantRent> rent = config.opt("rent").apply([](Json const& json) {
      return TenantRent{jsonToVec2F(json.get("periodRange")), json.getString("pool")};
    });

    return std::make_shared<Tenant>(Tenant{name, priority, colonyTagCriteria, tenants, rent, config});
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    throw TenantException::format("Error loading tenant '{}': {}", path, outputException(e, false));
  }
}

}
