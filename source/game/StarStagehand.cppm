module;
#include "StarJson.hpp"
import star.net_compatibility;
import star.byte_array;
import star.id_map;
#include "StarCasting.hpp"
import star.vector;
#include "StarDataStream.hpp"
import star.game_types;
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarLua.hpp"
#include "StarOrderedMap.hpp"
import star.rect;
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
import star.ref_ptr;
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
#include "StarLuaComponents.hpp"
import star.scripted_entity;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.net_element_basic_fields;
import star.net_element_float_fields;
import star.net_element_system;

namespace Star {

STAR_CLASS(Stagehand);
STAR_CLASS(BehaviorState);

class Stagehand : public virtual ScriptedEntity {
public:
  Stagehand(Json const& config);
  Stagehand(ByteArray const& netStore, NetCompatibilityRules rules = {});

  Json diskStore() const;
  ByteArray netStore(NetCompatibilityRules rules = {});

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  EntityType entityType() const override;

  void setPosition(Vec2F const& position);

  Vec2F position() const override;

  RectF metaBoundBox() const override;

  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  String name() const override;

  void update(float dt, uint64_t currentStep) override;

  bool shouldDestroy() const override;
  
  ClientEntityMode clientEntityMode() const override;

  Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  Maybe<LuaValue> evalScript(String const& code) override;

  String typeName() const;
  
  Json configValue(String const& name, Json const& def = Json()) const;

  Maybe<ChainableJsonMessageResponse> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  using Entity::setUniqueId;

private:
  Stagehand();

  void readConfig(Json config);

  LuaCallbacks makeStagehandCallbacks();

  Json m_config;

  RectF m_boundBox;

  bool m_dead = false;
      
  ClientEntityMode m_clientEntityMode;

  NetElementTopGroup m_netGroup;

  NetElementFloat m_xPosition;
  NetElementFloat m_yPosition;

  NetElementData<Maybe<String>> m_uniqueIdNetState;

  bool m_scripted = false;
  List<BehaviorStatePtr> m_behaviors;
  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>
      m_scriptComponent;
};

}

export module star.stagehand;

export namespace Star {
  using ::Star::Stagehand;
  using ::Star::StagehandPtr;
  using ::Star::StagehandConstPtr;
  using ::Star::StagehandWeakPtr;
  using ::Star::StagehandConstWeakPtr;
  using ::Star::StagehandUPtr;
  using ::Star::StagehandConstUPtr;
  using ::Star::BehaviorState;
  using ::Star::BehaviorStatePtr;
  using ::Star::BehaviorStateConstPtr;
  using ::Star::BehaviorStateWeakPtr;
  using ::Star::BehaviorStateConstWeakPtr;
  using ::Star::BehaviorStateUPtr;
  using ::Star::BehaviorStateConstUPtr;
}
