import std;
import star.json;
import star.json_extra;
import star.bi_map;
import star.either;
import star.player_types;

import star.algorithm;
namespace Star {

extern EnumMap<PlayerMode> const PlayerModeNames{
  {PlayerMode::Casual, "casual"},
  {PlayerMode::Survival, "survival"},
  {PlayerMode::Hardcore, "hardcore"}
};

extern EnumMap<PlayerBusyState> const PlayerBusyStateNames{
  {PlayerBusyState::None, "none"},
  {PlayerBusyState::Chatting, "chatting"},
  {PlayerBusyState::Menu, "menu"}
};

PlayerModeConfig::PlayerModeConfig() : PlayerModeConfig(Json{}) {}

PlayerModeConfig::PlayerModeConfig(Json config) {
  if (config.isNull())
    config = JsonObject();

  hunger = config.getBool("hunger", true);
  allowBeamUpUnderground = config.getBool("allowBeamUpUnderground", false);
  reviveCostPercentile = config.getFloat("reviveCostPercentile", 0.0f);
  auto deathDropItemTypesConfig = config.get("deathDropItemTypes", "none");
  if (deathDropItemTypesConfig.type() == Json::Type::Array)
    deathDropItemTypes.setRight(jsonToStringList(deathDropItemTypesConfig));
  else
    deathDropItemTypes.setLeft(deathDropItemTypesConfig.toString());
  permadeath = config.getBool("permadeath", false);
}

ShipUpgrades::ShipUpgrades() : ShipUpgrades(Json{}) {}

ShipUpgrades::ShipUpgrades(Json config) {
  if (config.isNull())
    config = JsonObject();

  shipLevel = config.getUInt("shipLevel", 0);
  maxFuel = config.getUInt("maxFuel", 0);
  crewSize = config.getUInt("crewSize", 0);
  fuelEfficiency = config.getFloat("fuelEfficiency", 1.0);
  shipSpeed = config.getUInt("shipSpeed", 0);
  capabilities.addAll(jsonToStringList(config.get("capabilities", JsonArray{})));
}

Json ShipUpgrades::toJson() const {
  return JsonObject{{"shipLevel", shipLevel},
      {"maxFuel", maxFuel},
      {"crewSize", crewSize},
      {"fuelEfficiency", fuelEfficiency},
      {"shipSpeed", shipSpeed},
      {"capabilities", capabilities.values().transformed(construct<Json>())}};
}

ShipUpgrades& ShipUpgrades::apply(Json const& upgrades) {
  shipLevel = std::max(shipLevel, (unsigned)upgrades.optUInt("shipLevel").value_or(shipLevel));
  maxFuel = upgrades.optUInt("maxFuel").value_or(maxFuel);
  crewSize = std::max(crewSize, (unsigned)upgrades.optUInt("crewSize").value_or(crewSize));
  fuelEfficiency = upgrades.optFloat("fuelEfficiency").value_or(fuelEfficiency);
  shipSpeed = upgrades.optFloat("shipSpeed").value_or(shipSpeed);
  if (upgrades.contains("capabilities"))
    capabilities.addAll(jsonToStringList(upgrades.get("capabilities", JsonArray{})));
  return *this;
}

bool ShipUpgrades::operator==(ShipUpgrades const& rhs) const {
  return std::tie(shipLevel, maxFuel, crewSize, fuelEfficiency, shipSpeed, capabilities)
      == std::tie(rhs.shipLevel, rhs.maxFuel, rhs.crewSize, rhs.fuelEfficiency, rhs.shipSpeed, rhs.capabilities);
}

}
