#pragma once

#include "StarAssets.hpp"
#include "StarRect.hpp"
#include "StarMap.hpp"
#include "StarString.hpp"
#include "StarThread.hpp"
#include "StarAssetPath.hpp"
#include "StarTtlCache.hpp"

namespace Star {

class ImageMetadataDatabase;
using ImageMetadataDatabasePtr = SharedPtr<ImageMetadataDatabase>;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;

// Caches image size, image spaces, and nonEmptyRegion completely until a
// reload, does not expire cached values in a TTL based way like Assets,
// because they are expensive to compute and cheap to keep around.
class ImageMetadataDatabase {
public:
  ImageMetadataDatabase(AssetsConstPtr assets);
  [[nodiscard]] Vec2U imageSize(AssetPath const& path) const;
  [[nodiscard]] List<Vec2I> imageSpaces(AssetPath const& path, Vec2F position, float fillLimit, bool flip) const;
  [[nodiscard]] RectU nonEmptyRegion(AssetPath const& path) const;
  void cleanup() const;

private:
  // Removes image processing directives that don't affect image spaces /
  // non-empty regions.
  [[nodiscard]] static AssetPath filterProcessing(AssetPath const& path);

  [[nodiscard]] Vec2U calculateImageSize(AssetPath const& path) const;

  // Path, position, fillLimit, and flip
  using SpacesEntry = tuple<AssetPath, Vec2I, float, bool>;

  AssetsConstPtr m_assets;
  mutable Mutex m_mutex;
  mutable HashTtlCache<AssetPath, Vec2U> m_sizeCache;
  mutable HashTtlCache<SpacesEntry, List<Vec2I>> m_spacesCache;
  mutable HashTtlCache<AssetPath, RectU> m_regionCache;
};

}
