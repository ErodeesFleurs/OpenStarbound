#pragma once

#include "StarException.hpp"
#include "StarTechDatabase.hpp"

namespace Star {

using PlayerTechException = ExceptionDerived<"PlayerTechException">;

// Set of player techs, techs can be either unavailable, available but not
// enabled, enabled but not equipped, or equipped.
class PlayerTech {
public:
  PlayerTech();
  PlayerTech(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto isAvailable(String const& techModule) const -> bool;
  void makeAvailable(String const& techModule);
  void makeUnavailable(String const& techModule);

  [[nodiscard]] auto isEnabled(String const& techModule) const -> bool;
  void enable(String const& techModule);
  void disable(String const& techModule);

  [[nodiscard]] auto isEquipped(String const& techModule) const -> bool;
  void equip(String const& techModule);
  void unequip(String const& techModule);

  [[nodiscard]] auto availableTechs() const -> StringSet const&;
  [[nodiscard]] auto enabledTechs() const -> StringSet const&;
  [[nodiscard]] auto equippedTechs() const -> HashMap<TechType, String> const&;
  [[nodiscard]] auto techModules() const -> StringList;

private:
  StringSet m_availableTechs;
  StringSet m_enabledTechs;
  HashMap<TechType, String> m_equippedTechs;
};

}// namespace Star
