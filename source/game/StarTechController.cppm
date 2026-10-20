module;
import std;
import star.lua;
import star.json;
import star.net_compatibility;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.bi_map;
import star.vector;
import star.audio;
import star.map;
import star.set;
import star.maybe;


import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.lua_components;
import star.lua_actor_movement_component;

import star.tech_database;

import star.game_types;
import star.net_element;
import star.net_element_basic_fields;
import star.net_element_float_fields;
import star.net_element_dynamic_group;
import star.net_element_group;

namespace Star {

class Root;

class StatusController;
class Entity;
class ActorMovementController;

// Class that acts as a movement controller for the parent entity that supports
// a variety scriptable "Tech" that the entity can use that affect movement,
// physics, sounds, particles, damage regions, etc.  Network capable, and all
// flags are sensibly set on both the client and server.
class TechController : public NetElementGroup {
public:
  enum class ParentState {
    Stand,
    Fly,
    Fall,
    Sit,
    Lay,
    Duck,
    Walk,
    Run,
    Swim,
    SwimIdle
  };
  static EnumMap<ParentState> const ParentStateNames;

  TechController(Root& root);

  Json diskStore();
  void diskLoad(Json const& store);

  void init(Entity* parentEntity, ActorMovementController* movementController, StatusController* statusController);
  void uninit();

  void setLoadedTech(StringList const& techModules, bool forceLoad = false);
  StringList loadedTech() const;
  void reloadTech();

  bool techOverridden() const;
  void setOverrideTech(StringList const& techModules);
  void clearOverrideTech();

  void setShouldRun(bool shouldRun);

  void beginPrimaryFire();
  void beginAltFire();
  void endPrimaryFire();
  void endAltFire();

  void moveUp();
  void moveDown();
  void moveLeft();
  void moveRight();
  void jump();
  void special(int specialKey);

  void setAimPosition(Vec2F const& aimPosition);

  void tickMaster(float dt);
  void tickSlave(float dt);

  Maybe<ParentState> parentState() const;
  DirectivesGroup const& parentDirectives() const;
  Vec2F parentOffset() const;
  bool toolUsageSuppressed() const;

  bool parentHidden() const;

  List<Drawable> backDrawables();
  List<Drawable> frontDrawables();

  List<LightSource> lightSources() const;

  List<std::shared_ptr<AudioInstance>> pullNewAudios();
  List<Particle> pullNewParticles();

  Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});

private:
  Root& m_root;
  struct TechAnimator : public NetElement {
    TechAnimator(Root& root);
    TechAnimator(Root& root, Maybe<String> animationConfig);

    void initNetVersion(NetElementVersion const* version = nullptr) override;

    void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
    void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

    void enableNetInterpolation(float extrapolationHint = 0.0f) override;
    void disableNetInterpolation() override;
    void tickNetInterpolation(float dt) override;

    bool writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules = {}) const override;
    void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
    void blankNetDelta(float interpolationTime) override;

    // If setting invisible, stops all playing audio
    void setVisible(bool visible);
    bool isVisible() const;

    Root& m_root;
    Maybe<String> animationConfig;
    NetworkedAnimator animator;
    NetworkedAnimator::DynamicTarget dynamicTarget;
    NetElementBool visible;
    NetElementGroup netGroup;
  };

  typedef NetElementDynamicGroup<TechAnimator> TechAnimatorGroup;

  struct TechModule {
    explicit TechModule(Root& root) : scriptComponent(root) {}

    TechConfig config;

    LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>>
        scriptComponent;
    bool visible;
    bool toolUsageSuppressed;
    Directives parentDirectives;
    TechAnimatorGroup::ElementId animatorId;
  };

  // Name of module, any existing module script data.
  void setupTechModules(List<std::tuple<String, JsonObject>> const& moduleInits);

  void unloadModule(TechModule& techModule);

  void initializeModules();

  void resetMoves();
  void updateAnimators(float dt);

  LuaCallbacks makeTechCallbacks(TechModule& techModule);

  Maybe<StringList> m_overriddenTech;
  LinkedList<TechModule> m_techModules;
  TechAnimatorGroup m_techAnimators;

  Entity* m_parentEntity;
  ActorMovementController* m_movementController;
  StatusController* m_statusController;

  bool m_moveRun;
  bool m_movePrimaryFire;
  bool m_moveAltFire;
  bool m_moveUp;
  bool m_moveDown;
  bool m_moveLeft;
  bool m_moveRight;
  bool m_moveJump;
  bool m_moveSpecial1;
  bool m_moveSpecial2;
  bool m_moveSpecial3;

  Vec2F m_aimPosition;

  NetElementData<Maybe<ParentState>> m_parentState;
  NetElementData<DirectivesGroup> m_parentDirectives;
  NetElementFloat m_xParentOffset;
  NetElementFloat m_yParentOffset;
  NetElementBool m_parentHidden;
  NetElementBool m_toolUsageSuppressed;
};

}

export module star.tech_controller;

export namespace Star {
  using ::Star::TechController;
  using ::Star::StatusController;
}
