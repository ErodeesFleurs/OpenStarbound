module;

import std;
import star.string;
import star.map;
import star.format;
import star.json;
import star.bi_map;
import star.game_types;

import star.tech_database;

namespace Star {

struct PlayerTechExceptionTag {
  static constexpr char const* name() { return "PlayerTechException"; }
};
using PlayerTechException = StarError<PlayerTechExceptionTag, StarException>;

class PlayerTech;
using PlayerTechPtr = std::shared_ptr<PlayerTech>;
using PlayerTechConstPtr = std::shared_ptr<const PlayerTech>;
using PlayerTechWeakPtr = std::weak_ptr<PlayerTech>;
using PlayerTechConstWeakPtr = std::weak_ptr<const PlayerTech>;
using PlayerTechUPtr = std::unique_ptr<PlayerTech>;
using PlayerTechConstUPtr = std::unique_ptr<const PlayerTech>;

// Set of player techs, techs can be either unavailable, available but not
// enabled, enabled but not equipped, or equipped.
class PlayerTech {
public:
  PlayerTech();
  PlayerTech(Json const& json);

  Json toJson() const;

  bool isAvailable(String const& techModule) const;
  void makeAvailable(String const& techModule);
  void makeUnavailable(String const& techModule);

  bool isEnabled(String const& techModule) const;
  void enable(String const& techModule);
  void disable(String const& techModule);

  bool isEquipped(String const& techModule) const;
  void equip(String const& techModule);
  void unequip(String const& techModule);

  StringSet const& availableTechs() const;
  StringSet const& enabledTechs() const;
  HashMap<TechType, String> const& equippedTechs() const;
  StringList techModules() const;

private:
  StringSet m_availableTechs;
  StringSet m_enabledTechs;
  HashMap<TechType, String> m_equippedTechs;
};

}

export module star.player_tech;

export namespace Star {
using ::Star::PlayerTech;
using ::Star::PlayerTechPtr;
using ::Star::PlayerTechConstPtr;
using ::Star::PlayerTechWeakPtr;
using ::Star::PlayerTechConstWeakPtr;
using ::Star::PlayerTechUPtr;
using ::Star::PlayerTechConstUPtr;
using ::Star::PlayerTechExceptionTag;
using ::Star::PlayerTechException;
}
