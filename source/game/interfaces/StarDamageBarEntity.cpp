#include "StarJson.hpp"
import star.id_map;
#include "StarString.hpp"
#include "StarDataStream.hpp"
import star.color;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
import star.game_types;
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

extern EnumMap<DamageBarType> const DamageBarTypeNames{
  {DamageBarType::Default, "Default"},
  {DamageBarType::None, "None"},
  {DamageBarType::Special, "Special"}
};

}
