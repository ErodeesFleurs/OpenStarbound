#include "StarCodexItem.hpp"

#include "StarCasting.hpp"
#include "StarCodex.hpp"
#include "StarConfig.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerCodexes.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

CodexItem::CodexItem(Json const& config, String const& directory, Json const& data)
    : Item(config, directory, data), SwingableItem(config) {
  setWindupTime(0.2f);
  setCooldownTime(0.5f);
  m_requireEdgeTrigger = true;
  m_codexId = instanceValue("codexId").toString();
  String iconPath = instanceValue("codexIcon").toString();
  m_iconDrawables = {Drawable::makeImage(iconPath, 1.0f, true, Vec2F())};
  m_worldDrawables = {Drawable::makeImage(iconPath, 1.0f / TilePixels, true, Vec2F())};
}

auto CodexItem::clone() const -> Ptr<Item> {
  return std::make_shared<CodexItem>(*this);
}

auto CodexItem::drawables() const -> List<Drawable> {
  return m_worldDrawables;
}

void CodexItem::fireTriggered() {
  if (auto player = as<Player>(owner())) {
    ConstPtr<Codex> codexLearned = player->codexes()->learnCodex(m_codexId);
    if (codexLearned) {
      player->queueUIMessage(Root::singleton().assets()->json("/codex.config:messages.learned").toString());
    } else {
      player->codexes()->markCodexUnread(m_codexId);
      player->queueUIMessage(Root::singleton().assets()->json("/codex.config:messages.alreadyKnown").toString());
    }
  }
}

auto CodexItem::iconDrawables() const -> List<Drawable> {
  return m_iconDrawables;
}

auto CodexItem::dropDrawables() const -> List<Drawable> {
  return m_worldDrawables;
}

}// namespace Star
