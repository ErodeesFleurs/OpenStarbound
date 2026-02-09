#pragma once

#include "StarConfig.hpp"
#include "StarItem.hpp"
#include "StarSwingableItem.hpp"

namespace Star {

class BlueprintItem : public Item, public SwingableItem {
public:
  BlueprintItem(Json const& config, String const& directory, Json const& data);
  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;

  void fireTriggered() override;

  auto iconDrawables() const -> List<Drawable> override;
  auto dropDrawables() const -> List<Drawable> override;

private:
  ItemDescriptor m_recipe;
  Drawable m_recipeIconUnderlay;
  List<Drawable> m_inHandDrawable;
};

}// namespace Star
