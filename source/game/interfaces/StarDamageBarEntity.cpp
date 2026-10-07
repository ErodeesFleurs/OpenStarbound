#include "StarIdMap.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
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
import star.damage_bar_entity;

namespace Star {

EnumMap<DamageBarType> const DamageBarTypeNames{
  {DamageBarType::Default, "Default"},
  {DamageBarType::None, "None"},
  {DamageBarType::Special, "Special"}
};

}
