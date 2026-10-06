module;

#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
import star.uuid;
#include "StarJson.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
#include "StarGameTypes.hpp"

namespace Star {

template <typename BaseType> struct CelestialWorldIdWrapper;
template <typename BaseType> struct ClientShipWorldIdWrapper;
template <typename BaseType> struct CustomWorldIdWrapper;
struct ClientCustomWorldId;

enum class WarpMode : uint8_t {
  None,
  BeamOnly,
  DeployOnly,
  BeamOrDeploy
};
extern EnumMap<WarpMode> WarpModeNames;

struct InstanceWorldId {
  friend DataStream& operator>>(DataStream& ds, InstanceWorldId& instanceWorldId);
  friend DataStream& operator<<(DataStream& ds, InstanceWorldId const& instanceWorldId);
  friend std::ostream& operator<<(std::ostream& os, InstanceWorldId const& worldId);
  friend std::ostream& operator<<(std::ostream& os,
      MVariant<CelestialWorldIdWrapper<CelestialCoordinate>, ClientShipWorldIdWrapper<Uuid>,
          InstanceWorldId, CustomWorldIdWrapper<String>, ClientCustomWorldId> const& worldId);

  String instance;
  Maybe<Uuid> uuid;
  Maybe<float> level;

  InstanceWorldId();
  InstanceWorldId(String instance, Maybe<Uuid> uuid = {}, Maybe<float> level = {});

  bool operator==(InstanceWorldId const& rhs) const;
  bool operator<(InstanceWorldId const& rhs) const;
};

template <>
struct hash<InstanceWorldId> {
  size_t operator()(InstanceWorldId const& id) const;
};

DataStream& operator>>(DataStream& ds, InstanceWorldId& missionWorldId);
DataStream& operator<<(DataStream& ds, InstanceWorldId const& missionWorldId);

struct ClientCustomWorldId {
  friend DataStream& operator>>(DataStream& ds, ClientCustomWorldId& clientWorldId);
  friend DataStream& operator<<(DataStream& ds, ClientCustomWorldId const& clientWorldId);
  friend std::ostream& operator<<(std::ostream& os, ClientCustomWorldId const& worldId);

  Uuid uuid;
  String name;

  ClientCustomWorldId();
  ClientCustomWorldId(Uuid uuid, String name);

  bool operator==(ClientCustomWorldId const& rhs) const;
  bool operator<(ClientCustomWorldId const& rhs) const;
};

template <>
struct hash<ClientCustomWorldId> {
  size_t operator()(ClientCustomWorldId const& id) const;
};

DataStream& operator>>(DataStream& ds, ClientCustomWorldId& clientWorldId);
DataStream& operator<<(DataStream& ds, ClientCustomWorldId const& clientWorldId);

  template <typename BaseType>
  struct CelestialWorldIdWrapper : BaseType {
    friend std::ostream& operator<<(std::ostream& os, CelestialWorldIdWrapper<CelestialCoordinate> const& worldId);
    using BaseType::BaseType;

    CelestialWorldIdWrapper() : BaseType() {}

    CelestialWorldIdWrapper(CelestialWorldIdWrapper const& nt) : BaseType(nt) {}

    CelestialWorldIdWrapper(CelestialWorldIdWrapper&& nt) : BaseType(std::move(nt)) {}

    explicit CelestialWorldIdWrapper(BaseType const& bt) : BaseType(bt) {}

    explicit CelestialWorldIdWrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

    CelestialWorldIdWrapper& operator=(CelestialWorldIdWrapper const& rhs) {
      BaseType::operator=(rhs);
      return *this;
    }

    CelestialWorldIdWrapper& operator=(CelestialWorldIdWrapper&& rhs) {
      BaseType::operator=(std::move(rhs));
      return *this;
    }

    template <class Arg>
    CelestialWorldIdWrapper& operator=(Arg&& other) {
      static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
              || std::is_same<CelestialWorldIdWrapper, typename std::decay<Arg>::type>::value,
          "" "CelestialWorldId" " can not implicitly be assigned from " "CelestialCoordinate" "-derived classes or strong " "CelestialCoordinate"
          " typedefs");

      BaseType::operator=(std::forward<Arg>(other));
      return *this;
    }
  };
  typedef CelestialWorldIdWrapper<CelestialCoordinate> CelestialWorldId;

  template <typename BaseType>
  struct ClientShipWorldIdWrapper : BaseType {
    friend std::ostream& operator<<(std::ostream& os, ClientShipWorldIdWrapper<Uuid> const& worldId);
    using BaseType::BaseType;

    ClientShipWorldIdWrapper() : BaseType() {}

    ClientShipWorldIdWrapper(ClientShipWorldIdWrapper const& nt) : BaseType(nt) {}

    ClientShipWorldIdWrapper(ClientShipWorldIdWrapper&& nt) : BaseType(std::move(nt)) {}

    explicit ClientShipWorldIdWrapper(BaseType const& bt) : BaseType(bt) {}

    explicit ClientShipWorldIdWrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

    ClientShipWorldIdWrapper& operator=(ClientShipWorldIdWrapper const& rhs) {
      BaseType::operator=(rhs);
      return *this;
    }

    ClientShipWorldIdWrapper& operator=(ClientShipWorldIdWrapper&& rhs) {
      BaseType::operator=(std::move(rhs));
      return *this;
    }

    template <class Arg>
    ClientShipWorldIdWrapper& operator=(Arg&& other) {
      static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
              || std::is_same<ClientShipWorldIdWrapper, typename std::decay<Arg>::type>::value,
          "" "ClientShipWorldId" " can not implicitly be assigned from " "Uuid" "-derived classes or strong " "Uuid"
          " typedefs");

      BaseType::operator=(std::forward<Arg>(other));
      return *this;
    }
  };
  typedef ClientShipWorldIdWrapper<Uuid> ClientShipWorldId;

  template <typename BaseType>
  struct CustomWorldIdWrapper : BaseType {
    friend std::ostream& operator<<(std::ostream& os, CustomWorldIdWrapper<String> const& worldId);
    using BaseType::BaseType;

    CustomWorldIdWrapper() : BaseType() {}

    CustomWorldIdWrapper(CustomWorldIdWrapper const& nt) : BaseType(nt) {}

    CustomWorldIdWrapper(CustomWorldIdWrapper&& nt) : BaseType(std::move(nt)) {}

    explicit CustomWorldIdWrapper(BaseType const& bt) : BaseType(bt) {}

    explicit CustomWorldIdWrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

    CustomWorldIdWrapper& operator=(CustomWorldIdWrapper const& rhs) {
      BaseType::operator=(rhs);
      return *this;
    }

    CustomWorldIdWrapper& operator=(CustomWorldIdWrapper&& rhs) {
      BaseType::operator=(std::move(rhs));
      return *this;
    }

    template <class Arg>
    CustomWorldIdWrapper& operator=(Arg&& other) {
      static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
              || std::is_same<CustomWorldIdWrapper, typename std::decay<Arg>::type>::value,
          "" "CustomWorldId" " can not implicitly be assigned from " "String" "-derived classes or strong " "String"
          " typedefs");

      BaseType::operator=(std::forward<Arg>(other));
      return *this;
    }
  };
  typedef CustomWorldIdWrapper<String> CustomWorldId;
typedef MVariant<CelestialWorldId, ClientShipWorldId, InstanceWorldId, CustomWorldId, ClientCustomWorldId> WorldId;

String printWorldId(WorldId const& worldId);
WorldId parseWorldId(String const& printedId);

// Same as outputting printWorldId
std::ostream& operator<<(std::ostream& os, CelestialWorldId const& worldId);
std::ostream& operator<<(std::ostream& os, ClientShipWorldId const& worldId);
std::ostream& operator<<(std::ostream& os, InstanceWorldId const& worldId);
std::ostream& operator<<(std::ostream& os, CustomWorldId const& worldId);
std::ostream& operator<<(std::ostream& os, ClientCustomWorldId const& worldId);
std::ostream& operator<<(std::ostream& os, WorldId const& worldId);

strong_typedef(String, SpawnTargetUniqueEntity);
strong_typedef(Vec2F, SpawnTargetPosition);
using SpawnTargetX = StrongTypedefBuiltin<float, struct SpawnTargetXTag>;
typedef MVariant<SpawnTargetUniqueEntity, SpawnTargetPosition, SpawnTargetX> SpawnTarget;

Json spawnTargetToJson(SpawnTarget spawnTarget);
SpawnTarget spawnTargetFromJson(Json v);

String printSpawnTarget(SpawnTarget spawnTarget);

struct WarpToWorld {
  friend DataStream& operator>>(DataStream& ds, WarpToWorld& warpToWorld);
  friend DataStream& operator<<(DataStream& ds, WarpToWorld const& warpToWorld);

  WarpToWorld();
  explicit WarpToWorld(WorldId world, SpawnTarget spawn = {});
  explicit WarpToWorld(Json v);

  WorldId world;
  SpawnTarget target;

  bool operator==(WarpToWorld const& rhs) const;
  explicit operator bool() const;

  Json toJson() const;
};

strong_typedef(Uuid, WarpToPlayer);

enum class WarpAlias {
  Return,
  OrbitedWorld,
  OwnShip
};

typedef MVariant<WarpToWorld, WarpToPlayer, WarpAlias> WarpAction;

WarpAction parseWarpAction(String const& warpString);
String printWarpAction(WarpAction const& warpAction);
JsonObject warpActionToJson(WarpAction const& warpAction);

DataStream& operator>>(DataStream& ds, WarpToWorld& warpToWorld);
DataStream& operator<<(DataStream& ds, WarpToWorld const& warpToWorld);

}

template <> struct fmt::formatter<Star::CelestialWorldId> : ostream_formatter {};
template <> struct fmt::formatter<Star::ClientShipWorldId> : ostream_formatter {};
template <> struct fmt::formatter<Star::InstanceWorldId> : ostream_formatter {};
template <> struct fmt::formatter<Star::CustomWorldId> : ostream_formatter {};
template <> struct fmt::formatter<Star::ClientCustomWorldId> : ostream_formatter {};
template <> struct fmt::formatter<Star::WorldId> : ostream_formatter {};
template <> struct fmt::formatter<Star::WarpToWorld> : ostream_formatter {};

export module star.warping;

export namespace Star {
  using ::Star::WarpMode;
  using ::Star::WarpModeNames;
  using ::Star::InstanceWorldId;
  using ::Star::ClientCustomWorldId;
  using ::Star::CelestialWorldIdWrapper;
  using ::Star::CelestialWorldId;
  using ::Star::ClientShipWorldIdWrapper;
  using ::Star::ClientShipWorldId;
  using ::Star::CustomWorldIdWrapper;
  using ::Star::CustomWorldId;
  using ::Star::WorldId;
  using ::Star::printWorldId;
  using ::Star::parseWorldId;
  using ::Star::SpawnTargetUniqueEntityWrapper;
  using ::Star::SpawnTargetUniqueEntity;
  using ::Star::SpawnTargetPositionWrapper;
  using ::Star::SpawnTargetPosition;
  using ::Star::SpawnTargetXTag;
  using ::Star::SpawnTargetX;
  using ::Star::SpawnTarget;
  using ::Star::spawnTargetToJson;
  using ::Star::spawnTargetFromJson;
  using ::Star::printSpawnTarget;
  using ::Star::WarpToWorld;
  using ::Star::WarpToPlayerWrapper;
  using ::Star::WarpToPlayer;
  using ::Star::WarpAlias;
  using ::Star::WarpAction;
  using ::Star::parseWarpAction;
  using ::Star::printWarpAction;
  using ::Star::warpActionToJson;
}
