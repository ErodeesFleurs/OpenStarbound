#pragma once

#include "StarItem.hpp"
#include "StarDrawable.hpp"
#include "StarSwingableItem.hpp"
#include "StarPreviewableItem.hpp"
#include "StarAssets.hpp"

namespace Star {

class ThrownItem : public Item, public SwingableItem, public PreviewableItem {
public:
  ThrownItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& itemParameters = JsonObject());

  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] List<Drawable> drawables() const override;
  [[nodiscard]] List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

protected:
  void fireTriggered() override;

private:
  String m_projectileType;
  Json m_projectileConfig;
  size_t m_ammoUsage;
  List<Drawable> m_drawables;
};

}
