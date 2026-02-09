#include "StarThrownItem.hpp"

#include "StarConfig.hpp"
#include "StarProjectileDatabase.hpp"// IWYU pragma: export
#include "StarRoot.hpp"
#include "StarWorld.hpp"

import std;

namespace Star {

ThrownItem::ThrownItem(Json const& config, String const& directory, Json const& itemParameters)
    : Item(config, directory, itemParameters), SwingableItem(config) {
  m_projectileType = instanceValue("projectileType").toString();
  m_projectileConfig = instanceValue("projectileConfig", {});
  m_ammoUsage = instanceValue("ammoUsage", 1).toUInt();

  auto image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_drawables = {Drawable::makeImage(image, 1.0f / TilePixels, true, Vec2F())};
}

auto ThrownItem::clone() const -> Ptr<Item> {
  return std::make_shared<ThrownItem>(*this);
}

auto ThrownItem::drawables() const -> List<Drawable> {
  return m_drawables;
}

auto ThrownItem::preview(Ptr<Player> const&) const -> List<Drawable> {
  return iconDrawables();
}

void ThrownItem::fireTriggered() {
  auto& root = Root::singleton();

  if (initialized()) {
    Vec2F direction = world()->geometry().diff(owner()->aimPosition(), owner()->position()).normalized();
    Vec2F firePosition = owner()->position() + ownerFirePosition();
    if (world()->lineTileCollision(owner()->position(), firePosition))
      return;

    if (consume(m_ammoUsage)) {
      auto projectile = root.projectileDatabase()->createProjectile(m_projectileType, m_projectileConfig);
      projectile->setInitialPosition(firePosition);
      projectile->setInitialDirection(direction);
      projectile->setSourceEntity(owner()->entityId(), false);
      projectile->setPowerMultiplier(owner()->powerMultiplier());
      world()->addEntity(projectile);
    }

    FireableItem::fireTriggered();
  } else {
    throw ItemException("Thrown item not init'd properly, or user not recognized as Tool User.");
  }
}

}// namespace Star
