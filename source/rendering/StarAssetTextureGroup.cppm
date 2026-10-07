module;

#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
import star.listener;
#include "StarVariant.hpp"
#include "StarImage.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarRefPtr.hpp"
import star.renderer;
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;

namespace Star {

STAR_CLASS(AssetTextureGroup);

// Creates a renderer texture group for textures loaded directly from Assets.
class AssetTextureGroup {
public:
  // Creates a texture group using the given renderer and textureFiltering for
  // the managed textures.
  AssetTextureGroup(TextureGroupPtr textureGroup);

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
  HashMap<AssetPath, pair<TexturePtr, int64_t>> m_textureMap;
  HashMap<ImageConstPtr, TexturePtr> m_textureDeduplicationMap;
  TrackerListenerPtr m_reloadTracker;
};

}

export module star.asset_texture_group;

export namespace Star {
  using ::Star::AssetTextureGroup;
  using ::Star::AssetTextureGroupPtr;
  using ::Star::AssetTextureGroupConstPtr;
  using ::Star::AssetTextureGroupWeakPtr;
  using ::Star::AssetTextureGroupConstWeakPtr;
  using ::Star::AssetTextureGroupUPtr;
  using ::Star::AssetTextureGroupConstUPtr;
}
