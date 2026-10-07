module;
#include "StarIdMap.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.tile_damage;

import star.interaction_types;

import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;

import star.interactive_entity;
import star.collision_block;

namespace Star {

STAR_CLASS(TileEntity);

struct MaterialSpace {
  friend DataStream& operator<<(DataStream& ds, MaterialSpace const& materialSpace);
  friend DataStream& operator>>(DataStream& ds, MaterialSpace& materialSpace);
  MaterialSpace();
  MaterialSpace(Vec2I space, MaterialId material);

  bool operator==(MaterialSpace const& rhs) const;

  Vec2I space;
  MaterialId material;
};

DataStream& operator<<(DataStream& ds, MaterialSpace const& materialSpace);
DataStream& operator>>(DataStream& ds, MaterialSpace& materialSpace);

// Entities that derive from TileEntity are those that can be placed in the
// tile grid, and occupy tile spaces, possibly affecting collision.
class TileEntity : public virtual InteractiveEntity {
public:
  TileEntity();

  // position() here is simply the tilePosition (but Vec2F)
  virtual Vec2F position() const override;

  // The base tile position of this object.
  virtual Vec2I tilePosition() const = 0;
  virtual void setTilePosition(Vec2I const& pos) = 0;

  // TileEntities occupy the given spaces in tile space.  This is relative to
  // the current base position, and may include negative positions.  A 1x1
  // object would occupy just (0, 0).
  virtual List<Vec2I> spaces() const;

  // Blocks that should be marked as "root", so that they are non-destroyable
  // until this entity is destroyable.  Should be outside of spaces(), and
  // after placement should remain static for the lifetime of the entity.
  virtual List<Vec2I> roots() const;

  // TileEntities may register some of their occupied spaces with metamaterials
  // to generate collidable regions
  virtual List<MaterialSpace> materialSpaces() const;
  
  // Returns whether the entity was destroyed
  virtual bool damageTiles(List<Vec2I> const& positions, Vec2F const& sourcePosition, TileDamage const& tileDamage);
  virtual bool canBeDamaged() const;

  // Forces the tile entity to do an immediate check if it has been invalidly
  // placed in some way.  The tile entity may do this check on its own, but
  // less often.
  virtual bool checkBroken() = 0;

  // If the entity accepts interaction through right clicking, by default,
  // returns false.
  virtual bool isInteractive() const override;
  // By default, does nothing.  Will be called only on the server.
  virtual InteractAction interact(InteractRequest const& request) override;
  // Specific subset spaces that are interactive, by default, just returns
  // spaces()
  virtual List<Vec2I> interactiveSpaces() const;

  virtual List<QuestArcDescriptor> offeredQuests() const override;
  virtual StringSet turnInQuests() const override;
  virtual Vec2F questIndicatorPosition() const override;

protected:
  // Checks whether any of a given spaces list (relative to current tile
  // position) is occupied by a real material.  (Does not include tile
  // entities).
  bool anySpacesOccupied(List<Vec2I> const& relativeSpaces) const;

  // Checks that *all* spaces are occupied by a real material.
  bool allSpacesOccupied(List<Vec2I> const& relativeSpaces) const;

  float spacesLiquidFillLevel(List<Vec2I> const& relativeSpaces) const;
};

inline MaterialSpace::MaterialSpace()
  : material(NullMaterialId) {}

inline MaterialSpace::MaterialSpace(Vec2I space, MaterialId material)
  : space(space), material(material) {}

inline bool MaterialSpace::operator==(MaterialSpace const& rhs) const {
  return space         == rhs.space
      && material      == rhs.material;
}

}

export module star.tile_entity;

export namespace Star {
  using ::Star::TileEntity;
  using ::Star::TileEntityPtr;
  using ::Star::TileEntityConstPtr;
  using ::Star::TileEntityWeakPtr;
  using ::Star::TileEntityConstWeakPtr;
  using ::Star::TileEntityUPtr;
  using ::Star::TileEntityConstUPtr;
  using ::Star::MaterialSpace;
}
