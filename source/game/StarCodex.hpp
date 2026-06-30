#pragma once

#include "StarJson.hpp"

namespace Star {

class Codex;
using CodexConstPtr = SharedPtr<Codex const>;

class Codex {
public:
  Codex(Json const& config, String const& path);
  [[nodiscard]] Json toJson() const;

  [[nodiscard]] String id() const;
  [[nodiscard]] String species() const;
  [[nodiscard]] String title() const;
  [[nodiscard]] String description() const;
  [[nodiscard]] String icon() const;
  [[nodiscard]] String page(size_t pageNum) const;
  [[nodiscard]] List<String> pages() const;
  [[nodiscard]] size_t pageCount() const;
  [[nodiscard]] Json itemConfig() const;
  [[nodiscard]] String directory() const;
  [[nodiscard]] String filename() const;

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

}
