#pragma once

#include "StarAssets.hpp"
#include "StarUuid.hpp"
#include "StarJson.hpp"

namespace Star {

class Codex;
using CodexConstPtr = SharedPtr<Codex const>;
class CodexDatabase;
using CodexDatabaseConstPtr = SharedPtr<CodexDatabase const>;

class PlayerCodexes {
public:
  struct CodexEntry {
    CodexConstPtr codex;
    bool read;

    bool operator==(CodexEntry const&) const = default;
  };

  PlayerCodexes(AssetsConstPtr assets, CodexDatabaseConstPtr codexDatabase, Json const& json = {});

  Json toJson() const;

  List<CodexEntry> codexes() const;

  bool codexKnown(String const& codexId) const;
  CodexConstPtr learnCodex(String const& codexId, bool markRead = false);

  bool codexRead(String const& codexId) const;
  bool markCodexRead(String const& codexId);
  bool markCodexUnread(String const& codexId);

  void learnInitialCodexes(String const& playerSpecies);

  CodexConstPtr firstNewCodex() const;

private:
  AssetsConstPtr m_assets;
  CodexDatabaseConstPtr m_codexDatabase;
  StringMap<CodexEntry> m_codexes;
};

using PlayerCodexesPtr = SharedPtr<PlayerCodexes>;
}
