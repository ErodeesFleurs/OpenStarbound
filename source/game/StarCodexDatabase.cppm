module;

import std;
import star.string;
import star.format;
import star.json;

namespace Star {
class Assets;
class Codex;

struct CodexDatabaseExceptionTag {
  static constexpr char const* name() { return "CodexDatabaseException"; }
};
using CodexDatabaseException = StarError<CodexDatabaseExceptionTag, StarException>;

class CodexDatabase {
public:
  [[nodiscard]] static std::expected<std::shared_ptr<CodexDatabase>, StarException> create(std::shared_ptr<Assets const> assets);

  StringMap<std::shared_ptr<Codex const>> codexes() const;
  std::shared_ptr<Codex const> codex(String const& codexId) const;

private:
  CodexDatabase() = default;
  StringMap<std::shared_ptr<Codex const>> m_codexes;
};

}

export module star.codex_database;

export namespace Star {
  using ::Star::CodexDatabaseExceptionTag;
  using ::Star::CodexDatabaseException;
  using ::Star::CodexDatabase;
}
