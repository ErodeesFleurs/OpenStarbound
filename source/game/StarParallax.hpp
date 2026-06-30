#pragma once

#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarPlantDatabase.hpp"
#include "StarDirectives.hpp"
#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

class Parallax;
using ParallaxPtr = SharedPtr<Parallax>;
struct ParallaxLayer;

struct ParallaxLayer {
  ParallaxLayer();
  ParallaxLayer(Json const& store);

  Json store() const;

  void addImageDirectives(Directives const& newDirectives);
  void fadeToSkyColor(Color skyColor);

  List<String> textures;
  Directives directives;
  unsigned frameNumber = 1;
  int frameOffset = 0;
  float animationCycle = 1.0f;
  float alpha = 1.0f;
  Vec2F parallaxValue;
  Vec2B repeat;
  Maybe<float> tileLimitTop;
  Maybe<float> tileLimitBottom;
  float verticalOrigin = 0.0f;
  float zLevel = 0.0f;
  Vec2F parallaxOffset;
  String timeOfDayCorrelation;
  Vec2F speed;
  bool unlit = false;
  bool lightMapped = false;
  float fadePercent = 0.0f;
};
using ParallaxLayers = List<ParallaxLayer>;

DataStream& operator>>(DataStream& ds, ParallaxLayer& parallaxLayer);
DataStream& operator<<(DataStream& ds, ParallaxLayer const& parallaxLayer);

class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;

// Object managing and rendering the parallax for a World
class Parallax {
public:
  Parallax(AssetsConstPtr assets,
      ImageMetadataDatabaseConstPtr imageMetadataDatabase,
      String const& assetFile,
      uint64_t seed,
      float verticalOrigin,
      float hueShift,
      Maybe<TreeVariant> parallaxTreeVariant = {});
  Parallax(Json const& store);

  Json store() const;

  void fadeToSkyColor(Color const& skyColor);

  ParallaxLayers const& layers() const;

private:
  void buildLayer(Json const& layerSettings, String const& kind);

  uint64_t m_seed;
  float m_verticalOrigin;
  Maybe<TreeVariant> m_parallaxTreeVariant;
  float m_hueShift;

  String m_imageDirectory;

  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;

  ParallaxLayers m_layers;
};

}
