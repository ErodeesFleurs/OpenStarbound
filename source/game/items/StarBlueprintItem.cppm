module;

#include "StarItem.hpp"
#include "StarWorld.hpp"
#include "StarSwingableItem.hpp"

#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"
#include "StarPlayer.hpp"
#include "StarAssets.hpp"
#include "StarPlayerBlueprints.hpp"

export module star.blueprint_item;

export namespace Star {

STAR_CLASS(BlueprintItem);

class BlueprintItem : public Item, public SwingableItem {
public:
  BlueprintItem(Json const& config, String const& directory, Json const& data);
  virtual ItemPtr clone() const override;

  virtual List<Drawable> drawables() const override;

  virtual void fireTriggered() override;

  virtual List<Drawable> iconDrawables() const override;
  virtual List<Drawable> dropDrawables() const override;

private:
  ItemDescriptor m_recipe;
  Drawable m_recipeIconUnderlay;
  List<Drawable> m_inHandDrawable;
};

}

namespace Star {

BlueprintItem::BlueprintItem(Json const& config, String const& directory, Json const& data)
  : Item(config, directory, data), SwingableItem(config) {
  setWindupTime(0.2f);
  setCooldownTime(0.1f);
  m_requireEdgeTrigger = true;
  m_recipe = ItemDescriptor(instanceValue("recipe"));

  m_recipeIconUnderlay = Drawable(Root::singleton().assets()->json("/blueprint.config:iconUnderlay"));
  m_inHandDrawable = {Drawable::makeImage(
      Root::singleton().assets()->json("/blueprint.config:inHandImage").toString(),
      1.0f / TilePixels,
      true,
      Vec2F())};

  setPrice(int(price() * Root::singleton().assets()->json("/items/defaultParameters.config:blueprintPriceFactor").toFloat()));
}

ItemPtr BlueprintItem::clone() const {
  return make_shared<BlueprintItem>(*this);
}

List<Drawable> BlueprintItem::drawables() const {
  return m_inHandDrawable;
}

void BlueprintItem::fireTriggered() {
  if (count())
    if (auto player = as<Player>(owner()))
      if (player->addBlueprint(m_recipe, true))
        setCount(count() - 1);
}

List<Drawable> BlueprintItem::iconDrawables() const {
  List<Drawable> result;
  result.append(m_recipeIconUnderlay);
  result.appendAll(Item::iconDrawables());
  return result;
}

List<Drawable> BlueprintItem::dropDrawables() const {
  return m_inHandDrawable;
}

}
