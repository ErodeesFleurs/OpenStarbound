#pragma once

#include <optional>

#include "StarItem.hpp"
#include "StarWorld.hpp"
#include "StarSwingableItem.hpp"
#include "StarPreviewableItem.hpp"

namespace Star {

STAR_CLASS(UnlockItem);

class UnlockItem : public Item, public SwingableItem, public PreviewableItem {
public:
  UnlockItem(Json const& config, String const& directory, Json const& itemParameters = JsonObject());

  ItemPtr clone() const override;

  List<Drawable> drawables() const override;
  List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

protected:
  void fireTriggered() override;

private:
  std::optional<String> m_sectorUnlock;
  std::optional<String> m_tierRecipesUnlock;
  std::optional<unsigned> m_shipUpgrade;
  String m_unlockMessage;
  List<Drawable> m_drawables;
};

}
