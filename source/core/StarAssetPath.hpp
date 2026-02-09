#pragma once

#include "StarDataStream.hpp"
#include "StarDirectives.hpp"
#include "StarHash.hpp"

import std;

namespace Star {

// Asset paths are not filesystem paths.  '/' is always the directory separator,
// and it is not possible to escape any asset source directory.  '\' is never a
// valid directory separator.  All asset paths are considered case-insensitive.
//
// In addition to the path portion of the asset path, some asset types may also
// have a sub-path, which is always separated from the path portion of the asset
// by ':'.  There can be at most 1 sub-path component.
//
// Image paths may also have a directives portion of the full asset path, which
// must come after the path and optional sub-path comopnent.  The directives
// portion of the path starts with a '?', and '?' separates each subsquent
// directive.
struct AssetPath {
  static auto split(String const& path) -> AssetPath;
  static auto join(AssetPath const& path) -> String;

  // Get / modify sub-path directly on a joined path string
  static auto setSubPath(String const& joinedPath, String const& subPath) -> String;
  static auto removeSubPath(String const& joinedPath) -> String;

  // Get / modify directives directly on a joined path string
  static auto getDirectives(String const& joinedPath) -> String;
  static auto addDirectives(String const& joinedPath, String const& directives) -> String;
  static auto removeDirectives(String const& joinedPath) -> String;

  // The base directory name for any given path, including the trailing '/'.
  // Ignores sub-path and directives.
  static auto directory(String const& path) -> String;

  // The file part of any given path, ignoring sub-path and directives.  Path
  // must be a file not a directory.
  static auto filename(String const& path) -> String;

  // The file extension of a given file path, ignoring directives and
  // sub-paths.
  static auto extension(String const& path) -> String;

  // Computes an absolute asset path from a relative path relative to another
  // asset.  The sourcePath must be an absolute path (may point to a directory
  // or an asset in a directory, and ignores ':' sub-path or ?  directives),
  // and the givenPath may be either an absolute *or* a relative path.  If it
  // is an absolute path, it is returned unchanged.  If it is a relative path,
  // then it is computed as relative to the directory component of the
  // sourcePath.
  static auto relativeTo(String const& sourcePath, String const& givenPath) -> String;

  AssetPath() = default;
  AssetPath(const char* path);
  AssetPath(String const& path);
  AssetPath(String&& basePath, std::optional<String>&& subPath, DirectivesGroup&& directives);
  AssetPath(const String& basePath, const std::optional<String>& subPath, const DirectivesGroup& directives);
  String basePath;
  std::optional<String> subPath;
  DirectivesGroup directives;

  auto operator==(AssetPath const& rhs) const -> bool;
};

auto operator>>(DataStream& ds, AssetPath& path) -> DataStream&;
auto operator<<(DataStream& ds, AssetPath const& path) -> DataStream&;

auto operator<<(std::ostream& os, AssetPath const& rhs) -> std::ostream&;

template <>
struct hash<AssetPath> {
  auto operator()(AssetPath const& s) const -> std::size_t;
};

}// namespace Star

template <>
struct std::formatter<Star::AssetPath> : Star::ostream_formatter {};
