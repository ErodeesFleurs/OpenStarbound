#pragma once

#include "StarCodex.hpp"
#include "StarConfig.hpp"
#include "StarException.hpp"

namespace Star {

using CodexDatabaseException = ExceptionDerived<"CodexDatabaseException">;

class CodexDatabase {
public:
  CodexDatabase();

  [[nodiscard]] auto codexes() const -> StringMap<ConstPtr<Codex>>;
  [[nodiscard]] auto codex(String const& codexId) const -> ConstPtr<Codex>;

private:
  StringMap<ConstPtr<Codex>> m_codexes;
};

}// namespace Star
