module;
import star.game_timers;
import star.json_extra;
import std;
import star.type_identity;
import star.json;
import star.id_map;
import star.random;
import star.periodic;
import star.periodic_function;
import star.casting;
import star.strong_typedef;
import star.set;
import star.color;
import star.lua;
import star.vector;
import star.data_stream;
import star.game_types;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.maybe;
import star.bi_map;
import star.string;
import star.poly;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ordered_map;

import star.net_element_basic_fields;
import star.net_element_float_fields;
import star.net_element_containers;
import star.net_element_system;

import star.net_compatibility;
import star.byte_array;
namespace Star {
class Root;
class AudioInstance;
class ObjectDatabase;
struct ObjectConfig;
struct ObjectOrientation;
class Object;
}

import star.lua_root;
import star.lua_components;
import star.lua_animation_component;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.damage_types;
import star.entity_rendering_types;
import star.entity_rendering;

namespace Star {


class Object
  : public virtual TileEntity,
    public virtual StatusEffectEntity,
    public virtual ScriptedEntity,
    public virtual ChattyEntity,
    public virtual InspectableEntity,
    public virtual WireEntity {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<Object, TileEntity, StatusEffectEntity, ScriptedEntity,
        ChattyEntity, InspectableEntity, WireEntity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<Object>();
  }

  Object(Root& rootContext, std::shared_ptr<ObjectConfig const> config, Json const& parameters = JsonObject());

  Json diskStore() const;
  ByteArray netStore(NetCompatibilityRules rules = {});

  virtual EntityType entityType() const override;
  virtual ClientEntityMode clientEntityMode() const override;

  virtual void init(World* world, EntityId entityId, EntityMode mode) override;
  virtual void uninit() override;

  virtual Vec2F position() const override;
  virtual RectF metaBoundBox() const override;

  virtual std::pair<ByteArray, std::uint64_t> writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  virtual void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  virtual String name() const override;
  virtual String description() const override;

  virtual bool inspectable() const override;
  virtual Maybe<String> inspectionLogName() const override;
  virtual Maybe<String> inspectionDescription(String const& species) const override;

  virtual List<LightSource> lightSources() const override;

  virtual bool shouldDestroy() const override;
  virtual void destroy(RenderCallback* renderCallback) override;

  virtual void update(float dt, std::uint64_t currentStep) override;

  virtual void render(RenderCallback* renderCallback) override;

  virtual void renderLightSources(RenderCallback* renderCallback) override;

  virtual bool checkBroken() override;

  virtual Vec2I tilePosition() const override;

  virtual List<Vec2I> spaces() const override;
  virtual List<MaterialSpace> materialSpaces() const override;
  virtual List<Vec2I> roots() const override;

  Direction direction() const;
  void setDirection(Direction direction);

  // Updates tile position and calls updateOrientation
  void setTilePosition(Vec2I const& pos) override;

  // Find a new valid orientation for the object
  void updateOrientation();
  List<Vec2I> anchorPositions() const;

  virtual List<Drawable> cursorHintDrawables() const;

  String shortDescription() const;
  String category() const;

  virtual std::shared_ptr<ObjectOrientation> currentOrientation() const;

  virtual List<PersistentStatusEffect> statusEffects() const override;
  virtual PolyF statusEffectArea() const override;

  virtual List<DamageSource> damageSources() const override;

  virtual Maybe<HitType> queryHit(DamageSource const& source) const override;
  Maybe<PolyF> hitPoly() const override;

  virtual List<DamageNotification> applyDamage(DamageRequest const& damage) override;

  virtual bool damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) override;
  virtual bool canBeDamaged() const override;

  RectF interactiveBoundBox() const override;

  bool isInteractive() const override;
  virtual InteractAction interact(InteractRequest const& request) override;
  List<Vec2I> interactiveSpaces() const override;

  Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  Maybe<LuaValue> evalScript(String const& code) override;

  virtual Vec2F mouthPosition() const override;
  virtual Vec2F mouthPosition(bool ignoreAdjustments) const override;
  virtual List<ChatAction> pullPendingChatActions() override;

  void breakObject(bool smash = true);

  virtual std::size_t nodeCount(WireDirection direction) const override;
  virtual Vec2I nodePosition(WireNode wireNode) const override;
  virtual List<WireConnection> connectionsForNode(WireNode wireNode) const override;
  virtual bool nodeState(WireNode wireNode) const override;

  virtual String nodeIcon(WireNode wireNode) const override;
  virtual Color nodeColor(WireNode wireNode) const override;

  virtual void addNodeConnection(WireNode wireNode, WireConnection nodeConnection) override;
  virtual void removeNodeConnection(WireNode wireNode, WireConnection nodeConnection) override;

  virtual void evaluate(WireCoordinator* coordinator) override;

  virtual List<QuestArcDescriptor> offeredQuests() const override;
  virtual StringSet turnInQuests() const override;
  virtual Vec2F questIndicatorPosition() const override;

  Maybe<ChainableJsonMessageResponse> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args = {}) override;

  // Check, in order, the passed in object parameters, the config parameters,
  // and then the orientation parameters for the given key.  Returns 'def' if
  // no value is found.
  Json configValue(String const& name, Json const& def = Json()) const;

  std::shared_ptr<ObjectConfig const> config() const;

  float liquidFillLevel() const;

  bool biomePlaced() const;

  using Entity::setUniqueId;

protected:
  friend class ObjectDatabase;

  // Will be automatically called at appropriate times.  Derived classes must
  // call base class versions.
  virtual void getNetStates(bool initial);
  virtual void setNetStates();

  virtual void readStoredData(Json const& diskStore);
  virtual Json writeStoredData() const;

  void setImageKey(String const& name, String const& value);

  std::size_t orientationIndex() const;
  virtual void setOrientationIndex(std::size_t orientationIndex);

  PolyF volume() const;

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

  LuaCallbacks makeObjectCallbacks();
  LuaCallbacks makeAnimatorObjectCallbacks();

  void ensureNetSetup();
  List<Drawable> orientationDrawables(std::size_t orientationIndex) const;

  void addChatMessage(String const& message, Json const& config, String const& portrait = "");

  void writeOutboundNode(Vec2I outboundNode, bool state);

  EntityRenderLayer renderLayer() const;

  // Base class render() simply calls all of these in turn.
  void renderLights(RenderCallback* renderCallback) const;
  void renderParticles(RenderCallback* renderCallback);
  void renderSounds(RenderCallback* renderCallback);

  List<std::shared_ptr<ObjectOrientation>> const& getOrientations() const;

  Vec2F damageShake() const;

  void checkLiquidBroken();
  GameTimer m_liquidCheckTimer;

  std::shared_ptr<ObjectConfig const> m_config;
  Maybe<List<std::shared_ptr<ObjectOrientation>>> m_orientations;
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

  std::shared_ptr<EntityTileDamageStatus> m_tileDamageStatus;

  bool m_broken;
  bool m_unbreakable;
  NetElementFloat m_health;

  std::size_t m_orientationIndex;
  NetElementSize m_orientationIndexNetState;
  NetElementHashMap<String, String> m_netImageKeys;
  mutable StringMap<String> m_imageKeys;

  void resetEmissionTimers();
  List<GameTimer> m_emissionTimers;

  NetElementBool m_soundEffectEnabled;
  std::shared_ptr<AudioInstance> m_soundEffect;

  NetElementData<Color> m_lightSourceColor;

  Vec2F m_animationPosition;
  float m_animationCenterLine;
  std::shared_ptr<NetworkedAnimator> m_networkedAnimator;
  NetworkedAnimator::DynamicTarget m_networkedAnimatorDynamicTarget;

  List<ChatAction> m_pendingChatActions;
  NetElementEvent m_newChatMessageEvent;
  NetElementString m_chatMessage;
  NetElementString m_chatPortrait;
  NetElementData<Json> m_chatConfig;

  mutable Maybe<std::pair<std::size_t, List<Drawable>>> m_orientationDrawablesCache;

  List<InputNode> m_inputNodes;
  List<OutputNode> m_outputNodes;

  NetElementData<List<QuestArcDescriptor>> m_offeredQuests;
  NetElementData<StringSet> m_turnInQuests;

  NetElementHashMap<String, Json> m_scriptedAnimationParameters;

  NetElementData<List<DamageSource>> m_damageSources;

  ClientEntityMode m_clientEntityMode;
};

}

export module star.object;

export namespace Star {
  using ::Star::AudioInstance;
  using ::Star::ObjectDatabase;
  using ::Star::ObjectConfig;
  using ::Star::ObjectOrientation;
  using ::Star::Object;
}
