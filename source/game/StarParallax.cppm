module;

#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarBiMap.hpp"
import star.tile_damage;

import star.plant_database;

namespace Star {

STAR_CLASS(Parallax);
STAR_STRUCT(ParallaxLayer);

struct ParallaxLayer {
  friend DataStream& operator>>(DataStream& ds, ParallaxLayer& parallaxLayer);
  friend DataStream& operator<<(DataStream& ds, ParallaxLayer const& parallaxLayer);

  ParallaxLayer();
  ParallaxLayer(Json const& store);

  Json store() const;

  void addImageDirectives(Directives const& newDirectives);
  void fadeToSkyColor(Color skyColor);

  List<String> textures;
  Directives directives;
  unsigned frameNumber;
  int frameOffset;
  float animationCycle;
  float alpha;
  Vec2F parallaxValue;
  Vec2B repeat;
  Maybe<float> tileLimitTop;
  Maybe<float> tileLimitBottom;
  float verticalOrigin;
  float zLevel;
  Vec2F parallaxOffset;
  String timeOfDayCorrelation;
  Vec2F speed;
  bool followsWind;
  Maybe<float> windSpeedMultiplier;
  bool unlit;
  bool lightMapped;
  float fadePercent;
};
typedef List<ParallaxLayer> ParallaxLayers;

DataStream& operator>>(DataStream& ds, ParallaxLayer& parallaxLayer);
DataStream& operator<<(DataStream& ds, ParallaxLayer const& parallaxLayer);

// Object managing and rendering the parallax for a World
class Parallax {
public:
  Parallax(String const& assetFile,
      uint64_t seed,
      float verticalOrigin,
      float hueShift,
      Maybe<TreeVariant> parallaxTreeVariant = {});
  Parallax(Json const& store);

  Json store() const;

  void fadeToSkyColor(Color const& skyColor);
  ParallaxPtr createOverlay(String const& assetFile) const;

  ParallaxLayers const& layers() const;

private:
  void buildLayer(Json const& layerSettings, String const& kind);

  uint64_t m_seed;
  float m_verticalOrigin;
  Maybe<TreeVariant> m_parallaxTreeVariant;
  float m_hueShift;

  String m_imageDirectory;

  ParallaxLayers m_layers;
};

}

export module star.parallax;

export namespace Star {
  using ::Star::Parallax;
  using ::Star::ParallaxPtr;
  using ::Star::ParallaxConstPtr;
  using ::Star::ParallaxWeakPtr;
  using ::Star::ParallaxConstWeakPtr;
  using ::Star::ParallaxUPtr;
  using ::Star::ParallaxConstUPtr;
  using ::Star::ParallaxLayer;
  using ::Star::ParallaxLayerPtr;
  using ::Star::ParallaxLayerConstPtr;
  using ::Star::ParallaxLayerWeakPtr;
  using ::Star::ParallaxLayerConstWeakPtr;
  using ::Star::ParallaxLayerUPtr;
  using ::Star::ParallaxLayerConstUPtr;
  using ::Star::ParallaxLayers;
}
