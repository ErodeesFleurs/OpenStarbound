module;
import std;
import star.string;
import star.list;
import star.json;

import star.data_stream;
import star.uuid;

namespace Star {

class Codex;
using CodexPtr = std::shared_ptr<Codex>;
using CodexConstPtr = std::shared_ptr<const Codex>;
using CodexWeakPtr = std::weak_ptr<Codex>;
using CodexConstWeakPtr = std::weak_ptr<const Codex>;
using CodexUPtr = std::unique_ptr<Codex>;
using CodexConstUPtr = std::unique_ptr<const Codex>;
struct CodexEntry;
using CodexEntryPtr = std::shared_ptr<CodexEntry>;
using CodexEntryConstPtr = std::shared_ptr<const CodexEntry>;
using CodexEntryWeakPtr = std::weak_ptr<CodexEntry>;
using CodexEntryConstWeakPtr = std::weak_ptr<const CodexEntry>;
using CodexEntryUPtr = std::unique_ptr<CodexEntry>;
using CodexEntryConstUPtr = std::unique_ptr<const CodexEntry>;
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<const UniverseClient>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<const UniverseClient>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<const UniverseClient>;

class PlayerCodexes {
public:
  typedef std::pair<CodexConstPtr, bool> CodexEntry;

  PlayerCodexes();
  PlayerCodexes(Json const& json);

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
  StringMap<CodexEntry> m_codexes;
};

typedef std::shared_ptr<PlayerCodexes> PlayerCodexesPtr;
}

export module star.player_codexes;

export namespace Star {
  using ::Star::Codex;
  using ::Star::CodexPtr;
  using ::Star::CodexConstPtr;
  using ::Star::CodexWeakPtr;
  using ::Star::CodexConstWeakPtr;
  using ::Star::CodexUPtr;
  using ::Star::CodexConstUPtr;
  using ::Star::CodexEntry;
  using ::Star::CodexEntryPtr;
  using ::Star::CodexEntryConstPtr;
  using ::Star::CodexEntryWeakPtr;
  using ::Star::CodexEntryConstWeakPtr;
  using ::Star::CodexEntryUPtr;
  using ::Star::CodexEntryConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::PlayerCodexes;
  using ::Star::PlayerCodexesPtr;
}
