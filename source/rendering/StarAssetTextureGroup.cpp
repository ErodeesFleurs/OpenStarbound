#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarBiMap.hpp"
import star.listener;
#include "StarIterator.hpp"
import star.time;
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;

#include "StarVariant.hpp"
#include "StarImage.hpp"
#include "StarPoly.hpp"
import star.renderer;
import star.asset_texture_group;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;


import star.image_metadata_database;

namespace Star {

AssetTextureGroup::AssetTextureGroup(TextureGroupPtr textureGroup)
  : m_textureGroup(std::move(textureGroup)) {
  m_reloadTracker = make_shared<TrackerListener>();
  Root::singleton().registerReloadListener(m_reloadTracker);
}

TexturePtr AssetTextureGroup::loadTexture(AssetPath const& imagePath) {
  return loadTexture(imagePath, false);
}

TexturePtr AssetTextureGroup::tryTexture(AssetPath const& imagePath) {
  return loadTexture(imagePath, true);
}

bool AssetTextureGroup::textureLoaded(AssetPath const& imagePath) const {
  return m_textureMap.contains(imagePath);
}

void AssetTextureGroup::cleanup(int64_t textureTimeout) {
  if (m_reloadTracker->pullTriggered()) {
    m_textureMap.clear();
    m_textureDeduplicationMap.clear();

  } else {
    int64_t time = Time::monotonicMilliseconds();

    List<Texture const*> liveTextures;
    filter(m_textureMap, [&](auto const& pair) {
        if (time - pair.second.second < textureTimeout) {
          liveTextures.append(pair.second.first.get());
          return true;
        }
        return false;
      });

    liveTextures.sort();

    eraseWhere(m_textureDeduplicationMap, [&](auto const& p) {
        return !liveTextures.containsSorted(p.second.get());
      });
  }
}

TexturePtr AssetTextureGroup::loadTexture(AssetPath const& imagePath, bool tryTexture) {
  if (auto p = m_textureMap.ptr(imagePath)) {
    p->second = Time::monotonicMilliseconds();
    return p->first;
  }

  auto assets = Root::singleton().assets();

  ImageConstPtr image;
  if (tryTexture)
    image = assets->tryImage(imagePath);
  else
    image = assets->image(imagePath);

  if (!image)
    return {};

  // Assets will return the same image ptr if two different asset paths point
  // to the same underlying cached image.  We should not make duplicate entries
  // in the texture group for these, so we keep track of the image pointers
  // returned to deduplicate them.
  if (auto existingTexture = m_textureDeduplicationMap.value(image)) {
    m_textureMap.add(imagePath, {existingTexture, Time::monotonicMilliseconds()});
    return existingTexture;
  } else {
    auto texture = m_textureGroup->create(*image);
    m_textureMap.add(imagePath, {texture, Time::monotonicMilliseconds()});
    m_textureDeduplicationMap.add(image, texture);
    return texture;
  }
}

}
