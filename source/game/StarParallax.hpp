#pragma once

#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarPlantDatabase.hpp"

import std;

namespace Star {

struct ParallaxLayer {
  ParallaxLayer();
  ParallaxLayer(Json const& store);

  [[nodiscard]] auto store() const -> Json;

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
  std::optional<float> tileLimitTop;
  std::optional<float> tileLimitBottom;
  float verticalOrigin;
  float zLevel;
  Vec2F parallaxOffset;
  String timeOfDayCorrelation;
  Vec2F speed;
  bool unlit;
  bool lightMapped;
  float fadePercent;
};
using ParallaxLayers = List<ParallaxLayer>;

auto operator>>(DataStream& ds, ParallaxLayer& parallaxLayer) -> DataStream&;
auto operator<<(DataStream& ds, ParallaxLayer const& parallaxLayer) -> DataStream&;

// Object managing and rendering the parallax for a World
class Parallax {
public:
  Parallax(String const& assetFile,
           std::uint64_t seed,
           float verticalOrigin,
           float hueShift,
           std::optional<TreeVariant> parallaxTreeVariant = {});
  Parallax(Json const& store);

  [[nodiscard]] auto store() const -> Json;

  void fadeToSkyColor(Color const& skyColor);

  [[nodiscard]] auto layers() const -> ParallaxLayers const&;

private:
  void buildLayer(Json const& layerSettings, String const& kind);

  std::uint64_t m_seed;
  float m_verticalOrigin;
  std::optional<TreeVariant> m_parallaxTreeVariant;
  float m_hueShift;

  String m_imageDirectory;

  ParallaxLayers m_layers;
};

}// namespace Star
