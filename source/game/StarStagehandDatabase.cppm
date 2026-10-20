module;

import std;
import star.string;
import star.format;
import star.json;

namespace Star {

class Root;

struct StagehandDatabaseExceptionTag {
  static constexpr char const* name() { return "StagehandDatabaseException"; }
};
using StagehandDatabaseException = StarError<StagehandDatabaseExceptionTag, StarException>;

class Stagehand;

class StagehandDatabase {
public:
  explicit StagehandDatabase(Root& root);

  std::shared_ptr<Stagehand> createStagehand(String const& stagehandType, Json const& extraConfig = Json()) const;

private:
  Root& m_root;
  StringMap<Json> m_stagehandTypes;
};

}

export module star.stagehand_database;

export namespace Star {
  using ::Star::StagehandDatabaseExceptionTag;
  using ::Star::StagehandDatabaseException;
  using ::Star::Stagehand;
  using ::Star::StagehandDatabase;
}
