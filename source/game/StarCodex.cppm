module;

import std;
import star.string;
import star.list;
import star.json;

namespace Star {

class Codex;
using CodexPtr = std::shared_ptr<Codex>;
using CodexConstPtr = std::shared_ptr<const Codex>;
using CodexWeakPtr = std::weak_ptr<Codex>;
using CodexConstWeakPtr = std::weak_ptr<const Codex>;
using CodexUPtr = std::unique_ptr<Codex>;
using CodexConstUPtr = std::unique_ptr<const Codex>;

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
  using ::Star::CodexPtr;
  using ::Star::CodexConstPtr;
  using ::Star::CodexWeakPtr;
  using ::Star::CodexConstWeakPtr;
  using ::Star::CodexUPtr;
  using ::Star::CodexConstUPtr;
}
