module;
import std;
import star.string;
import star.json;

import star.maybe;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.vector;
import star.bi_map;
import star.tile_damage;

import star.plant_database;

namespace Star {

class Parallax;
using ParallaxPtr = std::shared_ptr<Parallax>;
using ParallaxConstPtr = std::shared_ptr<const Parallax>;
using ParallaxWeakPtr = std::weak_ptr<Parallax>;
using ParallaxConstWeakPtr = std::weak_ptr<const Parallax>;
using ParallaxUPtr = std::unique_ptr<Parallax>;
using ParallaxConstUPtr = std::unique_ptr<const Parallax>;
struct ParallaxLayer;
using ParallaxLayerPtr = std::shared_ptr<ParallaxLayer>;
using ParallaxLayerConstPtr = std::shared_ptr<const ParallaxLayer>;
using ParallaxLayerWeakPtr = std::weak_ptr<ParallaxLayer>;
using ParallaxLayerConstWeakPtr = std::weak_ptr<const ParallaxLayer>;
using ParallaxLayerUPtr = std::unique_ptr<ParallaxLayer>;
using ParallaxLayerConstUPtr = std::unique_ptr<const ParallaxLayer>;

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
      std::uint64_t seed,
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

  std::uint64_t m_seed;
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
