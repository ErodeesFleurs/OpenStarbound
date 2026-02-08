#pragma once

#include "StarConfig.hpp"
#include "StarDataStream.hpp"
#include "StarDrawable.hpp"
#include "StarGameTypes.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarParticle.hpp"

import std;

namespace Star {

// Required for renderDummy
class HeadArmor;
class ChestArmor;
class LegsArmor;
class BackArmor;

struct Dance;

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
std::size_t const EmoteSize = 14;

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

auto parsePersonalityArray(Json const& config) -> Personality;

auto parsePersonality(Personality& personality, Json const& config) -> Personality&;
auto parsePersonality(Json const& config) -> Personality;

auto jsonFromPersonality(Personality const& personality) -> Json;

struct HumanoidIdentity {
  explicit HumanoidIdentity(Json config = Json());

  [[nodiscard]] auto toJson() const -> Json;

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

  std::optional<String> imagePath;
};

auto operator>>(DataStream& ds, HumanoidIdentity& identity) -> DataStream&;
auto operator<<(DataStream& ds, HumanoidIdentity const& identity) -> DataStream&;

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

  static auto globalHeadRotation() -> bool&;

  Humanoid();
  Humanoid(Json const& config);
  Humanoid(HumanoidIdentity const& identity, JsonObject parameters = JsonObject(), Json config = Json());
  Humanoid(Humanoid const&) = default;

  struct HumanoidTiming {
    explicit HumanoidTiming(Json config = Json());

    static auto cyclicState(State state) -> bool;
    static auto cyclicEmoteState(HumanoidEmote state) -> bool;

    [[nodiscard]] auto stateSeq(float timer, State state) const -> int;
    [[nodiscard]] auto emoteStateSeq(float timer, HumanoidEmote state) const -> int;
    [[nodiscard]] auto danceSeq(float timer, Ptr<Dance> dance) const -> int;
    [[nodiscard]] auto genericSeq(float timer, float cycle, unsigned frames, bool cyclic) const -> int;

    Array<float, STATESIZE> stateCycle;
    Array<unsigned, STATESIZE> stateFrames;

    Array<float, EmoteSize> emoteCycle;
    Array<unsigned, EmoteSize> emoteFrames;
  };

  void setIdentity(HumanoidIdentity const& identity);
  auto identity() const -> HumanoidIdentity const&;

  auto loadConfig(Json merger = JsonObject(), bool forceRefresh = false) -> bool;
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
    Array<std::uint8_t, 20> wornHeads;
    // chests and leg layering is interchangeable
    Array<std::uint8_t, 20> wornChestsLegs;
    Array<std::uint8_t, 20> wornBacks;
    bool wornHeadsChanged = true;
    bool wornChestsLegsChanged = true;
    bool wornBacksChanged = true;
    DirectivesGroup helmetMaskDirectivesGroup;
    bool helmetMasksChanged = true;
  };

  template <typename T>
  inline auto getLastWearableOfType() const -> T const*;

  void removeWearable(std::uint8_t slot);
  void setWearableFromHead(std::uint8_t slot, HeadArmor const& head, Gender gender);
  void setWearableFromChest(std::uint8_t slot, ChestArmor const& chest, Gender gender);
  void setWearableFromLegs(std::uint8_t slot, LegsArmor const& legs, Gender gender);
  void setWearableFromBack(std::uint8_t slot, BackArmor const& back, Gender gender);
  void refreshWearables(Fashion& fashion);

  // Legacy getters for all of the above, returns last found
  auto headArmorDirectives() const -> Directives const&;
  auto headArmorFrameset() const -> String const&;
  auto chestArmorDirectives() const -> Directives const&;
  auto chestArmorFrameset() const -> String const&;
  auto backSleeveFrameset() const -> String const&;
  auto frontSleeveFrameset() const -> String const&;
  auto legsArmorDirectives() const -> Directives const&;
  auto legsArmorFrameset() const -> String const&;
  auto backArmorDirectives() const -> Directives const&;
  auto backArmorFrameset() const -> String const&;

  void setBodyHidden(bool hidden);

  void setState(State state);
  void setEmoteState(HumanoidEmote state);
  void setDance(std::optional<String> const& dance);
  void setFacingDirection(Direction facingDirection);
  void setMovingBackwards(bool movingBackwards);
  void setHeadRotation(float headRotation);
  void setRotation(float rotation);
  void setScale(Vec2F scale);

  void setVaporTrail(bool enabled);

  auto state() const -> State;
  auto emoteState() const -> HumanoidEmote;
  auto dance() const -> std::optional<String>;
  auto danceCyclicOrEnded() const -> bool;
  auto facingDirection() const -> Direction;
  auto movingBackwards() const -> bool;

  // If not rotating, then the arms follow normal movement animation.  The
  // angle parameter should be in the range [-pi/2, pi/2] (the facing direction
  // should not be included in the angle).
  void setHandParameters(ToolHand hand, bool holdingItem, float angle, float itemAngle, bool twoHanded,
                         bool recoil, bool outsideOfHand);
  void setHandFrameOverrides(ToolHand hand, StringView back, StringView front);
  void setHandDrawables(ToolHand hand, List<Drawable> drawables);
  void setHandNonRotatedDrawables(ToolHand hand, List<Drawable> drawables);
  auto handHoldingItem(ToolHand hand) const -> bool;

  // Updates the animation based on whatever the current animation state is,
  // wrapping or clamping animation time as appropriate.
  void animate(float dt, NetworkedAnimator::DynamicTarget* dynamicTarget);

  // Reset animation time to 0.0f
  void resetAnimation();

  // Renders to centered drawables (centered on the normal image center for the
  // player graphics), (in world space, not pixels)
  auto render(bool withItems = true, bool withRotationAndScale = true) -> List<Drawable>;

  // Renders to centered drawables (centered on the normal image center for the
  // player graphics), (in pixels, not world space)
  auto renderPortrait(PortraitMode mode) const -> List<Drawable>;

  auto renderSkull() const -> List<Drawable>;

  static auto makeDummy(Gender gender) -> Ptr<Humanoid>;
  // Renders to centered drawables (centered on the normal image center for the
  // player graphics), (in pixels, not world space)
  auto renderDummy(Gender gender, HeadArmor const* head = {}, ChestArmor const* chest = {},
                   LegsArmor const* legs = {}, BackArmor const* back = {}) -> List<Drawable>;

  auto primaryHandPosition(Vec2F const& offset) const -> Vec2F;
  auto altHandPosition(Vec2F const& offset) const -> Vec2F;

  // Finds the arm position in world space if the humanoid was facing the given
  // direction and applying the given arm angle.  The offset given is from the
  // rotation center of the arm.
  auto primaryArmPosition(Direction facingDirection, float armAngle, Vec2F const& offset) const -> Vec2F;
  auto altArmPosition(Direction facingDirection, float armAngle, Vec2F const& offset) const -> Vec2F;

  // Gives the offset of the hand from the arm rotation center
  auto primaryHandOffset(Direction facingDirection) const -> Vec2F;
  auto altHandOffset(Direction facingDirection) const -> Vec2F;

  auto armAdjustment() const -> Vec2F;

  auto mouthOffset(bool ignoreAdjustments = false) const -> Vec2F;
  auto getBobYOffset() const -> float;
  auto feetOffset() const -> Vec2F;

  auto headArmorOffset() const -> Vec2F;
  auto chestArmorOffset() const -> Vec2F;
  auto legsArmorOffset() const -> Vec2F;
  auto backArmorOffset() const -> Vec2F;

  auto defaultDeathParticles() const -> String;
  auto particles(String const& name) const -> List<Particle>;

  auto defaultMovementParameters() const -> Json const&;
  auto playerMovementParameters() const -> std::optional<Json> const&;

  auto getHeadFromIdentity() const -> String;
  auto getBodyFromIdentity() const -> String;
  auto getBodyMaskFromIdentity() const -> String;
  auto getBodyHeadMaskFromIdentity() const -> String;
  auto getFacialEmotesFromIdentity() const -> String;
  auto getHairFromIdentity() const -> String;
  auto getFacialHairFromIdentity() const -> String;
  auto getFacialMaskFromIdentity() const -> String;
  auto getBackArmFromIdentity() const -> String;
  auto getFrontArmFromIdentity() const -> String;
  auto getVaporTrailFrameset() const -> String;

  auto networkedAnimator() -> NetworkedAnimator*;
  auto networkedAnimator() const -> NetworkedAnimator const*;
  auto animationScripts() const -> List<String>;

  auto humanoidConfig(bool withOverrides = true) -> Json;

  // Extracts scalenearest from directives and returns the combined scale and
  // a new Directives without those scalenearest directives.
  static auto extractScaleFromDirectives(Directives const& directives) -> std::pair<Vec2F, Directives>;

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

  auto getHand(ToolHand hand) const -> HandDrawingInfo const&;

  void wearableRemoved(Wearable const& wearable);

  auto frameBase(State state) const -> String;
  auto emoteFrameBase(HumanoidEmote state) const -> String;

  auto getBodyDirectives() const -> Directives const&;
  auto getHairDirectives() const -> Directives const&;
  auto getEmoteDirectives() const -> Directives const&;
  auto getFacialHairDirectives() const -> Directives const&;
  auto getFacialMaskDirectives() const -> Directives const&;
  auto getHelmetMaskDirectivesGroup() const -> DirectivesGroup const&;

  auto getEmoteStateSequence() const -> int;
  auto getArmStateSequence() const -> int;
  auto getBodyStateSequence() const -> int;

  auto getDance() const -> std::optional<Ptr<Dance>>;

  void refreshAnimationState(bool startNew = false);

  Json m_baseConfig;
  Json m_mergeConfig;

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
  std::optional<String> m_dance;
  Direction m_facingDirection;
  bool m_movingBackwards;
  float m_headRotation;
  float m_headRotationTarget;
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
  std::optional<Json> m_playerMovementParameters;
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

  std::pair<String, String> m_headRotationPoint;
  std::pair<String, String> m_frontArmRotationPoint;
  std::pair<String, String> m_backArmRotationPoint;

  String m_frontItemPart;
  String m_backItemPart;

  std::pair<String, String> m_mouthOffsetPoint;
  std::pair<String, String> m_headArmorOffsetPoint;
  std::pair<String, String> m_chestArmorOffsetPoint;
  std::pair<String, String> m_legsArmorOffsetPoint;
  std::pair<String, String> m_backArmorOffsetPoint;
  std::pair<String, String> m_feetOffsetPoint;
  std::pair<String, String> m_throwPoint;
  std::pair<String, String> m_interactPoint;
};

// this is because species can be changed on the fly and therefore the humanoid needs to re-initialize as the new species when it changes
// therefore we need to have these in a dynamic group in players and NPCs for the sake of the networked animator not breaking the game
class NetHumanoid : public NetElementSyncGroup {
public:
  NetHumanoid(HumanoidIdentity identity = HumanoidIdentity(), JsonObject parameters = JsonObject(), Json config = Json());

  void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
  void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

  auto humanoid() -> Ptr<Humanoid>;
  void setHumanoidParameters(JsonObject parameters);
  auto humanoidParameters() -> JsonObject;

private:
  void setupNetElements();

  Json m_config;
  NetElementHashMap<String, Json> m_humanoidParameters;
  Ptr<Humanoid> m_humanoid;
};

template <typename T>
inline auto Humanoid::getLastWearableOfType() const -> T const* {
  for (std::size_t i = m_fashion->wearables.size(); i != 0; --i) {
    if (auto ptr = m_fashion->wearables[i - 1].ptr<T>())
      return ptr;
  }
  return nullptr;
}

}// namespace Star
