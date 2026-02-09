#include "StarAssetTextureGroup.hpp"

#include "StarConfig.hpp"
#include "StarRoot.hpp"
#include "StarTime.hpp"

import std;

namespace Star {

AssetTextureGroup::AssetTextureGroup(Ptr<TextureGroup> textureGroup)
    : m_textureGroup(std::move(textureGroup)) {
  m_reloadTracker = std::make_shared<TrackerListener>();
  Root::singleton().registerReloadListener(m_reloadTracker);
}

auto AssetTextureGroup::loadTexture(AssetPath const& imagePath) -> RefPtr<Texture> {
  return loadTexture(imagePath, false);
}

auto AssetTextureGroup::tryTexture(AssetPath const& imagePath) -> RefPtr<Texture> {
  return loadTexture(imagePath, true);
}

auto AssetTextureGroup::textureLoaded(AssetPath const& imagePath) const -> bool {
  return m_textureMap.contains(imagePath);
}

void AssetTextureGroup::cleanup(std::int64_t textureTimeout) {
  if (m_reloadTracker->pullTriggered()) {
    m_textureMap.clear();
    m_textureDeduplicationMap.clear();

  } else {
    std::int64_t time = Time::monotonicMilliseconds();

    List<Texture const*> liveTextures;
    filter(m_textureMap, [&](auto const& pair) -> auto {
      if (time - pair.second.second < textureTimeout) {
        liveTextures.append(pair.second.first.get());
        return true;
      }
      return false;
    });

    liveTextures.sort();

    eraseWhere(m_textureDeduplicationMap, [&](auto const& p) -> auto {
      return !liveTextures.containsSorted(p.second.get());
    });
  }
}

auto AssetTextureGroup::loadTexture(AssetPath const& imagePath, bool tryTexture) -> RefPtr<Texture> {
  if (auto p = m_textureMap.ptr(imagePath)) {
    p->second = Time::monotonicMilliseconds();
    return p->first;
  }

  auto assets = Root::singleton().assets();

  ConstPtr<Image> image;
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

}// namespace Star
