module;

#include "StarDataStream.hpp"
#include "StarVariant.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
import star.collision_block;

namespace Star {

struct PlaceMaterial {
  friend DataStream& operator>>(DataStream& ds, PlaceMaterial& tileMaterialPlacement);
  friend DataStream& operator<<(DataStream& ds, PlaceMaterial const& tileMaterialPlacement);
  TileLayer layer;
  MaterialId material;
  // If the material hue shift is not set it will get the natural hue shift for
  // the environment.
  Maybe<MaterialHue> materialHueShift;
  TileCollisionOverride collisionOverride = TileCollisionOverride::None;
};
DataStream& operator>>(DataStream& ds, PlaceMaterial& tileMaterialPlacement);
DataStream& operator<<(DataStream& ds, PlaceMaterial const& tileMaterialPlacement);

struct PlaceMod {
  friend DataStream& operator>>(DataStream& ds, PlaceMod& tileModPlacement);
  friend DataStream& operator<<(DataStream& ds, PlaceMod const& tileModPlacement);
  TileLayer layer;
  ModId mod;

  // If the mod hue shift is not set it will get the natural hue shift for the
  // environment.
  Maybe<MaterialHue> modHueShift;
};
DataStream& operator>>(DataStream& ds, PlaceMod& tileModPlacement);
DataStream& operator<<(DataStream& ds, PlaceMod const& tileModPlacement);

struct PlaceMaterialColor {
  friend DataStream& operator>>(DataStream& ds, PlaceMaterialColor& tileMaterialColorPlacement);
  friend DataStream& operator<<(DataStream& ds, PlaceMaterialColor const& tileMaterialColorPlacement);
  TileLayer layer;
  MaterialColorVariant color;
};
DataStream& operator>>(DataStream& ds, PlaceMaterialColor& tileMaterialColorPlacement);
DataStream& operator<<(DataStream& ds, PlaceMaterialColor const& tileMaterialColorPlacement);

struct PlaceLiquid {
  friend DataStream& operator>>(DataStream& ds, PlaceLiquid& tileLiquidPlacement);
  friend DataStream& operator<<(DataStream& ds, PlaceLiquid const& tileLiquidPlacement);
  LiquidId liquid;
  float liquidLevel;
};
DataStream& operator>>(DataStream& ds, PlaceLiquid& tileLiquidPlacement);
DataStream& operator<<(DataStream& ds, PlaceLiquid const& tileLiquidPlacement);

typedef MVariant<PlaceMaterial, PlaceMod, PlaceMaterialColor, PlaceLiquid> TileModification;
typedef List<pair<Vec2I, TileModification>> TileModificationList;

}

export module star.tile_modification;

export namespace Star {
  using ::Star::PlaceMaterial;
  using ::Star::PlaceMod;
  using ::Star::PlaceMaterialColor;
  using ::Star::PlaceLiquid;
  using ::Star::TileModification;
  using ::Star::TileModificationList;
}
