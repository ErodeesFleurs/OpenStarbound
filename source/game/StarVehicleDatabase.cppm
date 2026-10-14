module;
import star.byte_array;

import std;
import star.string;
import star.format;
import star.json;
import star.thread;
import star.net_compatibility;

namespace Star {

class Rebuilder;
using RebuilderPtr = std::shared_ptr<Rebuilder>;
using RebuilderConstPtr = std::shared_ptr<const Rebuilder>;
using RebuilderWeakPtr = std::weak_ptr<Rebuilder>;
using RebuilderConstWeakPtr = std::weak_ptr<const Rebuilder>;
using RebuilderUPtr = std::unique_ptr<Rebuilder>;
using RebuilderConstUPtr = std::unique_ptr<const Rebuilder>;
class Vehicle;
using VehiclePtr = std::shared_ptr<Vehicle>;
using VehicleConstPtr = std::shared_ptr<const Vehicle>;
using VehicleWeakPtr = std::weak_ptr<Vehicle>;
using VehicleConstWeakPtr = std::weak_ptr<const Vehicle>;
using VehicleUPtr = std::unique_ptr<Vehicle>;
using VehicleConstUPtr = std::unique_ptr<const Vehicle>;
class VehicleDatabase;
using VehicleDatabasePtr = std::shared_ptr<VehicleDatabase>;
using VehicleDatabaseConstPtr = std::shared_ptr<const VehicleDatabase>;
using VehicleDatabaseWeakPtr = std::weak_ptr<VehicleDatabase>;
using VehicleDatabaseConstWeakPtr = std::weak_ptr<const VehicleDatabase>;
using VehicleDatabaseUPtr = std::unique_ptr<VehicleDatabase>;
using VehicleDatabaseConstUPtr = std::unique_ptr<const VehicleDatabase>;

struct VehicleDatabaseExceptionTag {
  static constexpr char const* name() { return "VehicleDatabaseException"; }
};
using VehicleDatabaseException = StarError<VehicleDatabaseExceptionTag, StarException>;

class VehicleDatabase {
public:
  VehicleDatabase();

  VehiclePtr create(String const& vehicleName, Json const& extraConfig = Json()) const;

  ByteArray netStore(VehiclePtr const& vehicle, NetCompatibilityRules rules) const;
  VehiclePtr netLoad(ByteArray const& netStore, NetCompatibilityRules rules) const;

  Json diskStore(VehiclePtr const& vehicle) const;
  VehiclePtr diskLoad(Json const& diskStore) const;

private:
  StringMap<std::pair<String, Json>> m_vehicles;

  mutable RecursiveMutex m_luaMutex;
  RebuilderPtr m_rebuilder;
};

}

export module star.vehicle_database;

export namespace Star {
  using ::Star::VehicleDatabaseExceptionTag;
  using ::Star::VehicleDatabaseException;
  using ::Star::Rebuilder;
  using ::Star::RebuilderPtr;
  using ::Star::RebuilderConstPtr;
  using ::Star::RebuilderWeakPtr;
  using ::Star::RebuilderConstWeakPtr;
  using ::Star::RebuilderUPtr;
  using ::Star::RebuilderConstUPtr;
  using ::Star::Vehicle;
  using ::Star::VehiclePtr;
  using ::Star::VehicleConstPtr;
  using ::Star::VehicleWeakPtr;
  using ::Star::VehicleConstWeakPtr;
  using ::Star::VehicleUPtr;
  using ::Star::VehicleConstUPtr;
  using ::Star::VehicleDatabase;
  using ::Star::VehicleDatabasePtr;
  using ::Star::VehicleDatabaseConstPtr;
  using ::Star::VehicleDatabaseWeakPtr;
  using ::Star::VehicleDatabaseConstWeakPtr;
  using ::Star::VehicleDatabaseUPtr;
  using ::Star::VehicleDatabaseConstUPtr;
}
