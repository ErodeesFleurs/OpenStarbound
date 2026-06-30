#include "StarObjectItem.hpp"
#include "StarObject.hpp"
#include "StarLogging.hpp"
#include "StarObjectDatabase.hpp"
#include "StarWorld.hpp"
#include "StarJsonExtra.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

ObjectItem::ObjectItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& objectParameters, ObjectDatabaseConstPtr objectDatabase)
  : Item(assets, imageMetadataDatabase, config, directory, objectParameters), FireableItem(config), BeamItem(std::move(assets), std::move(imageMetadataDatabase), config), m_objectDatabase(requireServiceValueAs<ItemException>(std::move(objectDatabase), "ObjectItem", "object database")) {
  setTwoHanded(config.getBool("twoHanded", true));

  // Make sure that all script objects that have retainObjectParametersInItem
  // start with a blank scriptStorage entry to help them stack properly.
  if (instanceValue("retainObjectParametersInItem", false).toBool() && instanceValue("scriptStorage").isNull())
    setInstanceValue("scriptStorage", JsonObject());
  m_shifting = false;
}

ItemPtr ObjectItem::clone() const {
  return make_shared<ObjectItem>(*this);
}

void ObjectItem::init(ToolUserEntity& owner, ToolHand hand) {
  FireableItem::init(owner, hand);
  BeamItem::init(owner, hand);
}

void ObjectItem::update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) {
  FireableItem::update(dt, fireMode, shifting, moves);
  BeamItem::update(dt, fireMode, shifting, moves);
  setEnd(BeamItem::EndType::Object);
  m_shifting = shifting;
}

[[nodiscard]] List<Drawable> ObjectItem::nonRotatedDrawables() const {
  return beamDrawables(canPlace(m_shifting));
}

[[nodiscard]] float ObjectItem::cooldownTime() const {
  // TODO: Hardcoded
  return 0.25f;
}

void ObjectItem::fire(FireMode mode, bool shifting, bool edgeTriggered) {
  if (!ready())
    return;

  if (placeInWorld(mode, shifting))
    FireableItem::fire(mode, shifting, edgeTriggered);
}

[[nodiscard]] String ObjectItem::objectName() const {
  return instanceValue("objectName", "<objectName missing>").toString();
}

[[nodiscard]] Json ObjectItem::objectParameters() const {
  Json objectParameters = parameters().opt().value(JsonObject{});
  if (!initialized())
    return objectParameters;
  return objectParameters.set("owner", jsonFromMaybe(owner().uniqueId()));
}

bool ObjectItem::placeInWorld(FireMode, bool shifting) {
  if (!initialized())
    throw ItemException("ObjectItem not init'd properly, or user not recognized as Tool User.");

  if (!ready())
    return false;

  if (!canPlace(shifting))
    return false;

  auto pos = Vec2I(owner().aimPosition().floor());
  try {
    if (auto object = m_objectDatabase->createForPlacement(*world(), objectName(), pos, owner().walkingDirection(), objectParameters())) {
      if (consume(1)) {
        world()->addEntity(object);
        return true;
      }
    }
  } catch (StarException const& e) {
    Logger::error("Failed to instantiate object for placement. {} {} : {}",
        objectName(),
        objectParameters().repr(0, true),
        outputException(e, true));
    return true;
  }

  return false;
}

[[nodiscard]] bool ObjectItem::canPlace(bool) const {
  if (initialized()) {
    if (owner().isAdmin() || owner().inToolRange()) {
      auto pos = Vec2I(owner().aimPosition().floor());
      return m_objectDatabase->canPlaceObject(*world(), pos, objectName());
    }
  }
  return false;
}

}
