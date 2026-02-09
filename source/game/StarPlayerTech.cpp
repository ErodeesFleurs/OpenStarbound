#include "StarPlayerTech.hpp"

#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

PlayerTech::PlayerTech() = default;

PlayerTech::PlayerTech(Json const& json) {
  m_availableTechs = jsonToStringSet(json.get("availableTechs"));
  m_enabledTechs = jsonToStringSet(json.get("enabledTechs"));
  auto techDatabase = Root::singleton().techDatabase();
  for (auto& p : json.getObject("equippedTechs")) {
    String techName = p.second.toString();
    if (techDatabase->contains(techName))
      m_equippedTechs.set(TechTypeNames.getLeft(p.first), techName);
    else
      Logger::warn("Unequipping unknown tech '{}' from slot '{}'", techName, p.first);
  }
}

auto PlayerTech::toJson() const -> Json {
  return JsonObject{
    {"availableTechs", jsonFromStringSet(m_availableTechs)},
    {"enabledTechs", jsonFromStringSet(m_enabledTechs)},
    {"equippedTechs", jsonFromMapK(m_equippedTechs, [](TechType t) -> String {
       return TechTypeNames.getRight(t);
     })},
  };
}

auto PlayerTech::isAvailable(String const& techModule) const -> bool {
  return m_availableTechs.contains(techModule);
}

void PlayerTech::makeAvailable(String const& techModule) {
  m_availableTechs.add(techModule);
}

void PlayerTech::makeUnavailable(String const& techModule) {
  disable(techModule);
  m_availableTechs.remove(techModule);
}

auto PlayerTech::isEnabled(String const& techModule) const -> bool {
  return m_enabledTechs.contains(techModule);
}

void PlayerTech::enable(String const& techModule) {
  if (!m_availableTechs.contains(techModule))
    throw PlayerTechException::format("Enabling tech module '{}' when not available", techModule);
  m_enabledTechs.add(techModule);
}

void PlayerTech::disable(String const& techModule) {
  unequip(techModule);
  m_enabledTechs.remove(techModule);
}

auto PlayerTech::isEquipped(String const& techModule) const -> bool {
  for (auto t : m_equippedTechs.keys()) {
    if (m_equippedTechs.get(t) == techModule)
      return true;
  }
  return false;
}

void PlayerTech::equip(String const& techModule) {
  if (!m_enabledTechs.contains(techModule))
    throw PlayerTechException::format("Equipping tech module '{}' when not enabled", techModule);

  auto techDatabase = Root::singleton().techDatabase();
  m_equippedTechs[techDatabase->tech(techModule).type] = techModule;
}

void PlayerTech::unequip(String const& techModule) {
  for (auto t : m_equippedTechs.keys()) {
    if (m_equippedTechs.get(t) == techModule)
      m_equippedTechs.remove(t);
  }
}

auto PlayerTech::availableTechs() const -> StringSet const& {
  return m_availableTechs;
}

auto PlayerTech::enabledTechs() const -> StringSet const& {
  return m_enabledTechs;
}

auto PlayerTech::equippedTechs() const -> HashMap<TechType, String> const& {
  return m_equippedTechs;
}

auto PlayerTech::techModules() const -> StringList {
  return m_equippedTechs.values();
}

}// namespace Star
