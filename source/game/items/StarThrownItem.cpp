#include "StarThrownItem.hpp"
#include "StarAssets.hpp"
#include "StarProjectile.hpp"
#include "StarProjectileDatabase.hpp"
#include "StarWorld.hpp"
#include "StarWorldServer.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

ThrownItem::ThrownItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& itemParameters)
    : Item(std::move(assets), std::move(imageMetadataDatabase), config, directory, itemParameters), SwingableItem(config) {
  m_projectileType = instanceValue("projectileType").toString();
  m_projectileConfig = instanceValue("projectileConfig", {});
  m_ammoUsage = instanceValue("ammoUsage", 1).toUInt();

  auto image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_drawables = {Drawable::makeImage(image, 1.0f / TilePixels, true, Vec2F(), Color::White, m_imageMetadataDatabase)};
}

ItemPtr ThrownItem::clone() const {
  return make_shared<ThrownItem>(*this);
}

List<Drawable> ThrownItem::drawables() const {
  return m_drawables;
}

List<Drawable> ThrownItem::preview(PlayerPtr const&) const {
  return iconDrawables();
}

void ThrownItem::fireTriggered() {
  if (initialized()) {
    Vec2F direction = world()->geometry().diff(owner()->aimPosition(), owner()->position()).normalized();
    Vec2F firePosition = owner()->position() + ownerFirePosition();
    if (world()->lineTileCollision(owner()->position(), firePosition))
      return;

    if (consume(m_ammoUsage)) {
      auto worldServer = as<WorldServer>(world());
      requireDependencyAs<ItemException>(worldServer, "Thrown item", "server world projectile database");
      auto projectileDb = worldServer->projectileDatabase();
      auto projectile = projectileDb->createProjectile(m_projectileType, m_projectileConfig);
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
