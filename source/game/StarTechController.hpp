#pragma once

#include "StarDirectives.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarTechDatabase.hpp"

namespace Star {

class TechController;
using TechControllerPtr = SharedPtr<TechController>;
class StatusController;
class Assets;
using AssetsConstPtr = SharedPtr<Assets const>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;

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

  TechController(AssetsConstPtr assets, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  TechController(Entity& parentEntity, ActorMovementController& movementController, StatusController& statusController, AssetsConstPtr assets, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  [[nodiscard]] Json diskStore();
  void diskLoad(Json const& store);

  void init(Entity& parentEntity, ActorMovementController& movementController, StatusController& statusController);
  void uninit();

  void setLoadedTech(StringList const& techModules, bool forceLoad = false);
  [[nodiscard]] StringList loadedTech() const;
  void reloadTech();

  [[nodiscard]] bool techOverridden() const;
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

  [[nodiscard]] Maybe<ParentState> parentState() const;
  [[nodiscard]] DirectivesGroup const& parentDirectives() const;
  [[nodiscard]] Vec2F parentOffset() const;
  [[nodiscard]] bool toolUsageSuppressed() const;

  [[nodiscard]] bool parentHidden() const;

  [[nodiscard]] List<Drawable> backDrawables();
  [[nodiscard]] List<Drawable> frontDrawables();

  [[nodiscard]] List<LightSource> lightSources() const;

  [[nodiscard]] List<AudioInstancePtr> pullNewAudios();
  [[nodiscard]] List<Particle> pullNewParticles();

  [[nodiscard]] Maybe<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});

private:
  struct TechAnimator : public NetElement {
    TechAnimator(Maybe<String> animationConfig = {}, AssetsConstPtr assets = {}, ParticleDatabaseConstPtr particleDatabase = {}, ImageMetadataDatabaseConstPtr imageMetadataDatabase = {});

    void initNetVersion(observer_ptr<NetElementVersion const> version = nullptr) override;

    void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
    void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

    void enableNetInterpolation(float extrapolationHint = 0.0f) override;
    void disableNetInterpolation() override;
    void tickNetInterpolation(float dt) override;

    [[nodiscard]] bool writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules = {}) const override;
    void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
    void blankNetDelta(float interpolationTime) override;

    // If setting invisible, stops all playing audio
    void setVisible(bool visible);
    [[nodiscard]] bool isVisible() const;

    Maybe<String> animationConfig;
    AssetsConstPtr assets;
    ParticleDatabaseConstPtr particleDatabase;
    ImageMetadataDatabaseConstPtr imageMetadataDatabase;
    NetworkedAnimator animator;
    NetworkedAnimator::DynamicTarget dynamicTarget;
    NetElementBool visible;
    NetElementGroup netGroup;
  };

  using TechAnimatorGroup = NetElementDynamicGroup<TechAnimator>;

  struct TechModule {
    TechConfig config;

    LuaMessageHandlingComponent<LuaStorableComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>>
      scriptComponent;
    bool visible;
    bool toolUsageSuppressed;
    Directives parentDirectives;
    TechAnimatorGroup::ElementId animatorId;
  };

  // Name of module, any existing module script data.
  void setupTechModules(List<tuple<String, JsonObject>> const& moduleInits);

  void unloadModule(TechModule& techModule);

  void initializeModules();

  void resetMoves();
  void updateAnimators(float dt);

  [[nodiscard]] LuaCallbacks makeTechCallbacks(TechModule& techModule);

  Maybe<StringList> m_overriddenTech;
  LinkedList<TechModule> m_techModules;
  TechAnimatorGroup m_techAnimators;

  observer_ptr<Entity> m_parentEntity;
  observer_ptr<ActorMovementController> m_movementController;
  observer_ptr<StatusController> m_statusController;
  AssetsConstPtr m_assets;
  ParticleDatabaseConstPtr m_particleDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;

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

}// namespace Star
