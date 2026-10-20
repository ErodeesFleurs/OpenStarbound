import star.json;
import star.json_extra;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.optional;
import star.string;
import star.format;
import star.version;

import star.codex;
import star.asset_source;
import star.assets;

import star.codex_database;

namespace Star {

CodexDatabase::CodexDatabase(std::shared_ptr<Assets const> assets) {
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
    } catch (std::exception const&) {
      ExceptionDiagnostic const e(std::current_exception());
      throw CodexDatabaseException(strf("Error reading codex config {}", file), e);
    }
  }
}

StringMap<std::shared_ptr<Codex const>> CodexDatabase::codexes() const {
  return m_codexes;
}

std::shared_ptr<Codex const> CodexDatabase::codex(String const& codexId) const {
  if (auto codex = m_codexes.maybe(codexId))
    return takeOptional(codex);
  return {};
}

}
