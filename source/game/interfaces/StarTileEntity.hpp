#pragma once

#include "StarInteractiveEntity.hpp"
#include "StarMaterialTypes.hpp"
#include "StarTileDamage.hpp"

namespace Star {

// STAR_CLASS(TileEntity);

struct MaterialSpace {
  MaterialSpace();
  MaterialSpace(Vec2I space, MaterialId material);

  auto operator==(MaterialSpace const& rhs) const -> bool;

  Vec2I space;
  MaterialId material;
};

auto operator<<(DataStream& ds, MaterialSpace const& materialSpace) -> DataStream&;
auto operator>>(DataStream& ds, MaterialSpace& materialSpace) -> DataStream&;

// Entities that derive from TileEntity are those that can be placed in the
// tile grid, and occupy tile spaces, possibly affecting collision.
class TileEntity : public virtual InteractiveEntity {
public:
  TileEntity();

  // position() here is simply the tilePosition (but Vec2F)
  [[nodiscard]] auto position() const -> Vec2F override;

  // The base tile position of this object.
  [[nodiscard]] virtual auto tilePosition() const -> Vec2I = 0;
  virtual void setTilePosition(Vec2I const& pos) = 0;

  // TileEntities occupy the given spaces in tile space.  This is relative to
  // the current base position, and may include negative positions.  A 1x1
  // object would occupy just (0, 0).
  [[nodiscard]] virtual auto spaces() const -> List<Vec2I>;

  // Blocks that should be marked as "root", so that they are non-destroyable
  // until this entity is destroyable.  Should be outside of spaces(), and
  // after placement should remain static for the lifetime of the entity.
  [[nodiscard]] virtual auto roots() const -> List<Vec2I>;

  // TileEntities may register some of their occupied spaces with metamaterials
  // to generate collidable regions
  [[nodiscard]] virtual auto materialSpaces() const -> List<MaterialSpace>;

  // Returns whether the entity was destroyed
  virtual auto damageTiles(List<Vec2I> const& positions, Vec2F const& sourcePosition, TileDamage const& tileDamage) -> bool;
  [[nodiscard]] virtual auto canBeDamaged() const -> bool;

  // Forces the tile entity to do an immediate check if it has been invalidly
  // placed in some way.  The tile entity may do this check on its own, but
  // less often.
  virtual auto checkBroken() -> bool = 0;

  // If the entity accepts interaction through right clicking, by default,
  // returns false.
  [[nodiscard]] auto isInteractive() const -> bool override;
  // By default, does nothing.  Will be called only on the server.
  auto interact(InteractRequest const& request) -> InteractAction override;
  // Specific subset spaces that are interactive, by default, just returns
  // spaces()
  [[nodiscard]] virtual auto interactiveSpaces() const -> List<Vec2I>;

  [[nodiscard]] List<QuestArcDescriptor> offeredQuests() const override;
  [[nodiscard]] auto turnInQuests() const -> StringSet override;
  [[nodiscard]] auto questIndicatorPosition() const -> Vec2F override;

protected:
  // Checks whether any of a given spaces list (relative to current tile
  // position) is occupied by a real material.  (Does not include tile
  // entities).
  [[nodiscard]] auto anySpacesOccupied(List<Vec2I> const& relativeSpaces) const -> bool;

  // Checks that *all* spaces are occupied by a real material.
  [[nodiscard]] auto allSpacesOccupied(List<Vec2I> const& relativeSpaces) const -> bool;

  [[nodiscard]] auto spacesLiquidFillLevel(List<Vec2I> const& relativeSpaces) const -> float;
};

inline MaterialSpace::MaterialSpace()
    : material(NullMaterialId) {}

inline MaterialSpace::MaterialSpace(Vec2I space, MaterialId material)
    : space(space), material(material) {}

inline auto MaterialSpace::operator==(MaterialSpace const& rhs) const -> bool {
  return space == rhs.space
    && material == rhs.material;
}

}// namespace Star
