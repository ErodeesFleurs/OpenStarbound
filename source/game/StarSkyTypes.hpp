#pragma once

#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

using SkyException = ExceptionDerived<"SkyException">;

enum class SkyType : std::uint8_t {
  Barren,
  Atmospheric,
  Atmosphereless,
  Orbital,
  Warp,
  Space
};
extern EnumMap<SkyType> const SkyTypeNames;

enum class FlyingType : std::uint8_t {
  None,
  Disembarking,
  Warp,
  Arriving
};
extern EnumMap<FlyingType> const FlyingTypeNames;

enum class WarpPhase : std::int8_t {
  SlowingDown = -1,
  Maintain = 0,
  SpeedingUp = 1
};
extern EnumMap<WarpPhase> const WarpPhaseNames;

struct SkyColoring {
  SkyColoring();
  explicit SkyColoring(Json const& variant);

  [[nodiscard]] auto toJson() const -> Json;

  Color mainColor;

  std::pair<Color, Color> morningColors;
  std::pair<Color, Color> dayColors;
  std::pair<Color, Color> eveningColors;
  std::pair<Color, Color> nightColors;

  Color morningLightColor;
  Color dayLightColor;
  Color eveningLightColor;
  Color nightLightColor;
};

auto operator>>(DataStream& ds, SkyColoring& skyColoring) -> DataStream&;
auto operator<<(DataStream& ds, SkyColoring const& skyColoring) -> DataStream&;

enum class SkyOrbiterType { Sun, Moon, HorizonCloud, SpaceDebris };

struct SkyOrbiter {
  SkyOrbiter();
  SkyOrbiter(SkyOrbiterType type, float scale, float angle, String const& image, Vec2F position);

  SkyOrbiterType type;
  float scale;
  float angle;
  String image;
  Vec2F position;
};

struct SkyWorldHorizon {
  SkyWorldHorizon();
  SkyWorldHorizon(Vec2F center, float scale, float rotation);

  [[nodiscard]] auto empty() const -> bool;

  Vec2F center;

  float scale;
  float rotation;

  // List of L/R images for each layer of the world horizon, bottom to top.
  List<std::pair<String, String>> layers;
};

}
