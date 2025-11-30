// ECS Vehicle Adapter Implementation for OpenStarbound

#include "StarVehicleAdapter.hpp"
#include "StarVehicleDatabase.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"
#include "StarWorld.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarLogging.hpp"

namespace Star {
namespace ECS {

// Static factory methods

shared_ptr<VehicleAdapter> VehicleAdapter::create(
    World* ecsWorld,
    Json baseConfig,
    String path,
    Json dynamicConfig) {
  
  Entity entity = ecsWorld->createEntity();
  
  // Add tag component
  ecsWorld->addComponent<VehicleTag>(entity);
  
  // Add data component
  auto& data = ecsWorld->addComponent<VehicleDataComponent>(entity);
  data.typeName = baseConfig.getString("name");
  data.baseConfig = baseConfig;
  data.path = path;
  data.dynamicConfig = dynamicConfig;
  
  // Parse configuration
  Json config = baseConfig;
  if (!dynamicConfig.isNull()) {
    config = jsonMerge(config, dynamicConfig);
  }
  
  data.boundBox = RectF(jsonToRectF(config.get("boundBox", JsonArray{-2, -2, 2, 2})));
  data.slaveControlTimeout = config.getFloat("slaveControlTimeout", 2.0f);
  data.receiveExtraControls = config.getBool("receiveExtraControls", false);
  data.clientEntityMode = ClientEntityModeNames.getLeft(config.getString("clientEntityMode", "ClientSlaveOnly"));
  data.interactive = config.getBool("interactive", true);
  
  // Parse lounge positions
  for (auto const& loungeConfig : config.getArray("loungePositions", JsonArray())) {
    VehicleLoungePositionConfig loungePos;
    
    String posName = loungeConfig.getString("name");
    loungePos.part = loungeConfig.getString("part");
    loungePos.partAnchor = loungeConfig.getString("partAnchor");
    
    if (loungeConfig.contains("exitBottomOffset"))
      loungePos.exitBottomOffset = jsonToVec2F(loungeConfig.get("exitBottomOffset"));
    
    loungePos.armorCosmeticOverrides = loungeConfig.getObject("armorCosmeticOverrides", JsonObject());
    
    if (loungeConfig.contains("cursorOverride"))
      loungePos.cursorOverride = loungeConfig.getString("cursorOverride");
    
    if (loungeConfig.contains("suppressTools"))
      loungePos.suppressTools = loungeConfig.getBool("suppressTools");
    
    loungePos.cameraFocus = loungeConfig.getBool("cameraFocus", false);
    loungePos.enabled = true;
    loungePos.orientation = LoungeOrientationNames.getLeft(loungeConfig.getString("orientation", "None"));
    
    if (loungeConfig.contains("emote"))
      loungePos.emote = loungeConfig.getString("emote");
    
    if (loungeConfig.contains("dance"))
      loungePos.dance = loungeConfig.getString("dance");
    
    data.loungePositions.set(posName, loungePos);
  }
  
  // Parse moving collisions
  for (auto const& collisionConfig : config.getArray("movingCollisions", JsonArray())) {
    VehicleMovingCollisionConfig collision;
    
    String collisionName = collisionConfig.getString("name");
    collision.movingCollision.poly = jsonToPolyF(collisionConfig.get("poly"));
    collision.movingCollision.collisionKind = CollisionKindNames.getLeft(
      collisionConfig.getString("collisionKind", "Slippery"));
    
    if (collisionConfig.contains("attachToPart"))
      collision.attachToPart = collisionConfig.getString("attachToPart");
    
    collision.enabled = collisionConfig.getBool("enabled", true);
    
    data.movingCollisions.set(collisionName, collision);
  }
  
  // Parse force regions
  for (auto const& forceConfig : config.getArray("forceRegions", JsonArray())) {
    VehicleForceRegionConfig forceRegion;
    
    String forceName = forceConfig.getString("name");
    forceRegion.forceRegion.region = jsonToPolyF(forceConfig.get("region"));
    forceRegion.forceRegion.xTargetVelocity = forceConfig.getFloat("xTargetVelocity", 0.0f);
    forceRegion.forceRegion.yTargetVelocity = forceConfig.getFloat("yTargetVelocity", 0.0f);
    forceRegion.forceRegion.controlForce = forceConfig.getFloat("controlForce", 0.0f);
    forceRegion.forceRegion.categoryFilter = PhysicsCategoryFilter::whitelist({"player", "monster", "npc", "itemdrop"});
    
    if (forceConfig.contains("attachToPart"))
      forceRegion.attachToPart = forceConfig.getString("attachToPart");
    
    forceRegion.enabled = forceConfig.getBool("enabled", true);
    
    data.forceRegions.set(forceName, forceRegion);
  }
  
  // Parse damage sources
  for (auto const& damageConfig : config.getArray("damageSources", JsonArray())) {
    VehicleDamageSourceConfig damageSource;
    
    String damageName = damageConfig.getString("name");
    damageSource.damageSource = DamageSource(damageConfig);
    
    if (damageConfig.contains("attachToPart"))
      damageSource.attachToPart = damageConfig.getString("attachToPart");
    
    damageSource.enabled = damageConfig.getBool("enabled", true);
    
    data.damageSources.set(damageName, damageSource);
  }
  
  // Parse damage team
  if (config.contains("damageTeam")) {
    data.damageTeam = EntityDamageTeam(config.get("damageTeam"));
  }
  
  // Parse render layer
  data.baseRenderLayer = parseRenderLayer(config.getString("renderLayer", "Vehicle"));
  
  auto adapter = make_shared<VehicleAdapter>(ecsWorld, entity);
  return adapter;
}

shared_ptr<VehicleAdapter> VehicleAdapter::createFromDiskStore(
    World* ecsWorld,
    Json const& diskStore) {
  
  String vehicleName = diskStore.getString("name");
  String path = diskStore.getString("path", "");
  
  auto vehicleDatabase = Root::singleton().vehicleDatabase();
  Json baseConfig = vehicleDatabase->baseConfig(vehicleName);
  Json dynamicConfig = diskStore.get("dynamicConfig", Json());
  
  auto adapter = create(ecsWorld, baseConfig, path, dynamicConfig);
  adapter->diskLoad(diskStore);
  
  return adapter;
}

// Constructor

VehicleAdapter::VehicleAdapter(World* ecsWorld, Entity ecsEntity)
  : EntityAdapter(ecsWorld, ecsEntity) {
  
  auto const* data = getComponent<VehicleDataComponent>();
  if (data) {
    // Initialize movement controller
    if (data->baseConfig.contains("movementSettings")) {
      m_movementController = MovementController(data->baseConfig.get("movementSettings"));
    }
    
    // Initialize animator
    if (data->baseConfig.contains("animation")) {
      auto assets = Root::singleton().assets();
      String animationPath = AssetPath::relativeTo(data->path, data->baseConfig.getString("animation"));
      m_networkedAnimator = NetworkedAnimator(assets->json(animationPath), animationPath);
    }
  }
  
  setupNetStates();
}

// Accessors

String VehicleAdapter::name() const {
  auto const* data = getComponent<VehicleDataComponent>();
  return data ? data->typeName : "";
}

Json VehicleAdapter::baseConfig() const {
  auto const* data = getComponent<VehicleDataComponent>();
  return data ? data->baseConfig : Json();
}

Json VehicleAdapter::dynamicConfig() const {
  auto const* data = getComponent<VehicleDataComponent>();
  return data ? data->dynamicConfig : Json();
}

// Serialization

Json VehicleAdapter::diskStore() const {
  auto const* data = getComponent<VehicleDataComponent>();
  if (!data)
    return Json();
  
  JsonObject store;
  store["name"] = data->typeName;
  store["path"] = data->path;
  
  if (!data->dynamicConfig.isNull())
    store["dynamicConfig"] = data->dynamicConfig;
  
  // Store position and velocity
  store["position"] = jsonFromVec2F(position());
  store["velocity"] = jsonFromVec2F(velocity());
  
  // Store script data
  Json scriptStorage = m_scriptComponent.getStoredData();
  if (!scriptStorage.isNull())
    store["scriptStorage"] = scriptStorage;
  
  return store;
}

void VehicleAdapter::diskLoad(Json diskStore) {
  if (diskStore.contains("position")) {
    setPosition(jsonToVec2F(diskStore.get("position")));
  }
  
  if (diskStore.contains("velocity")) {
    m_movementController.setVelocity(jsonToVec2F(diskStore.get("velocity")));
  }
  
  if (diskStore.contains("scriptStorage")) {
    m_scriptComponent.setStoredData(diskStore.get("scriptStorage"));
  }
}

// Entity interface

EntityType VehicleAdapter::entityType() const {
  return EntityType::Vehicle;
}

ClientEntityMode VehicleAdapter::clientEntityMode() const {
  auto const* data = getComponent<VehicleDataComponent>();
  return data ? data->clientEntityMode : ClientEntityMode::ClientSlaveOnly;
}

List<DamageSource> VehicleAdapter::damageSources() const {
  auto const* data = getComponent<VehicleDataComponent>();
  if (!data)
    return {};
  
  List<DamageSource> sources;
  
  for (auto const& pair : data->damageSources) {
    if (pair.second.enabled) {
      DamageSource source = pair.second.damageSource;
      source.sourceEntityId = entityId();
      source.team = data->damageTeam;
      
      // Apply part transformation if attached
      if (pair.second.attachToPart) {
        auto transform = m_networkedAnimator.partTransformation(*pair.second.attachToPart);
        source.damageArea.transform(transform);
      }
      
      sources.append(source);
    }
  }
  
  return sources;
}

Maybe<HitType> VehicleAdapter::queryHit(DamageSource const& source) const {
  auto poly = hitPoly();
  if (!poly)
    return {};
  
  if (source.intersectsWithPoly(world()->geometry(), *poly))
    return HitType::Hit;
  
  return {};
}

Maybe<PolyF> VehicleAdapter::hitPoly() const {
  auto const* data = getComponent<VehicleDataComponent>();
  if (!data)
    return {};
  
  return PolyF(data->boundBox).translated(position());
}

List<DamageNotification> VehicleAdapter::applyDamage(DamageRequest const& damage) {
  // Let script handle damage
  auto result = m_scriptComponent.invoke<Json>("applyDamage", damage.toJson());
  
  if (result && result->isType(Json::Type::Array)) {
    List<DamageNotification> notifications;
    for (auto const& notificationJson : result->toArray()) {
      notifications.append(DamageNotification(notificationJson));
    }
    return notifications;
  }
  
  return {};
}

List<DamageNotification> VehicleAdapter::selfDamageNotifications() {
  return {};
}

void VehicleAdapter::init(Star::World* world, EntityId entityId, EntityMode mode) {
  EntityAdapter::init(world, entityId, mode);
  
  auto* data = getComponent<VehicleDataComponent>();
  if (!data)
    return;
  
  m_movementController.init(world);
  m_networkedAnimator.init();
  
  // Initialize script
  if (data->baseConfig.contains("scripts")) {
    StringList scripts;
    for (auto const& script : data->baseConfig.getArray("scripts")) {
      scripts.append(AssetPath::relativeTo(data->path, script.toString()));
    }
    
    m_scriptComponent.setScripts(scripts);
    m_scriptComponent.setUpdateDelta(data->baseConfig.getUInt("scriptDelta", 1));
    
    m_scriptComponent.addCallbacks("vehicle", makeVehicleCallbacks());
    m_scriptComponent.init(world);
  }
}

void VehicleAdapter::uninit() {
  m_scriptComponent.uninit();
  m_scriptComponent.removeCallbacks("vehicle");
  
  m_movementController.uninit();
  m_networkedAnimator.uninit();
  
  EntityAdapter::uninit();
}

Vec2F VehicleAdapter::position() const {
  return m_movementController.position();
}

RectF VehicleAdapter::metaBoundBox() const {
  auto const* data = getComponent<VehicleDataComponent>();
  return data ? data->boundBox : RectF();
}

RectF VehicleAdapter::collisionArea() const {
  return metaBoundBox().translated(position());
}

Vec2F VehicleAdapter::velocity() const {
  return m_movementController.velocity();
}

pair<ByteArray, uint64_t> VehicleAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void VehicleAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
  setNetStates();
}

void VehicleAdapter::enableInterpolation(float extrapolationHint) {
  m_movementController.enableInterpolation(extrapolationHint);
  m_networkedAnimator.enableInterpolation(extrapolationHint);
}

void VehicleAdapter::disableInterpolation() {
  m_movementController.disableInterpolation();
  m_networkedAnimator.disableInterpolation();
}

void VehicleAdapter::update(float dt, uint64_t currentStep) {
  auto* data = getComponent<VehicleDataComponent>();
  if (!data)
    return;
  
  if (isMaster()) {
    // Update script
    m_scriptComponent.update(m_scriptComponent.updateDt(currentStep));
    
    // Update movement
    m_movementController.tickMaster(dt);
    
    // Update slave heartbeat timers
    for (auto& pair : data->aliveMasterConnections) {
      if (pair.second.tick(dt)) {
        // Timeout this connection
        data->aliveMasterConnections.remove(pair.first);
      }
    }
    
    // Get network states
    getNetStates();
  } else {
    // Slave update
    m_movementController.tickSlave(dt);
    
    // Update slave heartbeat
    if (data->slaveHeartbeatTimer.tick(dt)) {
      // Send heartbeat to master
      data->slaveHeartbeatTimer.reset();
    }
  }
  
  // Update animator
  m_networkedAnimator.update(dt, &m_networkedAnimatorDynamicTarget);
  
  // Update scripted animator
  m_scriptedAnimator.update(dt);
}

void VehicleAdapter::render(RenderCallback* renderer) {
  auto const* data = getComponent<VehicleDataComponent>();
  if (!data)
    return;
  
  Vec2F pos = position();
  
  // Render back layer
  for (auto& drawable : m_networkedAnimator.drawables(pos)) {
    renderer->addDrawable(drawable, renderLayer(VehicleLayerType::Back));
  }
  
  // Render front layer
  for (auto& drawable : m_networkedAnimator.drawables(pos)) {
    renderer->addDrawable(drawable, renderLayer(VehicleLayerType::Front));
  }
}

void VehicleAdapter::renderLightSources(RenderCallback* renderer) {
  for (auto const& light : lightSources()) {
    renderer->addLightSource(light);
  }
}

List<LightSource> VehicleAdapter::lightSources() const {
  return m_networkedAnimator.lightSources(position());
}

bool VehicleAdapter::shouldDestroy() const {
  auto const* data = getComponent<VehicleDataComponent>();
  return data ? data->shouldDestroy : false;
}

void VehicleAdapter::destroy(RenderCallback* renderCallback) {
  // Spawn destruction effects
  m_scriptComponent.invoke("destroy");
}

Maybe<Json> VehicleAdapter::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) {
  return m_scriptComponent.handleMessage(message, sendingConnection == world()->connection(), args);
}

// InteractiveEntity interface

RectF VehicleAdapter::interactiveBoundBox() const {
  return metaBoundBox();
}

bool VehicleAdapter::isInteractive() const {
  auto const* data = getComponent<VehicleDataComponent>();
  return data ? data->interactive : false;
}

InteractAction VehicleAdapter::interact(InteractRequest const& request) {
  auto result = m_scriptComponent.invoke<Json>("interact", request.toJson());
  
  if (result && !result->isNull()) {
    return InteractAction(*result);
  }
  
  return {};
}

// LoungeableEntity interface

size_t VehicleAdapter::anchorCount() const {
  auto const* data = getComponent<VehicleDataComponent>();
  return data ? data->loungePositions.size() : 0;
}

LoungeAnchorConstPtr VehicleAdapter::loungeAnchor(size_t positionIndex) const {
  auto const* data = getComponent<VehicleDataComponent>();
  if (!data || positionIndex >= data->loungePositions.size())
    return {};
  
  // Check cache
  if (m_loungeAnchorCache.contains(positionIndex))
    return m_loungeAnchorCache.get(positionIndex);
  
  auto it = data->loungePositions.begin();
  std::advance(it, positionIndex);
  auto const& loungePos = it->second;
  
  if (!loungePos.enabled)
    return {};
  
  auto anchor = make_shared<LoungeAnchor>();
  
  // Get position from animator part
  if (!loungePos.part.empty()) {
    auto partPos = m_networkedAnimator.partPoint(loungePos.part, loungePos.partAnchor);
    anchor->position = position() + partPos;
  } else {
    anchor->position = position();
  }
  
  anchor->exitBottomPosition = loungePos.exitBottomOffset;
  anchor->orientation = loungePos.orientation;
  anchor->armorCosmeticOverrides = loungePos.armorCosmeticOverrides;
  anchor->cursorOverride = loungePos.cursorOverride;
  anchor->suppressTools = loungePos.suppressTools;
  anchor->cameraFocus = loungePos.cameraFocus;
  anchor->emote = loungePos.emote;
  anchor->dance = loungePos.dance;
  anchor->directives = loungePos.directives;
  anchor->statusEffects = loungePos.statusEffects;
  
  const_cast<VehicleAdapter*>(this)->m_loungeAnchorCache.set(positionIndex, anchor);
  
  return anchor;
}

void VehicleAdapter::loungeControl(size_t positionIndex, LoungeControl loungeControl) {
  auto* data = getComponent<VehicleDataComponent>();
  if (!data || positionIndex >= data->loungePositions.size())
    return;
  
  auto it = data->loungePositions.begin();
  std::advance(it, positionIndex);
  
  it->second.slaveNewControls.add(loungeControl);
}

void VehicleAdapter::loungeAim(size_t positionIndex, Vec2F const& aimPosition) {
  auto* data = getComponent<VehicleDataComponent>();
  if (!data || positionIndex >= data->loungePositions.size())
    return;
  
  auto it = data->loungePositions.begin();
  std::advance(it, positionIndex);
  
  it->second.slaveNewAimPosition = aimPosition;
}

// PhysicsEntity interface

List<PhysicsForceRegion> VehicleAdapter::forceRegions() const {
  auto const* data = getComponent<VehicleDataComponent>();
  if (!data)
    return {};
  
  List<PhysicsForceRegion> regions;
  
  for (auto const& pair : data->forceRegions) {
    if (pair.second.enabled) {
      PhysicsForceRegion region = pair.second.forceRegion;
      
      // Apply part transformation if attached
      if (pair.second.attachToPart) {
        auto transform = m_networkedAnimator.partTransformation(*pair.second.attachToPart);
        region.region.transform(transform);
      }
      
      region.region.translate(position());
      regions.append(region);
    }
  }
  
  return regions;
}

size_t VehicleAdapter::movingCollisionCount() const {
  auto const* data = getComponent<VehicleDataComponent>();
  return data ? data->movingCollisions.size() : 0;
}

Maybe<PhysicsMovingCollision> VehicleAdapter::movingCollision(size_t positionIndex) const {
  auto const* data = getComponent<VehicleDataComponent>();
  if (!data || positionIndex >= data->movingCollisions.size())
    return {};
  
  auto it = data->movingCollisions.begin();
  std::advance(it, positionIndex);
  
  if (!it->second.enabled)
    return {};
  
  PhysicsMovingCollision collision = it->second.movingCollision;
  
  // Apply part transformation if attached
  if (it->second.attachToPart) {
    auto transform = m_networkedAnimator.partTransformation(*it->second.attachToPart);
    collision.poly.transform(transform);
  }
  
  collision.poly.translate(position());
  collision.position = position();
  
  return collision;
}

// ScriptedEntity interface

Maybe<LuaValue> VehicleAdapter::callScript(String const& func, LuaVariadic<LuaValue> const& args) {
  return m_scriptComponent.invoke(func, args);
}

Maybe<LuaValue> VehicleAdapter::evalScript(String const& code) {
  return m_scriptComponent.eval(code);
}

void VehicleAdapter::setPosition(Vec2F const& pos) {
  m_movementController.setPosition(pos);
}

// Private methods

EntityRenderLayer VehicleAdapter::renderLayer(VehicleLayerType vehicleLayer) const {
  auto const* data = getComponent<VehicleDataComponent>();
  if (!data)
    return RenderLayerVehicle;
  
  EntityRenderLayer base = data->overrideRenderLayer.value(data->baseRenderLayer);
  
  switch (vehicleLayer) {
    case VehicleLayerType::Back:
      return base - 1;
    case VehicleLayerType::Passenger:
      return base;
    case VehicleLayerType::Front:
      return base + 1;
    default:
      return base;
  }
}

LuaCallbacks VehicleAdapter::makeVehicleCallbacks() {
  LuaCallbacks callbacks;
  
  callbacks.registerCallback("name", [this]() -> String {
    return name();
  });
  
  callbacks.registerCallback("position", [this]() -> Vec2F {
    return position();
  });
  
  callbacks.registerCallback("velocity", [this]() -> Vec2F {
    return velocity();
  });
  
  callbacks.registerCallback("setPosition", [this](Vec2F pos) {
    setPosition(pos);
  });
  
  callbacks.registerCallback("setVelocity", [this](Vec2F vel) {
    m_movementController.setVelocity(vel);
  });
  
  callbacks.registerCallback("applyMovementForce", [this](Vec2F force) {
    m_movementController.addMomentum(force);
  });
  
  callbacks.registerCallback("setInteractive", [this](bool interactive) {
    auto* data = getComponent<VehicleDataComponent>();
    if (data) {
      data->interactive = interactive;
      markNetworkDirty();
    }
  });
  
  callbacks.registerCallback("setLoungeEnabled", [this](String const& name, bool enabled) {
    auto* data = getComponent<VehicleDataComponent>();
    if (data && data->loungePositions.contains(name)) {
      data->loungePositions.get(name).enabled = enabled;
      markNetworkDirty();
    }
  });
  
  callbacks.registerCallback("setMovingCollisionEnabled", [this](String const& name, bool enabled) {
    auto* data = getComponent<VehicleDataComponent>();
    if (data && data->movingCollisions.contains(name)) {
      data->movingCollisions.get(name).enabled = enabled;
      markNetworkDirty();
    }
  });
  
  callbacks.registerCallback("setForceRegionEnabled", [this](String const& name, bool enabled) {
    auto* data = getComponent<VehicleDataComponent>();
    if (data && data->forceRegions.contains(name)) {
      data->forceRegions.get(name).enabled = enabled;
      markNetworkDirty();
    }
  });
  
  callbacks.registerCallback("setDamageSourceEnabled", [this](String const& name, bool enabled) {
    auto* data = getComponent<VehicleDataComponent>();
    if (data && data->damageSources.contains(name)) {
      data->damageSources.get(name).enabled = enabled;
      markNetworkDirty();
    }
  });
  
  callbacks.registerCallback("destroy", [this]() {
    auto* data = getComponent<VehicleDataComponent>();
    if (data) {
      data->shouldDestroy = true;
    }
  });
  
  return callbacks;
}

Json VehicleAdapter::configValue(String const& name, Json def) const {
  auto const* data = getComponent<VehicleDataComponent>();
  if (!data)
    return def;
  
  // Check dynamic config first
  if (!data->dynamicConfig.isNull() && data->dynamicConfig.contains(name))
    return data->dynamicConfig.get(name);
  
  // Then base config
  if (data->baseConfig.contains(name))
    return data->baseConfig.get(name);
  
  return def;
}

void VehicleAdapter::setupNetStates() {
  auto* data = getComponent<VehicleDataComponent>();
  if (!data)
    return;
  
  m_netGroup.addNetElement(&m_interactiveNetState);
  m_netGroup.addNetElement(&m_damageTeamNetState);
  m_netGroup.addNetElement(&m_scriptedAnimationParametersNetState);
  
  // Add movement controller network elements
  m_movementController.addNetElements(m_netGroup);
  
  // Add animator network elements
  m_networkedAnimator.addNetElements(m_netGroup);
  
  // Set up per-lounge position network state
  for (auto& pair : data->loungePositions) {
    LoungePositionNetState netState;
    m_netGroup.addNetElement(&netState.enabled);
    m_netGroup.addNetElement(&netState.orientation);
    m_netGroup.addNetElement(&netState.emote);
    m_netGroup.addNetElement(&netState.dance);
    m_netGroup.addNetElement(&netState.directives);
    m_netGroup.addNetElement(&netState.statusEffects);
    m_loungePositionNetStates.set(pair.first, std::move(netState));
  }
  
  // Set up per-moving collision network state
  for (auto& pair : data->movingCollisions) {
    NetElementBool enabled;
    m_netGroup.addNetElement(&enabled);
    m_movingCollisionEnabledNetStates.set(pair.first, std::move(enabled));
  }
  
  // Set up per-force region network state
  for (auto& pair : data->forceRegions) {
    NetElementBool enabled;
    m_netGroup.addNetElement(&enabled);
    m_forceRegionEnabledNetStates.set(pair.first, std::move(enabled));
  }
  
  // Set up per-damage source network state
  for (auto& pair : data->damageSources) {
    NetElementBool enabled;
    m_netGroup.addNetElement(&enabled);
    m_damageSourceEnabledNetStates.set(pair.first, std::move(enabled));
  }
}

void VehicleAdapter::getNetStates() {
  auto* data = getComponent<VehicleDataComponent>();
  if (!data)
    return;
  
  m_interactiveNetState.set(data->interactive);
  m_damageTeamNetState.set(data->damageTeam);
  
  // Update lounge position states
  for (auto& pair : data->loungePositions) {
    if (m_loungePositionNetStates.contains(pair.first)) {
      auto& netState = m_loungePositionNetStates.get(pair.first);
      netState.enabled.set(pair.second.enabled);
      netState.orientation.set(pair.second.orientation);
      netState.emote.set(pair.second.emote);
      netState.dance.set(pair.second.dance);
      netState.directives.set(pair.second.directives);
      netState.statusEffects.set(pair.second.statusEffects);
    }
  }
  
  // Update collision enabled states
  for (auto& pair : data->movingCollisions) {
    if (m_movingCollisionEnabledNetStates.contains(pair.first)) {
      m_movingCollisionEnabledNetStates.get(pair.first).set(pair.second.enabled);
    }
  }
  
  // Update force region enabled states
  for (auto& pair : data->forceRegions) {
    if (m_forceRegionEnabledNetStates.contains(pair.first)) {
      m_forceRegionEnabledNetStates.get(pair.first).set(pair.second.enabled);
    }
  }
  
  // Update damage source enabled states
  for (auto& pair : data->damageSources) {
    if (m_damageSourceEnabledNetStates.contains(pair.first)) {
      m_damageSourceEnabledNetStates.get(pair.first).set(pair.second.enabled);
    }
  }
}

void VehicleAdapter::setNetStates() {
  auto* data = getComponent<VehicleDataComponent>();
  if (!data)
    return;
  
  data->interactive = m_interactiveNetState.get();
  data->damageTeam = m_damageTeamNetState.get();
  
  // Update lounge position states
  for (auto& pair : data->loungePositions) {
    if (m_loungePositionNetStates.contains(pair.first)) {
      auto& netState = m_loungePositionNetStates.get(pair.first);
      pair.second.enabled = netState.enabled.get();
      pair.second.orientation = netState.orientation.get();
      pair.second.emote = netState.emote.get();
      pair.second.dance = netState.dance.get();
      pair.second.directives = netState.directives.get();
      pair.second.statusEffects = netState.statusEffects.get();
    }
  }
  
  // Update collision enabled states
  for (auto& pair : data->movingCollisions) {
    if (m_movingCollisionEnabledNetStates.contains(pair.first)) {
      pair.second.enabled = m_movingCollisionEnabledNetStates.get(pair.first).get();
    }
  }
  
  // Update force region enabled states
  for (auto& pair : data->forceRegions) {
    if (m_forceRegionEnabledNetStates.contains(pair.first)) {
      pair.second.enabled = m_forceRegionEnabledNetStates.get(pair.first).get();
    }
  }
  
  // Update damage source enabled states
  for (auto& pair : data->damageSources) {
    if (m_damageSourceEnabledNetStates.contains(pair.first)) {
      pair.second.enabled = m_damageSourceEnabledNetStates.get(pair.first).get();
    }
  }
  
  // Clear lounge anchor cache when network state changes
  m_loungeAnchorCache.clear();
}

} // namespace ECS
} // namespace Star
