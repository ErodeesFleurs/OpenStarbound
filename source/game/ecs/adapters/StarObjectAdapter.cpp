// ECS Object Adapter Implementation for OpenStarbound

#include "StarObjectAdapter.hpp"
#include "StarObjectDatabase.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"
#include "StarWorld.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarLogging.hpp"

namespace Star {
namespace ECS {

// Static factory methods

shared_ptr<ObjectAdapter> ObjectAdapter::create(
    World* ecsWorld,
    ObjectConfigConstPtr config,
    Json const& parameters) {
  
  Entity entity = ecsWorld->createEntity();
  
  // Add tag component
  ecsWorld->addComponent<ObjectTag>(entity);
  
  // Add data component
  auto& data = ecsWorld->addComponent<ObjectDataComponent>(entity);
  data.config = config;
  data.parameters = parameters.toObject();
  
  // Initialize from config
  data.health = config->health;
  data.unbreakable = config->unbreakable;
  data.direction = Direction::Left;
  data.orientationIndex = 0;
  data.interactive = config->interactive;
  data.clientEntityMode = ClientEntityMode::ClientSlaveOnly;
  
  // Set up wire nodes from config
  for (auto const& inputConfig : config->inputNodes) {
    ObjectInputNode node;
    node.position = jsonToVec2I(inputConfig.get("position"));
    node.state = false;
    node.color = jsonToColor(inputConfig.get("color", "white"));
    node.icon = inputConfig.getString("icon", "");
    data.inputNodes.append(node);
  }
  
  for (auto const& outputConfig : config->outputNodes) {
    ObjectOutputNode node;
    node.position = jsonToVec2I(outputConfig.get("position"));
    node.state = false;
    node.color = jsonToColor(outputConfig.get("color", "white"));
    node.icon = outputConfig.getString("icon", "");
    data.outputNodes.append(node);
  }
  
  auto adapter = make_shared<ObjectAdapter>(ecsWorld, entity);
  return adapter;
}

shared_ptr<ObjectAdapter> ObjectAdapter::createFromDiskStore(
    World* ecsWorld,
    Json const& diskStore) {
  
  String objectName = diskStore.getString("name");
  auto objectDatabase = Root::singleton().objectDatabase();
  auto config = objectDatabase->getConfig(objectName);
  
  Json parameters = diskStore.get("parameters", JsonObject());
  auto adapter = create(ecsWorld, config, parameters);
  adapter->readStoredData(diskStore);
  
  return adapter;
}

shared_ptr<ObjectAdapter> ObjectAdapter::createFromNet(
    World* ecsWorld,
    ByteArray const& netStore,
    NetCompatibilityRules rules) {
  
  DataStreamBuffer ds(netStore);
  
  String objectName = ds.read<String>();
  auto objectDatabase = Root::singleton().objectDatabase();
  auto config = objectDatabase->getConfig(objectName);
  
  auto adapter = create(ecsWorld, config);
  
  // Read network initialization data
  auto* data = adapter->getComponent<ObjectDataComponent>();
  if (data) {
    data->tilePosition = ds.read<Vec2I>();
    data->direction = ds.read<Direction>();
    data->parameters = ds.read<JsonObject>();
  }
  
  return adapter;
}

// Constructor

ObjectAdapter::ObjectAdapter(World* ecsWorld, Entity ecsEntity)
  : EntityAdapter(ecsWorld, ecsEntity) {
  setupNetStates();
}

// Serialization

Json ObjectAdapter::diskStore() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return Json();
  
  JsonObject store;
  store["name"] = data->config->name;
  store["parameters"] = data->parameters;
  store["tilePosition"] = jsonFromVec2I(data->tilePosition);
  store["direction"] = DirectionNames.getRight(data->direction);
  store["orientationIndex"] = (int64_t)data->orientationIndex;
  store["health"] = data->health;
  
  // Write wire connections
  if (!data->inputNodes.empty()) {
    JsonArray inputConnections;
    for (auto const& node : data->inputNodes) {
      JsonArray connections;
      for (auto const& conn : node.connections) {
        connections.append(JsonArray{jsonFromVec2I(conn.entityPosition), (int64_t)conn.nodeIndex});
      }
      inputConnections.append(connections);
    }
    store["inputWireConnections"] = inputConnections;
  }
  
  if (!data->outputNodes.empty()) {
    JsonArray outputConnections;
    for (auto const& node : data->outputNodes) {
      JsonArray connections;
      for (auto const& conn : node.connections) {
        connections.append(JsonArray{jsonFromVec2I(conn.entityPosition), (int64_t)conn.nodeIndex});
      }
      outputConnections.append(connections);
    }
    store["outputWireConnections"] = outputConnections;
  }
  
  // Write stored script data
  Json scriptedData = writeStoredData();
  if (!scriptedData.isNull())
    store["scriptStorage"] = scriptedData;
  
  return store;
}

ByteArray ObjectAdapter::netStore(NetCompatibilityRules rules) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return ByteArray();
  
  DataStreamBuffer ds;
  ds.write(data->config->name);
  ds.write(data->tilePosition);
  ds.write(data->direction);
  ds.write(data->parameters);
  
  return ds.takeData();
}

// Entity interface

EntityType ObjectAdapter::entityType() const {
  return EntityType::Object;
}

ClientEntityMode ObjectAdapter::clientEntityMode() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->clientEntityMode : ClientEntityMode::ClientSlaveOnly;
}

void ObjectAdapter::init(Star::World* world, EntityId entityId, EntityMode mode) {
  EntityAdapter::init(world, entityId, mode);
  
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  // Initialize script component
  if (data->config->hasScripts() && (mode == EntityMode::Master || data->config->clientEntityMode == ClientEntityMode::ClientMasterAllowed)) {
    m_scriptComponent.setScripts(data->config->scripts);
    m_scriptComponent.setUpdateDelta(data->config->scriptDelta);
    
    m_scriptComponent.addCallbacks("object", makeObjectCallbacks());
    m_scriptComponent.init(world);
  }
  
  // Initialize animator
  if (data->networkedAnimator) {
    data->networkedAnimator->init();
  }
  
  // Reset emission timers
  resetEmissionTimers();
}

void ObjectAdapter::uninit() {
  m_scriptComponent.uninit();
  m_scriptComponent.removeCallbacks("object");
  
  auto* data = getComponent<ObjectDataComponent>();
  if (data && data->networkedAnimator) {
    data->networkedAnimator->uninit();
  }
  
  EntityAdapter::uninit();
}

Vec2F ObjectAdapter::position() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return Vec2F();
  
  auto orientation = currentOrientation();
  if (orientation) {
    return Vec2F(data->tilePosition) + orientation->positionOffset;
  }
  return Vec2F(data->tilePosition);
}

RectF ObjectAdapter::metaBoundBox() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return RectF();
  
  auto orientation = currentOrientation();
  if (orientation) {
    return orientation->metaBoundBox;
  }
  
  return RectF::withCenter(Vec2F(), Vec2F(1, 1));
}

pair<ByteArray, uint64_t> ObjectAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void ObjectAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
  setNetStates();
}

String ObjectAdapter::name() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->config->name : "";
}

String ObjectAdapter::description() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return "";
  
  return data->config->descriptions.getString("description", "");
}

bool ObjectAdapter::inspectable() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->config->inspectable : false;
}

Maybe<String> ObjectAdapter::inspectionLogName() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data || !data->config->inspectable)
    return {};
  
  if (data->config->descriptions.contains("inspectionLogName"))
    return data->config->descriptions.getString("inspectionLogName");
  
  return data->config->name;
}

Maybe<String> ObjectAdapter::inspectionDescription(String const& species) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  String descKey = species + "Description";
  if (data->config->descriptions.contains(descKey))
    return data->config->descriptions.getString(descKey);
  
  return data->config->descriptions.getString("description", "");
}

List<LightSource> ObjectAdapter::lightSources() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  List<LightSource> lights;
  
  auto orientation = currentOrientation();
  if (orientation && orientation->lightPosition) {
    LightSource light;
    light.position = Vec2F(data->tilePosition) + *orientation->lightPosition;
    light.color = data->lightSourceColor;
    light.pointLight = orientation->pointLight;
    light.pointBeam = orientation->pointBeam;
    light.beamAngle = orientation->beamAngle;
    light.beamAmbience = orientation->beamAmbience;
    lights.append(light);
  }
  
  return lights;
}

bool ObjectAdapter::shouldDestroy() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->broken : false;
}

void ObjectAdapter::destroy(RenderCallback* renderCallback) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  // Spawn break particles and sounds
  auto orientation = currentOrientation();
  if (orientation && renderCallback) {
    for (auto const& particle : orientation->particleEmitters) {
      if (particle.emissionVariance > 0) {
        // Skip timed particles during destroy
        continue;
      }
      // renderCallback->addParticle(...);
    }
  }
}

void ObjectAdapter::update(float dt, uint64_t currentStep) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  // Update animation
  data->animationTimer += dt;
  
  // Update wind animation if applicable
  auto orientation = currentOrientation();
  
  // Update scripted behavior
  if (isMaster()) {
    m_scriptComponent.update(m_scriptComponent.updateDt(currentStep));
    
    // Check liquid broken
    checkLiquidBroken();
    
    // Get updated network states
    getNetStates(false);
  }
  
  // Update networked animator
  if (data->networkedAnimator) {
    data->networkedAnimator->update(dt, &data->networkedAnimatorDynamicTarget);
  }
  
  // Update scripted animator
  m_scriptedAnimator.update(dt);
}

void ObjectAdapter::render(RenderCallback* renderCallback) {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  // Render orientation drawables
  auto drawables = orientationDrawables(data->orientationIndex);
  Vec2F pos = position();
  
  for (auto& drawable : drawables) {
    drawable.translate(pos);
    renderCallback->addDrawable(drawable, renderLayer());
  }
  
  // Render networked animator
  if (data->networkedAnimator) {
    for (auto& drawable : data->networkedAnimator->drawables(pos)) {
      renderCallback->addDrawable(drawable, renderLayer());
    }
  }
  
  // Render particles
  renderParticles(renderCallback);
  
  // Render sounds
  renderSounds(renderCallback);
}

void ObjectAdapter::renderLightSources(RenderCallback* renderCallback) {
  renderLights(renderCallback);
}

bool ObjectAdapter::checkBroken() {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return true;
  
  if (data->broken)
    return true;
  
  // Check if anchors are still valid
  // Check if any root positions are blocked
  // etc.
  
  return false;
}

Vec2I ObjectAdapter::tilePosition() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->tilePosition : Vec2I();
}

List<Vec2I> ObjectAdapter::spaces() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  auto orientation = currentOrientation();
  if (orientation) {
    List<Vec2I> result;
    for (auto const& space : orientation->spaces) {
      result.append(data->tilePosition + space);
    }
    return result;
  }
  
  return {data->tilePosition};
}

List<MaterialSpace> ObjectAdapter::materialSpaces() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->materialSpaces : List<MaterialSpace>();
}

List<Vec2I> ObjectAdapter::roots() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  auto orientation = currentOrientation();
  if (orientation) {
    List<Vec2I> result;
    for (auto const& anchor : orientation->anchors) {
      result.append(data->tilePosition + anchor.position);
    }
    return result;
  }
  
  return {data->tilePosition};
}

Direction ObjectAdapter::direction() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->direction : Direction::Left;
}

void ObjectAdapter::setDirection(Direction direction) {
  auto* data = getComponent<ObjectDataComponent>();
  if (data) {
    data->direction = direction;
    markNetworkDirty();
  }
}

void ObjectAdapter::setTilePosition(Vec2I const& pos) {
  auto* data = getComponent<ObjectDataComponent>();
  if (data) {
    data->tilePosition = pos;
    updateOrientation();
    markNetworkDirty();
  }
}

void ObjectAdapter::updateOrientation() {
  // Find valid orientation for current position
  // This is a simplified version
}

List<Vec2I> ObjectAdapter::anchorPositions() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  auto orientation = currentOrientation();
  if (orientation) {
    List<Vec2I> result;
    for (auto const& anchor : orientation->anchors) {
      result.append(data->tilePosition + anchor.position);
    }
    return result;
  }
  
  return {};
}

List<Drawable> ObjectAdapter::cursorHintDrawables() const {
  return orientationDrawables(orientationIndex());
}

String ObjectAdapter::shortDescription() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return "";
  
  return data->config->descriptions.getString("shortdescription", data->config->name);
}

String ObjectAdapter::category() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->config->category : "";
}

ObjectOrientationPtr ObjectAdapter::currentOrientation() const {
  auto const& orientations = getOrientations();
  auto const* data = getComponent<ObjectDataComponent>();
  
  if (data && data->orientationIndex < orientations.size()) {
    return orientations[data->orientationIndex];
  }
  
  return {};
}

List<PersistentStatusEffect> ObjectAdapter::statusEffects() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  auto orientation = currentOrientation();
  if (orientation) {
    return orientation->statusEffects;
  }
  
  return {};
}

PolyF ObjectAdapter::statusEffectArea() const {
  return volume();
}

List<DamageSource> ObjectAdapter::damageSources() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->damageSources : List<DamageSource>();
}

Maybe<HitType> ObjectAdapter::queryHit(DamageSource const& source) const {
  if (!canBeDamaged())
    return {};
  
  auto poly = hitPoly();
  if (!poly)
    return {};
  
  if (source.intersectsWithPoly(world()->geometry(), *poly))
    return HitType::Hit;
  
  return {};
}

Maybe<PolyF> ObjectAdapter::hitPoly() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data || data->unbreakable)
    return {};
  
  return volume();
}

List<DamageNotification> ObjectAdapter::applyDamage(DamageRequest const& damage) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data || data->unbreakable)
    return {};
  
  // Calculate damage amount
  float amount = damage.damage;
  
  data->health -= amount;
  
  if (data->health <= 0) {
    data->broken = true;
  }
  
  DamageNotification notification;
  notification.sourceEntityId = damage.sourceEntityId;
  notification.targetEntityId = entityId();
  notification.damageDealt = amount;
  notification.healthLost = amount;
  notification.hitType = HitType::Hit;
  notification.damageSourceKind = damage.damageSourceKind;
  notification.targetMaterialKind = data->config->materialKind;
  notification.position = position();
  
  return {notification};
}

bool ObjectAdapter::damageTiles(List<Vec2I> const& positions, Vec2F const& sourcePosition, TileDamage const& tileDamage) {
  // Delegate to tile damage system
  return false;
}

bool ObjectAdapter::canBeDamaged() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data && !data->unbreakable && data->health > 0;
}

RectF ObjectAdapter::interactiveBoundBox() const {
  return metaBoundBox();
}

bool ObjectAdapter::isInteractive() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->interactive : false;
}

InteractAction ObjectAdapter::interact(InteractRequest const& request) {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  // Call script interaction handler if available
  auto result = m_scriptComponent.handleMessage("interact", true, 
    {jsonFromVec2F(request.interactPosition), jsonFromVec2F(request.sourcePosition)});
  
  if (result && !result->isNull()) {
    return InteractAction(*result);
  }
  
  return data->config->defaultInteraction;
}

List<Vec2I> ObjectAdapter::interactiveSpaces() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  auto orientation = currentOrientation();
  if (orientation && !orientation->interactiveSpaces.empty()) {
    List<Vec2I> result;
    for (auto const& space : orientation->interactiveSpaces) {
      result.append(data->tilePosition + space);
    }
    return result;
  }
  
  return spaces();
}

Maybe<LuaValue> ObjectAdapter::callScript(String const& func, LuaVariadic<LuaValue> const& args) {
  return m_scriptComponent.invoke(func, args);
}

Maybe<LuaValue> ObjectAdapter::evalScript(String const& code) {
  return m_scriptComponent.eval(code);
}

Vec2F ObjectAdapter::mouthPosition() const {
  return mouthPosition(false);
}

Vec2F ObjectAdapter::mouthPosition(bool ignoreAdjustments) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return position();
  
  auto orientation = currentOrientation();
  if (orientation && orientation->chatPosition) {
    return Vec2F(data->tilePosition) + *orientation->chatPosition;
  }
  
  return position() + Vec2F(0, metaBoundBox().height() / 2);
}

List<ChatAction> ObjectAdapter::pullPendingChatActions() {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  return take(data->pendingChatActions);
}

void ObjectAdapter::breakObject(bool smash) {
  auto* data = getComponent<ObjectDataComponent>();
  if (data) {
    data->broken = true;
    markNetworkDirty();
  }
}

// WireEntity interface

size_t ObjectAdapter::nodeCount(WireDirection direction) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return 0;
  
  if (direction == WireDirection::Input)
    return data->inputNodes.size();
  else
    return data->outputNodes.size();
}

Vec2I ObjectAdapter::nodePosition(WireNode wireNode) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return Vec2I();
  
  if (wireNode.direction == WireDirection::Input && wireNode.index < data->inputNodes.size()) {
    return data->tilePosition + data->inputNodes[wireNode.index].position;
  } else if (wireNode.direction == WireDirection::Output && wireNode.index < data->outputNodes.size()) {
    return data->tilePosition + data->outputNodes[wireNode.index].position;
  }
  
  return Vec2I();
}

List<WireConnection> ObjectAdapter::connectionsForNode(WireNode wireNode) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  if (wireNode.direction == WireDirection::Input && wireNode.index < data->inputNodes.size()) {
    return data->inputNodes[wireNode.index].connections;
  } else if (wireNode.direction == WireDirection::Output && wireNode.index < data->outputNodes.size()) {
    return data->outputNodes[wireNode.index].connections;
  }
  
  return {};
}

bool ObjectAdapter::nodeState(WireNode wireNode) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return false;
  
  if (wireNode.direction == WireDirection::Input && wireNode.index < data->inputNodes.size()) {
    return data->inputNodes[wireNode.index].state;
  } else if (wireNode.direction == WireDirection::Output && wireNode.index < data->outputNodes.size()) {
    return data->outputNodes[wireNode.index].state;
  }
  
  return false;
}

String ObjectAdapter::nodeIcon(WireNode wireNode) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return "";
  
  if (wireNode.direction == WireDirection::Input && wireNode.index < data->inputNodes.size()) {
    return data->inputNodes[wireNode.index].icon;
  } else if (wireNode.direction == WireDirection::Output && wireNode.index < data->outputNodes.size()) {
    return data->outputNodes[wireNode.index].icon;
  }
  
  return "";
}

Color ObjectAdapter::nodeColor(WireNode wireNode) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return Color::White;
  
  if (wireNode.direction == WireDirection::Input && wireNode.index < data->inputNodes.size()) {
    return data->inputNodes[wireNode.index].color;
  } else if (wireNode.direction == WireDirection::Output && wireNode.index < data->outputNodes.size()) {
    return data->outputNodes[wireNode.index].color;
  }
  
  return Color::White;
}

void ObjectAdapter::addNodeConnection(WireNode wireNode, WireConnection nodeConnection) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  if (wireNode.direction == WireDirection::Input && wireNode.index < data->inputNodes.size()) {
    data->inputNodes[wireNode.index].connections.append(nodeConnection);
  } else if (wireNode.direction == WireDirection::Output && wireNode.index < data->outputNodes.size()) {
    data->outputNodes[wireNode.index].connections.append(nodeConnection);
  }
  
  markNetworkDirty();
}

void ObjectAdapter::removeNodeConnection(WireNode wireNode, WireConnection nodeConnection) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  if (wireNode.direction == WireDirection::Input && wireNode.index < data->inputNodes.size()) {
    data->inputNodes[wireNode.index].connections.removeAll(nodeConnection);
  } else if (wireNode.direction == WireDirection::Output && wireNode.index < data->outputNodes.size()) {
    data->outputNodes[wireNode.index].connections.removeAll(nodeConnection);
  }
  
  markNetworkDirty();
}

void ObjectAdapter::evaluate(WireCoordinator* coordinator) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  // Read input states from coordinator
  for (size_t i = 0; i < data->inputNodes.size(); ++i) {
    bool newState = false;
    for (auto const& conn : data->inputNodes[i].connections) {
      if (coordinator->readNode({conn.entityPosition, WireDirection::Output, conn.nodeIndex})) {
        newState = true;
        break;
      }
    }
    data->inputNodes[i].state = newState;
  }
  
  // Let script handle wire logic
  m_scriptComponent.invoke("onWireUpdate");
}

List<QuestArcDescriptor> ObjectAdapter::offeredQuests() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->offeredQuests : List<QuestArcDescriptor>();
}

StringSet ObjectAdapter::turnInQuests() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->turnInQuests : StringSet();
}

Vec2F ObjectAdapter::questIndicatorPosition() const {
  return mouthPosition();
}

Maybe<Json> ObjectAdapter::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) {
  return m_scriptComponent.handleMessage(message, sendingConnection == world()->connection(), args);
}

Json ObjectAdapter::configValue(String const& name, Json const& def) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return def;
  
  // Check parameters first
  if (data->parameters.contains(name))
    return data->parameters.get(name);
  
  // Then check config
  if (data->config->config.contains(name))
    return data->config->config.get(name);
  
  // Then check current orientation
  auto orientation = currentOrientation();
  if (orientation && orientation->config.contains(name))
    return orientation->config.get(name);
  
  return def;
}

ObjectConfigConstPtr ObjectAdapter::config() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->config : ObjectConfigConstPtr();
}

float ObjectAdapter::liquidFillLevel() const {
  // Calculate liquid level from world
  return 0.0f;
}

bool ObjectAdapter::biomePlaced() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->biomePlaced : false;
}

// Protected methods

void ObjectAdapter::getNetStates(bool initial) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  m_interactiveNetState.set(data->interactive);
  m_materialSpacesNetState.set(data->materialSpaces);
  m_xTilePositionNetState.set(data->tilePosition[0]);
  m_yTilePositionNetState.set(data->tilePosition[1]);
  m_directionNetState.set(data->direction);
  m_healthNetState.set(data->health);
  m_orientationIndexNetState.set(data->orientationIndex);
  m_lightSourceColorNetState.set(data->lightSourceColor);
  m_soundEffectEnabledNetState.set(data->soundEffectEnabled);
  
  // Update image keys
  for (auto const& pair : data->imageKeys) {
    m_imageKeysNetState.set(pair.first, pair.second);
  }
  
  // Update wire states
  for (size_t i = 0; i < data->inputNodes.size() && i < m_inputStateNetState.size(); ++i) {
    m_inputStateNetState[i].set(data->inputNodes[i].state);
    m_inputConnectionsNetState[i].set(data->inputNodes[i].connections);
  }
  
  for (size_t i = 0; i < data->outputNodes.size() && i < m_outputStateNetState.size(); ++i) {
    m_outputStateNetState[i].set(data->outputNodes[i].state);
    m_outputConnectionsNetState[i].set(data->outputNodes[i].connections);
  }
}

void ObjectAdapter::setNetStates() {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  data->interactive = m_interactiveNetState.get();
  data->materialSpaces = m_materialSpacesNetState.get();
  data->tilePosition = Vec2I(m_xTilePositionNetState.get(), m_yTilePositionNetState.get());
  data->direction = m_directionNetState.get();
  data->health = m_healthNetState.get();
  data->orientationIndex = m_orientationIndexNetState.get();
  data->lightSourceColor = m_lightSourceColorNetState.get();
  data->soundEffectEnabled = m_soundEffectEnabledNetState.get();
  
  // Update image keys
  data->imageKeys.clear();
  for (auto const& pair : m_imageKeysNetState) {
    data->imageKeys.set(pair.first, pair.second);
  }
  
  // Update wire states
  for (size_t i = 0; i < data->inputNodes.size() && i < m_inputStateNetState.size(); ++i) {
    data->inputNodes[i].state = m_inputStateNetState[i].get();
    data->inputNodes[i].connections = m_inputConnectionsNetState[i].get();
  }
  
  for (size_t i = 0; i < data->outputNodes.size() && i < m_outputStateNetState.size(); ++i) {
    data->outputNodes[i].state = m_outputStateNetState[i].get();
    data->outputNodes[i].connections = m_outputConnectionsNetState[i].get();
  }
}

void ObjectAdapter::readStoredData(Json const& diskStore) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  if (diskStore.contains("tilePosition"))
    data->tilePosition = jsonToVec2I(diskStore.get("tilePosition"));
  
  if (diskStore.contains("direction"))
    data->direction = DirectionNames.getLeft(diskStore.getString("direction"));
  
  if (diskStore.contains("orientationIndex"))
    data->orientationIndex = diskStore.getUInt("orientationIndex");
  
  if (diskStore.contains("health"))
    data->health = diskStore.getFloat("health");
  
  // Read wire connections
  if (diskStore.contains("inputWireConnections")) {
    auto inputConnections = diskStore.getArray("inputWireConnections");
    for (size_t i = 0; i < inputConnections.size() && i < data->inputNodes.size(); ++i) {
      data->inputNodes[i].connections.clear();
      for (auto const& connJson : inputConnections[i].toArray()) {
        auto connArray = connJson.toArray();
        WireConnection conn;
        conn.entityPosition = jsonToVec2I(connArray[0]);
        conn.nodeIndex = connArray[1].toUInt();
        data->inputNodes[i].connections.append(conn);
      }
    }
  }
  
  if (diskStore.contains("outputWireConnections")) {
    auto outputConnections = diskStore.getArray("outputWireConnections");
    for (size_t i = 0; i < outputConnections.size() && i < data->outputNodes.size(); ++i) {
      data->outputNodes[i].connections.clear();
      for (auto const& connJson : outputConnections[i].toArray()) {
        auto connArray = connJson.toArray();
        WireConnection conn;
        conn.entityPosition = jsonToVec2I(connArray[0]);
        conn.nodeIndex = connArray[1].toUInt();
        data->outputNodes[i].connections.append(conn);
      }
    }
  }
  
  // Read script storage
  if (diskStore.contains("scriptStorage")) {
    m_scriptComponent.setStoredData(diskStore.get("scriptStorage"));
  }
}

Json ObjectAdapter::writeStoredData() const {
  return m_scriptComponent.getStoredData();
}

void ObjectAdapter::setImageKey(String const& name, String const& value) {
  auto* data = getComponent<ObjectDataComponent>();
  if (data) {
    data->imageKeys.set(name, value);
    data->orientationDrawablesCache.reset();
    markNetworkDirty();
  }
}

size_t ObjectAdapter::orientationIndex() const {
  auto const* data = getComponent<ObjectDataComponent>();
  return data ? data->orientationIndex : 0;
}

void ObjectAdapter::setOrientationIndex(size_t orientationIndex) {
  auto* data = getComponent<ObjectDataComponent>();
  if (data) {
    data->orientationIndex = orientationIndex;
    data->orientationDrawablesCache.reset();
    markNetworkDirty();
  }
}

PolyF ObjectAdapter::volume() const {
  auto orientation = currentOrientation();
  if (orientation) {
    return orientation->poly.translated(Vec2F(tilePosition()));
  }
  return PolyF(RectF::withSize(Vec2F(tilePosition()), Vec2F(1, 1)));
}

// Private methods

LuaCallbacks ObjectAdapter::makeObjectCallbacks() {
  LuaCallbacks callbacks;
  
  callbacks.registerCallback("name", [this]() -> String {
    return name();
  });
  
  callbacks.registerCallback("direction", [this]() -> int {
    return (int)direction();
  });
  
  callbacks.registerCallback("position", [this]() -> Vec2F {
    return position();
  });
  
  callbacks.registerCallback("setInteractive", [this](bool interactive) {
    auto* data = getComponent<ObjectDataComponent>();
    if (data) {
      data->interactive = interactive;
      markNetworkDirty();
    }
  });
  
  callbacks.registerCallback("setLightColor", [this](Color color) {
    auto* data = getComponent<ObjectDataComponent>();
    if (data) {
      data->lightSourceColor = color;
      markNetworkDirty();
    }
  });
  
  callbacks.registerCallback("setOutputNodeLevel", [this](size_t nodeIndex, bool level) {
    auto* data = getComponent<ObjectDataComponent>();
    if (data && nodeIndex < data->outputNodes.size()) {
      data->outputNodes[nodeIndex].state = level;
      markNetworkDirty();
    }
  });
  
  callbacks.registerCallback("getInputNodeLevel", [this](size_t nodeIndex) -> bool {
    auto const* data = getComponent<ObjectDataComponent>();
    if (data && nodeIndex < data->inputNodes.size()) {
      return data->inputNodes[nodeIndex].state;
    }
    return false;
  });
  
  callbacks.registerCallback("smash", [this](Maybe<bool> smash) {
    breakObject(smash.value(true));
  });
  
  return callbacks;
}

LuaCallbacks ObjectAdapter::makeAnimatorObjectCallbacks() {
  return LuaCallbacks();
}

void ObjectAdapter::ensureNetSetup() {
  // Network state is set up in constructor
}

List<Drawable> ObjectAdapter::orientationDrawables(size_t orientationIndex) const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return {};
  
  // Check cache
  if (data->orientationDrawablesCache && data->orientationDrawablesCache->first == orientationIndex) {
    return data->orientationDrawablesCache->second;
  }
  
  List<Drawable> drawables;
  
  auto const& orientations = getOrientations();
  if (orientationIndex < orientations.size()) {
    auto const& orientation = orientations[orientationIndex];
    
    for (auto const& layer : orientation->imageLayers) {
      String imagePath = layer.imagePart;
      
      // Apply image keys
      for (auto const& pair : data->imageKeys) {
        imagePath = imagePath.replaceTags(StringMap<String>{{pair.first, pair.second}});
      }
      
      Drawable drawable = Drawable::makeImage(imagePath, 1.0f / TilePixels, false, layer.offset);
      drawable.fullbright = layer.fullbright;
      
      if (data->direction == Direction::Right) {
        drawable.scale(Vec2F(-1, 1), drawable.boundBox(false).center());
      }
      
      drawables.append(drawable);
    }
  }
  
  // Update cache (const_cast needed for mutable cache)
  const_cast<ObjectDataComponent*>(data)->orientationDrawablesCache = make_pair(orientationIndex, drawables);
  
  return drawables;
}

void ObjectAdapter::addChatMessage(String const& message, Json const& config, String const& portrait) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  ChatAction action;
  action.sourceEntityId = entityId();
  action.message = message;
  action.portrait = portrait;
  action.config = config;
  
  data->pendingChatActions.append(action);
}

void ObjectAdapter::writeOutboundNode(Vec2I outboundNode, bool state) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  for (size_t i = 0; i < data->outputNodes.size(); ++i) {
    if (data->outputNodes[i].position == outboundNode) {
      data->outputNodes[i].state = state;
      markNetworkDirty();
      break;
    }
  }
}

EntityRenderLayer ObjectAdapter::renderLayer() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    return RenderLayerObject;
  
  auto orientation = currentOrientation();
  if (orientation) {
    return orientation->renderLayer;
  }
  
  return RenderLayerObject;
}

void ObjectAdapter::renderLights(RenderCallback* renderCallback) const {
  for (auto const& light : lightSources()) {
    renderCallback->addLightSource(light);
  }
}

void ObjectAdapter::renderParticles(RenderCallback* renderCallback) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  auto orientation = currentOrientation();
  if (!orientation)
    return;
  
  // Update emission timers and spawn particles
  for (size_t i = 0; i < orientation->particleEmitters.size() && i < data->emissionTimers.size(); ++i) {
    if (data->emissionTimers[i].tick()) {
      auto const& emitter = orientation->particleEmitters[i];
      
      Particle particle;
      particle.type = Particle::Type::Ember; // Default
      particle.position = position() + emitter.position;
      
      renderCallback->addParticle(particle);
      
      data->emissionTimers[i].reset();
    }
  }
}

void ObjectAdapter::renderSounds(RenderCallback* renderCallback) {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  // Handle persistent sound effect
  if (data->soundEffectEnabled && data->soundEffect) {
    // Audio is handled elsewhere
  }
}

List<ObjectOrientationPtr> const& ObjectAdapter::getOrientations() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data)
    throw ObjectException("ObjectAdapter::getOrientations called without data component");
  
  if (!m_orientationsCache) {
    m_orientationsCache = data->config->orientations;
  }
  
  return *m_orientationsCache;
}

Vec2F ObjectAdapter::damageShake() const {
  auto const* data = getComponent<ObjectDataComponent>();
  if (!data || !data->tileDamageStatus)
    return Vec2F();
  
  // Calculate shake based on damage state
  return Vec2F();
}

void ObjectAdapter::checkLiquidBroken() {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  // Check if object is submerged in liquid that breaks it
  // This is a simplified version
}

void ObjectAdapter::resetEmissionTimers() {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  data->emissionTimers.clear();
  
  auto orientation = currentOrientation();
  if (orientation) {
    for (auto const& emitter : orientation->particleEmitters) {
      data->emissionTimers.append(GameTimer(emitter.emissionRate));
    }
  }
}

void ObjectAdapter::setupNetStates() {
  auto* data = getComponent<ObjectDataComponent>();
  if (!data)
    return;
  
  m_netGroup.addNetElement(&m_interactiveNetState);
  m_netGroup.addNetElement(&m_materialSpacesNetState);
  m_netGroup.addNetElement(&m_parametersNetState);
  m_netGroup.addNetElement(&m_uniqueIdNetState);
  m_netGroup.addNetElement(&m_xTilePositionNetState);
  m_netGroup.addNetElement(&m_yTilePositionNetState);
  m_netGroup.addNetElement(&m_directionNetState);
  m_netGroup.addNetElement(&m_healthNetState);
  m_netGroup.addNetElement(&m_orientationIndexNetState);
  m_netGroup.addNetElement(&m_imageKeysNetState);
  m_netGroup.addNetElement(&m_soundEffectEnabledNetState);
  m_netGroup.addNetElement(&m_lightSourceColorNetState);
  m_netGroup.addNetElement(&m_newChatMessageEventNetState);
  m_netGroup.addNetElement(&m_chatMessageNetState);
  m_netGroup.addNetElement(&m_chatPortraitNetState);
  m_netGroup.addNetElement(&m_chatConfigNetState);
  m_netGroup.addNetElement(&m_offeredQuestsNetState);
  m_netGroup.addNetElement(&m_turnInQuestsNetState);
  m_netGroup.addNetElement(&m_scriptedAnimationParametersNetState);
  m_netGroup.addNetElement(&m_damageSourcesNetState);
  
  // Set up wire node network states
  for (size_t i = 0; i < data->inputNodes.size(); ++i) {
    m_inputConnectionsNetState.emplace_back();
    m_inputStateNetState.emplace_back();
    m_netGroup.addNetElement(&m_inputConnectionsNetState.back());
    m_netGroup.addNetElement(&m_inputStateNetState.back());
  }
  
  for (size_t i = 0; i < data->outputNodes.size(); ++i) {
    m_outputConnectionsNetState.emplace_back();
    m_outputStateNetState.emplace_back();
    m_netGroup.addNetElement(&m_outputConnectionsNetState.back());
    m_netGroup.addNetElement(&m_outputStateNetState.back());
  }
}

} // namespace ECS
} // namespace Star
