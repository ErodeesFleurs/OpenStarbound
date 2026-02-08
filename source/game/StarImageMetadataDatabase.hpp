#pragma once

#include "StarAssetPath.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarTtlCache.hpp"

import std;

namespace Star {

// Caches image size, image spaces, and nonEmptyRegion completely until a
// reload, does not expire cached values in a TTL based way like Assets,
// because they are expensive to compute and cheap to keep around.
class ImageMetadataDatabase {
public:
  ImageMetadataDatabase();
  auto imageSize(AssetPath const& path) const -> Vec2U;
  auto imageSpaces(AssetPath const& path, Vec2F position, float fillLimit, bool flip) const -> List<Vec2I>;
  auto nonEmptyRegion(AssetPath const& path) const -> RectU;
  void cleanup() const;

private:
  // Removes image processing directives that don't affect image spaces /
  // non-empty regions.
  static auto filterProcessing(AssetPath const& path) -> AssetPath;

  auto calculateImageSize(AssetPath const& path) const -> Vec2U;

  // Path, position, fillLimit, and flip
  using SpacesEntry = std::tuple<AssetPath, Vec2I, float, bool>;

  mutable Mutex m_mutex;
  mutable HashTtlCache<AssetPath, Vec2U> m_sizeCache;
  mutable HashTtlCache<SpacesEntry, List<Vec2I>> m_spacesCache;
  mutable HashTtlCache<AssetPath, RectU> m_regionCache;
};

}// namespace Star
