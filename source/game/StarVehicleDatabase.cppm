module;

#include "StarJson.hpp"
#include "StarThread.hpp"
#include "StarNetCompatibility.hpp"

namespace Star {

STAR_CLASS(Rebuilder);
STAR_CLASS(Vehicle);
STAR_CLASS(VehicleDatabase);

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
  StringMap<pair<String, Json>> m_vehicles;

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
