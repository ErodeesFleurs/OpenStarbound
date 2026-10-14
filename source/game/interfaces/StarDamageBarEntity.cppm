module;
#include "StarConfig.hpp"
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
