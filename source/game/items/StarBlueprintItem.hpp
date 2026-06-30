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
  ItemPtr clone() const override;

  List<Drawable> drawables() const override;

  void fireTriggered() override;

  List<Drawable> iconDrawables() const override;
  List<Drawable> dropDrawables() const override;

private:
  ItemDescriptor m_recipe;
  Drawable m_recipeIconUnderlay;
  List<Drawable> m_inHandDrawable;
};

}
