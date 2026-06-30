#pragma once

#include "StarAssets.hpp"
#include "StarDataStream.hpp"
#include "StarDrawable.hpp"
#include "StarGameTypes.hpp"
#include "StarNetElement.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarParticle.hpp"

namespace Star {

// Required for renderDummy
class ArmorItem;
class HeadArmor;
class ChestArmor;
class LegsArmor;
class BackArmor;

class Humanoid;
using HumanoidPtr = SharedPtr<Humanoid>;

class SpeciesDatabase;
using SpeciesDatabaseConstPtr = SharedPtr<SpeciesDatabase const>;
class DanceDatabase;
using DanceDatabaseConstPtr = SharedPtr<DanceDatabase const>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;

struct Dance;
using DancePtr = SharedPtr<Dance>;

enum class HumanoidEmote {
  Idle,
  Blabbering,
  Shouting,
  Happy,
  Sad,
  NEUTRAL,
  Laugh,
  Annoyed,
  Oh,
  OOOH,
  Blink,
  Wink,
  Eat,
  Sleep
};
extern EnumMap<HumanoidEmote> const HumanoidEmoteNames;
size_t const EmoteSize = 14;

enum class HumanoidHand {
  Idle,
  Blabbering,
  Shouting,
  Happy,
  Sad,
  NEUTRAL,
  Laugh,
  Annoyed,
  Oh,
  OOOH,
  Blink,
  Wink,
  Eat,
  Sleep
};
extern EnumMap<HumanoidEmote> const HumanoidEmoteNames;

struct Personality {
  String idle = "idle.1";
  String armIdle = "idle.1";
  Vec2F headOffset = Vec2F();
  Vec2F armOffset = Vec2F();
};

[[nodiscard]] Personality parsePersonalityArray(Json const& config);

[[nodiscard]] Personality& parsePersonality(Personality& personality, Json const& config);
[[nodiscard]] Personality parsePersonality(Json const& config);

[[nodiscard]] Json jsonFromPersonality(Personality const& personality);

struct HumanoidIdentity {
  explicit HumanoidIdentity(Json config = Json());

  [[nodiscard]] Json toJson() const;

  String name;
  // Must have :idle[1-5], :sit, :duck, :walk[1-8], :run[1-8], :jump[1-4], and
  // :fall[1-4]
  String species;
  Gender gender;

  String hairGroup;
  // Must have :normal and :climb
  String hairType;
  Directives hairDirectives;
  Directives bodyDirectives;
  Directives emoteDirectives;
  String facialHairGroup;
  String facialHairType;
  Directives facialHairDirectives;
  String facialMaskGroup;
  String facialMaskType;
  Directives facialMaskDirectives;

  Personality personality;
  Vec4B color;

  Maybe<String> imagePath;
};

DataStream& operator>>(DataStream& ds, HumanoidIdentity& identity);
DataStream& operator<<(DataStream& ds, HumanoidIdentity const& identity);

class Humanoid {
public:
  enum State {
    Idle,    // 1 idle frame
    Walk,    // 8 walking frames
    Run,     // 8 run frames
    Jump,    // 4 jump frames
    Fall,    // 4 fall frames
    Swim,    // 7 swim frames
    SwimIdle,// 2 swim idle frame
    Duck,    // 1 ducking frame
    Sit,     // 1 sitting frame
    Lay,     // 1 laying frame
    STATESIZE
  };
  static EnumMap<State> const StateNames;

  explicit Humanoid(AssetsConstPtr assets = {}, ImageMetadataDatabaseConstPtr imageMetadataDatabase = {}, SpeciesDatabaseConstPtr speciesDatabase = {}, DanceDatabaseConstPtr danceDatabase = {}, ParticleDatabaseConstPtr particleDatabase = {});
  Humanoid(Json const& config, AssetsConstPtr assets = {}, ImageMetadataDatabaseConstPtr imageMetadataDatabase = {}, SpeciesDatabaseConstPtr speciesDatabase = {}, DanceDatabaseConstPtr danceDatabase = {}, ParticleDatabaseConstPtr particleDatabase = {});
  Humanoid(HumanoidIdentity const& identity, JsonObject parameters = JsonObject(), Json config = Json(), AssetsConstPtr assets = {}, ImageMetadataDatabaseConstPtr imageMetadataDatabase = {}, SpeciesDatabaseConstPtr speciesDatabase = {}, DanceDatabaseConstPtr danceDatabase = {}, ParticleDatabaseConstPtr particleDatabase = {});
  Humanoid(Humanoid const&) = default;

  struct HumanoidTiming {
    explicit HumanoidTiming(Json config = Json());
    [[nodiscard]] static HumanoidTiming sensibleDefaults(AssetsConstPtr assets);

    [[nodiscard]] static bool cyclicState(State state);
    [[nodiscard]] static bool cyclicEmoteState(HumanoidEmote state);

    [[nodiscard]] int stateSeq(float timer, State state) const;
    [[nodiscard]] int emoteStateSeq(float timer, HumanoidEmote state) const;
    [[nodiscard]] int danceSeq(float timer, DancePtr dance) const;
    [[nodiscard]] int genericSeq(float timer, float cycle, unsigned frames, bool cyclic) const;

    Array<float, STATESIZE> stateCycle;
    Array<unsigned, STATESIZE> stateFrames;

    Array<float, EmoteSize> emoteCycle;
    Array<unsigned, EmoteSize> emoteFrames;
  };

  void setIdentity(HumanoidIdentity const& identity);
  [[nodiscard]] HumanoidIdentity const& identity() const;

  [[nodiscard]] bool loadConfig(Json merger = JsonObject(), bool forceRefresh = false);
  void loadAnimation();
  void setHumanoidParameters(JsonObject parameters);

  // All of the image identifiers here are meant to be image *base* names, with
  // a collection of frames specific to each piece.  If an image is set to
  // empty string, it is disabled.
  struct WornAny {
    Directives directives;
    String frameset;
    bool rotateWithHead = false;
    bool bypassNude = false;
    bool fullbright = false;
    HashMap<String, String> animationTags = {};
  };

  // Must have :normal, climb
  struct WornHead : WornAny {
    Directives maskDirectives;
  };
  // Will have :run, :normal, and :duck
  struct WornChest : WornAny {
    String frontSleeveFrameset;
    String backSleeveFrameset;
  };
  // Must have :idle, :duck, :walk[1-8], :run[1-8], :jump[1-4], :fall[1-4]
  struct WornLegs : WornAny {};
  // Must have :idle, :duck, :walk[1-8], :run[1-8], :jump[1-4], :fall[1-4]
  struct WornBack : WornAny {};

  using Wearable = MVariant<WornHead, WornChest, WornLegs, WornBack>;

  struct Fashion {
    // 8 vanilla + 12 extra slots
    Array<Wearable, 20> wearables;
    // below 3 are recalculated when rendering updated wearables, null-terminated
    Array<uint8_t, 20> wornHeads;
    // chests and leg layering is interchangeable
    Array<uint8_t, 20> wornChestsLegs;
    Array<uint8_t, 20> wornBacks;
    bool wornHeadsChanged = true;
    bool wornChestsLegsChanged = true;
    bool wornBacksChanged = true;
    DirectivesGroup helmetMaskDirectivesGroup;
    bool helmetMasksChanged = true;
  };

  template <typename T>
  [[nodiscard]] inline T const* getLastWearableOfType() const;

  void removeWearable(uint8_t slot);
  void setWearableFromHead(uint8_t slot, HeadArmor const& head, Gender gender);
  void setWearableFromChest(uint8_t slot, ChestArmor const& chest, Gender gender);
  void setWearableFromLegs(uint8_t slot, LegsArmor const& legs, Gender gender);
  void setWearableFromBack(uint8_t slot, BackArmor const& back, Gender gender);
  void refreshWearables(Fashion& fashion);

  // Legacy getters for all of the above, returns last found
  [[nodiscard]] Directives const& headArmorDirectives() const;
  [[nodiscard]] String const& headArmorFrameset() const;
  [[nodiscard]] Directives const& chestArmorDirectives() const;
  [[nodiscard]] String const& chestArmorFrameset() const;
  [[nodiscard]] String const& backSleeveFrameset() const;
  [[nodiscard]] String const& frontSleeveFrameset() const;
  [[nodiscard]] Directives const& legsArmorDirectives() const;
  [[nodiscard]] String const& legsArmorFrameset() const;
  [[nodiscard]] Directives const& backArmorDirectives() const;
  [[nodiscard]] String const& backArmorFrameset() const;

  void setBodyHidden(bool hidden);

  void setState(State state);
  void setEmoteState(HumanoidEmote state);
  void setDance(Maybe<String> const& dance);
  void setFacingDirection(Direction facingDirection);
  void setMovingBackwards(bool movingBackwards);
  void setHeadRotation(float headRotation);
  void setHeadRotationEnabled(bool enabled);
  void setRotation(float rotation);
  void setScale(Vec2F scale);

  void setVaporTrail(bool enabled);

  [[nodiscard]] State state() const;
  [[nodiscard]] HumanoidEmote emoteState() const;
  [[nodiscard]] Maybe<String> dance() const;
  [[nodiscard]] bool danceCyclicOrEnded() const;
  [[nodiscard]] bool headRotationEnabled() const;
  [[nodiscard]] Direction facingDirection() const;
  [[nodiscard]] bool movingBackwards() const;

  // If not rotating, then the arms follow normal movement animation.  The
  // angle parameter should be in the range [-pi/2, pi/2] (the facing direction
  // should not be included in the angle).
  void setHandParameters(ToolHand hand, bool holdingItem, float angle, float itemAngle, bool twoHanded,
                         bool recoil, bool outsideOfHand);
  void setHandFrameOverrides(ToolHand hand, StringView back, StringView front);
  void setHandDrawables(ToolHand hand, List<Drawable> drawables);
  void setHandNonRotatedDrawables(ToolHand hand, List<Drawable> drawables);
  [[nodiscard]] bool handHoldingItem(ToolHand hand) const;

  // Updates the animation based on whatever the current animation state is,
  // wrapping or clamping animation time as appropriate.
  void animate(float dt, NetworkedAnimator::DynamicTarget* dynamicTarget);

  // Reset animation time to 0.0f
  void resetAnimation();

  // Renders to centered drawables (centered on the normal image center for the
  // player graphics), (in world space, not pixels)
  [[nodiscard]] List<Drawable> render(bool withItems = true, bool withRotationAndScale = true);

  // Renders to centered drawables (centered on the normal image center for the
  // player graphics), (in pixels, not world space)
  [[nodiscard]] List<Drawable> renderPortrait(PortraitMode mode) const;

  [[nodiscard]] List<Drawable> renderSkull() const;

  [[nodiscard]] static HumanoidPtr makeDummy(Gender gender, AssetsConstPtr assets = {}, ImageMetadataDatabaseConstPtr imageMetadataDatabase = {});
  // Renders to centered drawables (centered on the normal image center for the
  // player graphics), (in pixels, not world space)
  List<Drawable> renderDummy(Gender gender, HeadArmor const* head = {}, ChestArmor const* chest = {},
                             LegsArmor const* legs = {}, BackArmor const* back = {});

  [[nodiscard]] Vec2F primaryHandPosition(Vec2F const& offset) const;
  [[nodiscard]] Vec2F altHandPosition(Vec2F const& offset) const;

  // Finds the arm position in world space if the humanoid was facing the given
  // direction and applying the given arm angle.  The offset given is from the
  // rotation center of the arm.
  [[nodiscard]] Vec2F primaryArmPosition(Direction facingDirection, float armAngle, Vec2F const& offset) const;
  [[nodiscard]] Vec2F altArmPosition(Direction facingDirection, float armAngle, Vec2F const& offset) const;

  // Gives the offset of the hand from the arm rotation center
  [[nodiscard]] Vec2F primaryHandOffset(Direction facingDirection) const;
  [[nodiscard]] Vec2F altHandOffset(Direction facingDirection) const;

  [[nodiscard]] Vec2F armAdjustment() const;

  [[nodiscard]] Vec2F mouthOffset(bool ignoreAdjustments = false) const;
  [[nodiscard]] float getBobYOffset() const;
  [[nodiscard]] Vec2F feetOffset() const;

  [[nodiscard]] Vec2F headArmorOffset() const;
  [[nodiscard]] Vec2F chestArmorOffset() const;
  [[nodiscard]] Vec2F legsArmorOffset() const;
  [[nodiscard]] Vec2F backArmorOffset() const;

  [[nodiscard]] String defaultDeathParticles() const;
  [[nodiscard]] List<Particle> particles(String const& name) const;

  [[nodiscard]] Json const& defaultMovementParameters() const;
  [[nodiscard]] Maybe<Json> const& playerMovementParameters() const;

  [[nodiscard]] String getHeadFromIdentity() const;
  [[nodiscard]] String getBodyFromIdentity() const;
  [[nodiscard]] String getBodyMaskFromIdentity() const;
  [[nodiscard]] String getBodyHeadMaskFromIdentity() const;
  [[nodiscard]] String getFacialEmotesFromIdentity() const;
  [[nodiscard]] String getHairFromIdentity() const;
  [[nodiscard]] String getFacialHairFromIdentity() const;
  [[nodiscard]] String getFacialMaskFromIdentity() const;
  [[nodiscard]] String getBackArmFromIdentity() const;
  [[nodiscard]] String getFrontArmFromIdentity() const;
  [[nodiscard]] String getVaporTrailFrameset() const;

  [[nodiscard]] NetworkedAnimator* networkedAnimator();
  [[nodiscard]] NetworkedAnimator const* networkedAnimator() const;
  [[nodiscard]] List<String> animationScripts() const;

  [[nodiscard]] Json humanoidConfig(bool withOverrides = true);

  // Extracts scalenearest from directives and returns the combined scale and
  // a new Directives without those scalenearest directives.
  [[nodiscard]] static pair<Vec2F, Directives> extractScaleFromDirectives(Directives const& directives);

private:
  struct HandDrawingInfo {
    List<Drawable> itemDrawables;
    List<Drawable> nonRotatedDrawables;
    bool holdingItem = false;
    float angle = 0.0f;
    float itemAngle = 0.0f;
    String backFrame;
    String frontFrame;
    Directives backDirectives;
    Directives frontDirectives;
    float frameAngleAdjust = 0.0f;
    bool recoil = false;
    bool outsideOfHand = false;
  };

  [[nodiscard]] HandDrawingInfo const& getHand(ToolHand hand) const;
  [[nodiscard]] HandDrawingInfo& getHand(ToolHand hand);

  void wearableRemoved(Wearable const& wearable);

  [[nodiscard]] String frameBase(State state) const;
  [[nodiscard]] String emoteFrameBase(HumanoidEmote state) const;

  [[nodiscard]] Directives const& getBodyDirectives() const;
  [[nodiscard]] Directives const& getHairDirectives() const;
  [[nodiscard]] Directives const& getEmoteDirectives() const;
  [[nodiscard]] Directives const& getFacialHairDirectives() const;
  [[nodiscard]] Directives const& getFacialMaskDirectives() const;
  [[nodiscard]] DirectivesGroup const& getHelmetMaskDirectivesGroup() const;

  [[nodiscard]] int getEmoteStateSequence() const;
  [[nodiscard]] int getArmStateSequence() const;
  [[nodiscard]] int getBodyStateSequence() const;

  [[nodiscard]] Maybe<DancePtr> getDance() const;

  void refreshAnimationState(bool startNew = false);

  Json m_baseConfig;
  Json m_mergeConfig;
  AssetsConstPtr m_assets;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  DanceDatabaseConstPtr m_danceDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;

  Vec2F m_globalOffset;
  Vec2F m_headRunOffset;
  Vec2F m_headSwimOffset;
  Vec2F m_headDuckOffset;
  Vec2F m_headSitOffset;
  Vec2F m_headLayOffset;
  float m_runFallOffset;
  float m_duckOffset;
  float m_sitOffset;
  float m_layOffset;
  Vec2F m_recoilOffset;
  Vec2F m_mouthOffset;
  Vec2F m_feetOffset;

  Vec2F m_headArmorOffset;
  Vec2F m_chestArmorOffset;
  Vec2F m_legsArmorOffset;
  Vec2F m_backArmorOffset;

  bool m_useBodyMask;
  bool m_useBodyHeadMask;

  bool m_bodyHidden;

  List<int> m_armWalkSeq;
  List<int> m_armRunSeq;
  List<float> m_walkBob;
  List<float> m_runBob;
  List<float> m_swimBob;
  float m_jumpBob;
  Vec2F m_frontArmRotationCenter;
  Vec2F m_backArmRotationCenter;
  Vec2F m_frontHandPosition;
  Vec2F m_backArmOffset;

  Vec2F m_headRotationCenter;

  String m_headFrameset;
  String m_bodyFrameset;
  String m_bodyMaskFrameset;
  String m_bodyHeadMaskFrameset;
  String m_backArmFrameset;
  String m_frontArmFrameset;
  String m_emoteFrameset;
  String m_hairFrameset;
  String m_facialHairFrameset;
  String m_facialMaskFrameset;

  bool m_bodyFullbright;

  String m_vaporTrailFrameset;
  unsigned m_vaporTrailFrames;
  float m_vaporTrailCycle;

  std::shared_ptr<Fashion> m_fashion;

  State m_state;
  HumanoidEmote m_emoteState;
  Maybe<String> m_dance;
  Direction m_facingDirection;
  bool m_movingBackwards;
  float m_headRotation;
  float m_headRotationTarget;
  bool m_headRotationEnabled;
  float m_rotation;
  Vec2F m_scale;
  bool m_drawVaporTrail;

  HandDrawingInfo m_primaryHand;
  HandDrawingInfo m_altHand;

  bool m_twoHanded;

  HumanoidIdentity m_identity;
  HumanoidTiming m_timing;

  float m_animationTimer;
  float m_emoteAnimationTimer;
  float m_danceTimer;

  Json m_particleEmitters;
  String m_defaultDeathParticles;

  Json m_defaultMovementParameters;
  Maybe<Json> m_playerMovementParameters;
  bool m_useAnimation;

  NetworkedAnimator m_networkedAnimator;

  List<String> m_animationScripts;

  struct AnimationStateArgs {
    String state;
    bool startNew;
    bool reverse;
  };
  HashMap<Humanoid::State, HashMap<String, AnimationStateArgs>> m_animationStates;
  HashMap<Humanoid::State, HashMap<String, AnimationStateArgs>> m_animationStatesBackwards;
  HashMap<HumanoidEmote, HashMap<String, AnimationStateArgs>> m_emoteAnimationStates;
  HashMap<PortraitMode, HashMap<String, AnimationStateArgs>> m_portraitAnimationStates;

  HashMap<String, String> m_identityFramesetTags;

  struct PartPointReference {
    String partName;
    String pointName;
  };

  PartPointReference m_headRotationPoint;
  PartPointReference m_frontArmRotationPoint;
  PartPointReference m_backArmRotationPoint;

  String m_frontItemPart;
  String m_backItemPart;

  PartPointReference m_mouthOffsetPoint;
  PartPointReference m_headArmorOffsetPoint;
  PartPointReference m_chestArmorOffsetPoint;
  PartPointReference m_legsArmorOffsetPoint;
  PartPointReference m_backArmorOffsetPoint;
  PartPointReference m_feetOffsetPoint;
  PartPointReference m_throwPoint;
  PartPointReference m_interactPoint;
};

// this is because species can be changed on the fly and therefore the humanoid needs to re-initialize as the new species when it changes
// therefore we need to have these in a dynamic group in players and NPCs for the sake of the networked animator not breaking the game
class NetHumanoid : public NetElementSyncGroup {
public:
  NetHumanoid(HumanoidIdentity identity = HumanoidIdentity(), JsonObject parameters = JsonObject(), Json config = Json(), AssetsConstPtr assets = {}, ImageMetadataDatabaseConstPtr imageMetadataDatabase = {}, SpeciesDatabaseConstPtr speciesDatabase = {}, DanceDatabaseConstPtr danceDatabase = {}, ParticleDatabaseConstPtr particleDatabase = {});

  void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
  void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

  [[nodiscard]] HumanoidPtr humanoid();
  void setHumanoidParameters(JsonObject parameters);
  [[nodiscard]] JsonObject humanoidParameters();

private:
  void setupNetElements();

  Json m_config;
  AssetsConstPtr m_assets;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  DanceDatabaseConstPtr m_danceDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;
  NetElementHashMap<String, Json> m_humanoidParameters;
  HumanoidPtr m_humanoid;
};

template <typename T>
inline T const* Humanoid::getLastWearableOfType() const {
  for (size_t i = m_fashion->wearables.size(); i != 0; --i) {
    if (auto ptr = m_fashion->wearables[i - 1].ptr<T>())
      return ptr;
  }
  return nullptr;
}

}// namespace Star
