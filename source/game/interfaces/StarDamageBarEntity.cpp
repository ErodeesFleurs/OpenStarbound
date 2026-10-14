import std;
import star.json;
import star.id_map;
import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;
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
