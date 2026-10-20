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

class Root;

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
  Parallax(Root& root, String const& assetFile,
      std::uint64_t seed,
      float verticalOrigin,
      float hueShift,
      Maybe<TreeVariant> parallaxTreeVariant = {});
  Parallax(Root& root, Json const& store);

  Json store() const;

  void fadeToSkyColor(Color const& skyColor);
  std::shared_ptr<Parallax> createOverlay(String const& assetFile) const;

  ParallaxLayers const& layers() const;

private:
  void buildLayer(Json const& layerSettings, String const& kind);

  Root& m_root;
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
  using ::Star::ParallaxLayer;
  using ::Star::ParallaxLayers;
}
