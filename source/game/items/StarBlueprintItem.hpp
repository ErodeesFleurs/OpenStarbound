#pragma once

#include "StarItem.hpp"
#include "StarWorld.hpp"
#include "StarSwingableItem.hpp"
#include "StarAssets.hpp"

namespace Star {

class BlueprintItem;

class BlueprintItem : public Item, public SwingableItem {
public:
  BlueprintItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& data);
  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] List<Drawable> drawables() const override;

  void fireTriggered() override;

  [[nodiscard]] List<Drawable> iconDrawables() const override;
  [[nodiscard]] List<Drawable> dropDrawables() const override;

private:
  ItemDescriptor m_recipe;
  Drawable m_recipeIconUnderlay;
  List<Drawable> m_inHandDrawable;
};

}
