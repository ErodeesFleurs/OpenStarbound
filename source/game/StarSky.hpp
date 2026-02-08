#pragma once

#include "StarConfig.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSystem.hpp"
#include "StarSkyParameters.hpp"
#include "StarSkyRenderData.hpp"

import std;

namespace Star {

class Clock;
class AudioInstance;

// STAR_CLASS(Sky);

// Sky objects, such as stars and orbiters, are given in a pseudo screen space,
// "view space", that does not take the pixel ratio into account.  "viewSize"
// is the size of this space,  expected to be the size of the screen *after*
// dividing by the pixel ratio.
class Sky {
public:
  Sky();
  Sky(SkyParameters const& skyParameters, bool inOrbit);

  // Controls the space sky "flight" system
  void startFlying(bool enterHyperspace, bool startInWarp, Json settings = {});
  // Stops flying animation copying the new pertinant sky data from the given
  // sky, as though the sky as moved to a new world.
  void stopFlyingAt(std::optional<SkyParameters> SkyParameters);

  void jumpTo(SkyParameters SkyParameters);

  auto writeUpdate(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t>;
  void readUpdate(ByteArray data, NetCompatibilityRules rules = {});

  // handles flying and warp state transitions
  void stateUpdate();
  void update(double dt);

  void setType(SkyType type);
  auto type() const -> SkyType;

  auto inSpace() const -> bool;

  auto seed() const -> std::uint64_t;

  auto dayLength() const -> float;
  auto day() const -> std::uint32_t;
  auto timeOfDay() const -> float;

  // Total time since the 0th day for this world.
  auto epochTime() const -> double;
  void setEpochTime(double epochTime);

  // Altitude is used to determine, in Atmospheric skies, the percentage of the
  // atmosphere to draw and how much like space it should appear.
  auto altitude() const -> float;
  void setAltitude(float altitude);

  // If a reference clock is set, then the epoch time is driven by the
  // reference clock rather than an internal timer
  void setReferenceClock(ConstPtr<Clock> const& referenceClock);
  auto referenceClock() const -> ConstPtr<Clock>;

  auto ambientNoise() const -> String;
  auto pullSounds() -> List<Ptr<AudioInstance>>;

  // How close is the atmosphere to space?
  auto spaceLevel() const -> float;

  auto orbitAngle() const -> float;
  auto isDayTime() const -> bool;

  // Ranges from 0.0 to 1.0  Blended periodic curve with a period of
  // clock.dayLength, and the blend region size is determined by
  // the variant asset "dayTransitionTime"
  auto dayLevel() const -> float;

  // Returns a value that cycles through the range [0.0, 4.0).  0.0 / 4.0 is
  // mid-morning, 1.0 is mid-day, 2.0 is mid-evening, and 3.0 is mid-night.
  // Does not cycle through evenly, the value will "stick" to mid-day and
  // mid-night based on the value of the variant asset "dayTransitionTime"
  auto dayCycle() const -> float;

  auto skyAlpha() const -> float;

  auto environmentLight() const -> Color;
  auto mainSkyColor() const -> Color;

  // Base sky rect colors, top and bottom, includes calculation based on day /
  // night alpha
  auto skyRectColors() const -> std::pair<Color, Color>;
  auto skyFlashColor() const -> Color;

  auto flying() const -> bool;
  auto flyingType() const -> FlyingType;
  auto warpProgress() const -> float;
  auto warpPhase() const -> WarpPhase;
  auto inHyperspace() const -> bool;

  auto renderData() const -> SkyRenderData;

private:
  // TODO: This needs to be more explicit/handled better
  static constexpr float DefaultDayLength = 1000.0f;

  void writeNetStates();
  void readNetStates();

  void enterHyperspace();
  void exitHyperspace();
  auto controlledMovement(JsonArray const& path, Json const& origin, float timeOffset) -> bool;
  auto getStarOffset() const -> Vec2F;
  auto getStarRotation() const -> float;
  auto getWorldOffset() const -> Vec2F;
  auto getWorldRotation() const -> float;
  auto speedupTime() const -> float;
  auto slowdownTime() const -> float;

  void skyParametersUpdated();

  Json m_settings;
  SkyParameters m_skyParameters;
  bool m_skyParametersUpdated;

  SkyType m_skyType = SkyType::Orbital;

  double m_time = 0.0;

  ConstPtr<Clock> m_referenceClock;
  std::optional<double> m_clockTrackingTime;

  float m_altitude = 0.0f;

  FlyingType m_flyingType = FlyingType::None;
  FlyingType m_lastFlyingType = FlyingType::None;
  double m_flyingTimer = 0.0;

  bool m_enterHyperspace = false;
  bool m_startInWarp = false;

  WarpPhase m_warpPhase = WarpPhase::Maintain;
  WarpPhase m_lastWarpPhase = WarpPhase::Maintain;

  double m_flashTimer = 0.0;

  // The star and world offsets and rotations must be different for two
  // reasons: #1, the stars rotate over time, meaning that if they're not
  // different then the world will fly off in a random direction when we leave
  // #2, the stars move at a different, slower rate, controlled by JSON
  // "starVelocityFactor", because they're farther away
  Vec2F m_starOffset;
  double m_starRotation = 0.0;
  Vec2F m_starMoveOffset;

  Vec2F m_worldOffset;
  float m_worldRotation = 0.0f;
  Vec2F m_worldMoveOffset;

  // Finally, these are the offsets for the disembark and arrival paths they
  // are applied to BOTH world and star offsets governed by the
  // starVelocityFactor in the latter case

  Vec2F m_pathOffset;
  float m_pathRotation = 0.0f;

  std::size_t m_starFrames = 0;
  StringList m_starList;
  StringList m_hyperStarList;

  bool m_sentSFX = false;

  std::optional<SkyParameters> m_destWorld;

  bool m_netInit;
  NetElementTopGroup m_netGroup;

  NetElementBytes m_skyParametersNetState;
  NetElementInt m_skyTypeNetState;
  NetElementDouble m_timeNetState;
  NetElementUInt m_flyingTypeNetState;
  NetElementBool m_enterHyperspaceNetState;
  NetElementBool m_startInWarpNetState;
  NetElementInt m_warpPhaseNetState;
  NetElementData<Vec2F> m_worldMoveNetState;
  NetElementData<Vec2F> m_starMoveNetState;
  NetElementFloat m_flyingTimerNetState;
};

}// namespace Star
