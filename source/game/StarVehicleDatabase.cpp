#include "StarVehicleDatabase.hpp"
#include "StarAssets.hpp"
#include "StarJsonExtra.hpp"
#include "StarRebuilder.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarUtilityLuaBindings.hpp"
#include "StarVehicle.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

VehicleDatabase::VehicleDatabase(AssetsConstPtr assets, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, LuaRootServices luaRootServices)
    : m_assets(requireServiceValueAs<VehicleDatabaseException>(std::move(assets), "VehicleDatabase", "assets")),
      m_particleDatabase(requireServiceValueAs<VehicleDatabaseException>(std::move(particleDatabase), "VehicleDatabase", "particle database")),
      m_imageMetadataDatabase(requireServiceValueAs<VehicleDatabaseException>(std::move(imageMetadataDatabase), "VehicleDatabase", "image metadata database")),
      m_rebuilder(make_shared<Rebuilder>(m_assets, "vehicle", requireLuaRootServices(std::move(luaRootServices), "VehicleDatabase"))) {
  auto& files = m_assets->scanExtension("vehicle");
  m_assets->queueJsons(files);
  for (String file : files) {
    try {
      auto config = m_assets->json(file);
      String name = config.getString("name");

      if (m_vehicles.contains(name))
        throw VehicleDatabaseException::format("Repeat vehicle name '{}'", name);

      m_vehicles.add(std::move(name), make_pair(std::move(file), std::move(config)));
    } catch (StarException const& e) {
      throw VehicleDatabaseException(strf("Error loading vehicle '{}'", file), e);
    }
  }
}

VehiclePtr VehicleDatabase::create(String const& vehicleName, Json const& extraConfig) const {
  auto configPair = m_vehicles.ptr(vehicleName);
  if (!configPair)
    throw VehicleDatabaseException::format("No such vehicle named '{}'", vehicleName);
  return make_shared<Vehicle>(m_assets, m_particleDatabase, m_imageMetadataDatabase, configPair->second, configPair->first, extraConfig);
}

ByteArray VehicleDatabase::netStore(VehiclePtr const& vehicle, NetCompatibilityRules rules) const {
  DataStreamBuffer ds;
  ds.setStreamCompatibilityVersion(rules);

  ds.write(vehicle->baseConfig().getString("name"));
  ds.write(vehicle->dynamicConfig());
  return ds.takeData();
}

VehiclePtr VehicleDatabase::netLoad(ByteArray const& netStore, NetCompatibilityRules rules) const {
  DataStreamBuffer ds(netStore);
  ds.setStreamCompatibilityVersion(rules);

  String name = ds.read<String>();
  auto dynamicConfig = ds.read<Json>();
  auto vehicle = create(name, dynamicConfig);

  return vehicle;
}

Json VehicleDatabase::diskStore(VehiclePtr const& vehicle) const {
  return JsonObject{
    {"name", vehicle->baseConfig().getString("name")},
    {"dynamicConfig", vehicle->dynamicConfig()},
    {"state", vehicle->diskStore()}};
}

VehiclePtr VehicleDatabase::diskLoad(Json const& diskStore) const {
  VehiclePtr vehicle;
  try {
    vehicle = create(diskStore.getString("name"), diskStore.get("dynamicConfig"));
    vehicle->diskLoad(diskStore.get("state"));
    return vehicle;
  } catch (std::exception const& e) {
    vehicle.reset();
    auto exception = std::current_exception();
    bool success = m_rebuilder->rebuild(diskStore, strf("{}", outputException(e, false)), [&](Json const& store) -> String {
      try {
        vehicle = create(store.getString("name"), store.get("dynamicConfig"));
        vehicle->diskLoad(store.get("state"));
      } catch (std::exception const& e) {
        exception = std::current_exception();
        return strf("{}", outputException(e, false));
      }
      return {};
    });

    if (!success)
      std::rethrow_exception(exception);
  }
  return vehicle;
}

}// namespace Star
