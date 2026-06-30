#include "StarPlayerCodexes.hpp"
#include "StarAlgorithm.hpp"
#include "StarCodex.hpp"
#include "StarCodexDatabase.hpp"
#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"

namespace Star {

PlayerCodexes::PlayerCodexes(AssetsConstPtr assets, CodexDatabaseConstPtr codexDatabase, Json const& variant)
  : m_assets(std::move(assets)), m_codexDatabase(std::move(codexDatabase)) {
  requireNotNull(m_assets, "PlayerCodexes", "assets");
  requireNotNull(m_codexDatabase, "PlayerCodexes", "codex database");

  if (variant) {
    auto codexData = jsonToMapV<StringMap<bool>>(variant, mem_fn(&Json::toBool));
    for (auto pair : codexData) {
      if (auto codex = m_codexDatabase->codex(pair.first)) {
        m_codexes[pair.first] = CodexEntry{codex, pair.second};
      } else {
        Logger::debug("Failed to load missing codex '{}'", pair.first);
      }
    }
  }
}

Json PlayerCodexes::toJson() const {
  return jsonFromMapV<StringMap<CodexEntry>>(m_codexes, [](CodexEntry const& entry) { return entry.second; });
}

List<PlayerCodexes::CodexEntry> PlayerCodexes::codexes() const {
  List<CodexEntry> result;
  for (auto pair : m_codexes)
    result.append(pair.second);
  sort(result,
      [](CodexEntry const& left, CodexEntry const& right) -> bool {
        return make_tuple(left.second, left.first->title()) < make_tuple(right.second, right.first->title());
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
      return entry.first;
    }
  }
  return {};
}

bool PlayerCodexes::codexRead(String const& codexId) const {
  return m_codexes.contains(codexId) && m_codexes.get(codexId).second;
}

bool PlayerCodexes::markCodexRead(String const& codexId) {
  if (codexKnown(codexId) && !codexRead(codexId)) {
    m_codexes[codexId].second = true;
    return true;
  }
  return false;
}

bool PlayerCodexes::markCodexUnread(String const& codexId) {
  if (codexKnown(codexId) && codexRead(codexId)) {
    m_codexes[codexId].second = false;
    return true;
  }
  return false;
}

void PlayerCodexes::learnInitialCodexes(String const& playerSpecies) {
  for (auto codexId : jsonToStringList(m_assets->json(strf("/player.config:defaultCodexes.{}", playerSpecies))))
    learnCodex(codexId, true);
}

CodexConstPtr PlayerCodexes::firstNewCodex() const {
  for (auto pair : m_codexes) {
    if (!pair.second.second)
      return pair.second.first;
  }
  return {};
}

}
