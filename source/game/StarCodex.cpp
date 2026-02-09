#include "StarCodex.hpp"

#include "StarAssetPath.hpp"
#include "StarJsonExtra.hpp"

import std;

namespace Star {

Codex::Codex(Json const& config, String const& path) {
  m_directory = AssetPath::directory(path);
  m_filename = AssetPath::filename(path);
  m_id = config.getString("id");
  m_species = config.getString("species", "other");
  m_title = config.getString("title");
  m_description = config.getString("description", "");
  m_icon = config.getString("icon");
  m_pages = jsonToStringList(config.get("contentPages"));
  m_itemConfig = config.get("itemConfig", Json());
}

auto Codex::toJson() const -> Json {
  auto result = JsonObject{
    {"id", m_id},
    {"species", m_species},
    {"title", m_title},
    {"description", m_description},
    {"icon", m_icon},
    {"contentPages", jsonFromStringList(m_pages)},
    {"itemConfig", m_itemConfig}};
  return result;
}

auto Codex::id() const -> String {
  return m_id;
}

auto Codex::species() const -> String {
  return m_species;
}

auto Codex::title() const -> String {
  return m_title;
}

auto Codex::description() const -> String {
  return m_description;
}

auto Codex::icon() const -> String {
  return m_icon;
}

auto Codex::page(std::size_t pageNum) const -> String {
  if (pageNum < m_pages.size())
    return m_pages[pageNum];
  return "";
}

auto Codex::pages() const -> List<String> {
  return m_pages;
}

auto Codex::pageCount() const -> std::size_t {
  return m_pages.size();
}

auto Codex::itemConfig() const -> Json {
  return m_itemConfig;
}

auto Codex::directory() const -> String {
  return m_directory;
}

auto Codex::filename() const -> String {
  return m_filename;
}

}// namespace Star
