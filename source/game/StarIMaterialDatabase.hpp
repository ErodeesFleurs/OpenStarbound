#pragma once

#include "StarGameTypes.hpp"
#include "StarMaterialTypes.hpp"
#include "StarTileDamage.hpp"
#include "StarCollisionBlock.hpp"
#include "StarItemDescriptor.hpp"
#include "StarParticle.hpp"
#include "StarLiquidTypes.hpp"
#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarVector.hpp"

namespace Star {

class IMaterialDatabase {
public:
  virtual ~IMaterialDatabase() = default;

  virtual MaterialId materialId(String const& materialName) const = 0;
  virtual String materialName(MaterialId materialId) const = 0;
  virtual bool isValidMaterialId(MaterialId material) const = 0;

  virtual CollisionKind materialCollisionKind(MaterialId materialId) const = 0;
  virtual TileDamageParameters materialDamageParameters(MaterialId materialId) const = 0;

  virtual ModId modId(String const& modName) const = 0;
  virtual String const& modName(ModId modId) const = 0;
  virtual TileDamageParameters modDamageParameters(ModId modId) const = 0;
  virtual bool modBreaksWithTile(ModId modId) const = 0;

  virtual ItemDescriptor materialItemDrop(MaterialId materialId) const = 0;
  virtual ItemDescriptor modItemDrop(ModId modId) const = 0;

  virtual bool isMultiColor(MaterialId materialId) const = 0;
  virtual bool foregroundLightTransparent(MaterialId materialId) const = 0;
  virtual bool backgroundLightTransparent(MaterialId materialId) const = 0;

  virtual Color materialParticleColor(MaterialId materialId, MaterialHue hueShift) const = 0;
  virtual Vec3F radiantLight(MaterialId materialId, ModId modId) const = 0;

  virtual String miningSound(MaterialId materialId, ModId modId = NoModId) const = 0;
  virtual String footstepSound(MaterialId materialId, ModId modId = NoModId) const = 0;
  virtual String defaultFootstepSound() const = 0;

  virtual bool isFallingMaterial(MaterialId materialId) const = 0;
  virtual bool isCascadingFallingMaterial(MaterialId materialId) const = 0;
  virtual bool blocksLiquidFlow(MaterialId materialId) const = 0;

  virtual bool isSoil(MaterialId materialId) const = 0;
};

using IMaterialDatabasePtr = SharedPtr<IMaterialDatabase>;
using IMaterialDatabaseConstPtr = SharedPtr<IMaterialDatabase const>;

}
