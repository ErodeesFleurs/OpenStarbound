#include "StarVehicle.hpp"
#include "StarAlgorithm.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarEntityLuaBindings.hpp"
#include "StarEntityRendering.hpp"
#include "StarJsonExtra.hpp"
#include "StarLuaGameConverters.hpp"
#include "StarMovementControllerLuaBindings.hpp"
#include "StarNetworkedAnimatorLuaBindings.hpp"
#include "StarPlayer.hpp"
#include "StarPythonic.hpp"
#include "StarScriptedAnimatorLuaBindings.hpp"

namespace Star {

Vehicle::Vehicle(AssetsConstPtr assets, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json baseConfig, String path, Json dynamicConfig)
    : m_baseConfig(std::move(baseConfig)),
      m_particleDatabase(requireServiceValueAs<StarException>(std::move(particleDatabase), "Vehicle", "particle database")),
      m_imageMetadataDatabase(requireServiceValueAs<StarException>(std::move(imageMetadataDatabase), "Vehicle", "image metadata database")),
      m_path(std::move(path)),
      m_dynamicConfig(std::move(dynamicConfig)),
      m_movementController(MovementParameters(), requireServiceValueAs<StarException>(assets, "Vehicle", "assets")),
      m_scriptedAnimator(requireServiceValueAs<StarException>(assets, "Vehicle", "assets")) {
  assets = requireServiceValueAs<StarException>(std::move(assets), "Vehicle", "assets");

  m_typeName = m_baseConfig.getString("name");

  setPersistent(configValue("persistent", false).toBool());
  m_clientEntityMode = ClientEntityModeNames.getLeft(configValue("clientEntityMode", "ClientSlaveOnly").toString());

  m_scriptComponent.setScript(AssetPath::relativeTo(m_path, configValue("script").toString()));
  m_scriptComponent.setUpdateDelta(configValue("scriptDelta", 1).toUInt());
  m_boundBox = jsonToRectF(configValue("boundBox"));
  m_slaveControlTimeout = configValue("slaveControlTimeout").toFloat();
  m_receiveExtraControls = configValue("receiveExtraControls", false).toBool();
  m_slaveHeartbeatTimer = GameTimer(configValue("slaveControlHeartbeat").toFloat());
  m_damageTeam.set(configValue("damageTeam").opt().apply(construct<EntityDamageTeam>()).value());
  m_interactive.set(configValue("interactive", true).toBool());
  m_baseRenderLayer = parseRenderLayer(configValue("baseRenderLayer", "Vehicle").toString());
  if (!configValue("overrideRenderLayer").isNull()) {
    m_overrideRenderLayer = parseRenderLayer(configValue("overrideRenderLayer").toString());
  }

  if (auto animationScript = configValue("animationScript").optString())
    m_scriptedAnimator.setScript(*animationScript);

  for (auto const& [loungePositionName, loungePositionConfig] : configValue("loungePositions", JsonObject()).iterateObject()) {
    auto& loungePosition = m_loungePositions[loungePositionName];
    loungePosition.part = loungePositionConfig.getString("part");
    loungePosition.partAnchor = loungePositionConfig.getString("partAnchor");
    loungePosition.exitBottomOffset = loungePositionConfig.opt("exitBottomOffset").apply(jsonToVec2F);
    loungePosition.armorCosmeticOverrides = loungePositionConfig.getObject("armorCosmeticOverrides", JsonObject());
    loungePosition.cursorOverride = loungePositionConfig.optString("cursorOverride");
    loungePosition.cameraFocus = loungePositionConfig.getBool("cameraFocus", false);
    loungePosition.enabled.set(loungePositionConfig.getBool("enabled", true));
    if (auto orientation = loungePositionConfig.optString("orientation"))
      loungePosition.orientation.set(LoungeOrientationNames.getLeft(*orientation));
    loungePosition.emote.set(loungePositionConfig.optString("emote"));
    loungePosition.dance.set(loungePositionConfig.optString("dance"));
    loungePosition.directives.set(loungePositionConfig.optString("directives"));
    loungePosition.statusEffects.set(loungePositionConfig.getArray("statusEffects", {}).transformed(jsonToPersistentStatusEffect));
    loungePosition.suppressTools = loungePositionConfig.optBool("suppressTools");
  }

  for (auto const& [collisionName, collisionJson] : configValue("physicsCollisions", JsonObject()).iterateObject()) {
    auto& collisionConfig = m_movingCollisions[collisionName];
    collisionConfig.movingCollision = PhysicsMovingCollision::fromJson(collisionJson);
    collisionConfig.attachToPart = collisionJson.optString("attachToPart");
    collisionConfig.enabled.set(collisionJson.getBool("enabled", true));
  }

  for (auto const& [forceName, forceJson] : configValue("physicsForces", JsonObject()).iterateObject()) {
    auto& forceRegionConfig = m_forceRegions[forceName];
    forceRegionConfig.forceRegion = jsonToPhysicsForceRegion(forceJson);
    forceRegionConfig.attachToPart = forceJson.optString("attachToPart");
    forceRegionConfig.enabled.set(forceJson.getBool("enabled", true));
  }

  for (auto const& [damageSourceName, damageSourceJson] : configValue("damageSources", JsonObject()).iterateObject()) {
    auto& damageSourceConfig = m_damageSources[damageSourceName];
    damageSourceConfig.damageSource = DamageSource(damageSourceJson);
    damageSourceConfig.attachToPart = damageSourceJson.optString("attachToPart");
    damageSourceConfig.enabled.set(damageSourceJson.getBool("enabled", true));
  }

  auto animationConfig = assets->fetchJson(configValue("animation"), m_path);
  if (auto customConfig = configValue("animationCustom"))
    animationConfig = jsonMerge(animationConfig, customConfig);

  m_networkedAnimator = NetworkedAnimator(animationConfig, m_path, assets, m_imageMetadataDatabase, m_particleDatabase);

  for (auto const& [tagName, tagValue] : configValue("animationGlobalTags", JsonObject()).iterateObject())
    m_networkedAnimator.setGlobalTag(tagName, tagValue.toString());
  for (auto const& [partName, partTags] : configValue("animationPartTags", JsonObject()).iterateObject()) {
    for (auto const& [tagName, tagValue] : partTags.iterateObject())
      m_networkedAnimator.setPartTag(partName, tagName, tagValue.toString());
  }

  auto movementParameters = MovementParameters(configValue("movementSettings"));
  if (!movementParameters.physicsEffectCategories)
    movementParameters.physicsEffectCategories = StringSet({"vehicle"});
  m_movementController.resetParameters(movementParameters);

  m_netGroup.addNetElement(&m_interactive);
  m_netGroup.addNetElement(&m_movementController);
  m_netGroup.addNetElement(&m_networkedAnimator);
  m_netGroup.addNetElement(&m_damageTeam);

  m_loungePositions.sortByKey();
  for (auto& [loungePositionId, loungePosition] : m_loungePositions) {
    m_netGroup.addNetElement(&loungePosition.enabled);
    m_netGroup.addNetElement(&loungePosition.orientation);
    m_netGroup.addNetElement(&loungePosition.emote);
    m_netGroup.addNetElement(&loungePosition.dance);
    m_netGroup.addNetElement(&loungePosition.directives);
    m_netGroup.addNetElement(&loungePosition.statusEffects);
  }

  m_movingCollisions.sortByKey();
  for (auto& [collisionName, collision] : m_movingCollisions)
    m_netGroup.addNetElement(&collision.enabled);

  m_forceRegions.sortByKey();
  for (auto& [forceRegionName, forceRegion] : m_forceRegions)
    m_netGroup.addNetElement(&forceRegion.enabled);

  m_damageSources.sortByKey();
  for (auto& [damageSourceName, damageSource] : m_damageSources)
    m_netGroup.addNetElement(&damageSource.enabled);

  // don't interpolate scripted animation parameters
  m_netGroup.addNetElement(&m_scriptedAnimationParameters, false);
}

String Vehicle::name() const {
  return m_typeName;
}

Json Vehicle::baseConfig() const {
  return m_baseConfig;
}

Json Vehicle::dynamicConfig() const {
  return m_dynamicConfig;
}

Json Vehicle::diskStore() const {
  return JsonObject{
    {"movement", m_movementController.storeState()},
    {"damageTeam", m_damageTeam.get().toJson()},
    {"persistent", persistent()},
    {"scriptStorage", m_scriptComponent.getScriptStorage()}};
}

void Vehicle::diskLoad(Json diskStore) {
  m_movementController.loadState(diskStore.get("movement"));
  m_damageTeam.set(EntityDamageTeam(diskStore.get("damageTeam")));
  setPersistent(diskStore.getBool("persistent"));
  m_scriptComponent.setScriptStorage(diskStore.getObject("scriptStorage"));
}

EntityType Vehicle::entityType() const {
  return EntityType::Vehicle;
}

ClientEntityMode Vehicle::clientEntityMode() const {
  return m_clientEntityMode;
}

Maybe<HitType> Vehicle::queryHit(DamageSource const& source) const {
  if (source.intersectsWithPoly(world()->geometry(), m_movementController.collisionBody()))
    return HitType::Hit;

  return {};
}

Maybe<PolyF> Vehicle::hitPoly() const {
  return m_movementController.collisionBody();
}

List<DamageNotification> Vehicle::applyDamage(DamageRequest const& damage) {
  if (!inWorld())
    return {};

  return m_scriptComponent.invoke<List<DamageNotification>>("applyDamage", damage).value();
}

List<DamageNotification> Vehicle::selfDamageNotifications() {
  return m_scriptComponent.invoke<List<DamageNotification>>("selfDamageNotifications").value();
}

void Vehicle::init(World* world, EntityId entityId, EntityMode mode) {
  Entity::init(world, entityId, mode);
  m_movementController.init(*world);
  m_movementController.setIgnorePhysicsEntities({entityId});
  if (isMaster()) {
    m_scriptComponent.addCallbacks("vehicle", makeVehicleCallbacks());
    m_scriptComponent.addCallbacks(
      "config", LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) { return configValue(name, def); }));
    m_scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(*this));
    m_scriptComponent.addCallbacks("mcontroller", LuaBindings::makeMovementControllerCallbacks(m_movementController));
    m_scriptComponent.addCallbacks("animator", LuaBindings::makeNetworkedAnimatorCallbacks(m_networkedAnimator));
    m_scriptComponent.init(*world);
  } else {
    m_slaveHeartbeatTimer.reset();
  }

  if (world->isClient()) {
    m_scriptedAnimator.addCallbacks("animationConfig", LuaBindings::makeScriptedAnimatorCallbacks(m_networkedAnimator, [this](String const& name, Json const& defaultValue) -> Json {
                                      return m_scriptedAnimationParameters.value(name, defaultValue);
                                    }));
    m_scriptedAnimator.addCallbacks("config", LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) {
                                      return configValue(name, def);
                                    }));
    m_scriptedAnimator.addCallbacks("entity", LuaBindings::makeEntityCallbacks(*this));

    m_scriptedAnimator.init(*world);
  }
}

void Vehicle::uninit() {
  m_scriptComponent.uninit();
  m_scriptComponent.removeCallbacks("vehicle");
  m_scriptComponent.removeCallbacks("config");
  m_scriptComponent.removeCallbacks("entity");
  m_scriptComponent.removeCallbacks("mcontroller");
  m_scriptComponent.removeCallbacks("animator");
  m_movementController.uninit();

  if (world()->isClient()) {
    m_scriptedAnimator.removeCallbacks("animationConfig");
    m_scriptedAnimator.removeCallbacks("config");
    m_scriptedAnimator.removeCallbacks("entity");
  }

  Entity::uninit();
}

Vec2F Vehicle::position() const {
  return m_movementController.position();
}

RectF Vehicle::metaBoundBox() const {
  return m_boundBox;
}

RectF Vehicle::collisionArea() const {
  return m_movementController.collisionPoly().boundBox();
}

Vec2F Vehicle::velocity() const {
  return m_movementController.velocity();
}

pair<ByteArray, uint64_t> Vehicle::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void Vehicle::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
}

void Vehicle::enableInterpolation(float extrapolationHint) {
  m_netGroup.enableNetInterpolation(extrapolationHint);
}

void Vehicle::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

void Vehicle::update(float dt, uint64_t) {
  setTeam(m_damageTeam.get());

  if (world()->isClient()) {
    m_networkedAnimator.update(dt, &m_networkedAnimatorDynamicTarget);
    m_networkedAnimatorDynamicTarget.updatePosition(position());
  } else {
    m_networkedAnimator.update(dt, nullptr);
  }

  if (isMaster()) {
    m_movementController.tickMaster(dt);
    m_scriptComponent.update(m_scriptComponent.updateDt(dt));

    eraseWhere(m_aliveMasterConnections, [](auto& connectionTimer) {
      auto& [connectionId, timer] = connectionTimer;
      return timer.tick(GlobalTimestep);
    });

    for (auto& [loungePositionId, loungePosition] : m_loungePositions) {
      for (auto& [control, controlState] : loungePosition.masterControlState) {
        controlState.masterHeld = false;
        filter(controlState.slavesHeld, [this](ConnectionId id) {
          return m_aliveMasterConnections.contains(id);
        });
      }
    }
  } else {
    m_netGroup.tickNetInterpolation(dt);

    m_movementController.tickSlave(dt);

    bool heartbeat = m_slaveHeartbeatTimer.wrapTick();

    for (auto& [loungePositionId, loungePosition] : m_loungePositions) {
      if (heartbeat) {
        JsonArray allControlsHeld;
        for (LoungeControl control : loungePosition.slaveNewControls) {
          if (control > LoungeControl::Special3 && !m_receiveExtraControls)
            continue;
          allControlsHeld.append(LoungeControlNames.getRight(control));
        }
        world()->sendEntityMessage(entityId(), "control_all", {*m_loungePositions.indexOf(loungePositionId), std::move(allControlsHeld)});
      } else {
        for (auto control : loungePosition.slaveNewControls.difference(loungePosition.slaveOldControls)) {
          if (control > LoungeControl::Special3 && !m_receiveExtraControls)
            continue;
          world()->sendEntityMessage(entityId(), "control_on", {*m_loungePositions.indexOf(loungePositionId), LoungeControlNames.getRight(control)});
        }
        for (auto control : loungePosition.slaveOldControls.difference(loungePosition.slaveNewControls)) {
          if (control > LoungeControl::Special3 && !m_receiveExtraControls)
            continue;
          world()->sendEntityMessage(entityId(), "control_off", {*m_loungePositions.indexOf(loungePositionId), LoungeControlNames.getRight(control)});
        }
      }

      if (loungePosition.slaveOldAimPosition != loungePosition.slaveNewAimPosition)
        world()->sendEntityMessage(entityId(), "aim", {*m_loungePositions.indexOf(loungePositionId), loungePosition.slaveNewAimPosition[0], loungePosition.slaveNewAimPosition[1]});

      loungePosition.slaveOldControls = take(loungePosition.slaveNewControls);
      loungePosition.slaveOldAimPosition = loungePosition.slaveNewAimPosition;
    }
  }

  if (world()->isClient())
    m_scriptedAnimator.update();

  if (world()->isClient())
    SpatialLogger::logPoly("world", m_movementController.collisionBody(), {255, 255, 0, 255});
}

void Vehicle::render(RenderCallback* renderer) {
  for (auto& [drawable, zLevel] : m_networkedAnimator.drawablesWithZLevel(position())) {
    if (zLevel < 0.0f)
      renderer->addDrawable(std::move(drawable), renderLayer(VehicleLayer::Back));
    else
      renderer->addDrawable(std::move(drawable), renderLayer(VehicleLayer::Front));
  }

  renderer->addAudios(m_networkedAnimatorDynamicTarget.pullNewAudios());
  renderer->addParticles(m_networkedAnimatorDynamicTarget.pullNewParticles());

  for (auto const& [drawable, maybeRenderLayer] : m_scriptedAnimator.drawables())
    renderer->addDrawable(drawable, maybeRenderLayer.value(renderLayer(VehicleLayer::Front)));
  renderer->addAudios(m_scriptedAnimator.pullNewAudios());
  renderer->addParticles(m_scriptedAnimator.pullNewParticles());
}

void Vehicle::renderLightSources(RenderCallback* renderer) {
  renderer->addLightSources(m_networkedAnimator.lightSources(position()));
  renderer->addLightSources(m_scriptedAnimator.lightSources());
}

List<LightSource> Vehicle::lightSources() const {
  auto lightSources = m_networkedAnimator.lightSources(position());
  return lightSources;
}

bool Vehicle::shouldDestroy() const {
  return m_shouldDestroy;
}

void Vehicle::destroy(RenderCallback* renderCallback) {
  if (renderCallback) {
    m_networkedAnimator.update(0.0, &m_networkedAnimatorDynamicTarget);

    renderCallback->addAudios(m_networkedAnimatorDynamicTarget.pullNewAudios());
    renderCallback->addParticles(m_networkedAnimatorDynamicTarget.pullNewParticles());
  }
}

Maybe<Json> Vehicle::receiveMessage(ConnectionId connectionId, String const& message, JsonArray const& args) {
  m_aliveMasterConnections[connectionId] = GameTimer(m_slaveControlTimeout);
  if (message.equalsIgnoreCase("control_on")) {
    auto& loungePosition = m_loungePositions.valueAt(args.at(0).toUInt());
    loungePosition.masterControlState[LoungeControlNames.getLeft(args.at(1).toString())].slavesHeld.add(connectionId);
    return Json();
  } else if (message.equalsIgnoreCase("control_off")) {
    auto& loungePosition = m_loungePositions.valueAt(args.at(0).toUInt());
    loungePosition.masterControlState[LoungeControlNames.getLeft(args.at(1).toString())].slavesHeld.remove(connectionId);
    return Json();
  } else if (message.equalsIgnoreCase("control_all")) {
    auto& loungePosition = m_loungePositions.valueAt(args.at(0).toUInt());
    Set<LoungeControl> allControlsHeld;
    for (auto const& s : args.at(1).iterateArray())
      allControlsHeld.add(LoungeControlNames.getLeft(s.toString()));
    for (auto& [control, controlState] : loungePosition.masterControlState) {
      if (allControlsHeld.contains(control))
        controlState.slavesHeld.add(connectionId);
      else
        controlState.slavesHeld.remove(connectionId);
    }
    return Json();
  } else if (message.equalsIgnoreCase("aim")) {
    auto& loungePosition = m_loungePositions.valueAt(args.at(0).toUInt());
    loungePosition.masterAimPosition = {args.at(1).toFloat(), args.at(2).toFloat()};
    return Json();
  } else {
    return m_scriptComponent.handleMessage(message, connectionId == world()->connection(), args);
  }
}

RectF Vehicle::interactiveBoundBox() const {
  return collisionArea();
}

bool Vehicle::isInteractive() const {
  return m_interactive.get();
}

InteractAction Vehicle::interact(InteractRequest const& request) {
  auto result = m_scriptComponent.invoke<Json>("onInteraction", JsonObject{{"sourceId", request.sourceId}, {"sourcePosition", jsonFromVec2F(request.sourcePosition)}, {"interactPosition", jsonFromVec2F(request.interactPosition)}}).value();

  if (result.isType(Json::Type::String))
    return InteractAction(result.toString(), entityId(), Json());
  else if (!result.isNull())
    return InteractAction(result.getString(0), entityId(), result.get(1));

  Maybe<size_t> index;
  float bestDistance = 0.0f;
  for (auto const& [loungeEntry, loungeIndex] : enumerateIterator(m_loungePositions)) {
    auto const& [_, lounge] = loungeEntry;
    if (!lounge.enabled.get())
      continue;

    Vec2F loungePosition = *m_networkedAnimator.partPoint(lounge.part, lounge.partAnchor) + position();
    float distance = vmagSquared(loungePosition - request.interactPosition);
    if (!index || distance < bestDistance) {
      index = loungeIndex;
      bestDistance = distance;
    }
  }

  if (index)
    return InteractAction(InteractActionType::SitDown, entityId(), *index);

  return InteractAction();
}

size_t Vehicle::anchorCount() const {
  return m_loungePositions.size();
}

LoungeAnchorConstPtr Vehicle::loungeAnchor(size_t positionIndex) const {
  auto const& positionConfig = m_loungePositions.valueAt(positionIndex);
  if (!positionConfig.enabled.get())
    return {};

  Mat3F partTransformation = m_networkedAnimator.finalPartTransformation(positionConfig.part);
  Vec2F partAnchor = jsonToVec2F(m_networkedAnimator.partProperty(positionConfig.part, positionConfig.partAnchor));

  auto loungePosition = make_shared<LoungeAnchor>();
  loungePosition->position = partTransformation.transformVec2(partAnchor) + position();
  if (positionConfig.exitBottomOffset)
    loungePosition->exitBottomPosition = partTransformation.transformVec2(partAnchor + positionConfig.exitBottomOffset.value()) + position();
  loungePosition->direction = partTransformation.determinant() > 0 ? Direction::Right : Direction::Left;
  loungePosition->angle = partTransformation.transformAngle(0.0f);
  if (loungePosition->direction == Direction::Left)
    loungePosition->angle += Constants::pi;
  loungePosition->controllable = true;
  loungePosition->loungeRenderLayer = renderLayer(VehicleLayer::Passenger);
  loungePosition->orientation = positionConfig.orientation.get();
  loungePosition->emote = positionConfig.emote.get();
  loungePosition->dance = positionConfig.dance.get();
  loungePosition->directives = positionConfig.directives.get();
  loungePosition->statusEffects = positionConfig.statusEffects.get();
  loungePosition->armorCosmeticOverrides = positionConfig.armorCosmeticOverrides;
  loungePosition->cursorOverride = positionConfig.cursorOverride;
  loungePosition->cameraFocus = positionConfig.cameraFocus;
  loungePosition->suppressTools = positionConfig.suppressTools;
  return loungePosition;
}

void Vehicle::loungeControl(size_t index, LoungeControl loungeControl) {
  auto& loungePosition = m_loungePositions.valueAt(index);
  if (isSlave())
    loungePosition.slaveNewControls.add(loungeControl);
  else
    loungePosition.masterControlState[loungeControl].masterHeld = true;
}

void Vehicle::loungeAim(size_t index, Vec2F const& aimPosition) {
  auto& loungePosition = m_loungePositions.valueAt(index);
  if (isSlave())
    loungePosition.slaveNewAimPosition = aimPosition;
  else
    loungePosition.masterAimPosition = aimPosition;
}

List<PhysicsForceRegion> Vehicle::forceRegions() const {
  List<PhysicsForceRegion> forces;
  for (auto const& [_, forceRegionConfig] : m_forceRegions) {
    if (forceRegionConfig.enabled.get()) {
      PhysicsForceRegion forceRegion = forceRegionConfig.forceRegion;

      Vec2F translatePos = position();
      if (forceRegionConfig.attachToPart) {
        Mat3F partTransformation = m_networkedAnimator.finalPartTransformation(forceRegionConfig.attachToPart.get());
        Vec2F localTranslation = partTransformation.transformVec2(Vec2F());
        translatePos += localTranslation;
      }

      forceRegion.call([translatePos](auto& fr) { fr.translate(translatePos); });
      forces.append(std::move(forceRegion));
    }
  }
  return forces;
}

List<DamageSource> Vehicle::damageSources() const {
  List<DamageSource> sources;
  for (auto const& [_, damageSourceConfig] : m_damageSources) {
    if (damageSourceConfig.enabled.get()) {
      DamageSource damageSource = damageSourceConfig.damageSource;

      if (damageSourceConfig.attachToPart) {
        Mat3F partTransformation = m_networkedAnimator.finalPartTransformation(damageSourceConfig.attachToPart.get());
        damageSource.damageArea.call([partTransformation](auto& da) { da.transform(partTransformation); });
      }

      damageSource.team = m_damageTeam.get();
      damageSource.sourceEntityId = entityId();

      sources.append(std::move(damageSource));
    }
  }
  return sources;
}

size_t Vehicle::movingCollisionCount() const {
  return m_movingCollisions.size();
}

Maybe<PhysicsMovingCollision> Vehicle::movingCollision(size_t positionIndex) const {
  auto const& collisionConfig = m_movingCollisions.valueAt(positionIndex);
  if (!collisionConfig.enabled.get())
    return {};

  PhysicsMovingCollision collision = collisionConfig.movingCollision;

  if (collisionConfig.attachToPart) {
    Mat3F partTransformation = m_networkedAnimator.finalPartTransformation(*collisionConfig.attachToPart);

    Vec2F localTranslation = partTransformation.transformVec2(Vec2F());
    collision.position += localTranslation;

    Mat3F localTransform = Mat3F::translation(-localTranslation) * partTransformation;
    collision.collision.transform(localTransform);
  }

  collision.position += position();

  return collision;
}

Maybe<LuaValue> Vehicle::callScript(String const& func, LuaVariadic<LuaValue> const& args) {
  return m_scriptComponent.invoke(func, args);
}

Maybe<LuaValue> Vehicle::evalScript(String const& code) {
  return m_scriptComponent.eval(code);
}

void Vehicle::setPosition(Vec2F const& position) {
  m_movementController.setPosition(position);
}

EntityRenderLayer Vehicle::renderLayer(VehicleLayer vehicleLayer) const {
  // Z-offset based on entity id, so vehicles don't overlap strangely.
  return m_overrideRenderLayer ? (*m_overrideRenderLayer + static_cast<unsigned>(vehicleLayer)) : (m_baseRenderLayer + (static_cast<EntityRenderLayer>(entityId() * 4 + static_cast<unsigned>(vehicleLayer)) & RenderLayerLowerMask));
}

LuaCallbacks Vehicle::makeVehicleCallbacks() {
  LuaCallbacks callbacks;

  callbacks.registerCallback("controlHeld", [this](String const& loungeName, String const& controlName) {
    auto const& mc = m_loungePositions.get(loungeName).masterControlState[LoungeControlNames.getLeft(controlName)];
    return mc.masterHeld || !mc.slavesHeld.empty();
  });

  callbacks.registerCallback("shiftingHeld", [this](String const& loungeName) {
    auto const& mc = m_loungePositions.get(loungeName).masterControlState[LoungeControl::Walk];
    if (mc.masterHeld || !mc.slavesHeld.empty())
      return true;
    else {
      for (EntityId entity : entitiesLoungingIn(*m_loungePositions.indexOf(loungeName))) {
        if (auto player = world()->get<Player>(entity))
          return player->shifting();
      }
    }
    return false;
  });

  callbacks.registerCallback("aimPosition", [this](String const& loungeName) {
    return m_loungePositions.get(loungeName).masterAimPosition;
  });

  callbacks.registerCallback("entityLoungingIn", [this](String const& name) -> LuaValue {
    auto entitiesIn = entitiesLoungingIn(*m_loungePositions.indexOf(name));
    if (entitiesIn.empty())
      return LuaNil;
    return LuaInt(entitiesIn.first());
  });

  callbacks.registerCallback("setLoungeEnabled", [this](String const& name, bool enabled) {
    m_loungePositions.get(name).enabled.set(enabled);
  });

  callbacks.registerCallback("setLoungeOrientation", [this](String const& name, String const& orientation) {
    m_loungePositions.get(name).orientation.set(LoungeOrientationNames.getLeft(orientation));
  });

  callbacks.registerCallback("setLoungeEmote", [this](String const& name, Maybe<String> emote) {
    m_loungePositions.get(name).emote.set(std::move(emote));
  });

  callbacks.registerCallback("setLoungeDance", [this](String const& name, Maybe<String> dance) {
    m_loungePositions.get(name).dance.set(std::move(dance));
  });

  callbacks.registerCallback("setLoungeDirectives", [this](String const& name, Maybe<String> directives) {
    m_loungePositions.get(name).directives.set(std::move(directives));
  });

  callbacks.registerCallback("setLoungeStatusEffects", [this](String const& name, JsonArray const& statusEffects) {
    m_loungePositions.get(name).statusEffects.set(statusEffects.transformed(jsonToPersistentStatusEffect));
  });

  callbacks.registerCallback("setPersistent", [this](bool persistent) {
    setPersistent(persistent);
  });

  callbacks.registerCallback("setInteractive", [this](bool interactive) {
    m_interactive.set(interactive);
  });

  callbacks.registerCallback("setDamageTeam", [this](Json damageTeam) {
    m_damageTeam.set(EntityDamageTeam(damageTeam));
  });

  callbacks.registerCallback("setDamageSourceEnabled", [this](String const& name, bool enabled) {
    m_damageSources.get(name).enabled.set(enabled);
  });

  callbacks.registerCallback("setMovingCollisionEnabled", [this](String const& name, bool enabled) {
    m_movingCollisions.get(name).enabled.set(enabled);
  });

  callbacks.registerCallback("setForceRegionEnabled", [this](String const& name, bool enabled) {
    m_forceRegions.get(name).enabled.set(enabled);
  });

  callbacks.registerCallback("destroy", [this]() {
    m_shouldDestroy = true;
  });

  callbacks.registerCallback("setAnimationParameter", [this](String name, Json value) {
    m_scriptedAnimationParameters.set(std::move(name), std::move(value));
  });

  return callbacks;
}

Json Vehicle::configValue(String const& name, Json def) const {
  return jsonMergeQueryDef(name, std::move(def), m_baseConfig, m_dynamicConfig);
}

}// namespace Star
