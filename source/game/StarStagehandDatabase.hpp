#pragma once

#include "StarJson.hpp"
#include "StarAssets.hpp"

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
  StagehandDatabase(AssetsConstPtr assets);

  [[nodiscard]] StagehandPtr createStagehand(String const& stagehandType, Json const& extraConfig = Json()) const;

private:
  StringMap<Json> m_stagehandTypes;
};

}
