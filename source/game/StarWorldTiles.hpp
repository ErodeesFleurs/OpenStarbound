#pragma once

#include "StarCollisionGenerator.hpp"
#include "StarGameTypes.hpp"
#include "StarLiquidTypes.hpp"
#include "StarMaterialTypes.hpp"
#include "StarTileDamage.hpp"
#include "StarTileSectorArray.hpp"
#include "StarVersion.hpp"
#include "StarWorldLayout.hpp"

import std;

namespace Star {

struct WorldTile {
  WorldTile();

  // Copy constructor and operator= do not preserve collision cache.
  WorldTile(WorldTile const& worldTile);
  auto operator=(WorldTile const& worldTile) -> WorldTile&;

  [[nodiscard]] auto material(TileLayer layer) const -> MaterialId;
  [[nodiscard]] auto mod(TileLayer layer) const -> ModId;
  [[nodiscard]] auto materialColor(TileLayer layer) const -> MaterialColorVariant;
  [[nodiscard]] auto getCollision() const -> CollisionKind;
  [[nodiscard]] auto materialAndColor(TileLayer layer) const -> std::tuple<MaterialId, MaterialHue, MaterialColorVariant>;
  [[nodiscard]] auto isConnectable(TileLayer layer, bool materialOnly) const -> bool;
  [[nodiscard]] auto isColliding(CollisionSet const& collisionSet) const -> bool;

  MaterialId foreground;
  MaterialHue foregroundHueShift{};
  ModId foregroundMod;
  MaterialHue foregroundModHueShift{};
  MaterialColorVariant foregroundColorVariant;

  MaterialId background;
  MaterialHue backgroundHueShift{};
  ModId backgroundMod;
  MaterialHue backgroundModHueShift{};
  MaterialColorVariant backgroundColorVariant;

  CollisionKind collision{};

  bool collisionCacheDirty{};
  StaticList<CollisionBlock, CollisionGenerator::MaximumCollisionsPerSpace> collisionCache;

  BiomeIndex blockBiomeIndex{};
  BiomeIndex environmentBiomeIndex{};

  bool biomeTransition;

  TileDamageStatus foregroundDamage;
  TileDamageStatus backgroundDamage;

  // If block is part of a dungeon then that affects spawns/drops,
  // as well as governing block protection
  DungeonId dungeonId{};
};

struct ServerTile : public WorldTile {
  static VersionNumber const CurrentSerializationVersion;

  ServerTile();

  ServerTile(ServerTile const& serverTile);
  auto operator=(ServerTile const& serverTile) -> ServerTile&;

  [[nodiscard]] auto isColliding(CollisionSet const& collisionSet) const -> bool;

  void write(DataStream& ds) const;
  void read(DataStream& ds, VersionNumber serializationVersion);

  // Updates collision, clears cache, and if the collision kind does not
  // support liquid destroys it.
  auto updateCollision(CollisionKind kind) -> bool;
  // Used for setting the second collision kind calculated by object material spaces.
  auto updateObjectCollision(CollisionKind kind) -> bool;

  // Calculates the actually-used collision kind based on the tile and object collision kinds.
  [[nodiscard]] auto getCollision() const -> CollisionKind;

  LiquidStore liquid;

  // If set, a plant or object is rooted to the tile and tile damage
  // should be redirected to this position
  std::optional<Vec2I> rootSource;

  // Do not serialize - calculated at runtime
  CollisionKind objectCollision;
};
using ServerTileSectorArray = TileSectorArray<ServerTile, WorldSectorSize>;
using ServerTileSectorArrayPtr = std::shared_ptr<ServerTileSectorArray>;

struct ClientTile : public WorldTile {
  ClientTile();

  ClientTile(ClientTile const& clientTile);
  auto operator=(ClientTile const& clientTile) -> ClientTile&;

  bool backgroundLightTransparent{};
  bool foregroundLightTransparent{};

  LiquidLevel liquid;

  float gravity{};
};
using ClientTileSectorArray = TileSectorArray<ClientTile, WorldSectorSize>;
using ClientTileSectorArrayPtr = std::shared_ptr<ClientTileSectorArray>;

// Tile structure to transfer all data from client to server
struct NetTile {
  NetTile();

  MaterialId background;
  MaterialHue backgroundHueShift{};
  MaterialColorVariant backgroundColorVariant{};
  ModId backgroundMod;
  MaterialHue backgroundModHueShift{};
  MaterialId foreground;
  MaterialHue foregroundHueShift{};
  MaterialColorVariant foregroundColorVariant{};
  ModId foregroundMod;
  MaterialHue foregroundModHueShift{};
  CollisionKind collision{};
  BiomeIndex blockBiomeIndex{};
  BiomeIndex environmentBiomeIndex{};
  LiquidNetUpdate liquid;
  DungeonId dungeonId{};
};
auto operator>>(DataStream& ds, NetTile& tile) -> DataStream&;
auto operator<<(DataStream& ds, NetTile const& tile) -> DataStream&;

// For storing predicted tile state.
struct PredictedTile {
  int64_t time;
  std::optional<MaterialId> background;
  std::optional<MaterialHue> backgroundHueShift;
  std::optional<MaterialColorVariant> backgroundColorVariant;
  std::optional<ModId> backgroundMod;
  std::optional<MaterialHue> backgroundModHueShift;
  std::optional<MaterialId> foreground;
  std::optional<MaterialHue> foregroundHueShift;
  std::optional<MaterialColorVariant> foregroundColorVariant;
  std::optional<ModId> foregroundMod;
  std::optional<MaterialHue> foregroundModHueShift;
  std::optional<LiquidLevel> liquid;
  std::optional<CollisionKind> collision;

  operator bool() const;
  template <typename Tile>
  void apply(Tile& tile) {
    if (foreground)
      tile.foreground = *foreground;
    if (foregroundMod)
      tile.foregroundMod = *foregroundMod;
    if (foregroundHueShift)
      tile.foregroundHueShift = *foregroundHueShift;
    if (foregroundModHueShift)
      tile.foregroundModHueShift = *foregroundModHueShift;

    if (background)
      tile.background = *background;
    if (backgroundMod)
      tile.backgroundMod = *backgroundMod;
    if (backgroundHueShift)
      tile.backgroundHueShift = *backgroundHueShift;
    if (backgroundModHueShift)
      tile.backgroundModHueShift = *backgroundModHueShift;
  }
};

// Just the parts of a tile that are used to render.  The members here are laid
// out specifically to avoid padding bytes so that a fast path can be taken
// when hashing for chunk render caching.
struct RenderTile {
  MaterialId foreground;
  ModId foregroundMod;

  MaterialId background;
  ModId backgroundMod;

  MaterialHue foregroundHueShift;
  MaterialHue foregroundModHueShift;
  MaterialColorVariant foregroundColorVariant;
  TileDamageType foregroundDamageType;
  uint8_t foregroundDamageLevel;

  MaterialHue backgroundHueShift;
  MaterialHue backgroundModHueShift;
  MaterialColorVariant backgroundColorVariant;
  TileDamageType backgroundDamageType;
  uint8_t backgroundDamageLevel;

  LiquidId liquidId;
  uint8_t liquidLevel;

  template <typename Hasher>
  void hashPushTerrain(Hasher& hasher) const;

  template <typename Hasher>
  void hashPushLiquid(Hasher& hasher) const;
};
auto operator>>(DataStream& ds, RenderTile& tile) -> DataStream&;
auto operator<<(DataStream& ds, RenderTile const& tile) -> DataStream&;

using RenderTileArray = MultiArray<RenderTile, 2>;

inline WorldTile::WorldTile()
    : foreground(NullMaterialId),
      foregroundMod(NoModId),
      foregroundColorVariant(DefaultMaterialColorVariant),
      background(NullMaterialId),
      backgroundMod(NoModId),
      backgroundColorVariant(DefaultMaterialColorVariant) {}

inline WorldTile::WorldTile(WorldTile const& worldTile) {
  *this = worldTile;
}

inline auto WorldTile::operator=(WorldTile const& worldTile) -> WorldTile& {
  foreground = worldTile.foreground;
  foregroundHueShift = worldTile.foregroundHueShift;
  foregroundMod = worldTile.foregroundMod;
  foregroundModHueShift = worldTile.foregroundModHueShift;
  foregroundColorVariant = worldTile.foregroundColorVariant;

  background = worldTile.background;
  backgroundHueShift = worldTile.backgroundHueShift;
  backgroundMod = worldTile.backgroundMod;
  backgroundModHueShift = worldTile.backgroundModHueShift;
  backgroundColorVariant = worldTile.backgroundColorVariant;

  // Don't bother copying collision cache
  collisionCacheDirty = true;

  collision = worldTile.collision;
  blockBiomeIndex = worldTile.blockBiomeIndex;
  environmentBiomeIndex = worldTile.environmentBiomeIndex;

  foregroundDamage = worldTile.foregroundDamage;
  backgroundDamage = worldTile.backgroundDamage;

  dungeonId = worldTile.dungeonId;

  return *this;
}

inline auto WorldTile::material(TileLayer layer) const -> MaterialId {
  if (layer == TileLayer::Foreground)
    return foreground;
  else
    return background;
}

inline auto WorldTile::mod(TileLayer layer) const -> ModId {
  if (layer == TileLayer::Foreground)
    return foregroundMod;
  else
    return backgroundMod;
}

inline auto WorldTile::materialColor(TileLayer layer) const -> MaterialColorVariant {
  if (layer == TileLayer::Foreground)
    return foregroundColorVariant;
  else
    return backgroundColorVariant;
}

inline auto WorldTile::getCollision() const -> CollisionKind {
  return collision;
}

inline auto WorldTile::materialAndColor(TileLayer layer) const -> std::tuple<MaterialId, MaterialHue, MaterialColorVariant> {
  if (layer == TileLayer::Foreground)
    return std::tuple<MaterialId, MaterialHue, MaterialColorVariant>{
      foreground, foregroundHueShift, foregroundColorVariant};
  else
    return std::tuple<MaterialId, MaterialHue, MaterialColorVariant>{
      background, backgroundHueShift, backgroundColorVariant};
}

inline ClientTile::ClientTile() = default;

inline ClientTile::ClientTile(ClientTile const& clientTile) : WorldTile() {
  *this = clientTile;
}

inline auto ClientTile::operator=(ClientTile const& clientTile) -> ClientTile& = default;

inline NetTile::NetTile()
    : background(NullMaterialId),
      backgroundMod(NoModId),
      foreground(NullMaterialId),
      foregroundMod(NoModId) {}

template <typename Hasher>
inline void RenderTile::hashPushTerrain(Hasher& hasher) const {
  // Do the fast path hash if the last (terrain relevant) field is at byte 20, because that means
  // there are no padding bytes between any field and we can simply pass the
  // entire tile as one block of memory.
  static constexpr std::size_t TerrainEndOffset = offsetof(RenderTile, liquidId);
  static constexpr std::size_t TotalTerrainSize =
    sizeof(MaterialId) * 2 + sizeof(ModId) * 2 + sizeof(MaterialHue) * 4 + sizeof(MaterialColorVariant) * 2 * sizeof(TileDamageType) * 2 + 2;

#ifdef STAR_DEBUG
  static constexpr bool FastHash = false;
#else
  static constexpr bool FastHash = TerrainEndOffset == TotalTerrainSize;
#endif

  if (FastHash) {
    hasher.push((char const*)this, TerrainEndOffset);
  } else {
    std::array<char, TotalTerrainSize> buffer;
    size_t bufferSize = 0;

    auto hashTilePart = [&](void const* data, size_t size) {
      memcpy(buffer.data() + bufferSize, data, size);
      bufferSize += size;
    };

    hashTilePart(&foreground, sizeof(foreground));
    hashTilePart(&foregroundMod, sizeof(foregroundMod));

    hashTilePart(&background, sizeof(background));
    hashTilePart(&backgroundMod, sizeof(backgroundMod));

    hashTilePart(&foregroundHueShift, sizeof(foregroundHueShift));
    hashTilePart(&foregroundModHueShift, sizeof(foregroundModHueShift));
    hashTilePart(&foregroundColorVariant, sizeof(foregroundColorVariant));
    hashTilePart(&foregroundDamageType, sizeof(foregroundDamageType));
    hashTilePart(&foregroundDamageLevel, sizeof(foregroundDamageLevel));

    hashTilePart(&backgroundHueShift, sizeof(backgroundHueShift));
    hashTilePart(&backgroundModHueShift, sizeof(backgroundModHueShift));
    hashTilePart(&backgroundColorVariant, sizeof(backgroundColorVariant));
    hashTilePart(&backgroundDamageType, sizeof(backgroundDamageType));
    hashTilePart(&backgroundDamageLevel, sizeof(backgroundDamageLevel));

    hasher.push(buffer, TotalTerrainSize);
  }
}

template <typename Hasher>
inline void RenderTile::hashPushLiquid(Hasher& hasher) const {
  std::array<char, sizeof(liquidLevel) + sizeof(liquidId)> buffer;

  memcpy(buffer.data(), &liquidLevel, sizeof(liquidLevel));
  memcpy(buffer.data() + sizeof(liquidLevel), &liquidId, sizeof(liquidId));

  hasher.push(buffer, sizeof(liquidLevel) + sizeof(liquidId));
}

}// namespace Star
