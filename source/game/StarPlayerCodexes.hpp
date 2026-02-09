#pragma once

#include "StarConfig.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

class Codex;

class PlayerCodexes {
public:
  using CodexEntry = std::pair<ConstPtr<Codex>, bool>;

  PlayerCodexes(Json const& json = {});

  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto codexes() const -> List<CodexEntry>;

  [[nodiscard]] auto codexKnown(String const& codexId) const -> bool;
  auto learnCodex(String const& codexId, bool markRead = false) -> ConstPtr<Codex>;

  [[nodiscard]] auto codexRead(String const& codexId) const -> bool;
  auto markCodexRead(String const& codexId) -> bool;
  auto markCodexUnread(String const& codexId) -> bool;

  void learnInitialCodexes(String const& playerSpecies);

  [[nodiscard]] auto firstNewCodex() const -> ConstPtr<Codex>;

private:
  StringMap<CodexEntry> m_codexes;
};

}// namespace Star
