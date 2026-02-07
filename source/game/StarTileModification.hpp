#pragma once

#include "StarCollisionBlock.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarLiquidTypes.hpp"
#include "StarMaterialTypes.hpp"
#include "StarVariant.hpp"

import std;

namespace Star {

struct PlaceMaterial {
  TileLayer layer;
  MaterialId material;
  // If the material hue shift is not set it will get the natural hue shift for
  // the environment.
  std::optional<MaterialHue> materialHueShift;
  TileCollisionOverride collisionOverride = TileCollisionOverride::None;
};
auto operator>>(DataStream& ds, PlaceMaterial& tileMaterialPlacement) -> DataStream&;
auto operator<<(DataStream& ds, PlaceMaterial const& tileMaterialPlacement) -> DataStream&;

struct PlaceMod {
  TileLayer layer;
  ModId mod;

  // If the mod hue shift is not set it will get the natural hue shift for the
  // environment.
  std::optional<MaterialHue> modHueShift;
};
auto operator>>(DataStream& ds, PlaceMod& tileModPlacement) -> DataStream&;
auto operator<<(DataStream& ds, PlaceMod const& tileModPlacement) -> DataStream&;

struct PlaceMaterialColor {
  TileLayer layer;
  MaterialColorVariant color;
};
auto operator>>(DataStream& ds, PlaceMaterialColor& tileMaterialColorPlacement) -> DataStream&;
auto operator<<(DataStream& ds, PlaceMaterialColor const& tileMaterialColorPlacement) -> DataStream&;

struct PlaceLiquid {
  LiquidId liquid;
  float liquidLevel;
};
auto operator>>(DataStream& ds, PlaceLiquid& tileLiquidPlacement) -> DataStream&;
auto operator<<(DataStream& ds, PlaceLiquid const& tileLiquidPlacement) -> DataStream&;

using TileModification = MVariant<PlaceMaterial, PlaceMod, PlaceMaterialColor, PlaceLiquid>;
using TileModificationList = List<std::pair<Vec2I, TileModification>>;

}// namespace Star
