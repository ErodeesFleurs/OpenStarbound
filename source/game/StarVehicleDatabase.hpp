#pragma once

#include "StarJson.hpp"
#include "StarVehicle.hpp"

namespace Star {

class Rebuilder;
using RebuilderPtr = SharedPtr<Rebuilder>;

struct VehicleDatabaseExceptionTag { static constexpr char const* typeName = "VehicleDatabaseException"; };
using VehicleDatabaseException = TypedException<StarException, VehicleDatabaseExceptionTag>;

class VehicleDatabase {
public:
  VehicleDatabase();

  VehiclePtr create(String const& vehicleName, Json const& extraConfig = Json()) const;

  ByteArray netStore(VehiclePtr const& vehicle, NetCompatibilityRules rules) const;
  VehiclePtr netLoad(ByteArray const& netStore, NetCompatibilityRules rules) const;

  Json diskStore(VehiclePtr const& vehicle) const;
  VehiclePtr diskLoad(Json const& diskStore) const;

private:
  StringMap<pair<String, Json>> m_vehicles;

  mutable RecursiveMutex m_luaMutex;
  RebuilderPtr m_rebuilder;
};

}
