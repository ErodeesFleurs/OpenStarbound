#include "StarBlueprintItem.hpp"
#include "StarJsonExtra.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerBlueprints.hpp"

namespace Star {

BlueprintItem::BlueprintItem(AssetsConstPtr assets, Json const& config, String const& directory, Json const& data)
  : Item(assets, config, directory, data), SwingableItem(config) {
  if (!assets)
    throw ItemException("BlueprintItem requires assets service");

  setWindupTime(0.2f);
  setCooldownTime(0.1f);
  m_requireEdgeTrigger = true;
  m_recipe = ItemDescriptor(instanceValue("recipe"));

  m_recipeIconUnderlay = Drawable(assets->json("/blueprint.config:iconUnderlay"));
  m_inHandDrawable = {Drawable::makeImage(
      assets->json("/blueprint.config:inHandImage").toString(),
      1.0f / TilePixels,
      true,
      Vec2F())};

  setPrice(int(price() * assets->json("/items/defaultParameters.config:blueprintPriceFactor").toFloat()));
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
