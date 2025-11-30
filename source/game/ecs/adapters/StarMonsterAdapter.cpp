#include "StarMonsterAdapter.hpp"
#include "StarWorld.hpp"
#include "StarRoot.hpp"
#include "StarDamageManager.hpp"
#include "StarDamageDatabase.hpp"
#include "StarTreasure.hpp"
#include "StarJsonExtra.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarEntityLuaBindings.hpp"
#include "StarWorldLuaBindings.hpp"
#include "StarNetworkedAnimatorLuaBindings.hpp"
#include "StarStatusControllerLuaBindings.hpp"
#include "StarScriptedAnimatorLuaBindings.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarBehaviorLuaBindings.hpp"
#include "StarStoredFunctions.hpp"
#include "StarItemDrop.hpp"
#include "StarAssets.hpp"
#include "StarTime.hpp"
#include "StarStatusController.hpp"
#include "StarMonsterDatabase.hpp"

namespace Star {
namespace ECS {

shared_ptr<MonsterAdapter> MonsterAdapter::create(World& ecsWorld, MonsterVariant const& variant, Maybe<float> level) {
  Entity entity = ecsWorld.createEntity();
  
  // Add tag components
  ecsWorld.addComponent<MonsterTag>(entity);
  
  // Add data component
  auto& data = ecsWorld.addComponent<MonsterDataComponent>(entity);
  data.variant = variant;
  data.level = level;
  data.dropPool = variant.dropPoolConfig;
  data.questIndicatorOffset = jsonToVec2F(Root::singleton().assets()->json("/quests/quests.config:defaultIndicatorOffset"));
  
  // Initialize networked animator
  data.networkedAnimator = NetworkedAnimator(variant.animatorConfig);
  for (auto const& pair : variant.animatorPartTags)
    data.networkedAnimator.setPartTag(pair.first, "partImage", pair.second);
  data.networkedAnimator.setZoom(variant.animatorZoom);
  
  auto colorSwap = variant.colorSwap.value(Root::singleton().monsterDatabase()->colorSwap(variant.parameters.getString("colors", "default"), variant.seed));
  if (!colorSwap.empty())
    data.networkedAnimator.setProcessingDirectives(imageOperationToString(ColorReplaceImageOperation{colorSwap}));
  
  // Initialize status controller
  data.statusController = make_shared<StatusController>(variant.statusSettings);
  
  // Initialize script component
  data.scriptComponent.setScripts(variant.parameters.optArray("scripts").apply(jsonToStringList).value(variant.scripts));
  data.scriptComponent.setUpdateDelta(variant.initialScriptDelta);
  
  // Initialize movement controller
  auto movementParameters = ActorMovementParameters::sensibleDefaults().merge(ActorMovementParameters(variant.movementSettings));
  if (movementParameters.standingPoly)
    movementParameters.standingPoly->scale(variant.animatorZoom);
  if (movementParameters.crouchingPoly)
    movementParameters.crouchingPoly->scale(variant.animatorZoom);
  *movementParameters.walkSpeed *= variant.walkMultiplier;
  *movementParameters.runSpeed *= variant.runMultiplier;
  *movementParameters.airJumpProfile.jumpSpeed *= variant.jumpMultiplier;
  *movementParameters.liquidJumpProfile.jumpSpeed *= variant.jumpMultiplier;
  *movementParameters.mass *= variant.weightMultiplier;
  if (!movementParameters.physicsEffectCategories)
    movementParameters.physicsEffectCategories = StringSet{"monster"};
  data.movementController = make_shared<ActorMovementController>(movementParameters);
  
  auto adapter = make_shared<MonsterAdapter>(ecsWorld, entity);
  adapter->setTeam(EntityDamageTeam(variant.damageTeamType, variant.damageTeam));
  adapter->setPersistent(variant.persistent);
  adapter->setupNetStates();
  adapter->setNetStates();
  
  return adapter;
}

shared_ptr<MonsterAdapter> MonsterAdapter::createFromDiskStore(World& ecsWorld, Json const& diskStore) {
  auto variant = Root::singleton().monsterDatabase()->readMonsterVariantFromJson(diskStore.get("monsterVariant"));
  auto adapter = create(ecsWorld, variant);
  
  auto* data = adapter->getData();
  data->level = diskStore.optFloat("monsterLevel");
  data->movementController->loadState(diskStore.get("movementState"));
  data->statusController->diskLoad(diskStore.get("statusController"));
  data->damageOnTouch = diskStore.getBool("damageOnTouch");
  data->aggressive = diskStore.getBool("aggressive");
  data->deathParticleBurst = diskStore.getString("deathParticleBurst");
  data->deathSound = diskStore.getString("deathSound");
  data->activeSkillName = diskStore.getString("activeSkillName");
  data->dropPool = diskStore.get("dropPool");
  data->effectEmitter.fromJson(diskStore.get("effectEmitter"));
  data->scriptComponent.setScriptStorage(diskStore.getObject("scriptStorage"));
  
  adapter->setUniqueId(diskStore.optString("uniqueId"));
  if (diskStore.contains("team"))
    adapter->setTeam(EntityDamageTeam(diskStore.get("team")));
  
  return adapter;
}

MonsterAdapter::MonsterAdapter(World& ecsWorld, Entity entity)
  : EntityAdapter(ecsWorld, entity) {
}

Json MonsterAdapter::diskStore() const {
  auto const* data = getData();
  return JsonObject{
    {"monsterLevel", jsonFromMaybe(data->level)},
    {"movementState", data->movementController->storeState()},
    {"statusController", data->statusController->diskStore()},
    {"damageOnTouch", data->damageOnTouch},
    {"aggressive", aggressive()},
    {"deathParticleBurst", data->deathParticleBurst},
    {"deathSound", data->deathSound},
    {"activeSkillName", data->activeSkillName},
    {"dropPool", data->dropPool},
    {"effectEmitter", data->effectEmitter.toJson()},
    {"monsterVariant", Root::singleton().monsterDatabase()->writeMonsterVariantToJson(data->variant)},
    {"scriptStorage", data->scriptComponent.getScriptStorage()},
    {"uniqueId", jsonFromMaybe(uniqueId())},
    {"team", getTeam().toJson()}
  };
}

ByteArray MonsterAdapter::netStore(NetCompatibilityRules rules) {
  auto const* data = getData();
  return Root::singleton().monsterDatabase()->writeMonsterVariant(data->variant, rules);
}

EntityType MonsterAdapter::entityType() const {
  return EntityType::Monster;
}

ClientEntityMode MonsterAdapter::clientEntityMode() const {
  auto const* data = getData();
  return data->variant.clientEntityMode;
}

void MonsterAdapter::init(::Star::World* world, EntityId entityId, EntityMode mode) {
  EntityAdapter::init(world, entityId, mode);
  
  auto* data = getData();
  
  data->movementController->init(world);
  data->movementController->setIgnorePhysicsEntities({entityId});
  data->statusController->init(this, data->movementController.get());
  
  if (!data->level)
    data->level = world->threatLevel();
  
  if (isMaster()) {
    auto functionDatabase = Root::singleton().functionDatabase();
    float healthMultiplier = data->variant.healthMultiplier * functionDatabase->function(data->variant.healthLevelFunction)->evaluate(*data->level);
    data->statusController->setPersistentEffects("innate", {StatModifier(StatBaseMultiplier{"maxHealth", healthMultiplier})});
    
    data->scriptComponent.addCallbacks("monster", makeMonsterCallbacks());
    data->scriptComponent.addCallbacks("config", LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) {
        return getData()->variant.parameters.query(name, def);
      }));
    data->scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(this));
    data->scriptComponent.addCallbacks("animator", LuaBindings::makeNetworkedAnimatorCallbacks(&getData()->networkedAnimator));
    data->scriptComponent.addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(data->statusController.get()));
    data->scriptComponent.addCallbacks("behavior", LuaBindings::makeBehaviorCallbacks(&data->behaviors));
    data->scriptComponent.addActorMovementCallbacks(data->movementController.get());
    data->scriptComponent.init(world);
  }
  
  if (world->isClient()) {
    data->scriptedAnimator.setScripts(data->variant.animationScripts);
    
    data->scriptedAnimator.addCallbacks("animationConfig", LuaBindings::makeScriptedAnimatorCallbacks(&data->networkedAnimator,
      [this](String const& name, Json const& defaultValue) -> Json {
        return getData()->scriptedAnimationParameters.value(name, defaultValue);
      }));
    data->scriptedAnimator.addCallbacks("config", LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) {
        return getData()->variant.parameters.query(name, def);
      }));
    data->scriptedAnimator.addCallbacks("entity", LuaBindings::makeEntityCallbacks(this));
    data->scriptedAnimator.init(world);
  }
  
  setPosition(position());
}

void MonsterAdapter::uninit() {
  auto* data = getData();
  
  if (isMaster()) {
    data->scriptComponent.uninit();
    data->scriptComponent.removeCallbacks("monster");
    data->scriptComponent.removeCallbacks("config");
    data->scriptComponent.removeCallbacks("entity");
    data->scriptComponent.removeCallbacks("animator");
    data->scriptComponent.removeCallbacks("status");
    data->scriptComponent.removeActorMovementCallbacks();
  }
  if (EntityAdapter::world()->isClient()) {
    data->scriptedAnimator.removeCallbacks("animationConfig");
    data->scriptedAnimator.removeCallbacks("config");
    data->scriptedAnimator.removeCallbacks("entity");
  }
  data->statusController->uninit();
  data->movementController->uninit();
  EntityAdapter::uninit();
}

Vec2F MonsterAdapter::mouthOffset() const {
  return getAbsolutePosition(getData()->variant.mouthOffset) - position();
}

Vec2F MonsterAdapter::feetOffset() const {
  return getAbsolutePosition(getData()->variant.feetOffset) - position();
}

Vec2F MonsterAdapter::position() const {
  return getData()->movementController->position();
}

RectF MonsterAdapter::metaBoundBox() const {
  return getData()->variant.metaBoundBox;
}

RectF MonsterAdapter::collisionArea() const {
  return getData()->movementController->collisionPoly().boundBox();
}

Vec2F MonsterAdapter::velocity() const {
  return getData()->movementController->velocity();
}

pair<ByteArray, uint64_t> MonsterAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void MonsterAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
}

void MonsterAdapter::enableInterpolation(float extrapolationHint) {
  m_netGroup.enableNetInterpolation(extrapolationHint);
}

void MonsterAdapter::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

String MonsterAdapter::name() const {
  auto const* data = getData();
  return data->name.orMaybe(data->variant.shortDescription).value("");
}

String MonsterAdapter::description() const {
  return getData()->variant.description.value("Some indescribable horror");
}

Maybe<HitType> MonsterAdapter::queryHit(DamageSource const& source) const {
  auto const* data = getData();
  if (!inWorld() || data->knockedOut || data->statusController->statPositive("invulnerable"))
    return {};
  
  if (source.intersectsWithPoly(EntityAdapter::world()->geometry(), hitPoly().get()))
    return HitType::Hit;
  
  return {};
}

Maybe<PolyF> MonsterAdapter::hitPoly() const {
  auto const* data = getData();
  PolyF hitBody = data->variant.selfDamagePoly;
  hitBody.rotate(data->movementController->rotation());
  hitBody.translate(position());
  return hitBody;
}

List<DamageNotification> MonsterAdapter::applyDamage(DamageRequest const& damage) {
  if (!inWorld())
    return {};
  
  auto* data = getData();
  auto notifications = data->statusController->applyDamageRequest(damage);
  
  float totalDamage = 0.0f;
  for (auto const& notification : notifications)
    totalDamage += notification.healthLost;
  
  if (totalDamage > 0.0f) {
    data->scriptComponent.invoke("damage", JsonObject{
        {"sourceId", damage.sourceEntityId},
        {"damage", totalDamage},
        {"sourceDamage", damage.damage},
        {"sourceKind", damage.damageSourceKind}
      });
  }
  
  if (!data->statusController->resourcePositive("health"))
    data->deathDamageSourceKinds.add(damage.damageSourceKind);
  
  return notifications;
}

List<DamageNotification> MonsterAdapter::selfDamageNotifications() {
  return getData()->statusController->pullSelfDamageNotifications();
}

List<DamageSource> MonsterAdapter::damageSources() const {
  auto const* data = getData();
  List<DamageSource> sources = data->damageSources;
  
  float levelPowerMultiplier = Root::singleton().functionDatabase()->function(data->variant.powerLevelFunction)->evaluate(*data->level);
  if (data->damageOnTouch && !data->variant.touchDamageConfig.isNull()) {
    DamageSource damageSource(data->variant.touchDamageConfig);
    if (auto damagePoly = damageSource.damageArea.ptr<PolyF>())
      damagePoly->rotate(data->movementController->rotation());
    damageSource.damage *= data->variant.touchDamageMultiplier * levelPowerMultiplier * data->statusController->stat("powerMultiplier");
    damageSource.sourceEntityId = entityId();
    damageSource.team = getTeam();
    sources.append(damageSource);
  }
  
  for (auto pair : data->variant.animationDamageParts) {
    if (!data->animationDamageParts.contains(pair.first))
      continue;
    
    String anchorPart = pair.second.getString("anchorPart");
    DamageSource ds = DamageSource(pair.second.get("damageSource"));
    ds.damage *= levelPowerMultiplier * data->statusController->stat("powerMultiplier");
    ds.damageArea.call([&data, &anchorPart](auto& poly) {
      poly.transform(data->networkedAnimator.partTransformation(anchorPart));
      if (data->networkedAnimator.flipped())
        poly.flipHorizontal(data->networkedAnimator.flippedRelativeCenterLine());
    });
    if (ds.knockback.is<Vec2F>()) {
      Vec2F knockback = ds.knockback.get<Vec2F>();
      knockback = data->networkedAnimator.partTransformation(anchorPart).transformVec2(knockback);
      if (data->networkedAnimator.flipped())
        knockback = Vec2F(-knockback[0], knockback[1]);
      ds.knockback = knockback;
    }
    
    List<DamageSource> partSources;
    if (auto line = ds.damageArea.maybe<Line2F>()) {
      if (pair.second.getBool("checkLineCollision", false)) {
        Line2F worldLine = line.value().translated(position());
        float length = worldLine.length();
        
        auto bounces = pair.second.getInt("bounces", 0);
        while (auto collision = EntityAdapter::world()->lineTileCollisionPoint(worldLine.min(), worldLine.max())) {
          worldLine = Line2F(worldLine.min(), collision.value().first);
          ds.damageArea = worldLine.translated(-position());
          length = length - worldLine.length();
          
          if (--bounces >= 0 && length > 0.0f) {
            partSources.append(ds);
            ds = DamageSource(ds);
            Vec2F dir = worldLine.direction();
            Vec2F normal = Vec2F((*collision).second);
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
    sources.appendAll(partSources);
  }
  
  return sources;
}

bool MonsterAdapter::shouldDie() {
  auto* data = getData();
  if (auto res = data->scriptComponent.invoke<bool>("shouldDie"))
    return *res;
  else if (!data->statusController->resourcePositive("health") || data->scriptComponent.error())
    return true;
  else
    return false;
}

void MonsterAdapter::knockout() {
  auto* data = getData();
  data->knockedOut = true;
  data->knockoutTimer = data->variant.parameters.getFloat("knockoutTime", 1.0f);
  
  data->damageOnTouch = false;
  
  String knockoutEffect = data->variant.parameters.getString("knockoutEffect");
  if (!knockoutEffect.empty())
    data->networkedAnimator.setEffectEnabled(knockoutEffect, true);
  
  auto knockoutAnimationStates = data->variant.parameters.getObject("knockoutAnimationStates", JsonObject());
  for (auto pair : knockoutAnimationStates)
    data->networkedAnimator.setState(pair.first, pair.second.toString());
}

bool MonsterAdapter::shouldDestroy() const {
  auto const* data = getData();
  return data->knockedOut && data->knockoutTimer <= 0;
}

void MonsterAdapter::destroy(RenderCallback* renderCallback) {
  auto* data = getData();
  data->scriptComponent.invoke("die");
  
  if (isMaster() && !data->dropPool.isNull()) {
    auto treasureDatabase = Root::singleton().treasureDatabase();
    
    String treasurePool;
    if (data->dropPool.isType(Json::Type::String)) {
      treasurePool = data->dropPool.toString();
    } else {
      for (auto const& damageSourceKind : data->deathDamageSourceKinds) {
        if (data->dropPool.contains(damageSourceKind))
          treasurePool = data->dropPool.getString(damageSourceKind);
      }
      
      if (treasurePool.empty())
        treasurePool = data->dropPool.getString("default");
    }
    
    for (auto const& treasureItem : treasureDatabase->createTreasure(treasurePool, *data->level))
      EntityAdapter::world()->addEntity(ItemDrop::createRandomizedDrop(treasureItem, position()));
  }
  
  if (renderCallback) {
    if (!data->deathParticleBurst.empty())
      data->networkedAnimator.burstParticleEmitter(data->deathParticleBurst);
    
    if (!data->deathSound.empty())
      data->networkedAnimator.playSound(data->deathSound);
    
    data->networkedAnimator.update(0.0, &data->animatorDynamicTarget);
    
    renderCallback->addAudios(data->animatorDynamicTarget.pullNewAudios());
    renderCallback->addParticles(data->animatorDynamicTarget.pullNewParticles());
    renderCallback->addParticles(data->statusController->pullNewParticles());
  }
  
  data->deathDamageSourceKinds.clear();
  
  if (isMaster())
    setNetStates();
}

List<LightSource> MonsterAdapter::lightSources() const {
  auto const* data = getData();
  auto lightSources = data->networkedAnimator.lightSources(position());
  lightSources.appendAll(data->statusController->lightSources());
  return lightSources;
}

void MonsterAdapter::hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) {
  if (inWorld() && isMaster())
    getData()->statusController->hitOther(targetEntityId, damageRequest);
}

void MonsterAdapter::damagedOther(DamageNotification const& damage) {
  if (inWorld() && isMaster())
    getData()->statusController->damagedOther(damage);
}

void MonsterAdapter::update(float dt, uint64_t) {
  if (!inWorld())
    return;
  
  auto* data = getData();
  
  data->movementController->setTimestep(dt);
  
  if (isMaster()) {
    data->networkedAnimator.setFlipped((data->movementController->facingDirection() == Direction::Left) != data->variant.reversed);
    
    if (data->knockedOut) {
      data->knockoutTimer -= dt;
    } else {
      if (data->scriptComponent.updateReady())
        data->physicsForces = {};
      data->scriptComponent.update(data->scriptComponent.updateDt(dt));
      
      if (shouldDie())
        knockout();
    }
    
    data->movementController->tickMaster(dt);
    
    data->statusController->tickMaster(dt);
    updateStatus(dt);
  } else {
    m_netGroup.tickNetInterpolation(dt);
    
    data->statusController->tickSlave(dt);
    updateStatus(dt);
    
    data->movementController->tickSlave(dt);
  }
  
  if (EntityAdapter::world()->isServer()) {
    data->networkedAnimator.update(dt, nullptr);
  } else {
    data->networkedAnimator.update(dt, &data->animatorDynamicTarget);
    data->animatorDynamicTarget.updatePosition(position());
    
    data->scriptedAnimator.update();
    
    SpatialLogger::logPoly("world", data->movementController->collisionBody(), { 255, 0, 0, 255 });
  }
}

void MonsterAdapter::render(RenderCallback* renderCallback) {
  auto* data = getData();
  
  for (auto& drawable : data->networkedAnimator.drawables(position())) {
    if (drawable.isImage())
      drawable.imagePart().addDirectivesGroup(data->statusController->parentDirectives(), true);
    renderCallback->addDrawable(std::move(drawable), data->variant.renderLayer);
  }
  
  renderCallback->addAudios(data->animatorDynamicTarget.pullNewAudios());
  renderCallback->addParticles(data->animatorDynamicTarget.pullNewParticles());
  
  renderCallback->addDrawables(data->statusController->drawables(), data->variant.renderLayer);
  renderCallback->addParticles(data->statusController->pullNewParticles());
  renderCallback->addAudios(data->statusController->pullNewAudios());
  
  data->effectEmitter.render(renderCallback);
  
  for (auto drawablePair : data->scriptedAnimator.drawables())
    renderCallback->addDrawable(drawablePair.first, drawablePair.second.value(data->variant.renderLayer));
  renderCallback->addAudios(data->scriptedAnimator.pullNewAudios());
  renderCallback->addParticles(data->scriptedAnimator.pullNewParticles());
}

void MonsterAdapter::renderLightSources(RenderCallback* renderCallback) {
  auto const* data = getData();
  renderCallback->addLightSources(data->networkedAnimator.lightSources(position()));
  renderCallback->addLightSources(data->statusController->lightSources());
  renderCallback->addLightSources(data->scriptedAnimator.lightSources());
}

void MonsterAdapter::setPosition(Vec2F const& pos) {
  getData()->movementController->setPosition(pos);
}

Maybe<Json> MonsterAdapter::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) {
  auto* data = getData();
  Maybe<Json> result = data->scriptComponent.handleMessage(message, EntityAdapter::world()->connection() == sendingConnection, args);
  if (!result)
    result = data->statusController->receiveMessage(message, EntityAdapter::world()->connection() == sendingConnection, args);
  return result;
}

float MonsterAdapter::maxHealth() const {
  return *getData()->statusController->resourceMax("health");
}

float MonsterAdapter::health() const {
  return getData()->statusController->resource("health");
}

DamageBarType MonsterAdapter::damageBar() const {
  return getData()->damageBar;
}

Vec2F MonsterAdapter::getAbsolutePosition(Vec2F relativePosition) const {
  auto const* data = getData();
  if (data->movementController->facingDirection() == Direction::Left)
    relativePosition[0] *= -1;
  if (data->movementController->rotation() != 0)
    relativePosition = relativePosition.rotate(data->movementController->rotation());
  return data->movementController->position() + relativePosition;
}

void MonsterAdapter::updateStatus(float dt) {
  auto* data = getData();
  data->effectEmitter.setSourcePosition("normal", position());
  data->effectEmitter.setSourcePosition("mouth", position() + mouthOffset());
  data->effectEmitter.setSourcePosition("feet", position() + feetOffset());
  data->effectEmitter.setDirection(data->movementController->facingDirection());
  data->effectEmitter.tick(dt, *entityMode());
}

LuaCallbacks MonsterAdapter::makeMonsterCallbacks() {
  LuaCallbacks callbacks;
  
  callbacks.registerCallback("type", [this]() {
      return getData()->variant.type;
    });
  
  callbacks.registerCallback("seed", [this]() {
      return toString(getData()->variant.seed);
    });
  
  callbacks.registerCallback("uniqueParameters", [this]() {
      return getData()->variant.uniqueParameters;
    });
  
  callbacks.registerCallback("level", [this]() {
      return *getData()->level;
    });
  
  callbacks.registerCallback("setDamageOnTouch", [this](bool arg1) {
      getData()->damageOnTouch = arg1;
    });
  
  callbacks.registerCallback("setDamageSources", [this](Maybe<JsonArray> const& damageSources) {
      getData()->damageSources = damageSources.value().transformed(construct<DamageSource>());
    });
  
  callbacks.registerCallback("setDamageParts", [this](StringSet const& parts) {
      getData()->animationDamageParts = parts;
    });
  
  callbacks.registerCallback("setAggressive", [this](bool arg1) {
      getData()->aggressive = arg1;
    });
  
  callbacks.registerCallback("setActiveSkillName", [this](Maybe<String> const& activeSkillName) {
      getData()->activeSkillName = activeSkillName.value();
    });
  
  callbacks.registerCallback("setDropPool", [this](Json dropPool) {
      getData()->dropPool = std::move(dropPool);
    });
  
  callbacks.registerCallback("toAbsolutePosition", [this](Vec2F const& p) {
      return getAbsolutePosition(p);
    });
  
  callbacks.registerCallback("mouthPosition", [this]() {
      return mouthPosition();
    });
  
  callbacks.registerCallback("flyTo", [this](Vec2F const& arg1) {
      getData()->movementController->controlFly(EntityAdapter::world()->geometry().diff(arg1, position()));
    });
  
  callbacks.registerCallback("setDeathParticleBurst", [this](Maybe<String> const& arg1) {
      getData()->deathParticleBurst = arg1.value();
    });
  
  callbacks.registerCallback("setDeathSound", [this](Maybe<String> const& arg1) {
      getData()->deathSound = arg1.value();
    });
  
  callbacks.registerCallback("setPhysicsForces", [this](JsonArray const& forces) {
      getData()->physicsForces = forces.transformed(jsonToPhysicsForceRegion);
    });
  
  callbacks.registerCallback("setName", [this](String const& name) {
      getData()->name = name;
    });
  callbacks.registerCallback("setDisplayNametag", [this](bool display) {
      getData()->displayNametag = display;
    });
  
  callbacks.registerCallback("say", [this](String line, Maybe<StringMap<String>> const& tags) {
      if (tags)
        line = line.replaceTags(*tags, false);
      
      if (!line.empty()) {
        addChatMessage(line);
        return true;
      }
      
      return false;
    });
  
  callbacks.registerCallback("sayPortrait", [this](String line, String portrait, Maybe<StringMap<String>> const& tags) {
      if (tags)
        line = line.replaceTags(*tags, false);
      
      if (!line.empty()) {
        addChatMessage(line, portrait);
        return true;
      }
      
      return false;
    });
  
  callbacks.registerCallback("setDamageTeam", [this](Json const& team) {
      setTeam(EntityDamageTeam(team));
    });
  
  callbacks.registerCallback("setUniqueId", [this](Maybe<String> uniqueId) {
      EntityAdapter::setUniqueId(uniqueId);
    });
  
  callbacks.registerCallback("setDamageBar", [this](String const& damageBarType) {
      getData()->damageBar = DamageBarTypeNames.getLeft(damageBarType);
    });
  
  callbacks.registerCallback("setInteractive", [this](bool interactive) {
      getData()->interactive = interactive;
    });
  
  callbacks.registerCallback("setAnimationParameter", [this](String name, Json value) {
      getData()->scriptedAnimationParameters[std::move(name)] = std::move(value);
    });
  
  return callbacks;
}

void MonsterAdapter::addChatMessage(String const& message, String const& portrait) {
  auto* data = getData();
  data->chatMessage = message;
  data->chatPortrait = portrait;
  m_newChatMessageEvent.trigger();
  if (portrait.empty())
    data->pendingChatActions.append(SayChatAction{entityId(), message, mouthPosition()});
  else
    data->pendingChatActions.append(PortraitChatAction{entityId(), portrait, message, mouthPosition()});
}

void MonsterAdapter::setupNetStates() {
  auto* data = getData();
  
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
  m_netGroup.addNetElement(&m_nameNetState);
  m_netGroup.addNetElement(&m_displayNametagNetState);
  m_netGroup.addNetElement(&m_dropPoolNetState);
  m_netGroup.addNetElement(&m_physicsForces);
  
  m_netGroup.addNetElement(&data->networkedAnimator);
  m_netGroup.addNetElement(data->movementController.get());
  m_netGroup.addNetElement(data->statusController.get());
  m_netGroup.addNetElement(&data->effectEmitter);
  
  m_netGroup.addNetElement(&m_newChatMessageEvent);
  m_netGroup.addNetElement(&m_chatMessage);
  m_netGroup.addNetElement(&m_chatPortrait);
  
  m_netGroup.addNetElement(&m_damageBarNetState);
  m_netGroup.addNetElement(&m_interactiveNetState);
  
  // don't interpolate scripted animation parameters or animation damage parts
  m_netGroup.addNetElement(&m_animationDamageParts, false);
  m_netGroup.addNetElement(&m_scriptedAnimationParameters, false);
  
  m_netGroup.setNeedsLoadCallback(bind(&MonsterAdapter::getNetStates, this, _1));
  m_netGroup.setNeedsStoreCallback(bind(&MonsterAdapter::setNetStates, this));
}

void MonsterAdapter::setNetStates() {
  auto* data = getData();
  
  m_uniqueIdNetState.set(uniqueId());
  m_teamNetState.set(getTeam());
  m_monsterLevelNetState.set(data->level);
  m_damageOnTouchNetState.set(data->damageOnTouch);
  m_damageSources.set(data->damageSources);
  m_aggressiveNetState.set(aggressive());
  m_knockedOutNetState.set(data->knockedOut);
  m_deathParticleBurstNetState.set(data->deathParticleBurst);
  m_deathSoundNetState.set(data->deathSound);
  m_activeSkillNameNetState.set(data->activeSkillName);
  m_nameNetState.set(data->name);
  m_displayNametagNetState.set(data->displayNametag);
  m_dropPoolNetState.set(data->dropPool);
  m_physicsForces.set(data->physicsForces);
  m_damageBarNetState.set(data->damageBar);
  m_interactiveNetState.set(data->interactive);
}

void MonsterAdapter::getNetStates(bool initial) {
  auto* data = getData();
  
  setUniqueId(m_uniqueIdNetState.get());
  setTeam(m_teamNetState.get());
  data->level = m_monsterLevelNetState.get();
  data->damageOnTouch = m_damageOnTouchNetState.get();
  data->damageSources = m_damageSources.get();
  data->aggressive = m_aggressiveNetState.get();
  data->knockedOut = m_knockedOutNetState.get();
  if (m_deathParticleBurstNetState.pullUpdated())
    data->deathParticleBurst = m_deathParticleBurstNetState.get();
  if (m_deathSoundNetState.pullUpdated())
    data->deathSound = m_deathSoundNetState.get();
  if (m_activeSkillNameNetState.pullUpdated())
    data->activeSkillName = m_activeSkillNameNetState.get();
  data->name = m_nameNetState.get();
  data->displayNametag = m_displayNametagNetState.get();
  if (m_dropPoolNetState.pullUpdated())
    data->dropPool = m_dropPoolNetState.get();
  data->physicsForces = m_physicsForces.get();
  data->damageBar = m_damageBarNetState.get();
  data->interactive = m_interactiveNetState.get();
  data->animationDamageParts = m_animationDamageParts.get();
  
  if (m_newChatMessageEvent.pullOccurred() && !initial) {
    if (m_chatPortrait.get().empty())
      data->pendingChatActions.append(SayChatAction{entityId(), m_chatMessage.get(), mouthPosition()});
    else
      data->pendingChatActions.append(
          PortraitChatAction{entityId(), m_chatPortrait.get(), m_chatMessage.get(), mouthPosition()});
  }
}

float MonsterAdapter::monsterLevel() const {
  return *getData()->level;
}

MonsterSkillInfo MonsterAdapter::activeSkillInfo() const {
  MonsterSkillInfo skillInfo;
  
  auto const* data = getData();
  if (!data->activeSkillName.empty()) {
    auto monsterDatabase = Root::singleton().monsterDatabase();
    auto monsterSkillInfo = monsterDatabase->skillInfo(data->activeSkillName);
    skillInfo.label = monsterSkillInfo.first;
    skillInfo.image = monsterSkillInfo.second;
  }
  
  return skillInfo;
}

List<Drawable> MonsterAdapter::portrait(PortraitMode) const {
  auto const* data = getData();
  if (data->variant.portraitIcon) {
    return {Drawable::makeImage(*data->variant.portraitIcon, 1.0f, true, Vec2F())};
  } else {
    auto animator = data->networkedAnimator;
    animator.setFlipped(!data->variant.reversed);
    auto drawables = animator.drawables();
    Drawable::scaleAll(drawables, TilePixels);
    return drawables;
  }
}

String MonsterAdapter::typeName() const {
  return getData()->variant.type;
}

MonsterVariant MonsterAdapter::monsterVariant() const {
  return getData()->variant;
}

Maybe<String> MonsterAdapter::statusText() const {
  return {};
}

bool MonsterAdapter::displayNametag() const {
  return getData()->displayNametag;
}

Vec3B MonsterAdapter::nametagColor() const {
  return getData()->variant.nametagColor;
}

Vec2F MonsterAdapter::nametagOrigin() const {
  return mouthPosition(false);
}

String MonsterAdapter::nametag() const {
  return name();
}

bool MonsterAdapter::aggressive() const {
  return getData()->aggressive;
}

Maybe<LuaValue> MonsterAdapter::callScript(String const& func, LuaVariadic<LuaValue> const& args) {
  return getData()->scriptComponent.invoke(func, args);
}

Maybe<LuaValue> MonsterAdapter::evalScript(String const& code) {
  return getData()->scriptComponent.eval(code);
}

Vec2F MonsterAdapter::mouthPosition() const {
  return mouthOffset() + position();
}

Vec2F MonsterAdapter::mouthPosition(bool) const {
  return mouthPosition();
}

List<ChatAction> MonsterAdapter::pullPendingChatActions() {
  return std::move(getData()->pendingChatActions);
}

List<PhysicsForceRegion> MonsterAdapter::forceRegions() const {
  return getData()->physicsForces;
}

InteractAction MonsterAdapter::interact(InteractRequest const& request) {
  auto result = getData()->scriptComponent.invoke<Json>("interact", JsonObject{{"sourceId", request.sourceId}, {"sourcePosition", jsonFromVec2F(request.sourcePosition)}}).value();
  
  if (result.isNull())
    return {};
  
  if (result.isType(Json::Type::String))
    return InteractAction(result.toString(), entityId(), {});
  
  return InteractAction(result.getString(0), entityId(), result.get(1));
}

bool MonsterAdapter::isInteractive() const {
  return getData()->interactive;
}

Vec2F MonsterAdapter::questIndicatorPosition() const {
  Vec2F pos = position() + getData()->questIndicatorOffset;
  pos[1] += collisionArea().yMax();
  return pos;
}

ActorMovementController* MonsterAdapter::movementController() {
  return getData()->movementController.get();
}

StatusController* MonsterAdapter::statusController() {
  return getData()->statusController.get();
}

MonsterDataComponent* MonsterAdapter::getData() {
  return m_ecsWorld.getComponent<MonsterDataComponent>(m_entity);
}

MonsterDataComponent const* MonsterAdapter::getData() const {
  return m_ecsWorld.getComponent<MonsterDataComponent>(m_entity);
}

} // namespace ECS
} // namespace Star
