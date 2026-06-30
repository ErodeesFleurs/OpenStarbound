#pragma once

#include "StarAssetPath.hpp"
#include "StarByteArray.hpp"
#include "StarJson.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarVector.hpp"
#include "StarImage.hpp"

namespace Star {

class Audio;
using AudioConstPtr = SharedPtr<Audio const>;
class Font;
using FontConstPtr = SharedPtr<Font const>;

class IAssets {
public:
  virtual ~IAssets() = default;

  virtual Json json(String const& path) const = 0;
  virtual Json fetchJson(Json const& v, String const& dir = "/") const = 0;

  virtual ImageConstPtr image(AssetPath const& path) const = 0;
  virtual ImageConstPtr tryImage(AssetPath const& path) const = 0;

  virtual AudioConstPtr audio(String const& path) const = 0;

  virtual ByteArrayConstPtr bytes(String const& path) const = 0;

  virtual FontConstPtr font(String const& path) const = 0;

  virtual ByteArray digest() const = 0;

  virtual bool assetExists(String const& path) const = 0;
  virtual StringList scan(String const& suffix) const = 0;
  virtual StringList scan(String const& prefix, String const& suffix) const = 0;
  virtual StringList assetSources() const = 0;
};

using IAssetsPtr = SharedPtr<IAssets>;
using IAssetsConstPtr = SharedPtr<IAssets const>;

}

