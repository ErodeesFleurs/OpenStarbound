module;
import star.format;

import std;
import star.string;
import star.data_stream;
import star.json;
import star.bi_map;
import star.either;

namespace Star {

enum class PlayerMode {
  Casual,
  Survival,
  Hardcore
};
extern std::expected<EnumMap<PlayerMode>, StarException> const PlayerModeNames;

enum class PlayerBusyState {
  None,
  Chatting,
  Menu
};
extern std::expected<EnumMap<PlayerBusyState>, StarException> const PlayerBusyStateNames;

struct PlayerWarpRequest {
  String action;
  std::optional<String> animation;
  bool deploy;
};

struct PlayerModeConfig {
  explicit PlayerModeConfig();
  [[nodiscard]] static std::expected<PlayerModeConfig, StarException> create(Json config);

  bool hunger;
  bool allowBeamUpUnderground;
  float reviveCostPercentile;
  Either<String, StringList> deathDropItemTypes;
  bool permadeath;
};

struct ShipUpgrades {
  explicit ShipUpgrades();
  [[nodiscard]] static std::expected<ShipUpgrades, StarException> create(Json config);
  Json toJson() const;

  [[nodiscard]] std::expected<ShipUpgrades*, StarException> apply(Json const& upgrades);

  bool operator==(ShipUpgrades const& rhs) const;

  unsigned shipLevel;
  unsigned maxFuel;
  unsigned crewSize;
  float fuelEfficiency;
  float shipSpeed;
  StringSet capabilities;
};

[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, ShipUpgrades& upgrades);
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, ShipUpgrades const& upgrades);

}

export module star.player_types;

export namespace Star {
  using ::Star::PlayerMode;
  using ::Star::PlayerModeNames;
  using ::Star::PlayerBusyState;
  using ::Star::PlayerBusyStateNames;
  using ::Star::PlayerWarpRequest;
  using ::Star::PlayerModeConfig;
  using ::Star::ShipUpgrades;
  using ::Star::operator>>;
  using ::Star::operator<<;
}
