#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarVehicle.hpp"

import std;

namespace Star {

class Rebuilder;

using VehicleDatabaseException = ExceptionDerived<"VehicleDatabaseException">;

class VehicleDatabase {
public:
  VehicleDatabase();

  auto create(String const& vehicleName, Json const& extraConfig = Json()) const -> Ptr<Vehicle>;

  auto netStore(Ptr<Vehicle> const& vehicle, NetCompatibilityRules rules) const -> ByteArray;
  auto netLoad(ByteArray const& netStore, NetCompatibilityRules rules) const -> Ptr<Vehicle>;

  auto diskStore(Ptr<Vehicle> const& vehicle) const -> Json;
  auto diskLoad(Json const& diskStore) const -> Ptr<Vehicle>;

private:
  StringMap<std::pair<String, Json>> m_vehicles;

  mutable RecursiveMutex m_luaMutex;
  Ptr<Rebuilder> m_rebuilder;
};

}// namespace Star
