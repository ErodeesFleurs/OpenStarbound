#pragma once

#include "StarException.hpp"
#include "StarJson.hpp"

namespace Star {

using StagehandDatabaseException = ExceptionDerived<"StagehandDatabaseException", StarException>;

class Stagehand;

class StagehandDatabase {
public:
  StagehandDatabase();

  [[nodiscard]] auto createStagehand(String const& stagehandType, Json const& extraConfig = Json()) const -> Ptr<Stagehand>;

private:
  StringMap<Json> m_stagehandTypes;
};

}// namespace Star
