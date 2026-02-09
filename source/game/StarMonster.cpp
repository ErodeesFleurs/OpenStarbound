#include "StarMonster.hpp"

#include "StarBehaviorLuaBindings.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarDamageManager.hpp"
#include "StarEntityLuaBindings.hpp"
#include "StarItemDrop.hpp"
#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"
#include "StarNetworkedAnimatorLuaBindings.hpp"
#include "StarRoot.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarScriptedAnimatorLuaBindings.hpp"
#include "StarStatusController.hpp"
#include "StarStatusControllerLuaBindings.hpp"
#include "StarStoredFunctions.hpp"
#include "StarTreasure.hpp"
#include "StarWorld.hpp"

import std;

namespace Star {

Monster::Monster(MonsterVariant const& monsterVariant, std::optional<float> level) {
  m_monsterLevel = level;

  m_damageOnTouch = false;
  m_aggressive = false;

  m_knockedOut = false;
  m_knockoutTimer = 0.0;

  m_dropPool = monsterVariant.dropPoolConfig;

  m_monsterVariant = monsterVariant;

  m_questIndicatorOffset = jsonToVec2F(Root::singleton().assets()->json("/quests/quests.config:defaultIndicatorOffset"));

  setTeam(EntityDamageTeam(m_monsterVariant.damageTeamType, m_monsterVariant.damageTeam));

  m_networkedAnimator = NetworkedAnimator(m_monsterVariant.animatorConfig);
  for (auto const& pair : m_monsterVariant.animatorPartTags)
    m_networkedAnimator.setPartTag(pair.first, "partImage", pair.second);
  m_networkedAnimator.setZoom(m_monsterVariant.animatorZoom);
  auto colorSwap = m_monsterVariant.colorSwap.value_or(Root::singleton().monsterDatabase()->colorSwap(m_monsterVariant.parameters.getString("colors", "default"), m_monsterVariant.seed));
  if (!colorSwap.empty())
    m_networkedAnimator.setProcessingDirectives(imageOperationToString(ColorReplaceImageOperation{colorSwap}));

  m_statusController = std::make_shared<StatusController>(m_monsterVariant.statusSettings);

  m_scriptComponent.setScripts(m_monsterVariant.parameters.optArray("scripts").transform(jsonToStringList).value_or(m_monsterVariant.scripts));
  m_scriptComponent.setUpdateDelta(m_monsterVariant.initialScriptDelta);

  auto movementParameters = ActorMovementParameters::sensibleDefaults().merge(ActorMovementParameters(monsterVariant.movementSettings));
  if (movementParameters.standingPoly)
    movementParameters.standingPoly->scale(m_monsterVariant.animatorZoom);
  if (movementParameters.crouchingPoly)
    movementParameters.crouchingPoly->scale(m_monsterVariant.animatorZoom);
  *movementParameters.walkSpeed *= monsterVariant.walkMultiplier;
  *movementParameters.runSpeed *= monsterVariant.runMultiplier;
  *movementParameters.airJumpProfile.jumpSpeed *= monsterVariant.jumpMultiplier;
  *movementParameters.liquidJumpProfile.jumpSpeed *= monsterVariant.jumpMultiplier;
  *movementParameters.mass *= monsterVariant.weightMultiplier;
  if (!movementParameters.physicsEffectCategories)
    movementParameters.physicsEffectCategories = StringSet{"monster"};
  m_movementController = std::make_shared<ActorMovementController>(movementParameters);

  setPersistent(m_monsterVariant.persistent);

  setupNetStates();
  setNetStates();
}

Monster::Monster(Json const& diskStore)
    : Monster(Root::singleton().monsterDatabase()->readMonsterVariantFromJson(diskStore.get("monsterVariant"))) {
  m_monsterLevel = diskStore.optFloat("monsterLevel");
  m_movementController->loadState(diskStore.get("movementState"));
  m_statusController->diskLoad(diskStore.get("statusController"));
  m_damageOnTouch = diskStore.getBool("damageOnTouch");
  m_aggressive = diskStore.getBool("aggressive");
  m_deathParticleBurst = diskStore.getString("deathParticleBurst");
  m_deathSound = diskStore.getString("deathSound");
  m_activeSkillName = diskStore.getString("activeSkillName");
  m_dropPool = diskStore.get("dropPool");
  m_effectEmitter.fromJson(diskStore.get("effectEmitter"));
  m_scriptComponent.setScriptStorage(diskStore.getObject("scriptStorage"));

  setUniqueId(diskStore.optString("uniqueId"));
  if (diskStore.contains("team"))
    setTeam(EntityDamageTeam(diskStore.get("team")));
}

auto Monster::diskStore() const -> Json {
  return JsonObject{
    {"monsterLevel", jsonFromMaybe(m_monsterLevel)},
    {"movementState", m_movementController->storeState()},
    {"statusController", m_statusController->diskStore()},
    {"damageOnTouch", m_damageOnTouch},
    {"aggressive", aggressive()},
    {"deathParticleBurst", m_deathParticleBurst},
    {"deathSound", m_deathSound},
    {"activeSkillName", m_activeSkillName},
    {"dropPool", m_dropPool},
    {"effectEmitter", m_effectEmitter.toJson()},
    {"monsterVariant", Root::singleton().monsterDatabase()->writeMonsterVariantToJson(m_monsterVariant)},
    {"scriptStorage", m_scriptComponent.getScriptStorage()},
    {"uniqueId", jsonFromMaybe(uniqueId())},
    {"team", getTeam().toJson()}};
}

auto Monster::netStore(NetCompatibilityRules rules) -> ByteArray {
  return Root::singleton().monsterDatabase()->writeMonsterVariant(m_monsterVariant, rules);
}

auto Monster::entityType() const -> EntityType {
  return EntityType::Monster;
}

auto Monster::clientEntityMode() const -> ClientEntityMode {
  return m_monsterVariant.clientEntityMode;
}

void Monster::init(World* world, EntityId entityId, EntityMode mode) {
  Entity::init(world, entityId, mode);

  m_movementController->init(world);
  m_movementController->setIgnorePhysicsEntities({entityId});
  m_statusController->init(this, m_movementController.get());

  if (!m_monsterLevel)
    m_monsterLevel = world->threatLevel();

  if (isMaster()) {
    ConstPtr<FunctionDatabase> functionDatabase = Root::singleton().functionDatabase();
    float healthMultiplier = m_monsterVariant.healthMultiplier * functionDatabase->function(m_monsterVariant.healthLevelFunction)->evaluate(*m_monsterLevel);
    m_statusController->setPersistentEffects("innate", {StatModifier(StatBaseMultiplier{.statName = "maxHealth", .baseMultiplier = healthMultiplier})});

    m_scriptComponent.addCallbacks("monster", makeMonsterCallbacks());
    m_scriptComponent.addCallbacks("config", LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) -> Json {
                                     return m_monsterVariant.parameters.query(name, def);
                                   }));
    m_scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(this));
    m_scriptComponent.addCallbacks("animator", LuaBindings::makeNetworkedAnimatorCallbacks(&m_networkedAnimator));
    m_scriptComponent.addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(m_statusController.get()));
    m_scriptComponent.addCallbacks("behavior", LuaBindings::makeBehaviorCallbacks(&m_behaviors));
    m_scriptComponent.addActorMovementCallbacks(m_movementController.get());
    m_scriptComponent.init(world);
  }

  if (world->isClient()) {
    m_scriptedAnimator.setScripts(m_monsterVariant.animationScripts);

    m_scriptedAnimator.addCallbacks("animationConfig", LuaBindings::makeScriptedAnimatorCallbacks(&m_networkedAnimator, [this](String const& name, Json const& defaultValue) -> Json {
                                      return m_scriptedAnimationParameters.value(name, defaultValue);
                                    }));
    m_scriptedAnimator.addCallbacks("config", LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) -> Json {
                                      return m_monsterVariant.parameters.query(name, def);
                                    }));
    m_scriptedAnimator.addCallbacks("entity", LuaBindings::makeEntityCallbacks(this));
    m_scriptedAnimator.init(world);
  }

  setPosition(position());
}

void Monster::uninit() {
  if (isMaster()) {
    m_scriptComponent.uninit();
    m_scriptComponent.removeCallbacks("monster");
    m_scriptComponent.removeCallbacks("config");
    m_scriptComponent.removeCallbacks("entity");
    m_scriptComponent.removeCallbacks("animator");
    m_scriptComponent.removeCallbacks("status");
    m_scriptComponent.removeActorMovementCallbacks();
  }
  if (world()->isClient()) {
    m_scriptedAnimator.removeCallbacks("animationConfig");
    m_scriptedAnimator.removeCallbacks("config");
    m_scriptedAnimator.removeCallbacks("entity");
  }
  m_statusController->uninit();
  m_movementController->uninit();
  Entity::uninit();
}

auto Monster::mouthOffset() const -> Vec2F {
  return getAbsolutePosition(m_monsterVariant.mouthOffset) - position();
}

auto Monster::feetOffset() const -> Vec2F {
  return getAbsolutePosition(m_monsterVariant.feetOffset) - position();
}

auto Monster::position() const -> Vec2F {
  return m_movementController->position();
}

auto Monster::metaBoundBox() const -> RectF {
  return m_monsterVariant.metaBoundBox;
}

auto Monster::collisionArea() const -> RectF {
  return m_movementController->collisionPoly().boundBox();
}

auto Monster::velocity() const -> Vec2F {
  return m_movementController->velocity();
}

auto Monster::writeNetState(std::uint64_t fromVersion, NetCompatibilityRules rules) -> std::pair<ByteArray, std::uint64_t> {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void Monster::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
}

void Monster::enableInterpolation(float extrapolationHint) {
  m_netGroup.enableNetInterpolation(extrapolationHint);
}

void Monster::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

auto Monster::name() const -> String {
  return m_name.get().or_else([&] -> std::optional<String> { return m_monsterVariant.shortDescription; }).value_or("");
}

auto Monster::description() const -> String {
  return m_monsterVariant.description.value_or("Some indescribable horror");
}

auto Monster::queryHit(DamageSource const& source) const -> std::optional<HitType> {
  if (!inWorld() || m_knockedOut || m_statusController->statPositive("invulnerable"))
    return {};

  if (source.intersectsWithPoly(world()->geometry(), *hitPoly()))
    return HitType::Hit;

  return {};
}

auto Monster::hitPoly() const -> std::optional<PolyF> {
  PolyF hitBody = m_monsterVariant.selfDamagePoly;
  hitBody.rotate(m_movementController->rotation());
  hitBody.translate(position());
  return hitBody;
}

auto Monster::applyDamage(DamageRequest const& damage) -> List<DamageNotification> {
  if (!inWorld())
    return {};

  auto notifications = m_statusController->applyDamageRequest(damage);

  float totalDamage = 0.0f;
  for (auto const& notification : notifications)
    totalDamage += notification.healthLost;

  if (totalDamage > 0.0f) {
    m_scriptComponent.invoke("damage", JsonObject{{"sourceId", damage.sourceEntityId}, {"damage", totalDamage}, {"sourceDamage", damage.damage}, {"sourceKind", damage.damageSourceKind}});
  }

  if (!m_statusController->resourcePositive("health"))
    m_deathDamageSourceKinds.add(damage.damageSourceKind);

  return notifications;
}

auto Monster::selfDamageNotifications() -> List<DamageNotification> {
  return m_statusController->pullSelfDamageNotifications();
}

auto Monster::damageSources() const -> List<DamageSource> {
  List<DamageSource> damageSources = m_damageSources.get();

  float levelPowerMultiplier = Root::singleton().functionDatabase()->function(m_monsterVariant.powerLevelFunction)->evaluate(*m_monsterLevel);
  if (m_damageOnTouch && !m_monsterVariant.touchDamageConfig.isNull()) {
    DamageSource damageSource(m_monsterVariant.touchDamageConfig);
    if (auto damagePoly = damageSource.damageArea.ptr<PolyF>())
      damagePoly->rotate(m_movementController->rotation());
    damageSource.damage *= m_monsterVariant.touchDamageMultiplier * levelPowerMultiplier * m_statusController->stat("powerMultiplier");
    damageSource.sourceEntityId = entityId();
    damageSource.team = getTeam();
    damageSources.append(damageSource);
  }

  auto animationDamageParts = m_monsterVariant.animationDamageParts;
  for (auto pair : m_monsterVariant.animationDamageParts) {
    if (!m_animationDamageParts.get().contains(pair.first))
      continue;

    String anchorPart = pair.second.getString("anchorPart");
    DamageSource ds = DamageSource(pair.second.get("damageSource"));
    ds.damage *= levelPowerMultiplier * m_statusController->stat("powerMultiplier");
    ds.damageArea.call([this, &anchorPart](auto& poly) -> auto {
      poly.transform(m_networkedAnimator.partTransformation(anchorPart));
      if (m_networkedAnimator.flipped())
        poly.flipHorizontal(m_networkedAnimator.flippedRelativeCenterLine());
    });
    if (ds.knockback.is<Vec2F>()) {
      Vec2F knockback = ds.knockback.get<Vec2F>();
      knockback = m_networkedAnimator.partTransformation(anchorPart).transformVec2(knockback);
      if (m_networkedAnimator.flipped())
        knockback = Vec2F(-knockback[0], knockback[1]);
      ds.knockback = knockback;
    }

    List<DamageSource> partSources;
    if (auto line = ds.damageArea.maybe<Line2F>()) {
      if (pair.second.getBool("checkLineCollision", false)) {
        Line2F worldLine = line->translated(position());
        float length = worldLine.length();

        auto bounces = pair.second.getInt("bounces", 0);
        while (auto collision = world()->lineTileCollisionPoint(worldLine.min(), worldLine.max())) {
          worldLine = Line2F(worldLine.min(), collision.value().first);
          ds.damageArea = worldLine.translated(-position());
          length = length - worldLine.length();

          if (--bounces >= 0 && length > 0.0f) {
            partSources.append(ds);
            ds = DamageSource(ds);
            Vec2F dir = worldLine.direction();
            auto normal = Vec2F((*collision).second);
            Vec2F reflection = dir - (2 * dir.piecewiseMultiply(normal).sum() * normal);
            if (ds.knockback.is<Vec2F>())
              ds.knockback = ds.knockback.get<Vec2F>().rotate(reflection.angleBetween(worldLine.direction()));

            worldLine.min() = (*collision).first;
            worldLine.max() = worldLine.min() + (reflection * length);
            ds.damageArea = worldLine.translated(-position());
          } else {
            break;
          }
        }
        partSources.append(ds);
      }
    } else {
      partSources.append(ds);
    }
    damageSources.appendAll(partSources);
  }

  return damageSources;
}

auto Monster::shouldDie() -> bool {
  if (auto res = m_scriptComponent.invoke<bool>("shouldDie"))
    return *res;
  else if (!m_statusController->resourcePositive("health") || m_scriptComponent.error())
    return true;
  else
    return false;
}

void Monster::knockout() {
  m_knockedOut = true;
  m_knockoutTimer = m_monsterVariant.parameters.getFloat("knockoutTime", 1.0f);

  m_damageOnTouch = false;

  String knockoutEffect = m_monsterVariant.parameters.getString("knockoutEffect");
  if (!knockoutEffect.empty())
    m_networkedAnimator.setEffectEnabled(knockoutEffect, true);

  auto knockoutAnimationStates = m_monsterVariant.parameters.getObject("knockoutAnimationStates", JsonObject());
  for (auto pair : knockoutAnimationStates)
    m_networkedAnimator.setState(pair.first, pair.second.toString());
}

auto Monster::shouldDestroy() const -> bool {
  return m_knockedOut && m_knockoutTimer <= 0;
}

void Monster::destroy(RenderCallback* renderCallback) {
  m_scriptComponent.invoke("die");

  if (isMaster() && !m_dropPool.isNull()) {
    auto treasureDatabase = Root::singleton().treasureDatabase();

    String treasurePool;
    if (m_dropPool.isType(Json::Type::String)) {
      treasurePool = m_dropPool.toString();
    } else {
      // Check to see whether any of the damage types that were used to cause
      // death are in the damage pool map, if so spawn treasure from that,
      // otherwise set the treasure pool to the "default" entry.

      for (auto const& damageSourceKind : m_deathDamageSourceKinds) {
        if (m_dropPool.contains(damageSourceKind))
          treasurePool = m_dropPool.getString(damageSourceKind);
      }

      if (treasurePool.empty())
        treasurePool = m_dropPool.getString("default");
    }

    for (auto const& treasureItem : treasureDatabase->createTreasure(treasurePool, *m_monsterLevel))
      world()->addEntity(ItemDrop::createRandomizedDrop(treasureItem, position()));
  }

  if (renderCallback) {
    if (!m_deathParticleBurst.empty())
      m_networkedAnimator.burstParticleEmitter(m_deathParticleBurst);

    if (!m_deathSound.empty())
      m_networkedAnimator.playSound(m_deathSound);

    m_networkedAnimator.update(0.0, &m_networkedAnimatorDynamicTarget);

    renderCallback->addAudios(m_networkedAnimatorDynamicTarget.pullNewAudios());
    renderCallback->addParticles(m_networkedAnimatorDynamicTarget.pullNewParticles());
    renderCallback->addParticles(m_statusController->pullNewParticles());
  }

  m_deathDamageSourceKinds.clear();

  if (isMaster())
    setNetStates();
}

auto Monster::lightSources() const -> List<LightSource> {
  auto lightSources = m_networkedAnimator.lightSources(position());
  lightSources.appendAll(m_statusController->lightSources());
  return lightSources;
}

void Monster::hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) {
  if (inWorld() && isMaster())
    m_statusController->hitOther(targetEntityId, damageRequest);
}

void Monster::damagedOther(DamageNotification const& damage) {
  if (inWorld() && isMaster())
    m_statusController->damagedOther(damage);
}

void Monster::update(float dt, std::uint64_t) {
  if (!inWorld())
    return;

  m_movementController->setTimestep(dt);

  if (isMaster()) {
    m_networkedAnimator.setFlipped((m_movementController->facingDirection() == Direction::Left) != m_monsterVariant.reversed);

    if (m_knockedOut) {
      m_knockoutTimer -= dt;
    } else {
      if (m_scriptComponent.updateReady())
        m_physicsForces.set({});
      m_scriptComponent.update(m_scriptComponent.updateDt(dt));

      if (shouldDie())
        knockout();
    }

    m_movementController->tickMaster(dt);

    m_statusController->tickMaster(dt);
    updateStatus(dt);
  } else {
    m_netGroup.tickNetInterpolation(dt);

    m_statusController->tickSlave(dt);
    updateStatus(dt);

    m_movementController->tickSlave(dt);
  }

  if (world()->isServer()) {
    m_networkedAnimator.update(dt, nullptr);
  } else {
    m_networkedAnimator.update(dt, &m_networkedAnimatorDynamicTarget);
    m_networkedAnimatorDynamicTarget.updatePosition(position());

    m_scriptedAnimator.update();

    SpatialLogger::logPoly("world", m_movementController->collisionBody(), {255, 0, 0, 255});
  }
}

void Monster::render(RenderCallback* renderCallback) {
  for (auto& drawable : m_networkedAnimator.drawables(position())) {
    if (drawable.isImage())
      drawable.imagePart().addDirectivesGroup(m_statusController->parentDirectives(), true);
    renderCallback->addDrawable(std::move(drawable), m_monsterVariant.renderLayer);
  }

  renderCallback->addAudios(m_networkedAnimatorDynamicTarget.pullNewAudios());
  renderCallback->addParticles(m_networkedAnimatorDynamicTarget.pullNewParticles());

  renderCallback->addDrawables(m_statusController->drawables(), m_monsterVariant.renderLayer);
  renderCallback->addParticles(m_statusController->pullNewParticles());
  renderCallback->addAudios(m_statusController->pullNewAudios());

  m_effectEmitter.render(renderCallback);

  for (auto drawablePair : m_scriptedAnimator.drawables())
    renderCallback->addDrawable(drawablePair.first, drawablePair.second.value_or(m_monsterVariant.renderLayer));
  renderCallback->addAudios(m_scriptedAnimator.pullNewAudios());
  renderCallback->addParticles(m_scriptedAnimator.pullNewParticles());
}

void Monster::renderLightSources(RenderCallback* renderCallback) {
  renderCallback->addLightSources(m_networkedAnimator.lightSources(position()));
  renderCallback->addLightSources(m_statusController->lightSources());
  renderCallback->addLightSources(m_scriptedAnimator.lightSources());
}

void Monster::setPosition(Vec2F const& pos) {
  m_movementController->setPosition(pos);
}

auto Monster::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) -> std::optional<Json> {
  std::optional<Json> result = m_scriptComponent.handleMessage(message, world()->connection() == sendingConnection, args);
  if (!result)
    result = m_statusController->receiveMessage(message, world()->connection() == sendingConnection, args);
  return result;
}

auto Monster::maxHealth() const -> float {
  return *m_statusController->resourceMax("health");
}

auto Monster::health() const -> float {
  return m_statusController->resource("health");
}

auto Monster::damageBar() const -> DamageBarType {
  return m_damageBar.get();
}

auto Monster::getAbsolutePosition(Vec2F relativePosition) const -> Vec2F {
  if (m_movementController->facingDirection() == Direction::Left)
    relativePosition[0] *= -1;
  if (m_movementController->rotation() != 0)
    relativePosition = relativePosition.rotate(m_movementController->rotation());
  return m_movementController->position() + relativePosition;
}

void Monster::updateStatus(float dt) {
  m_effectEmitter.setSourcePosition("normal", position());
  m_effectEmitter.setSourcePosition("mouth", position() + mouthOffset());
  m_effectEmitter.setSourcePosition("feet", position() + feetOffset());
  m_effectEmitter.setDirection(m_movementController->facingDirection());
  m_effectEmitter.tick(dt, *entityMode());
}

auto Monster::makeMonsterCallbacks() -> LuaCallbacks {
  LuaCallbacks callbacks;

  callbacks.registerCallback("type", [this]() -> String {
    return m_monsterVariant.type;
  });

  callbacks.registerCallback("seed", [this]() -> std::string {
    return toString(m_monsterVariant.seed);
  });

  callbacks.registerCallback("uniqueParameters", [this]() -> Json {
    return m_monsterVariant.uniqueParameters;
  });

  callbacks.registerCallback("level", [this]() -> float {
    return *m_monsterLevel;
  });

  callbacks.registerCallback("setDamageOnTouch", [this](bool arg1) -> void {
    m_damageOnTouch = arg1;
  });

  callbacks.registerCallback("setDamageSources", [this](std::optional<JsonArray> const& damageSources) -> void {
    m_damageSources.set(damageSources.value_or(JsonArray()).transformed(construct<DamageSource>()));
  });

  callbacks.registerCallback("setDamageParts", [this](StringSet const& parts) -> void {
    m_animationDamageParts.set(parts);
  });

  callbacks.registerCallback("setAggressive", [this](bool arg1) -> void {
    m_aggressive = arg1;
  });

  callbacks.registerCallback("setActiveSkillName", [this](std::optional<String> const& activeSkillName) -> void {
    m_activeSkillName = activeSkillName.value_or("");
  });

  callbacks.registerCallback("setDropPool", [this](Json dropPool) -> void {
    m_dropPool = std::move(dropPool);
  });

  callbacks.registerCallback("toAbsolutePosition", [this](Vec2F const& p) -> Vec2F {
    return getAbsolutePosition(p);
  });

  callbacks.registerCallback("mouthPosition", [this]() -> Vec2F {
    return mouthPosition();
  });

  // This callback is registered here rather than in
  // makeActorMovementControllerCallbacks
  // because it requires access to world
  callbacks.registerCallback("flyTo", [this](Vec2F const& arg1) -> void {
    m_movementController->controlFly(world()->geometry().diff(arg1, position()));
  });

  callbacks.registerCallback("setDeathParticleBurst", [this](std::optional<String> const& arg1) -> void {
    m_deathParticleBurst = arg1.value_or("");
  });

  callbacks.registerCallback("setDeathSound", [this](std::optional<String> const& arg1) -> void {
    m_deathSound = arg1.value_or("");
  });

  callbacks.registerCallback("setPhysicsForces", [this](JsonArray const& forces) -> void {
    m_physicsForces.set(forces.transformed(jsonToPhysicsForceRegion));
  });

  callbacks.registerCallback("setName", [this](String const& name) -> void {
    m_name.set(name);
  });
  callbacks.registerCallback("setDisplayNametag", [this](bool display) -> void {
    m_displayNametag.set(display);
  });

  callbacks.registerCallback("say", [this](String line, std::optional<StringMap<String>> const& tags) -> bool {
    if (tags)
      line = line.replaceTags(*tags, false);

    if (!line.empty()) {
      addChatMessage(line);
      return true;
    }

    return false;
  });

  callbacks.registerCallback("sayPortrait", [this](String line, String portrait, std::optional<StringMap<String>> const& tags) -> bool {
    if (tags)
      line = line.replaceTags(*tags, false);

    if (!line.empty()) {
      addChatMessage(line, portrait);
      return true;
    }

    return false;
  });

  callbacks.registerCallback("setDamageTeam", [this](Json const& team) -> void {
    setTeam(EntityDamageTeam(team));
  });

  callbacks.registerCallback("setUniqueId", [this](std::optional<String> uniqueId) -> void {
    setUniqueId(uniqueId);
  });

  callbacks.registerCallback("setDamageBar", [this](String const& damageBarType) -> void {
    m_damageBar.set(DamageBarTypeNames.getLeft(damageBarType));
  });

  callbacks.registerCallback("setInteractive", [this](bool interactive) -> void {
    m_interactive.set(interactive);
  });

  callbacks.registerCallback("setAnimationParameter", [this](String name, Json value) -> void {
    m_scriptedAnimationParameters.set(std::move(name), std::move(value));
  });

  return callbacks;
}

void Monster::addChatMessage(String const& message, String const& portrait) {
  m_chatMessage.set(message);
  m_chatPortrait.set(portrait);
  m_newChatMessageEvent.trigger();
  if (portrait.empty())
    m_pendingChatActions.append(SayChatAction{entityId(), message, mouthPosition()});
  else
    m_pendingChatActions.append(PortraitChatAction{entityId(), portrait, message, mouthPosition()});
}

void Monster::setupNetStates() {
  m_netGroup.addNetElement(&m_uniqueIdNetState);
  m_netGroup.addNetElement(&m_teamNetState);
  m_netGroup.addNetElement(&m_monsterLevelNetState);
  m_netGroup.addNetElement(&m_damageOnTouchNetState);
  m_netGroup.addNetElement(&m_damageSources);
  m_netGroup.addNetElement(&m_aggressiveNetState);
  m_netGroup.addNetElement(&m_knockedOutNetState);
  m_netGroup.addNetElement(&m_deathParticleBurstNetState);
  m_netGroup.addNetElement(&m_deathSoundNetState);
  m_netGroup.addNetElement(&m_activeSkillNameNetState);
  m_netGroup.addNetElement(&m_name);
  m_netGroup.addNetElement(&m_displayNametag);
  m_netGroup.addNetElement(&m_dropPoolNetState);
  m_netGroup.addNetElement(&m_physicsForces);

  m_netGroup.addNetElement(&m_networkedAnimator);
  m_netGroup.addNetElement(m_movementController.get());
  m_netGroup.addNetElement(m_statusController.get());
  m_netGroup.addNetElement(&m_effectEmitter);

  m_netGroup.addNetElement(&m_newChatMessageEvent);
  m_netGroup.addNetElement(&m_chatMessage);
  m_netGroup.addNetElement(&m_chatPortrait);

  m_netGroup.addNetElement(&m_damageBar);
  m_netGroup.addNetElement(&m_interactive);

  // don't interpolate scripted animation parameters or animationdamageparts
  m_netGroup.addNetElement(&m_animationDamageParts, false);
  m_netGroup.addNetElement(&m_scriptedAnimationParameters, false);

  m_netGroup.setNeedsLoadCallback([this](auto&& PH1) -> auto { getNetStates(std::forward<decltype(PH1)>(PH1)); });
  m_netGroup.setNeedsStoreCallback([this] -> void { setNetStates(); });
}

void Monster::setNetStates() {
  m_uniqueIdNetState.set(uniqueId());
  m_teamNetState.set(getTeam());
  m_monsterLevelNetState.set(m_monsterLevel);
  m_damageOnTouchNetState.set(m_damageOnTouch);
  m_aggressiveNetState.set(aggressive());
  m_knockedOutNetState.set(m_knockedOut);
  m_deathParticleBurstNetState.set(m_deathParticleBurst);
  m_deathSoundNetState.set(m_deathSound);
  m_activeSkillNameNetState.set(m_activeSkillName);
  m_dropPoolNetState.set(m_dropPool);
}

void Monster::getNetStates(bool initial) {
  setUniqueId(m_uniqueIdNetState.get());
  setTeam(m_teamNetState.get());
  m_monsterLevel = m_monsterLevelNetState.get();
  m_damageOnTouch = m_damageOnTouchNetState.get();
  m_aggressive = m_aggressiveNetState.get();
  m_knockedOut = m_knockedOutNetState.get();
  if (m_deathParticleBurstNetState.pullUpdated())
    m_deathParticleBurst = m_deathParticleBurstNetState.get();
  if (m_deathSoundNetState.pullUpdated())
    m_deathSound = m_deathSoundNetState.get();
  if (m_activeSkillNameNetState.pullUpdated())
    m_activeSkillName = m_activeSkillNameNetState.get();
  if (m_dropPoolNetState.pullUpdated())
    m_dropPool = m_dropPoolNetState.get();

  if (m_newChatMessageEvent.pullOccurred() && !initial) {
    if (m_chatPortrait.get().empty())
      m_pendingChatActions.append(SayChatAction{entityId(), m_chatMessage.get(), mouthPosition()});
    else
      m_pendingChatActions.append(
        PortraitChatAction{entityId(), m_chatPortrait.get(), m_chatMessage.get(), mouthPosition()});
  }
}

auto Monster::monsterLevel() const -> float {
  return *m_monsterLevel;
}

auto Monster::activeSkillInfo() const -> Monster::SkillInfo {
  SkillInfo skillInfo;

  if (!m_activeSkillName.empty()) {
    auto monsterDatabase = Root::singleton().monsterDatabase();
    auto monsterSkillInfo = monsterDatabase->skillInfo(m_activeSkillName);
    skillInfo.label = monsterSkillInfo.first;
    skillInfo.image = monsterSkillInfo.second;
  }

  return skillInfo;
}

auto Monster::portrait(PortraitMode) const -> List<Drawable> {
  if (m_monsterVariant.portraitIcon) {
    return {Drawable::makeImage(*m_monsterVariant.portraitIcon, 1.0f, true, Vec2F())};
  } else {
    auto animator = m_networkedAnimator;
    animator.setFlipped(!m_monsterVariant.reversed);
    auto drawables = animator.drawables();
    Drawable::scaleAll(drawables, TilePixels);
    return drawables;
  }
}

auto Monster::typeName() const -> String {
  return m_monsterVariant.type;
}

auto Monster::monsterVariant() const -> MonsterVariant {
  return m_monsterVariant;
}

auto Monster::statusText() const -> std::optional<String> {
  return {};
}

auto Monster::displayNametag() const -> bool {
  return m_displayNametag.get();
}

auto Monster::nametagColor() const -> Vec3B {
  return m_monsterVariant.nametagColor;
}

auto Monster::nametagOrigin() const -> Vec2F {
  return mouthPosition(false);
}

auto Monster::nametag() const -> String {
  return name();
}

auto Monster::aggressive() const -> bool {
  return m_aggressive;
}

auto Monster::callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> {
  return m_scriptComponent.invoke(func, args);
}

auto Monster::evalScript(String const& code) -> std::optional<LuaValue> {
  return m_scriptComponent.eval(code);
}

auto Monster::mouthPosition() const -> Vec2F {
  return mouthOffset() + position();
}

auto Monster::mouthPosition(bool) const -> Vec2F {
  return mouthPosition();
}

auto Monster::pullPendingChatActions() -> List<ChatAction> {
  return std::move(m_pendingChatActions);
}

auto Monster::forceRegions() const -> List<PhysicsForceRegion> {
  return m_physicsForces.get();
}

auto Monster::interact(InteractRequest const& request) -> InteractAction {
  auto result = m_scriptComponent.invoke<Json>("interact", JsonObject{{"sourceId", request.sourceId}, {"sourcePosition", jsonFromVec2F(request.sourcePosition)}}).value();

  if (result.isNull())
    return {};

  if (result.isType(Json::Type::String))
    return {result.toString(), entityId(), {}};

  return {result.getString(0), entityId(), result.get(1)};
}

auto Monster::isInteractive() const -> bool {
  return m_interactive.get();
}

auto Monster::questIndicatorPosition() const -> Vec2F {
  Vec2F pos = position() + m_questIndicatorOffset;
  pos[1] += collisionArea().yMax();
  return pos;
}

auto Monster::movementController() -> ActorMovementController* {
  return m_movementController.get();
}

auto Monster::statusController() -> StatusController* {
  return m_statusController.get();
}

}// namespace Star
