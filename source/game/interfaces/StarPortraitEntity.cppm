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

namespace Star {

STAR_CLASS(PortraitEntity);

class PortraitEntity : public virtual Entity {
public:
  virtual List<Drawable> portrait(PortraitMode mode) const = 0;
};

}

export module star.portrait_entity;

export namespace Star {
  using ::Star::PortraitEntity;
  using ::Star::PortraitEntityPtr;
  using ::Star::PortraitEntityConstPtr;
  using ::Star::PortraitEntityWeakPtr;
  using ::Star::PortraitEntityConstWeakPtr;
  using ::Star::PortraitEntityUPtr;
  using ::Star::PortraitEntityConstUPtr;
}
