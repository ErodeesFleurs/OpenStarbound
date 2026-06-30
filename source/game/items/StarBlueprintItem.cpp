#include "StarBlueprintItem.hpp"
#include "StarJsonExtra.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerBlueprints.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

BlueprintItem::BlueprintItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& data)
  : Item(assets, std::move(imageMetadataDatabase), config, directory, data), SwingableItem(config) {
  assets = requireServiceValueAs<ItemException>(std::move(assets), "BlueprintItem", "assets");

  setWindupTime(0.2f);
  setCooldownTime(0.1f);
  m_requireEdgeTrigger = true;
  m_recipe = ItemDescriptor(instanceValue("recipe"));

  m_recipeIconUnderlay = Drawable(assets->json("/blueprint.config:iconUnderlay"), m_imageMetadataDatabase);
  m_inHandDrawable = {Drawable::makeImage(
      assets->json("/blueprint.config:inHandImage").toString(),
      1.0f / TilePixels,
      true,
      Vec2F(),
      Color::White,
      m_imageMetadataDatabase)};

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
