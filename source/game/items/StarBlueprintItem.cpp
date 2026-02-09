#include "StarBlueprintItem.hpp"

#include "StarCasting.hpp"
#include "StarConfig.hpp"
#include "StarPlayer.hpp"
#include "StarRoot.hpp"

import std;

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

auto BlueprintItem::clone() const -> Ptr<Item> {
  return std::make_shared<BlueprintItem>(*this);
}

auto BlueprintItem::drawables() const -> List<Drawable> {
  return m_inHandDrawable;
}

void BlueprintItem::fireTriggered() {
  if (count())
    if (auto player = as<Player>(owner()))
      if (player->addBlueprint(m_recipe, true))
        setCount(count() - 1);
}

auto BlueprintItem::iconDrawables() const -> List<Drawable> {
  List<Drawable> result;
  result.append(m_recipeIconUnderlay);
  result.appendAll(Item::iconDrawables());
  return result;
}

auto BlueprintItem::dropDrawables() const -> List<Drawable> {
  return m_inHandDrawable;
}

}// namespace Star
