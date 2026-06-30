#pragma once

#include "StarItem.hpp"
#include "StarWorld.hpp"
#include "StarSwingableItem.hpp"
#include "StarPreviewableItem.hpp"
#include "StarAssets.hpp"

namespace Star {

class UnlockItem;

class UnlockItem : public Item, public SwingableItem, public PreviewableItem {
public:
  UnlockItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& itemParameters = JsonObject());

  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] List<Drawable> drawables() const override;
  [[nodiscard]] List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

protected:
  void fireTriggered() override;

private:
  AssetsConstPtr m_assets;
  Maybe<String> m_sectorUnlock;
  Maybe<String> m_tierRecipesUnlock;
  Maybe<unsigned> m_shipUpgrade;
  String m_unlockMessage;
  List<Drawable> m_drawables;
};

}
