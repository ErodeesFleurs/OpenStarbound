#pragma once

#include "StarBiMap.hpp"
#include "StarEither.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

enum class PlayerMode {
  Casual,
  Survival,
  Hardcore
};
extern EnumMap<PlayerMode> const PlayerModeNames;

enum class PlayerBusyState {
  None,
  Chatting,
  Menu
};
extern EnumMap<PlayerBusyState> const PlayerBusyStateNames;

struct PlayerWarpRequest {
  String action;
  std::optional<String> animation;
  bool deploy;
};

struct PlayerModeConfig {
  explicit PlayerModeConfig(Json config = {});

  bool hunger;
  bool allowBeamUpUnderground;
  float reviveCostPercentile;
  Either<String, StringList> deathDropItemTypes;
  bool permadeath;
};

struct ShipUpgrades {
  explicit ShipUpgrades(Json config = {});
  [[nodiscard]] auto toJson() const -> Json;

  auto apply(Json const& upgrades) -> ShipUpgrades&;

  auto operator==(ShipUpgrades const& rhs) const -> bool;

  unsigned shipLevel;
  unsigned maxFuel;
  unsigned crewSize;
  float fuelEfficiency;
  float shipSpeed;
  StringSet capabilities;
};

auto operator>>(DataStream& ds, ShipUpgrades& upgrades) -> DataStream&;
auto operator<<(DataStream& ds, ShipUpgrades const& upgrades) -> DataStream&;

}// namespace Star
