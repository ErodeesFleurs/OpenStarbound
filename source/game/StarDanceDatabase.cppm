module;
import star.json;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
#include "StarConfig.hpp"
import star.string;
import star.maybe;
import star.version;


import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.vector;
namespace Star {

STAR_STRUCT(DanceStep);
STAR_STRUCT(Dance);
STAR_CLASS(DanceDatabase);

struct DanceStep {
  Maybe<String> bodyFrame;
  Maybe<String> frontArmFrame;
  Maybe<String> backArmFrame;
  Vec2F headOffset;
  Vec2F frontArmOffset;
  Vec2F backArmOffset;
  float frontArmRotation;
  float backArmRotation;
};

struct Dance {
  String name;
  List<String> states;
  float cycle;
  bool cyclic;
  float duration;
  List<DanceStep> steps;
};

class DanceDatabase {
public:
  DanceDatabase();

  DancePtr getDance(String const& name) const;

private:
  static DancePtr readDance(String const& path);

  StringMap<DancePtr> m_dances;
};

}

export module star.dance_database;

export namespace Star {
  using ::Star::DanceStep;
  using ::Star::DanceStepPtr;
  using ::Star::DanceStepConstPtr;
  using ::Star::Dance;
  using ::Star::DancePtr;
  using ::Star::DanceConstPtr;
  using ::Star::DanceDatabase;
  using ::Star::DanceDatabasePtr;
  using ::Star::DanceDatabaseConstPtr;
}
