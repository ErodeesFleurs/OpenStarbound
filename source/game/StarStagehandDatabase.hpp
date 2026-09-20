#pragma once

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
