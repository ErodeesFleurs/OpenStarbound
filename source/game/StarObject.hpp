#pragma once

#include "StarChattyEntity.hpp"
#include "StarColor.hpp"
#include "StarConfig.hpp"
#include "StarDamageTypes.hpp"
#include "StarEntityRendering.hpp"
#include "StarInspectableEntity.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarScriptedEntity.hpp"
#include "StarStatusEffectEntity.hpp"
#include "StarTileEntity.hpp"
#include "StarWireEntity.hpp"

import std;

namespace Star {

struct ObjectConfig;
struct ObjectOrientation;

class Object
    : public virtual TileEntity,
      public virtual StatusEffectEntity,
      public virtual ScriptedEntity,
      public virtual ChattyEntity,
      public virtual InspectableEntity,
      public virtual WireEntity {
public:
  Object(ConstPtr<ObjectConfig> config, Json const& parameters = JsonObject());

  auto diskStore() const -> Json;
  auto netStore(NetCompatibilityRules rules = {}) -> ByteArray;

  auto entityType() const -> EntityType override;
  auto clientEntityMode() const -> ClientEntityMode override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  auto position() const -> Vec2F override;
  auto metaBoundBox() const -> RectF override;

  auto writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t> override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  auto name() const -> String override;
  auto description() const -> String override;

  auto inspectable() const -> bool override;
  auto inspectionLogName() const -> std::optional<String> override;
  auto inspectionDescription(String const& species) const -> std::optional<String> override;

  auto lightSources() const -> List<LightSource> override;

  auto shouldDestroy() const -> bool override;
  void destroy(RenderCallback* renderCallback) override;

  void update(float dt, std::uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  void renderLightSources(RenderCallback* renderCallback) override;

  auto checkBroken() -> bool override;

  auto tilePosition() const -> Vec2I override;

  auto spaces() const -> List<Vec2I> override;
  auto materialSpaces() const -> List<MaterialSpace> override;
  auto roots() const -> List<Vec2I> override;

  auto direction() const -> Direction;
  void setDirection(Direction direction);

  // Updates tile position and calls updateOrientation
  void setTilePosition(Vec2I const& pos) override;

  // Find a new valid orientation for the object
  void updateOrientation();
  auto anchorPositions() const -> List<Vec2I>;

  virtual auto cursorHintDrawables() const -> List<Drawable>;

  auto shortDescription() const -> String;
  auto category() const -> String;

  virtual auto currentOrientation() const -> Ptr<ObjectOrientation>;

  auto statusEffects() const -> List<PersistentStatusEffect> override;
  auto statusEffectArea() const -> PolyF override;

  auto damageSources() const -> List<DamageSource> override;

  auto queryHit(DamageSource const& source) const -> std::optional<HitType> override;
  auto hitPoly() const -> std::optional<PolyF> override;

  auto applyDamage(DamageRequest const& damage) -> List<DamageNotification> override;

  auto damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) -> bool override;
  auto canBeDamaged() const -> bool override;

  auto interactiveBoundBox() const -> RectF override;

  auto isInteractive() const -> bool override;
  auto interact(InteractRequest const& request) -> InteractAction override;
  auto interactiveSpaces() const -> List<Vec2I> override;

  auto callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> override;
  auto evalScript(String const& code) -> std::optional<LuaValue> override;

  auto mouthPosition() const -> Vec2F override;
  auto mouthPosition(bool ignoreAdjustments) const -> Vec2F override;
  auto pullPendingChatActions() -> List<ChatAction> override;

  void breakObject(bool smash = true);

  auto nodeCount(WireDirection direction) const -> size_t override;
  auto nodePosition(WireNode wireNode) const -> Vec2I override;
  auto connectionsForNode(WireNode wireNode) const -> List<WireConnection> override;
  auto nodeState(WireNode wireNode) const -> bool override;

  auto nodeIcon(WireNode wireNode) const -> String override;
  auto nodeColor(WireNode wireNode) const -> Color override;

  void addNodeConnection(WireNode wireNode, WireConnection nodeConnection) override;
  void removeNodeConnection(WireNode wireNode, WireConnection nodeConnection) override;

  void evaluate(WireCoordinator* coordinator) override;

  auto offeredQuests() const -> List<QuestArcDescriptor> override;
  auto turnInQuests() const -> StringSet override;
  auto questIndicatorPosition() const -> Vec2F override;

  auto receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args = {}) -> std::optional<Json> override;

  // Check, in order, the passed in object parameters, the config parameters,
  // and then the orientation parameters for the given key.  Returns 'def' if
  // no value is found.
  auto configValue(String const& name, Json const& def = Json()) const -> Json;

  auto config() const -> ConstPtr<ObjectConfig>;

  auto liquidFillLevel() const -> float;

  auto biomePlaced() const -> bool;

  using Entity::setUniqueId;

protected:
  friend class ObjectDatabase;

  // Will be automatically called at appropriate times.  Derived classes must
  // call base class versions.
  virtual void getNetStates(bool initial);
  virtual void setNetStates();

  virtual void readStoredData(Json const& diskStore);
  virtual auto writeStoredData() const -> Json;

  void setImageKey(String const& name, String const& value);

  auto orientationIndex() const -> size_t;
  virtual void setOrientationIndex(size_t orientationIndex);

  auto volume() const -> PolyF;

  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>> m_scriptComponent;
  mutable LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;

  NetElementTopGroup m_netGroup;
  NetElementBool m_interactive;
  NetElementData<List<MaterialSpace>> m_materialSpaces;

private:
  struct InputNode {
    InputNode(Json positionConfig, Json config);
    Vec2I position;
    NetElementData<List<WireConnection>> connections;
    NetElementBool state;
    Color color;
    String icon;
  };

  struct OutputNode {
    OutputNode(Json positionConfig, Json config);
    Vec2I position;
    NetElementData<List<WireConnection>> connections;
    NetElementBool state;
    Color color;
    String icon;
  };

  auto makeObjectCallbacks() -> LuaCallbacks;
  auto makeAnimatorObjectCallbacks() -> LuaCallbacks;

  void ensureNetSetup();
  auto orientationDrawables(size_t orientationIndex) const -> List<Drawable>;

  void addChatMessage(String const& message, Json const& config, String const& portrait = "");

  void writeOutboundNode(Vec2I outboundNode, bool state);

  auto renderLayer() const -> EntityRenderLayer;

  // Base class render() simply calls all of these in turn.
  void renderLights(RenderCallback* renderCallback) const;
  void renderParticles(RenderCallback* renderCallback);
  void renderSounds(RenderCallback* renderCallback);

  auto getOrientations() const -> List<Ptr<ObjectOrientation>> const&;

  auto damageShake() const -> Vec2F;

  void checkLiquidBroken();
  GameTimer m_liquidCheckTimer;

  ConstPtr<ObjectConfig> m_config;
  std::optional<List<Ptr<ObjectOrientation>>> m_orientations;
  NetElementHashMap<String, Json> m_parameters;

  NetElementData<std::optional<String>> m_uniqueIdNetState;

  NetElementInt m_xTilePosition;
  NetElementInt m_yTilePosition;
  NetElementEnum<Direction> m_direction;
  float m_animationTimer;
  int m_currentFrame;

  Directives m_directives;
  Directives m_colorDirectives;
  String m_colorSuffix;

  std::optional<PeriodicFunction<float>> m_lightFlickering;

  Ptr<EntityTileDamageStatus> m_tileDamageStatus;

  bool m_broken;
  bool m_unbreakable;
  NetElementFloat m_health;

  size_t m_orientationIndex;
  NetElementSize m_orientationIndexNetState;
  NetElementHashMap<String, String> m_netImageKeys;
  mutable StringMap<String> m_imageKeys;

  void resetEmissionTimers();
  List<GameTimer> m_emissionTimers;

  NetElementBool m_soundEffectEnabled;
  Ptr<AudioInstance> m_soundEffect;

  NetElementData<Color> m_lightSourceColor;

  Vec2F m_animationPosition;
  float m_animationCenterLine;
  Ptr<NetworkedAnimator> m_networkedAnimator;
  NetworkedAnimator::DynamicTarget m_networkedAnimatorDynamicTarget;

  List<ChatAction> m_pendingChatActions;
  NetElementEvent m_newChatMessageEvent;
  NetElementString m_chatMessage;
  NetElementString m_chatPortrait;
  NetElementData<Json> m_chatConfig;

  mutable std::optional<std::pair<size_t, List<Drawable>>> m_orientationDrawablesCache;

  List<InputNode> m_inputNodes;
  List<OutputNode> m_outputNodes;

  NetElementData<List<QuestArcDescriptor>> m_offeredQuests;
  NetElementData<StringSet> m_turnInQuests;

  NetElementHashMap<String, Json> m_scriptedAnimationParameters;

  NetElementData<List<DamageSource>> m_damageSources;

  ClientEntityMode m_clientEntityMode;
};

}// namespace Star
