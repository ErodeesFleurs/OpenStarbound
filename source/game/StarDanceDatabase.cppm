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
import std;
import star.string;
import star.maybe;
import star.version;


import star.asset_source;
import star.assets;

import star.vector;
namespace Star {

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
  DanceDatabase(std::shared_ptr<Assets const> assets, Logger& logger);

  std::shared_ptr<Dance> getDance(String const& name) const;

private:
  static std::shared_ptr<Dance> readDance(Assets const& assets, String const& path);

  Logger& m_logger;
  StringMap<std::shared_ptr<Dance>> m_dances;
};

}

export module star.dance_database;

export namespace Star {
  using ::Star::DanceStep;
  using ::Star::Dance;
  using ::Star::DanceDatabase;
}
