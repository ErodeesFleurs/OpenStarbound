import star.json;
#include "StarJsonExtra.hpp"
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
import star.string;
import star.format;
import star.version;

import star.codex;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.codex_database;

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

StringMap<CodexConstPtr> CodexDatabase::codexes() const {
  return m_codexes;
}

CodexConstPtr CodexDatabase::codex(String const& codexId) const {
  if (auto codex = m_codexes.maybe(codexId))
    return codex.take();
  return {};
}

}
