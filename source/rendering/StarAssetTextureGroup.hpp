#pragma once

#include "StarAssetPath.hpp"
#include "StarConfig.hpp"
#include "StarListener.hpp"
#include "StarRenderer.hpp"

import std;

namespace Star {

// Creates a renderer texture group for textures loaded directly from Assets.
class AssetTextureGroup {
public:
  // Creates a texture group using the given renderer and textureFiltering for
  // the managed textures.
  AssetTextureGroup(Ptr<TextureGroup> textureGroup);

  // Load the given texture into the texture group if it is not loaded, and
  // return the texture pointer.
  auto loadTexture(AssetPath const& imagePath) -> RefPtr<Texture>;

  // If the texture is loaded and ready, returns the texture pointer, otherwise
  // queues the texture using Assets::tryImage and returns nullptr.
  auto tryTexture(AssetPath const& imagePath) -> RefPtr<Texture>;

  // Has the texture been loaded?
  [[nodiscard]] auto textureLoaded(AssetPath const& imagePath) const -> bool;

  // Frees textures that haven't been used in more than 'textureTimeout' time.
  // If Root has been reloaded, will simply clear the texture group.
  void cleanup(std::int64_t textureTimeout);

private:
  // Returns the texture parameters.  If tryTexture is true, then returns none
  // if the texture is not loaded, and queues it, otherwise loads texture
  // immediately
  auto loadTexture(AssetPath const& imagePath, bool tryTexture) -> RefPtr<Texture>;

  Ptr<TextureGroup> m_textureGroup;
  HashMap<AssetPath, std::pair<RefPtr<Texture>, std::int64_t>> m_textureMap;
  HashMap<ConstPtr<Image>, RefPtr<Texture>> m_textureDeduplicationMap;
  Ptr<TrackerListener> m_reloadTracker;
};

}// namespace Star
