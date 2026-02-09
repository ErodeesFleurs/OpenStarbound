#include "StarAssetPath.hpp"

import std;

namespace Star {

// The filename is everything after the last slash (excluding directives) and
// up to the first directive marker.
static auto findFilenameRange(std::string const& pathUtf8) -> std::optional<std::pair<std::size_t, std::size_t>> {
  std::size_t firstDirectiveOrSubPath = pathUtf8.find_first_of(":?");
  std::size_t filenameStart = 0;
  while (true) {
    std::size_t find = pathUtf8.find('/', filenameStart);
    if (find >= firstDirectiveOrSubPath)
      break;
    filenameStart = find + 1;
  }

  if (filenameStart == std::numeric_limits<std::size_t>::max()) {
    return std::nullopt;
  } else if (firstDirectiveOrSubPath == std::numeric_limits<std::size_t>::max()) {
    return {{filenameStart, pathUtf8.size()}};
  } else {
    return {{filenameStart, firstDirectiveOrSubPath}};
  }
}

auto AssetPath::split(String const& path) -> AssetPath {
  AssetPath components;

  std::string const& str = path.utf8();

  //base paths cannot have any ':' or '?' characters, stop at the first one.
  std::size_t end = str.find_first_of(":?");
  components.basePath = str.substr(0, end);

  if (end == std::numeric_limits<std::size_t>::max())
    return components;

  // Sub-paths must immediately follow base paths and must start with a ':',
  // after this point any further ':' characters are not special.
  if (str[end] == ':') {
    std::size_t beg = end + 1;
    if (beg != str.size()) {
      end = str.find_first_of('?', beg);
      if (end == std::numeric_limits<std::size_t>::max() && beg + 1 != str.size())
        components.subPath.emplace(str.substr(beg));
      else if (std::size_t len = end - beg)
        components.subPath.emplace(str.substr(beg, len));
    }
  }

  if (end == std::numeric_limits<std::size_t>::max())
    return components;

  // Directives must follow the base path and optional sub-path, and each
  // directive is separated by one or more '?' characters.
  if (str[end] == '?')
    components.directives = String(str.substr(end));

  return components;
}

auto AssetPath::join(AssetPath const& components) -> String {
  return toString(components);
}

auto AssetPath::setSubPath(String const& path, String const& subPath) -> String {
  auto components = split(path);
  components.subPath = subPath;
  return join(components);
}

auto AssetPath::removeSubPath(String const& path) -> String {
  auto components = split(path);
  components.subPath.reset();
  return join(components);
}

auto AssetPath::getDirectives(String const& path) -> String {
  std::size_t firstDirective = path.find('?');
  if (firstDirective == std::numeric_limits<std::size_t>::max())
    return {};
  return path.substr(firstDirective + 1);
}

auto AssetPath::addDirectives(String const& path, String const& directives) -> String {
  return String::joinWith("?", path, directives);
}

auto AssetPath::removeDirectives(String const& path) -> String {
  std::size_t firstDirective = path.find('?');
  if (firstDirective == std::numeric_limits<std::size_t>::max())
    return path;
  return path.substr(0, firstDirective);
}

auto AssetPath::directory(String const& path) -> String {
  if (auto p = findFilenameRange(path.utf8())) {
    return {path.utf8().substr(0, p->first)};
  } else {
    return {};
  }
}

auto AssetPath::filename(String const& path) -> String {
  if (auto p = findFilenameRange(path.utf8())) {
    return {path.utf8().substr(p->first, p->second)};
  } else {
    return {};
  }
}

auto AssetPath::extension(String const& path) -> String {
  auto file = filename(path);
  auto lastDot = file.findLast(".");
  if (lastDot == std::numeric_limits<std::size_t>::max())
    return "";

  return file.substr(lastDot + 1);
}

auto AssetPath::relativeTo(String const& sourcePath, String const& givenPath) -> String {
  if (!givenPath.empty() && givenPath[0] == '/')
    return givenPath;

  auto path = directory(sourcePath);
  path.append(givenPath);
  return path;
}

auto AssetPath::operator==(AssetPath const& rhs) const -> bool {
  return tie(basePath, subPath, directives) == tie(rhs.basePath, rhs.subPath, rhs.directives);
}

AssetPath::AssetPath(const char* path) {
  *this = AssetPath::split(path);
}

AssetPath::AssetPath(String const& path) {
  *this = AssetPath::split(path);
}

AssetPath::AssetPath(String&& basePath, std::optional<String>&& subPath, DirectivesGroup&& directives) {
  this->basePath = std::move(basePath);
  this->subPath = std::move(subPath);
  this->directives = std::move(directives);
}

AssetPath::AssetPath(String const& basePath, const std::optional<String>& subPath, DirectivesGroup const& directives) {
  this->basePath = basePath;
  this->subPath = subPath;
  this->directives = directives;
}

auto operator<<(std::ostream& os, AssetPath const& rhs) -> std::ostream& {
  os << rhs.basePath;
  if (rhs.subPath) {
    os << ":";
    os << *rhs.subPath;
  }

  rhs.directives.forEach([&](Directives::Entry const& entry, Directives const& directives) -> void {
    os << "?";
    os << entry.string(*directives);
  });

  return os;
}

auto hash<AssetPath>::operator()(AssetPath const& s) const -> std::size_t {
  return hashOf(s.basePath, s.subPath, s.directives);
}

auto operator>>(DataStream& ds, AssetPath& path) -> DataStream& {
  String string;
  ds.read(string);

  path = std::move(string);

  return ds;
}

auto operator<<(DataStream& ds, AssetPath const& path) -> DataStream& {
  ds.write(AssetPath::join(path));

  return ds;
}

}// namespace Star
