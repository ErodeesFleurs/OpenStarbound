#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarTtlCache.hpp"

import std;

namespace Star {

using TenantException = ExceptionDerived<"TenantException">;

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

using TenantSpawnable = MVariant<TenantNpcSpawnable, TenantMonsterSpawnable>;

struct TenantRent {
  Vec2F periodRange;
  String pool;
};

struct Tenant {
  [[nodiscard]] auto criteriaSatisfied(StringMap<unsigned> const& colonyTags) const -> bool;

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
  TenantDatabase();

  void cleanup();

  auto getTenant(String const& name) const -> Ptr<Tenant>;

  // Return the list of all tenants for which colonyTags is a superset of
  // colonyTagCriteria
  auto getMatchingTenants(StringMap<unsigned> const& colonyTags) const -> List<Ptr<Tenant>>;

private:
  static auto readTenant(String const& path) -> Ptr<Tenant>;

  Map<String, String> m_paths;
  mutable Mutex m_cacheMutex;
  mutable HashTtlCache<String, Ptr<Tenant>> m_tenantCache;
};

}// namespace Star
