#pragma once

#include "StarJson.hpp"

import std;

namespace Star {

class Codex {
public:
  Codex(Json const& config, String const& path);
  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto id() const -> String;
  [[nodiscard]] auto species() const -> String;
  [[nodiscard]] auto title() const -> String;
  [[nodiscard]] auto description() const -> String;
  [[nodiscard]] auto icon() const -> String;
  [[nodiscard]] auto page(std::size_t pageNum) const -> String;
  [[nodiscard]] auto pages() const -> List<String>;
  [[nodiscard]] auto pageCount() const -> std::size_t;
  [[nodiscard]] auto itemConfig() const -> Json;
  [[nodiscard]] auto directory() const -> String;
  [[nodiscard]] auto filename() const -> String;

private:
  String m_id;
  String m_species;
  String m_title;
  String m_description;
  String m_icon;
  List<String> m_pages;
  Json m_itemConfig;
  String m_directory;
  String m_filename;
};

}// namespace Star
