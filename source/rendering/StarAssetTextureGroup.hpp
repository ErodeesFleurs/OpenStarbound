#pragma once

#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarBiMap.hpp"
#include "StarListener.hpp"
#include "StarRenderer.hpp"
#include "StarAssetPath.hpp"

namespace Star {

class AssetTextureGroup;
using AssetTextureGroupPtr = SharedPtr<AssetTextureGroup>;
class Assets;
using AssetsConstPtr = SharedPtr<Assets const>;

// Creates a renderer texture group for textures loaded directly from Assets.
class AssetTextureGroup {
public:
  // Creates a texture group using the given renderer and textureFiltering for
  // the managed textures.
  explicit AssetTextureGroup(TextureGroupPtr textureGroup, AssetsConstPtr assets, function<void(ListenerWeakPtr)> registerReloadListener);

  // Load the given texture into the texture group if it is not loaded, and
  // return the texture pointer.
  TexturePtr loadTexture(AssetPath const& imagePath);

  // If the texture is loaded and ready, returns the texture pointer, otherwise
  // queues the texture using Assets::tryImage and returns nullptr.
  TexturePtr tryTexture(AssetPath const& imagePath);

  // Has the texture been loaded?
  bool textureLoaded(AssetPath const& imagePath) const;

  // Frees textures that haven't been used in more than 'textureTimeout' time.
  // If Root has been reloaded, will simply clear the texture group.
  void cleanup(int64_t textureTimeout);

private:
  // Returns the texture parameters.  If tryTexture is true, then returns none
  // if the texture is not loaded, and queues it, otherwise loads texture
  // immediately
  TexturePtr loadTexture(AssetPath const& imagePath, bool tryTexture);

  TextureGroupPtr m_textureGroup;
  AssetsConstPtr m_assets;
  struct TextureCacheEntry {
    TexturePtr texture;
    int64_t lastUsedTime;
  };
  HashMap<AssetPath, TextureCacheEntry> m_textureMap;
  HashMap<ImageConstPtr, TexturePtr> m_textureDeduplicationMap;
  TrackerListenerPtr m_reloadTracker;
};

}
