module;

import std;
import star.string;
import star.format;
import star.json;

namespace Star {

class Root;

struct StagehandDatabaseExceptionTag {
  static constexpr std::string_view name() { return "StagehandDatabaseException"; }
};
using StagehandDatabaseException = StarError<StagehandDatabaseExceptionTag, StarException>;

class Stagehand;

class StagehandDatabase {
public:
  [[nodiscard]] static std::expected<std::shared_ptr<StagehandDatabase>, StarException> create(Root& root);

  [[nodiscard]] std::expected<std::shared_ptr<Stagehand>, StarException> createStagehand(String const& stagehandType, Json const& extraConfig = Json()) const;

private:
  explicit StagehandDatabase(Root& root) : m_root(root) {}
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
