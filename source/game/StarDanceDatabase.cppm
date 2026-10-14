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
import star.root_base;
import star.configuration;
import star.root;

import star.vector;
namespace Star {

struct DanceStep;
using DanceStepPtr = std::shared_ptr<DanceStep>;
using DanceStepConstPtr = std::shared_ptr<const DanceStep>;
using DanceStepWeakPtr = std::weak_ptr<DanceStep>;
using DanceStepConstWeakPtr = std::weak_ptr<const DanceStep>;
using DanceStepUPtr = std::unique_ptr<DanceStep>;
using DanceStepConstUPtr = std::unique_ptr<const DanceStep>;
struct Dance;
using DancePtr = std::shared_ptr<Dance>;
using DanceConstPtr = std::shared_ptr<const Dance>;
using DanceWeakPtr = std::weak_ptr<Dance>;
using DanceConstWeakPtr = std::weak_ptr<const Dance>;
using DanceUPtr = std::unique_ptr<Dance>;
using DanceConstUPtr = std::unique_ptr<const Dance>;
class DanceDatabase;
using DanceDatabasePtr = std::shared_ptr<DanceDatabase>;
using DanceDatabaseConstPtr = std::shared_ptr<const DanceDatabase>;
using DanceDatabaseWeakPtr = std::weak_ptr<DanceDatabase>;
using DanceDatabaseConstWeakPtr = std::weak_ptr<const DanceDatabase>;
using DanceDatabaseUPtr = std::unique_ptr<DanceDatabase>;
using DanceDatabaseConstUPtr = std::unique_ptr<const DanceDatabase>;

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
