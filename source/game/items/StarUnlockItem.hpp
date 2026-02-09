#pragma once

#include "StarConfig.hpp"
#include "StarItem.hpp"
#include "StarPreviewableItem.hpp"
#include "StarSwingableItem.hpp"

import std;

namespace Star {

class UnlockItem : public Item, public SwingableItem, public PreviewableItem {
public:
  UnlockItem(Json const& config, String const& directory, Json const& itemParameters = JsonObject());

  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;
  auto preview(Ptr<Player> const& viewer = {}) const -> List<Drawable> override;

protected:
  void fireTriggered() override;

private:
  std::optional<String> m_sectorUnlock;
  std::optional<String> m_tierRecipesUnlock;
  std::optional<unsigned> m_shipUpgrade;
  String m_unlockMessage;
  List<Drawable> m_drawables;
};

}// namespace Star
