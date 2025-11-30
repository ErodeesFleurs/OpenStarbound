#pragma once

// ECS Object Adapter for OpenStarbound
// This adapter implements the Object entity using ECS components
// Objects are TileEntities that represent interactive world objects
// (furniture, containers, crafting stations, wiring, etc.)

#include "StarEntityAdapter.hpp"
#include "StarObjectDatabase.hpp"
#include "StarTileEntity.hpp"
#include "StarStatusEffectEntity.hpp"
#include "StarScriptedEntity.hpp"
#include "StarChattyEntity.hpp"
#include "StarWireEntity.hpp"
#include "StarInspectableEntity.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarGameTimers.hpp"
#include "StarColor.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarEntityRendering.hpp"

namespace Star {
namespace ECS {

// Wire node structures (matching original Object)
struct ObjectInputNode {
  Vec2I position;
  List<WireConnection> connections;
  bool state = false;
  Color color;
  String icon;
};

struct ObjectOutputNode {
  Vec2I position;
  List<WireConnection> connections;
  bool state = false;
  Color color;
  String icon;
};

// Object-specific component storing all object state
struct ObjectDataComponent {
  // Configuration
  ObjectConfigConstPtr config;
  JsonObject parameters;
  
  // Position and orientation
  Vec2I tilePosition;
  Direction direction = Direction::Left;
  size_t orientationIndex = 0;
  
  // Visual state
  float animationTimer = 0.0f;
  int currentFrame = 0;
  Directives directives;
  Directives colorDirectives;
  String colorSuffix;
  StringMap<String> imageKeys;
  Maybe<pair<size_t, List<Drawable>>> orientationDrawablesCache;
  
  // Health and damage
  float health = 1.0f;
  bool broken = false;
  bool unbreakable = false;
  EntityTileDamageStatusPtr tileDamageStatus;
  
  // Interaction
  bool interactive = false;
  List<MaterialSpace> materialSpaces;
  
  // Light and effects
  Color lightSourceColor;
  Maybe<PeriodicFunction<float>> lightFlickering;
  List<GameTimer> emissionTimers;
  bool soundEffectEnabled = false;
  AudioInstancePtr soundEffect;
  
  // Animation
  Vec2F animationPosition;
  float animationCenterLine = 0.0f;
  NetworkedAnimatorPtr networkedAnimator;
  NetworkedAnimator::DynamicTarget networkedAnimatorDynamicTarget;
  StringMap<Json> scriptedAnimationParameters;
  
  // Chat
  List<ChatAction> pendingChatActions;
  String chatMessage;
  String chatPortrait;
  Json chatConfig;
  
  // Wiring
  List<ObjectInputNode> inputNodes;
  List<ObjectOutputNode> outputNodes;
  
  // Quests
  List<QuestArcDescriptor> offeredQuests;
  StringSet turnInQuests;
  
  // Damage sources
  List<DamageSource> damageSources;
  
  // Mode
  ClientEntityMode clientEntityMode = ClientEntityMode::ClientSlaveOnly;
  
  // Misc
  bool biomePlaced = false;
  float liquidCheckTime = 0.0f;
  Maybe<String> uniqueId;
  
  // Bound box cache
  RectF metaBoundBox;
  bool metaBoundBoxValid = false;
};

// Tag component for identifying objects
struct ObjectTag {};

// Object adapter that wraps ECS entity to implement Object interfaces
class ObjectAdapter : public EntityAdapter,
                      public virtual TileEntity,
                      public virtual StatusEffectEntity,
                      public virtual ScriptedEntity,
                      public virtual ChattyEntity,
                      public virtual InspectableEntity,
                      public virtual WireEntity {
public:
  // Create from config and parameters
  static shared_ptr<ObjectAdapter> create(
    World* ecsWorld,
    ObjectConfigConstPtr config,
    Json const& parameters = JsonObject());
  
  // Create from disk store
  static shared_ptr<ObjectAdapter> createFromDiskStore(
    World* ecsWorld,
    Json const& diskStore);
  
  // Create from network data
  static shared_ptr<ObjectAdapter> createFromNet(
    World* ecsWorld,
    ByteArray const& netStore,
    NetCompatibilityRules rules = {});
  
  // Construct from existing ECS entity
  ObjectAdapter(World* ecsWorld, Entity ecsEntity);
  
  // Serialization
  Json diskStore() const;
  ByteArray netStore(NetCompatibilityRules rules = {}) const;
  
  // Entity interface
  EntityType entityType() const override;
  ClientEntityMode clientEntityMode() const override;
  
  void init(Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;
  
  Vec2F position() const override;
  RectF metaBoundBox() const override;
  
  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  
  String name() const override;
  String description() const override;
  
  // InspectableEntity interface
  bool inspectable() const override;
  Maybe<String> inspectionLogName() const override;
  Maybe<String> inspectionDescription(String const& species) const override;
  
  List<LightSource> lightSources() const override;
  
  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;
  
  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;
  
  // TileEntity interface
  bool checkBroken() override;
  Vec2I tilePosition() const override;
  List<Vec2I> spaces() const override;
  List<MaterialSpace> materialSpaces() const override;
  List<Vec2I> roots() const override;
  
  Direction direction() const;
  void setDirection(Direction direction);
  void setTilePosition(Vec2I const& pos) override;
  void updateOrientation();
  List<Vec2I> anchorPositions() const;
  
  virtual List<Drawable> cursorHintDrawables() const;
  String shortDescription() const;
  String category() const;
  virtual ObjectOrientationPtr currentOrientation() const;
  
  // StatusEffectEntity interface
  List<PersistentStatusEffect> statusEffects() const override;
  PolyF statusEffectArea() const override;
  
  // DamageableEntity interface
  List<DamageSource> damageSources() const override;
  Maybe<HitType> queryHit(DamageSource const& source) const override;
  Maybe<PolyF> hitPoly() const override;
  List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  
  bool damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) override;
  bool canBeDamaged() const override;
  
  // InteractiveEntity interface
  RectF interactiveBoundBox() const override;
  bool isInteractive() const override;
  InteractAction interact(InteractRequest const& request) override;
  List<Vec2I> interactiveSpaces() const override;
  
  // ScriptedEntity interface
  Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  Maybe<LuaValue> evalScript(String const& code) override;
  
  // ChattyEntity interface
  Vec2F mouthPosition() const override;
  Vec2F mouthPosition(bool ignoreAdjustments) const override;
  List<ChatAction> pullPendingChatActions() override;
  
  void breakObject(bool smash = true);
  
  // WireEntity interface
  size_t nodeCount(WireDirection direction) const override;
  Vec2I nodePosition(WireNode wireNode) const override;
  List<WireConnection> connectionsForNode(WireNode wireNode) const override;
  bool nodeState(WireNode wireNode) const override;
  String nodeIcon(WireNode wireNode) const override;
  Color nodeColor(WireNode wireNode) const override;
  void addNodeConnection(WireNode wireNode, WireConnection nodeConnection) override;
  void removeNodeConnection(WireNode wireNode, WireConnection nodeConnection) override;
  void evaluate(WireCoordinator* coordinator) override;
  
  // QuestIndicatorEntity interface
  List<QuestArcDescriptor> offeredQuests() const override;
  StringSet turnInQuests() const override;
  Vec2F questIndicatorPosition() const override;
  
  Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args = {}) override;
  
  // Config access
  Json configValue(String const& name, Json const& def = Json()) const;
  ObjectConfigConstPtr config() const;
  
  float liquidFillLevel() const;
  bool biomePlaced() const;
  
  using Entity::setUniqueId;

protected:
  virtual void getNetStates(bool initial);
  virtual void setNetStates();
  
  virtual void readStoredData(Json const& diskStore);
  virtual Json writeStoredData() const;
  
  void setImageKey(String const& name, String const& value);
  
  size_t orientationIndex() const;
  virtual void setOrientationIndex(size_t orientationIndex);
  
  PolyF volume() const;

private:
  LuaCallbacks makeObjectCallbacks();
  LuaCallbacks makeAnimatorObjectCallbacks();
  
  void ensureNetSetup();
  List<Drawable> orientationDrawables(size_t orientationIndex) const;
  
  void addChatMessage(String const& message, Json const& config, String const& portrait = "");
  void writeOutboundNode(Vec2I outboundNode, bool state);
  
  EntityRenderLayer renderLayer() const;
  
  void renderLights(RenderCallback* renderCallback) const;
  void renderParticles(RenderCallback* renderCallback);
  void renderSounds(RenderCallback* renderCallback);
  
  List<ObjectOrientationPtr> const& getOrientations() const;
  Vec2F damageShake() const;
  void checkLiquidBroken();
  
  void resetEmissionTimers();
  
  void setupNetStates();
  
  // Lua scripting
  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>> m_scriptComponent;
  mutable LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;
  
  // Network state
  NetElementTopGroup m_netGroup;
  NetElementBool m_interactiveNetState;
  NetElementData<List<MaterialSpace>> m_materialSpacesNetState;
  NetElementHashMap<String, Json> m_parametersNetState;
  NetElementData<Maybe<String>> m_uniqueIdNetState;
  NetElementInt m_xTilePositionNetState;
  NetElementInt m_yTilePositionNetState;
  NetElementEnum<Direction> m_directionNetState;
  NetElementFloat m_healthNetState;
  NetElementSize m_orientationIndexNetState;
  NetElementHashMap<String, String> m_imageKeysNetState;
  NetElementBool m_soundEffectEnabledNetState;
  NetElementData<Color> m_lightSourceColorNetState;
  NetElementEvent m_newChatMessageEventNetState;
  NetElementString m_chatMessageNetState;
  NetElementString m_chatPortraitNetState;
  NetElementData<Json> m_chatConfigNetState;
  NetElementData<List<QuestArcDescriptor>> m_offeredQuestsNetState;
  NetElementData<StringSet> m_turnInQuestsNetState;
  NetElementHashMap<String, Json> m_scriptedAnimationParametersNetState;
  NetElementData<List<DamageSource>> m_damageSourcesNetState;
  
  // Input/output node network state (dynamic based on config)
  List<NetElementData<List<WireConnection>>> m_inputConnectionsNetState;
  List<NetElementBool> m_inputStateNetState;
  List<NetElementData<List<WireConnection>>> m_outputConnectionsNetState;
  List<NetElementBool> m_outputStateNetState;
  
  // Cached orientations
  mutable Maybe<List<ObjectOrientationPtr>> m_orientationsCache;
};

using ObjectAdapterPtr = shared_ptr<ObjectAdapter>;

} // namespace ECS
} // namespace Star
