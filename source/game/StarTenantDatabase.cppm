module;
import std;
import star.string;
import star.list;
import star.map;
import star.format;
import star.json;

import star.vector;
import star.thread;
import star.ordered_map;
import star.lru_cache;
import star.time;
import star.random;
import star.ttl_cache;

namespace Star {

class Assets;
class Root;

struct TenantExceptionTag {
  static constexpr char const* name() { return "TenantException"; }
};
using TenantException = StarError<TenantExceptionTag, StarException>;

struct TenantNpcSpawnable {
  List<String> species;
  String type;
  std::optional<float> level;
  std::optional<Json> overrides;
};

struct TenantMonsterSpawnable {
  String type;
  std::optional<float> level;
  std::optional<Json> overrides;
};

typedef std::variant<std::monostate, TenantNpcSpawnable, TenantMonsterSpawnable> TenantSpawnable;

struct TenantRent {
  Vec2F periodRange;
  String pool;
};

struct Tenant {
  bool criteriaSatisfied(StringMap<unsigned> const& colonyTags) const;

  String name;
  float priority;

  // The colonyTag multiset the house must contain in order to satisfy this
  // tenant.
  StringMap<unsigned> colonyTagCriteria;

  List<TenantSpawnable> tenants;

  std::optional<TenantRent> rent;

  // The Json this tenant was parsed from
  Json config;
};

class TenantDatabase {
public:
  explicit TenantDatabase(Root& root);

  void cleanup();

  std::shared_ptr<Tenant> getTenant(String const& name) const;

  // Return the list of all tenants for which colonyTags is a superset of
  // colonyTagCriteria
  List<std::shared_ptr<Tenant>> getMatchingTenants(StringMap<unsigned> const& colonyTags) const;

private:
  static std::shared_ptr<Tenant> readTenant(Assets const& assets, String const& path);

  Root& m_root;
  Map<String, String> m_paths;
  mutable Mutex m_cacheMutex;
  mutable HashTtlCache<String, std::shared_ptr<Tenant>> m_tenantCache;
};

}

export module star.tenant_database;

export namespace Star {
  using ::Star::Tenant;
  using ::Star::TenantDatabase;
  using ::Star::TenantExceptionTag;
  using ::Star::TenantException;
  using ::Star::TenantNpcSpawnable;
  using ::Star::TenantMonsterSpawnable;
  using ::Star::TenantSpawnable;
  using ::Star::TenantRent;
}
