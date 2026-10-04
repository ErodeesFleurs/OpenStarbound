module;

#include "StarJson.hpp"
#include "StarCodex.hpp"

namespace Star {

struct CodexDatabaseExceptionTag {
  static constexpr char const* name() { return "CodexDatabaseException"; }
};
using CodexDatabaseException = StarError<CodexDatabaseExceptionTag, StarException>;

STAR_CLASS(CodexDatabase);

class CodexDatabase {
public:
  CodexDatabase();

  StringMap<CodexConstPtr> codexes() const;
  CodexConstPtr codex(String const& codexId) const;

private:
  StringMap<CodexConstPtr> m_codexes;
};

}

export module star.codex_database;

export namespace Star {
  using ::Star::CodexDatabaseExceptionTag;
  using ::Star::CodexDatabaseException;
  using ::Star::CodexDatabase;
  using ::Star::CodexDatabasePtr;
  using ::Star::CodexDatabaseConstPtr;
}
