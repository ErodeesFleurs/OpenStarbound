#pragma once

#include "StarEntity.hpp"
#include "StarTileDamage.hpp"
#include "StarInteractiveEntity.hpp"
#include "StarCollisionBlock.hpp"

namespace Star {

class TileEntity;
using TileEntityPtr = SharedPtr<TileEntity>;

struct MaterialSpace {
  MaterialSpace() = default;
  MaterialSpace(Vec2I space, MaterialId material);

  [[nodiscard]] bool operator==(MaterialSpace const& rhs) const;

  Vec2I space;
  MaterialId material = NullMaterialId;
};

DataStream& operator<<(DataStream& ds, MaterialSpace const& materialSpace);
DataStream& operator>>(DataStream& ds, MaterialSpace& materialSpace);

// Entities that derive from TileEntity are those that can be placed in the
// tile grid, and occupy tile spaces, possibly affecting collision.
class TileEntity : public virtual InteractiveEntity {
public:
  TileEntity();

  // position() here is simply the tilePosition (but Vec2F)
  [[nodiscard]] Vec2F position() const override;

  // The base tile position of this object.
  [[nodiscard]] virtual Vec2I tilePosition() const = 0;
  virtual void setTilePosition(Vec2I const& pos) = 0;

  // TileEntities occupy the given spaces in tile space.  This is relative to
  // the current base position, and may include negative positions.  A 1x1
  // object would occupy just (0, 0).
  [[nodiscard]] virtual List<Vec2I> spaces() const;

  // Blocks that should be marked as "root", so that they are non-destroyable
  // until this entity is destroyable.  Should be outside of spaces(), and
  // after placement should remain static for the lifetime of the entity.
  [[nodiscard]] virtual List<Vec2I> roots() const;

  // TileEntities may register some of their occupied spaces with metamaterials
  // to generate collidable regions
  [[nodiscard]] virtual List<MaterialSpace> materialSpaces() const;
  
  // Returns whether the entity was destroyed
  [[nodiscard]] virtual bool damageTiles(List<Vec2I> const& positions, Vec2F const& sourcePosition, TileDamage const& tileDamage);
  [[nodiscard]] virtual bool canBeDamaged() const;

  // Forces the tile entity to do an immediate check if it has been invalidly
  // placed in some way.  The tile entity may do this check on its own, but
  // less often.
  [[nodiscard]] virtual bool checkBroken() = 0;

  // If the entity accepts interaction through right clicking, by default,
  // returns false.
  [[nodiscard]] bool isInteractive() const override;
  // By default, does nothing.  Will be called only on the server.
  [[nodiscard]] InteractAction interact(InteractRequest const& request) override;
  // Specific subset spaces that are interactive, by default, just returns
  // spaces()
  [[nodiscard]] virtual List<Vec2I> interactiveSpaces() const;

  [[nodiscard]] List<QuestArcDescriptor> offeredQuests() const override;
  [[nodiscard]] StringSet turnInQuests() const override;
  [[nodiscard]] Vec2F questIndicatorPosition() const override;

protected:
  // Checks whether any of a given spaces list (relative to current tile
  // position) is occupied by a real material.  (Does not include tile
  // entities).
  [[nodiscard]] bool anySpacesOccupied(List<Vec2I> const& relativeSpaces) const;

  // Checks that *all* spaces are occupied by a real material.
  [[nodiscard]] bool allSpacesOccupied(List<Vec2I> const& relativeSpaces) const;

  [[nodiscard]] float spacesLiquidFillLevel(List<Vec2I> const& relativeSpaces) const;
};

inline MaterialSpace::MaterialSpace(Vec2I space, MaterialId material)
  : space(space), material(material) {}

[[nodiscard]] inline bool MaterialSpace::operator==(MaterialSpace const& rhs) const {
  return space         == rhs.space
      && material      == rhs.material;
}

}
