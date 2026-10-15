module;

import std;
import star.string;
import star.format;
import star.json;

namespace Star {
class Codex;

struct CodexDatabaseExceptionTag {
  static constexpr char const* name() { return "CodexDatabaseException"; }
};
using CodexDatabaseException = StarError<CodexDatabaseExceptionTag, StarException>;

class CodexDatabase {
public:
  CodexDatabase();

  StringMap<std::shared_ptr<Codex const>> codexes() const;
  std::shared_ptr<Codex const> codex(String const& codexId) const;

private:
  StringMap<std::shared_ptr<Codex const>> m_codexes;
};

}

export module star.codex_database;

export namespace Star {
  using ::Star::CodexDatabaseExceptionTag;
  using ::Star::CodexDatabaseException;
  using ::Star::CodexDatabase;
}
