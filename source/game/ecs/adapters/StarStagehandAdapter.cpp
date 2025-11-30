#include "StarStagehandAdapter.hpp"
#include "StarWorld.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarJsonExtra.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarEntityLuaBindings.hpp"
#include "StarBehaviorLuaBindings.hpp"
#include "StarLuaGameConverters.hpp"
#include "StarLogging.hpp"

namespace Star {
namespace ECS {

shared_ptr<StagehandAdapter> StagehandAdapter::create(
    World* ecsWorld,
    Json const& config) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<StagehandAdapter>(ecsWorld, entity);
  
  adapter->addComponent<StagehandTag>();
  adapter->addComponent<PositionComponent>(Vec2F());
  adapter->addComponent<BoundingBoxComponent>(RectF());
  auto& data = adapter->addComponent<StagehandDataComponent>();
  
  adapter->setUniqueId(config.optString("uniqueId"));
  adapter->readConfig(config);
  
  return adapter;
}

shared_ptr<StagehandAdapter> StagehandAdapter::createFromNet(
    World* ecsWorld,
    ByteArray const& netStore,
    NetCompatibilityRules) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<StagehandAdapter>(ecsWorld, entity);
  
  adapter->addComponent<StagehandTag>();
  adapter->addComponent<PositionComponent>(Vec2F());
  adapter->addComponent<BoundingBoxComponent>(RectF());
  adapter->addComponent<StagehandDataComponent>();
  
  adapter->readConfig(DataStreamBuffer::deserialize<Json>(netStore));
  
  return adapter;
}

StagehandAdapter::StagehandAdapter(World* ecsWorld, Entity ecsEntity)
  : EntityAdapter(ecsWorld, ecsEntity) {
  setPersistent(true);
  
  m_netGroup.addNetElement(&m_xPosition);
  m_netGroup.addNetElement(&m_yPosition);
  m_netGroup.addNetElement(&m_uniqueIdNetState);
  
  m_netGroup.setNeedsLoadCallback([this](bool) {
      setUniqueId(m_uniqueIdNetState.get());
    });
  
  m_netGroup.setNeedsStoreCallback([this]() {
      m_uniqueIdNetState.set(uniqueId());
    });
}

Json StagehandAdapter::diskStore() const {
  auto* data = getComponent<StagehandDataComponent>();
  if (!data)
    return {};
    
  auto saveData = data->config.setAll({
      {"position", jsonFromVec2F({m_xPosition.get(), m_yPosition.get()})},
      {"uniqueId", jsonFromMaybe(uniqueId())}
    });
  
  if (!data->scripted)
    return saveData;
  else
    return saveData.set("scriptStorage", m_scriptComponent.getScriptStorage());
}

ByteArray StagehandAdapter::netStore(NetCompatibilityRules) {
  auto* data = getComponent<StagehandDataComponent>();
  if (!data)
    return {};
  return DataStreamBuffer::serialize(data->config);
}

EntityType StagehandAdapter::entityType() const {
  return EntityType::Stagehand;
}

void StagehandAdapter::init(Star::World* world, EntityId entityId, EntityMode mode) {
  Entity::init(world, entityId, mode);
  
  auto* data = getComponent<StagehandDataComponent>();
  if (!data)
    return;
  
  if (isMaster() && data->scripted) {
    m_scriptComponent.addCallbacks("stagehand", makeStagehandCallbacks());
    m_scriptComponent.addCallbacks("config", LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) {
        auto* data = getComponent<StagehandDataComponent>();
        return data ? data->config.query(name, def) : def;
      }));
    m_scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(this));
    m_scriptComponent.addCallbacks("behavior", LuaBindings::makeBehaviorCallbacks(&m_behaviors));
    m_scriptComponent.init(world);
  }
}

void StagehandAdapter::uninit() {
  Entity::uninit();
  
  auto* data = getComponent<StagehandDataComponent>();
  if (data && data->scripted) {
    m_scriptComponent.uninit();
    m_scriptComponent.removeCallbacks("stagehand");
    m_scriptComponent.removeCallbacks("config");
    m_scriptComponent.removeCallbacks("entity");
  }
}

String StagehandAdapter::name() const {
  return typeName();
}

pair<ByteArray, uint64_t> StagehandAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void StagehandAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
}

Vec2F StagehandAdapter::position() const {
  return {m_xPosition.get(), m_yPosition.get()};
}

void StagehandAdapter::setPosition(Vec2F const& position) {
  m_xPosition.set(position[0]);
  m_yPosition.set(position[1]);
  
  auto* pos = getComponent<PositionComponent>();
  if (pos)
    pos->position = position;
}

RectF StagehandAdapter::metaBoundBox() const {
  auto* data = getComponent<StagehandDataComponent>();
  return data ? data->boundBox : RectF(-5, -5, 5, 5);
}

bool StagehandAdapter::shouldDestroy() const {
  auto* data = getComponent<StagehandDataComponent>();
  return data ? data->dead : true;
}

ClientEntityMode StagehandAdapter::clientEntityMode() const {
  auto* data = getComponent<StagehandDataComponent>();
  return data ? data->clientEntityMode : ClientEntityMode::ClientSlaveOnly;
}

void StagehandAdapter::update(float dt, uint64_t) {
  if (!inWorld())
    return;
  
  auto* data = getComponent<StagehandDataComponent>();
  if (!data)
    return;
  
  if (isMaster() && data->scripted)
    m_scriptComponent.update(m_scriptComponent.updateDt(dt));
  
  if (world()->isClient()) {
    auto boundBox = metaBoundBox().translated(position());
    SpatialLogger::logPoly("world", PolyF(boundBox), { 0, 255, 255, 255 });
    SpatialLogger::logLine("world", boundBox.min(), boundBox.max(), {0, 255, 255, 255});
    SpatialLogger::logLine("world", { boundBox.xMin(), boundBox.yMax() }, { boundBox.xMax(), boundBox.yMin() }, {0, 255, 255, 255});
  }
}

Maybe<LuaValue> StagehandAdapter::callScript(String const& func, LuaVariadic<LuaValue> const& args) {
  return m_scriptComponent.invoke(func, args);
}

Maybe<LuaValue> StagehandAdapter::evalScript(String const& code) {
  return m_scriptComponent.eval(code);
}

String StagehandAdapter::typeName() const {
  auto* data = getComponent<StagehandDataComponent>();
  return data ? data->config.getString("type", "") : "";
}

Json StagehandAdapter::configValue(String const& name, Json const& def) const {
  auto* data = getComponent<StagehandDataComponent>();
  return data ? data->config.query(name, def) : def;
}

Maybe<Json> StagehandAdapter::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) {
  return m_scriptComponent.handleMessage(message, sendingConnection == world()->connection(), args);
}

void StagehandAdapter::readConfig(Json config) {
  auto* data = getComponent<StagehandDataComponent>();
  if (!data)
    return;
    
  data->config = config;
  data->scripted = config.contains("scripts");
  data->clientEntityMode = ClientEntityModeNames.getLeft(config.getString("clientEntityMode", "ClientSlaveOnly"));
  
  if (config.contains("position")) {
    auto pos = jsonToVec2F(config.get("position"));
    m_xPosition.set(pos[0]);
    m_yPosition.set(pos[1]);
    
    auto* posComp = getComponent<PositionComponent>();
    if (posComp)
      posComp->position = pos;
  }
  
  Maybe<RectF> broadcastArea = jsonToMaybe<RectF>(config.opt("broadcastArea").value(Json()), jsonToRectF);
  if (broadcastArea && (broadcastArea->size()[0] < 0.0 || broadcastArea->size()[1] < 0.0))
    broadcastArea.reset();
  data->boundBox = broadcastArea.value(RectF(-5.0, -5.0, 5.0, 5.0));
  
  auto* bbox = getComponent<BoundingBoxComponent>();
  if (bbox)
    bbox->boundingBox = data->boundBox;
  
  if (data->scripted) {
    m_scriptComponent.setScripts(jsonToStringList(config.getArray("scripts", JsonArray())));
    m_scriptComponent.setUpdateDelta(config.getInt("scriptDelta", 5));
    
    if (config.contains("scriptStorage"))
      m_scriptComponent.setScriptStorage(config.getObject("scriptStorage"));
  }
  
  setKeepAlive(config.getBool("keepAlive", false));
}

LuaCallbacks StagehandAdapter::makeStagehandCallbacks() {
  LuaCallbacks callbacks;
  
  callbacks.registerCallback("id", [this]() {
      return entityId();
    });
  
  callbacks.registerCallback("position", [this]() {
      return position();
    });
  
  callbacks.registerCallback("setPosition", [this](Vec2F const& position) {
      setPosition(position);
    });
  
  callbacks.registerCallback("die", [this]() {
      auto* data = getComponent<StagehandDataComponent>();
      if (data)
        data->dead = true;
    });
  
  callbacks.registerCallback("typeName", [this]() {
      return typeName();
    });
  
  callbacks.registerCallback("setUniqueId", [this](Maybe<String> const& uniqueId) {
      setUniqueId(uniqueId);
    });
  
  return callbacks;
}

} // namespace ECS
} // namespace Star
