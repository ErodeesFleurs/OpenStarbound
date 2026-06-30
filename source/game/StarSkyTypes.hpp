#pragma once

#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarJson.hpp"

namespace Star {

struct SkyExceptionTag { static constexpr char const* typeName = "SkyException"; };
using SkyException = TypedException<StarException, SkyExceptionTag>;

enum class SkyType : uint8_t {
  Barren,
  Atmospheric,
  Atmosphereless,
  Orbital,
  Warp,
  Space
};
extern EnumMap<SkyType> const SkyTypeNames;

enum class FlyingType : uint8_t {
  None,
  Disembarking,
  Warp,
  Arriving
};
extern EnumMap<FlyingType> const FlyingTypeNames;

enum class WarpPhase : int8_t {
  SlowingDown = -1,
  Maintain = 0,
  SpeedingUp = 1
};
extern EnumMap<WarpPhase> const WarpPhaseNames;

struct SkyColoring {
  SkyColoring() = default;
  explicit SkyColoring(Json const& variant);

  Json toJson() const;

  Color mainColor = Color::Clear;

  pair<Color, Color> morningColors = {Color::Clear, Color::Clear};
  pair<Color, Color> dayColors = {Color::Clear, Color::Clear};
  pair<Color, Color> eveningColors = {Color::Clear, Color::Clear};
  pair<Color, Color> nightColors = {Color::Clear, Color::Clear};

  Color morningLightColor = Color::Clear;
  Color dayLightColor = Color::Clear;
  Color eveningLightColor = Color::Clear;
  Color nightLightColor = Color::Clear;
};

DataStream& operator>>(DataStream& ds, SkyColoring& skyColoring);
DataStream& operator<<(DataStream& ds, SkyColoring const& skyColoring);

enum class SkyOrbiterType { Sun, Moon, HorizonCloud, SpaceDebris };

struct SkyOrbiter {
  SkyOrbiter() = default;
  SkyOrbiter(SkyOrbiterType type, float scale, float angle, String const& image, Vec2F position);

  SkyOrbiterType type = SkyOrbiterType::Sun;
  float scale = 0.0f;
  float angle = 0.0f;
  String image;
  Vec2F position;
};

struct SkyWorldHorizon {
  SkyWorldHorizon() = default;
  SkyWorldHorizon(Vec2F center, float scale, float rotation);

  bool empty() const;

  Vec2F center;

  float scale = 0.0f;
  float rotation = 0.0f;

  // List of L/R images for each layer of the world horizon, bottom to top.
  List<pair<String, String>> layers;
};

}
