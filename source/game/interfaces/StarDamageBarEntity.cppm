module;
#include "StarIdMap.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"

import star.drawable;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

import star.portrait_entity;

namespace Star {

STAR_CLASS(DamageBarEntity);

enum class DamageBarType : uint8_t {
  Default,
  None,
  Special
};
extern EnumMap<DamageBarType> const DamageBarTypeNames;

class DamageBarEntity : public virtual PortraitEntity {
public:
  virtual float health() const = 0;
  virtual float maxHealth() const = 0;
  virtual DamageBarType damageBar() const = 0;
};

}

export module star.damage_bar_entity;

export namespace Star {
  using ::Star::DamageBarEntity;
  using ::Star::DamageBarEntityPtr;
  using ::Star::DamageBarEntityConstPtr;
  using ::Star::DamageBarEntityWeakPtr;
  using ::Star::DamageBarEntityConstWeakPtr;
  using ::Star::DamageBarEntityUPtr;
  using ::Star::DamageBarEntityConstUPtr;
  using ::Star::DamageBarType;
  using ::Star::DamageBarTypeNames;
}
