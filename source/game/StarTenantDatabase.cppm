module;
import std;
import star.string;
import star.list;
import star.map;
import star.variant;
import star.maybe;
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

struct Tenant;
using TenantPtr = std::shared_ptr<Tenant>;
using TenantConstPtr = std::shared_ptr<const Tenant>;
using TenantWeakPtr = std::weak_ptr<Tenant>;
using TenantConstWeakPtr = std::weak_ptr<const Tenant>;
using TenantUPtr = std::unique_ptr<Tenant>;
using TenantConstUPtr = std::unique_ptr<const Tenant>;
class TenantDatabase;
using TenantDatabasePtr = std::shared_ptr<TenantDatabase>;
using TenantDatabaseConstPtr = std::shared_ptr<const TenantDatabase>;
using TenantDatabaseWeakPtr = std::weak_ptr<TenantDatabase>;
using TenantDatabaseConstWeakPtr = std::weak_ptr<const TenantDatabase>;
using TenantDatabaseUPtr = std::unique_ptr<TenantDatabase>;
using TenantDatabaseConstUPtr = std::unique_ptr<const TenantDatabase>;

struct TenantExceptionTag {
  static constexpr char const* name() { return "TenantException"; }
};
using TenantException = StarError<TenantExceptionTag, StarException>;

struct TenantNpcSpawnable {
  List<String> species;
  String type;
  Maybe<float> level;
  Maybe<Json> overrides;
};

struct TenantMonsterSpawnable {
  String type;
  Maybe<float> level;
  Maybe<Json> overrides;
};

typedef MVariant<TenantNpcSpawnable, TenantMonsterSpawnable> TenantSpawnable;

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

  Maybe<TenantRent> rent;

  // The Json this tenant was parsed from
  Json config;
};

class TenantDatabase {
public:
  TenantDatabase();

  void cleanup();

  TenantPtr getTenant(String const& name) const;

  // Return the list of all tenants for which colonyTags is a superset of
  // colonyTagCriteria
  List<TenantPtr> getMatchingTenants(StringMap<unsigned> const& colonyTags) const;

private:
  static TenantPtr readTenant(String const& path);

  Map<String, String> m_paths;
  mutable Mutex m_cacheMutex;
  mutable HashTtlCache<String, TenantPtr> m_tenantCache;
};

}

export module star.tenant_database;

export namespace Star {
  using ::Star::Tenant;
  using ::Star::TenantPtr;
  using ::Star::TenantConstPtr;
  using ::Star::TenantDatabase;
  using ::Star::TenantDatabasePtr;
  using ::Star::TenantDatabaseConstPtr;
  using ::Star::TenantExceptionTag;
  using ::Star::TenantException;
  using ::Star::TenantNpcSpawnable;
  using ::Star::TenantMonsterSpawnable;
  using ::Star::TenantSpawnable;
  using ::Star::TenantRent;
}
