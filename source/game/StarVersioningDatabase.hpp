#pragma once

#include "StarDataStream.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarLuaRoot.hpp"
#include "StarThread.hpp"
#include "StarVersion.hpp"

namespace Star {

using VersionedJsonException = ExceptionDerived<"VersionedJsonException">;
using VersioningDatabaseException = ExceptionDerived<"VersioningDatabaseException">;

struct VersionedJson {
  static char const* const Magic;
  static size_t const MagicStringSize;
  static VersionNumber const SubVersioning;

  // Writes and reads a binary file containing a versioned json with a magic
  // header marking it as a starbound versioned json file.  Writes using a
  // safe write/flush/swap.
  static auto readFile(String const& filename) -> VersionedJson;
  static void writeFile(VersionedJson const& versionedJson, String const& filename);
  static void writeSubVersioning(DataStream& ds, VersionedJson const& versionedJson);
  static void readSubVersioning(DataStream& ds, VersionedJson& versionedJson);

  // Writes and reads a json containing a versioned json
  // This allows embedding versioned metadata within a file
  static auto fromJson(Json const& source) -> VersionedJson;
  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto empty() const -> bool;

  // If the identifier does not match the given identifier, throws a
  // VersionedJsonException.
  void expectIdentifier(String const& expectedIdentifier) const;

  String identifier;
  VersionNumber version;
  Json content;
  StringMap<VersionNumber> subVersions;
};

auto operator>>(DataStream& ds, VersionedJson& versionedJson) -> DataStream&;
auto operator<<(DataStream& ds, VersionedJson const& versionedJson) -> DataStream&;

class VersioningDatabase {
public:
  VersioningDatabase();

  // Converts the given content Json to a VersionedJson by marking it with the
  // given identifier and the current version configured in the versioning
  // config file.
  auto makeCurrentVersionedJson(String const& identifier, Json const& content) const -> VersionedJson;

  // Returns true if the version in this VersionedJson matches the configured
  // current version and does not need updating.
  auto versionedJsonCurrent(VersionedJson const& versionedJson) const -> bool;

  // Brings the given versioned json up to the current configured latest
  // version using update scripts.  If successful, returns the up to date
  // VersionedJson, otherwise throws VersioningDatabaseException.
  auto updateVersionedJson(VersionedJson const& versionedJson) const -> VersionedJson;

  // Convenience method, checkts the versionedJson expected identifier and then
  // brings the given versionedJson up to date and returns the content.
  auto loadVersionedJson(VersionedJson const& versionedJson, String const& expectedIdentifier) const -> Json;

private:
  struct VersionUpdateScript {
    String script;
    VersionNumber fromVersion;
    VersionNumber toVersion;
  };

  auto makeVersioningCallbacks() const -> LuaCallbacks;

  mutable RecursiveMutex m_mutex;
  mutable LuaRoot m_luaRoot;

  StringMap<VersionNumber> m_currentVersions;
  StringMap<List<VersionUpdateScript>> m_versionUpdateScripts;

  StringMap<StringMap<VersionNumber>> m_currentSubVersions;
  StringMap<HashMap<VersionNumber, StringMap<List<VersionUpdateScript>>>> m_subVersionUpdateScripts;
};

}// namespace Star
