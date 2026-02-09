#include "StarCodexDatabase.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

CodexDatabase::CodexDatabase() {
  auto assets = Root::singleton().assets();
  auto& files = assets->scanExtension("codex");
  auto codexConfig = assets->json("/codex.config");
  assets->queueJsons(files);
  for (auto& file : files) {
    try {
      auto codexJson = assets->json(file);
      codexJson = codexJson.set("icon",
                                AssetPath::relativeTo(AssetPath::directory(file), codexJson.getString("icon", codexConfig.getString("defaultIcon"))));

      auto codex = std::make_shared<Codex>(codexJson, file);

      if (m_codexes.contains(codex->id()))
        throw CodexDatabaseException::format("Duplicate codex named '{}', config file '{}'", codex->id(), file);

      m_codexes[codex->id()] = codex;
    } catch (std::exception const& e) {
      throw CodexDatabaseException(strf("Error reading codex config {}", file), e);
    }
  }
}

auto CodexDatabase::codexes() const -> StringMap<ConstPtr<Codex>> {
  return m_codexes;
}

auto CodexDatabase::codex(String const& codexId) const -> ConstPtr<Codex> {
  if (auto codex = m_codexes.maybe(codexId))
    return std::move(*codex);
  return {};
}

}// namespace Star
