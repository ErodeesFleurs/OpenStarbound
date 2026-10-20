module;
import star.byte_array;

import std;
import star.string;
import star.format;
import star.json;
import star.thread;
import star.net_compatibility;

namespace Star {

class Root;
class Rebuilder;
class Vehicle;

struct VehicleDatabaseExceptionTag {
  static constexpr char const* name() { return "VehicleDatabaseException"; }
};
using VehicleDatabaseException = StarError<VehicleDatabaseExceptionTag, StarException>;

class VehicleDatabase {
public:
  explicit VehicleDatabase(Root& root);

  std::shared_ptr<Vehicle> create(String const& vehicleName, Json const& extraConfig = Json()) const;

  ByteArray netStore(std::shared_ptr<Vehicle> const& vehicle, NetCompatibilityRules rules) const;
  std::shared_ptr<Vehicle> netLoad(ByteArray const& netStore, NetCompatibilityRules rules) const;

  Json diskStore(std::shared_ptr<Vehicle> const& vehicle) const;
  std::shared_ptr<Vehicle> diskLoad(Json const& diskStore) const;

private:
  Root& m_root;

  StringMap<std::pair<String, Json>> m_vehicles;

  mutable RecursiveMutex m_luaMutex;
  std::shared_ptr<Rebuilder> m_rebuilder;
};

}

export module star.vehicle_database;

export namespace Star {
  using ::Star::VehicleDatabaseExceptionTag;
  using ::Star::VehicleDatabaseException;
  using ::Star::Rebuilder;
  using ::Star::Vehicle;
  using ::Star::VehicleDatabase;
}
