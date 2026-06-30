#pragma once

#include "StarTechDatabase.hpp"

namespace Star {

struct PlayerTechExceptionTag { static constexpr char const* typeName = "PlayerTechException"; };
using PlayerTechException = TypedException<StarException, PlayerTechExceptionTag>;

class PlayerTech;
using PlayerTechPtr = SharedPtr<PlayerTech>;

// Set of player techs, techs can be either unavailable, available but not
// enabled, enabled but not equipped, or equipped.
class PlayerTech {
public:
  PlayerTech(TechDatabaseConstPtr techDatabase);
  PlayerTech(Json const& json, TechDatabaseConstPtr techDatabase);

  [[nodiscard]] Json toJson() const;

  [[nodiscard]] bool isAvailable(String const& techModule) const;
  void makeAvailable(String const& techModule);
  void makeUnavailable(String const& techModule);

  [[nodiscard]] bool isEnabled(String const& techModule) const;
  void enable(String const& techModule);
  void disable(String const& techModule);

  [[nodiscard]] bool isEquipped(String const& techModule) const;
  void equip(String const& techModule);
  void unequip(String const& techModule);

  [[nodiscard]] StringSet const& availableTechs() const;
  [[nodiscard]] StringSet const& enabledTechs() const;
  [[nodiscard]] HashMap<TechType, String> const& equippedTechs() const;
  [[nodiscard]] StringList techModules() const;

private:
  TechDatabaseConstPtr m_techDatabase;
  StringSet m_availableTechs;
  StringSet m_enabledTechs;
  HashMap<TechType, String> m_equippedTechs;
};

}
