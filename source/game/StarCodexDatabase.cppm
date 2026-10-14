module;

import std;
import star.string;
import star.format;
import star.json;

namespace Star {
class Codex;
using CodexPtr = std::shared_ptr<Codex>;
using CodexConstPtr = std::shared_ptr<const Codex>;
using CodexWeakPtr = std::weak_ptr<Codex>;
using CodexConstWeakPtr = std::weak_ptr<const Codex>;
using CodexUPtr = std::unique_ptr<Codex>;
using CodexConstUPtr = std::unique_ptr<const Codex>;

struct CodexDatabaseExceptionTag {
  static constexpr char const* name() { return "CodexDatabaseException"; }
};
using CodexDatabaseException = StarError<CodexDatabaseExceptionTag, StarException>;

class CodexDatabase;
using CodexDatabasePtr = std::shared_ptr<CodexDatabase>;
using CodexDatabaseConstPtr = std::shared_ptr<const CodexDatabase>;
using CodexDatabaseWeakPtr = std::weak_ptr<CodexDatabase>;
using CodexDatabaseConstWeakPtr = std::weak_ptr<const CodexDatabase>;
using CodexDatabaseUPtr = std::unique_ptr<CodexDatabase>;
using CodexDatabaseConstUPtr = std::unique_ptr<const CodexDatabase>;

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
