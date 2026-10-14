module;

import std;
import star.string;
import star.format;
import star.json;

namespace Star {

struct StagehandDatabaseExceptionTag {
  static constexpr char const* name() { return "StagehandDatabaseException"; }
};
using StagehandDatabaseException = StarError<StagehandDatabaseExceptionTag, StarException>;

class Stagehand;
using StagehandPtr = std::shared_ptr<Stagehand>;
using StagehandConstPtr = std::shared_ptr<const Stagehand>;
using StagehandWeakPtr = std::weak_ptr<Stagehand>;
using StagehandConstWeakPtr = std::weak_ptr<const Stagehand>;
using StagehandUPtr = std::unique_ptr<Stagehand>;
using StagehandConstUPtr = std::unique_ptr<const Stagehand>;

class StagehandDatabase;
using StagehandDatabasePtr = std::shared_ptr<StagehandDatabase>;
using StagehandDatabaseConstPtr = std::shared_ptr<const StagehandDatabase>;
using StagehandDatabaseWeakPtr = std::weak_ptr<StagehandDatabase>;
using StagehandDatabaseConstWeakPtr = std::weak_ptr<const StagehandDatabase>;
using StagehandDatabaseUPtr = std::unique_ptr<StagehandDatabase>;
using StagehandDatabaseConstUPtr = std::unique_ptr<const StagehandDatabase>;

class StagehandDatabase {
public:
  StagehandDatabase();

  StagehandPtr createStagehand(String const& stagehandType, Json const& extraConfig = Json()) const;

private:
  StringMap<Json> m_stagehandTypes;
};

}

export module star.stagehand_database;

export namespace Star {
  using ::Star::StagehandDatabaseExceptionTag;
  using ::Star::StagehandDatabaseException;
  using ::Star::Stagehand;
  using ::Star::StagehandPtr;
  using ::Star::StagehandConstPtr;
  using ::Star::StagehandDatabase;
  using ::Star::StagehandDatabasePtr;
  using ::Star::StagehandDatabaseConstPtr;
}
