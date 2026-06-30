#include "StarPlayerCodexes.hpp"
#include "StarAlgorithm.hpp"
#include "StarCodex.hpp"
#include "StarCodexDatabase.hpp"
#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"

namespace Star {

PlayerCodexes::PlayerCodexes(AssetsConstPtr assets, CodexDatabaseConstPtr codexDatabase, Json const& variant)
  : m_assets(requireServiceValueAs<StarException>(std::move(assets), "PlayerCodexes", "assets")),
    m_codexDatabase(requireServiceValueAs<StarException>(std::move(codexDatabase), "PlayerCodexes", "codex database")) {
  if (variant) {
    auto codexData = jsonToMapV<StringMap<bool>>(variant, mem_fn(&Json::toBool));
    for (auto const& [codexId, codexRead] : codexData) {
      if (auto codex = m_codexDatabase->codex(codexId)) {
        m_codexes[codexId] = CodexEntry{codex, codexRead};
      } else {
        Logger::debug("Failed to load missing codex '{}'", codexId);
      }
    }
  }
}

Json PlayerCodexes::toJson() const {
  return jsonFromMapV<StringMap<CodexEntry>>(m_codexes, [](CodexEntry const& entry) { return entry.read; });
}

List<PlayerCodexes::CodexEntry> PlayerCodexes::codexes() const {
  List<CodexEntry> result;
  for (auto const& [codexId, codexEntry] : m_codexes)
    result.append(codexEntry);
  sort(result,
      [](CodexEntry const& left, CodexEntry const& right) -> bool {
        return tuple<bool, String>{left.read, left.codex->title()} < tuple<bool, String>{right.read, right.codex->title()};
      });
  return result;
}

bool PlayerCodexes::codexKnown(String const& codexId) const {
  return m_codexes.contains(codexId);
}

CodexConstPtr PlayerCodexes::learnCodex(String const& codexId, bool markRead) {
  if (!codexKnown(codexId)) {
    if (auto codex = m_codexDatabase->codex(codexId)) {
      auto entry = CodexEntry{codex, markRead};
      m_codexes[codexId] = entry;
      return entry.codex;
    }
  }
  return {};
}

bool PlayerCodexes::codexRead(String const& codexId) const {
  return m_codexes.contains(codexId) && m_codexes.get(codexId).read;
}

bool PlayerCodexes::markCodexRead(String const& codexId) {
  if (codexKnown(codexId) && !codexRead(codexId)) {
    m_codexes[codexId].read = true;
    return true;
  }
  return false;
}

bool PlayerCodexes::markCodexUnread(String const& codexId) {
  if (codexKnown(codexId) && codexRead(codexId)) {
    m_codexes[codexId].read = false;
    return true;
  }
  return false;
}

void PlayerCodexes::learnInitialCodexes(String const& playerSpecies) {
  for (auto codexId : jsonToStringList(m_assets->json(strf("/player.config:defaultCodexes.{}", playerSpecies))))
    learnCodex(codexId, true);
}

CodexConstPtr PlayerCodexes::firstNewCodex() const {
  for (auto const& [codexId, codexEntry] : m_codexes) {
    if (!codexEntry.read)
      return codexEntry.codex;
  }
  return {};
}

}
