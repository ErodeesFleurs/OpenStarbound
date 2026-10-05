module;

#include "StarUuid.hpp"
#include "StarJson.hpp"

namespace Star {

STAR_CLASS(Codex);
STAR_STRUCT(CodexEntry);
STAR_CLASS(UniverseClient);

class PlayerCodexes {
public:
  typedef pair<CodexConstPtr, bool> CodexEntry;

  PlayerCodexes(Json const& json = {});

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

typedef shared_ptr<PlayerCodexes> PlayerCodexesPtr;
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
