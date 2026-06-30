#pragma once

#include "StarPeriodic.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarNetElementSystem.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarTileEntity.hpp"
#include "StarStatusEffectEntity.hpp"
#include "StarSet.hpp"
#include "StarColor.hpp"
#include "StarScriptedEntity.hpp"
#include "StarChattyEntity.hpp"
#include "StarWireEntity.hpp"
#include "StarInspectableEntity.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarDamageTypes.hpp"
#include "StarEntityRendering.hpp"

namespace Star {

class AudioInstance;
using AudioInstancePtr = SharedPtr<AudioInstance>;
class ObjectDatabase;
using ObjectDatabasePtr = SharedPtr<ObjectDatabase>;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
struct ObjectConfig;
using ObjectConfigConstPtr = SharedPtr<ObjectConfig const>;
struct ObjectOrientation;
using ObjectOrientationPtr = SharedPtr<ObjectOrientation>;
class Object;
using ObjectPtr = SharedPtr<Object>;

class Object
  : public virtual TileEntity,
    public virtual StatusEffectEntity,
    public virtual ScriptedEntity,
    public virtual ChattyEntity,
    public virtual InspectableEntity,
    public virtual WireEntity {
public:

  Object(ObjectConfigConstPtr config, Json const& parameters = JsonObject());

  [[nodiscard]] Json diskStore() const;
  [[nodiscard]] ByteArray netStore(NetCompatibilityRules rules = {});

  [[nodiscard]] EntityType entityType() const override;
  [[nodiscard]] ClientEntityMode clientEntityMode() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  [[nodiscard]] Vec2F position() const override;
  [[nodiscard]] RectF metaBoundBox() const override;

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  [[nodiscard]] String name() const override;
  [[nodiscard]] String description() const override;

  [[nodiscard]] bool inspectable() const override;
  [[nodiscard]] Maybe<String> inspectionLogName() const override;
  [[nodiscard]] Maybe<String> inspectionDescription(String const& species) const override;

  [[nodiscard]] List<LightSource> lightSources() const override;

  [[nodiscard]] bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  void update(float dt, uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;

  void renderLightSources(RenderCallback* renderCallback) override;

  [[nodiscard]] bool checkBroken() override;

  [[nodiscard]] Vec2I tilePosition() const override;

  [[nodiscard]] List<Vec2I> spaces() const override;
  [[nodiscard]] List<MaterialSpace> materialSpaces() const override;
  [[nodiscard]] List<Vec2I> roots() const override;

  [[nodiscard]] Direction direction() const;
  void setDirection(Direction direction);

  // Updates tile position and calls updateOrientation
  void setTilePosition(Vec2I const& pos) override;

  // Find a new valid orientation for the object
  void updateOrientation();
  [[nodiscard]] List<Vec2I> anchorPositions() const;

  [[nodiscard]] virtual List<Drawable> cursorHintDrawables() const;

  [[nodiscard]] String shortDescription() const;
  [[nodiscard]] String category() const;

  [[nodiscard]] virtual ObjectOrientationPtr currentOrientation() const;

  [[nodiscard]] List<PersistentStatusEffect> statusEffects() const override;
  [[nodiscard]] PolyF statusEffectArea() const override;

  [[nodiscard]] List<DamageSource> damageSources() const override;

  [[nodiscard]] Maybe<HitType> queryHit(DamageSource const& source) const override;
  [[nodiscard]] Maybe<PolyF> hitPoly() const override;

  [[nodiscard]] List<DamageNotification> applyDamage(DamageRequest const& damage) override;

  [[nodiscard]] bool damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) override;
  [[nodiscard]] bool canBeDamaged() const override;

  [[nodiscard]] RectF interactiveBoundBox() const override;

  [[nodiscard]] bool isInteractive() const override;
  [[nodiscard]] InteractAction interact(InteractRequest const& request) override;
  [[nodiscard]] List<Vec2I> interactiveSpaces() const override;

  [[nodiscard]] Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  [[nodiscard]] Maybe<LuaValue> evalScript(String const& code) override;

  [[nodiscard]] Vec2F mouthPosition() const override;
  [[nodiscard]] Vec2F mouthPosition(bool ignoreAdjustments) const override;
  [[nodiscard]] List<ChatAction> pullPendingChatActions() override;

  void breakObject(bool smash = true);

  [[nodiscard]] size_t nodeCount(WireDirection direction) const override;
  [[nodiscard]] Vec2I nodePosition(WireNode wireNode) const override;
  [[nodiscard]] List<WireConnection> connectionsForNode(WireNode wireNode) const override;
  [[nodiscard]] bool nodeState(WireNode wireNode) const override;

  [[nodiscard]] String nodeIcon(WireNode wireNode) const override;
  [[nodiscard]] Color nodeColor(WireNode wireNode) const override;

  void addNodeConnection(WireNode wireNode, WireConnection nodeConnection) override;
  void removeNodeConnection(WireNode wireNode, WireConnection nodeConnection) override;

  void evaluate(WireCoordinator* coordinator) override;

  [[nodiscard]] List<QuestArcDescriptor> offeredQuests() const override;
  [[nodiscard]] StringSet turnInQuests() const override;
  [[nodiscard]] Vec2F questIndicatorPosition() const override;

  [[nodiscard]] Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args = {}) override;

  // Check, in order, the passed in object parameters, the config parameters,
  // and then the orientation parameters for the given key.  Returns 'def' if
  // no value is found.
  [[nodiscard]] Json configValue(String const& name, Json const& def = Json()) const;

  [[nodiscard]] ObjectConfigConstPtr config() const;

  [[nodiscard]] float liquidFillLevel() const;

  [[nodiscard]] bool biomePlaced() const;

  using Entity::setUniqueId;

protected:
  friend class ObjectDatabase;

  // Will be automatically called at appropriate times.  Derived classes must
  // call base class versions.
  virtual void getNetStates(bool initial);
  virtual void setNetStates();

  virtual void readStoredData(Json const& diskStore);
  [[nodiscard]] virtual Json writeStoredData() const;

  void setImageKey(String const& name, String const& value);

  [[nodiscard]] size_t orientationIndex() const;
  virtual void setOrientationIndex(size_t orientationIndex);

  [[nodiscard]] PolyF volume() const;

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

  [[nodiscard]] LuaCallbacks makeObjectCallbacks();
  [[nodiscard]] LuaCallbacks makeAnimatorObjectCallbacks();

  void ensureNetSetup();
  [[nodiscard]] List<Drawable> orientationDrawables(size_t orientationIndex) const;

  void addChatMessage(String const& message, Json const& config, String const& portrait = "");

  void writeOutboundNode(Vec2I outboundNode, bool state);

  [[nodiscard]] EntityRenderLayer renderLayer() const;

  // Base class render() simply calls all of these in turn.
  void renderLights(RenderCallback* renderCallback) const;
  void renderParticles(RenderCallback* renderCallback);
  void renderSounds(RenderCallback* renderCallback);

  [[nodiscard]] List<ObjectOrientationPtr> const& getOrientations() const;

  [[nodiscard]] Vec2F damageShake() const;

  void checkLiquidBroken();
  GameTimer m_liquidCheckTimer;

  ObjectConfigConstPtr m_config;
  Maybe<List<ObjectOrientationPtr>> m_orientations;
  NetElementHashMap<String, Json> m_parameters;

  NetElementData<Maybe<String>> m_uniqueIdNetState;

  NetElementInt m_xTilePosition;
  NetElementInt m_yTilePosition;
  NetElementEnum<Direction> m_direction;
  float m_animationTimer;
  int m_currentFrame;

  Directives m_directives;
  Directives m_colorDirectives;
  String m_colorSuffix;

  Maybe<PeriodicFunction<float>> m_lightFlickering;

  EntityTileDamageStatusPtr m_tileDamageStatus;

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
  AudioInstancePtr m_soundEffect;

  NetElementData<Color> m_lightSourceColor;

  Vec2F m_animationPosition;
  float m_animationCenterLine;
  NetworkedAnimatorPtr m_networkedAnimator;
  NetworkedAnimator::DynamicTarget m_networkedAnimatorDynamicTarget;

  List<ChatAction> m_pendingChatActions;
  NetElementEvent m_newChatMessageEvent;
  NetElementString m_chatMessage;
  NetElementString m_chatPortrait;
  NetElementData<Json> m_chatConfig;

  struct OrientationDrawablesCache {
    size_t orientationIndex;
    List<Drawable> drawables;
  };
  mutable Maybe<OrientationDrawablesCache> m_orientationDrawablesCache;

  List<InputNode> m_inputNodes;
  List<OutputNode> m_outputNodes;

  NetElementData<List<QuestArcDescriptor>> m_offeredQuests;
  NetElementData<StringSet> m_turnInQuests;

  NetElementHashMap<String, Json> m_scriptedAnimationParameters;

  NetElementData<List<DamageSource>> m_damageSources;

  ClientEntityMode m_clientEntityMode;
};

}
