#pragma once

#include "StarJson.hpp"
#include "StarCodex.hpp"
#include "StarAssets.hpp"

namespace Star {

struct CodexDatabaseExceptionTag { static constexpr char const* typeName = "CodexDatabaseException"; };
using CodexDatabaseException = TypedException<StarException, CodexDatabaseExceptionTag>;

class CodexDatabase;
using CodexDatabasePtr = SharedPtr<CodexDatabase>;
using CodexDatabaseConstPtr = SharedPtr<CodexDatabase const>;

class CodexDatabase {
public:
  CodexDatabase(AssetsConstPtr assets);

  [[nodiscard]] StringMap<CodexConstPtr> codexes() const;
  [[nodiscard]] CodexConstPtr codex(String const& codexId) const;

private:
  StringMap<CodexConstPtr> m_codexes;
};

}
