module;

import std;
import star.string;
import star.list;
import star.json;

namespace Star {

class Codex {
public:
  Codex(Json const& config, String const& path);
  Json toJson() const;

  String id() const;
  String species() const;
  String title() const;
  String description() const;
  String icon() const;
  String page(std::size_t pageNum) const;
  List<String> pages() const;
  std::size_t pageCount() const;
  Json itemConfig() const;
  String directory() const;
  String filename() const;

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

export module star.codex;

export namespace Star {
  using ::Star::Codex;
}
