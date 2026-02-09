#include "StarPlayerCodexes.hpp"

#include "StarCodex.hpp"
#include "StarCodexDatabase.hpp"// IWYU pragma: export
#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

PlayerCodexes::PlayerCodexes(Json const& variant) {
  if (variant) {
    auto assets = Root::singleton().assets();
    auto codexData = jsonToMapV<StringMap<bool>>(variant, std::mem_fn(&Json::toBool));
    for (auto pair : codexData) {
      if (auto codex = Root::singleton().codexDatabase()->codex(pair.first)) {
        m_codexes[pair.first] = CodexEntry{codex, pair.second};
      } else {
        Logger::debug("Failed to load missing codex '{}'", pair.first);
      }
    }
  }
}

auto PlayerCodexes::toJson() const -> Json {
  return jsonFromMapV<StringMap<CodexEntry>>(m_codexes, [](CodexEntry const& entry) -> bool { return entry.second; });
}

auto PlayerCodexes::codexes() const -> List<PlayerCodexes::CodexEntry> {
  List<CodexEntry> result;
  for (auto pair : m_codexes)
    result.append(pair.second);
  sort(result,
       [](CodexEntry const& left, CodexEntry const& right) -> bool {
         return std::make_tuple(left.second, left.first->title()) < std::make_tuple(right.second, right.first->title());
       });
  return result;
}

auto PlayerCodexes::codexKnown(String const& codexId) const -> bool {
  return m_codexes.contains(codexId);
}

auto PlayerCodexes::learnCodex(String const& codexId, bool markRead) -> ConstPtr<Codex> {
  if (!codexKnown(codexId)) {
    if (auto codex = Root::singleton().codexDatabase()->codex(codexId)) {
      auto entry = CodexEntry{codex, markRead};
      m_codexes[codexId] = entry;
      return entry.first;
    }
  }
  return {};
}

auto PlayerCodexes::codexRead(String const& codexId) const -> bool {
  return m_codexes.contains(codexId) && m_codexes.get(codexId).second;
}

auto PlayerCodexes::markCodexRead(String const& codexId) -> bool {
  if (codexKnown(codexId) && !codexRead(codexId)) {
    m_codexes[codexId].second = true;
    return true;
  }
  return false;
}

auto PlayerCodexes::markCodexUnread(String const& codexId) -> bool {
  if (codexKnown(codexId) && codexRead(codexId)) {
    m_codexes[codexId].second = false;
    return true;
  }
  return false;
}

void PlayerCodexes::learnInitialCodexes(String const& playerSpecies) {
  for (auto codexId : jsonToStringList(Root::singleton().assets()->json(strf("/player.config:defaultCodexes.{}", playerSpecies))))
    learnCodex(codexId, true);
}

auto PlayerCodexes::firstNewCodex() const -> ConstPtr<Codex> {
  for (auto pair : m_codexes) {
    if (!pair.second.second)
      return pair.second.first;
  }
  return {};
}

}// namespace Star
