module;

#include "StarJson.hpp"

namespace Star {

struct StagehandDatabaseExceptionTag {
  static constexpr char const* name() { return "StagehandDatabaseException"; }
};
using StagehandDatabaseException = StarError<StagehandDatabaseExceptionTag, StarException>;

STAR_CLASS(Stagehand);

STAR_CLASS(StagehandDatabase);

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
