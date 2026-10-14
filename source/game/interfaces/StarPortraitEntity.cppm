module;
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

namespace Star {

class PortraitEntity;
using PortraitEntityPtr = std::shared_ptr<PortraitEntity>;
using PortraitEntityConstPtr = std::shared_ptr<const PortraitEntity>;
using PortraitEntityWeakPtr = std::weak_ptr<PortraitEntity>;
using PortraitEntityConstWeakPtr = std::weak_ptr<const PortraitEntity>;
using PortraitEntityUPtr = std::unique_ptr<PortraitEntity>;
using PortraitEntityConstUPtr = std::unique_ptr<const PortraitEntity>;

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
