#pragma once

#include "StarJson.hpp"

namespace Star {

struct StagehandDatabaseExceptionTag { static constexpr char const* typeName = "StagehandDatabaseException"; };
using StagehandDatabaseException = TypedException<StarException, StagehandDatabaseExceptionTag>;

class Stagehand;
using StagehandPtr = SharedPtr<Stagehand>;

class StagehandDatabase;
using StagehandDatabasePtr = SharedPtr<StagehandDatabase>;
using StagehandDatabaseConstPtr = SharedPtr<StagehandDatabase const>;

class StagehandDatabase {
public:
  StagehandDatabase();

  StagehandPtr createStagehand(String const& stagehandType, Json const& extraConfig = Json()) const;

private:
  StringMap<Json> m_stagehandTypes;
};

}
