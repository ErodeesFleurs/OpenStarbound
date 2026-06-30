#pragma once

#include "StarTerrainDatabase.hpp"
#include "StarLruCache.hpp"
#include "StarPerlin.hpp"
#include "StarVector.hpp"

namespace Star {

// --- ConstantSelector ---
struct ConstantSelector : TerrainSelector {
  static char const* const Name;

  ConstantSelector(Json const& config, TerrainSelectorParameters const& parameters);

  [[nodiscard]] float get(int x, int y) const override;

  float m_value;
};

// --- PerlinSelector ---
struct PerlinSelector : TerrainSelector {
  static char const* const Name;

  PerlinSelector(Json const& config, TerrainSelectorParameters const& parameters);

  [[nodiscard]] float get(int x, int y) const override;

  PerlinF function;

  float xInfluence;
  float yInfluence;
};

// --- CacheSelector ---
struct CacheSelector : TerrainSelector {
  static char const* const Name;

  CacheSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] float get(int x, int y) const override;

  TerrainSelectorConstPtr m_source;
  mutable HashLruCache<Vec2I, float> m_cache;
};

// --- DisplacementSelector ---
struct DisplacementSelector : TerrainSelector {
  static char const* const Name;

  DisplacementSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] float get(int x, int y) const override;

  PerlinF xDisplacementFunction;
  PerlinF yDisplacementFunction;

  float xXInfluence;
  float xYInfluence;
  float yXInfluence;
  float yYInfluence;

  bool yClamp;
  Vec2F yClampRange;
  float yClampSmoothing;

  [[nodiscard]] float clampY(float v) const;

  TerrainSelectorConstPtr m_source;
};

// --- FlatSurfaceSelector ---
struct FlatSurfaceSelector : TerrainSelector {
  static char const* const Name;

  FlatSurfaceSelector(Json const& config, TerrainSelectorParameters const& parameters);

  [[nodiscard]] float get(int x, int y) const override;

  float surfaceLevel;
  float adjustment;
  float flip;
};

// --- IslandSurfaceSelector ---
struct IslandColumn {
  float topLevel;
  float bottomLevel;
};

struct IslandSurfaceSelector : TerrainSelector {
  static char const* const Name;

  IslandSurfaceSelector(Json const& config, TerrainSelectorParameters const& parameters);

  [[nodiscard]] float get(int x, int y) const override;

  [[nodiscard]] IslandColumn generateColumn(int x) const;

  mutable HashLruCache<int, IslandColumn> columnCache;

  PerlinF islandHeight;
  PerlinF islandDepth;
  PerlinF islandDecision;

  float islandTaperPoint;
  float islandElevation;

  float layerBaseHeight;
  int worldWidth;
};

// --- MaxSelector ---
struct MaxSelector : TerrainSelector {
  static char const* const Name;

  MaxSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] float get(int x, int y) const override;

  List<TerrainSelectorConstPtr> m_sources;
};

// --- MinMaxSelector ---
struct MinMaxSelector : TerrainSelector {
  static char const* const Name;

  MinMaxSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] float get(int x, int y) const override;

  List<TerrainSelectorConstPtr> m_sources;
};

// --- MixSelector ---
struct MixSelector : TerrainSelector {
  static char const* const Name;

  MixSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] float get(int x, int y) const override;

  TerrainSelectorConstPtr m_mixSource;
  TerrainSelectorConstPtr m_aSource;
  TerrainSelectorConstPtr m_bSource;
};

// --- RidgeBlocksSelector ---
struct RidgeBlocksSelector : TerrainSelector {
  static char const* const Name;

  RidgeBlocksSelector(Json const& config, TerrainSelectorParameters const& parameters);

  [[nodiscard]] float get(int x, int y) const override;

  float commonality;

  float amplitude;
  float frequency;
  float bias;

  float noiseAmplitude;
  float noiseFrequency;

  PerlinF ridgePerlin1;
  PerlinF ridgePerlin2;
  PerlinF noisePerlin;
};

// --- RotateSelector ---
struct RotateSelector : TerrainSelector {
  static char const* const Name;

  RotateSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] float get(int x, int y) const override;

  float rotation;
  Vec2F rotationCenter;

  TerrainSelectorConstPtr m_source;
};

// --- WormCaveSelector ---
class WormCaveSector {
public:
  WormCaveSector(int sectorSize, Vec2I sector, Json const& config, size_t seed, float commonality);

  [[nodiscard]] float get(int x, int y);

private:
  [[nodiscard]] bool inside(int x, int y);
  void set(int x, int y, float value);

  int m_sectorSize;
  Vec2I m_sector;
  List<float> m_values;

  float m_maxValue;
};

class WormCaveSelector : public TerrainSelector {
public:
  static char const* const Name;

  WormCaveSelector(Json const& config, TerrainSelectorParameters const& parameters);

  [[nodiscard]] float get(int x, int y) const override;

private:
  int m_sectorSize;
  mutable HashLruCache<Vec2I, WormCaveSector> m_cache;
};

// --- KarstCaveSelector ---
class KarstCaveSelector : public TerrainSelector {
public:
  static char const* const Name;

  KarstCaveSelector(Json const& config, TerrainSelectorParameters const& parameters);

  [[nodiscard]] float get(int x, int y) const override;

private:
  struct LayerPerlins {
    PerlinF caveDecision;
    PerlinF layerHeightVariation;
    PerlinF caveHeightVariation;
    PerlinF caveFloorVariation;
  };

  struct Sector {
    Sector(KarstCaveSelector const* parent, Vec2I sector);

    [[nodiscard]] float get(int x, int y);

    [[nodiscard]] bool inside(int x, int y);
    void set(int x, int y, float value);

    KarstCaveSelector const* parent;
    Vec2I sector;
    List<float> values;

    float m_maxValue;
  };

  [[nodiscard]] LayerPerlins const& layerPerlins(int y) const;

  int m_sectorSize;
  int m_layerResolution;
  float m_layerDensity;
  int m_bufferHeight;
  float m_caveTaperPoint;

  Json m_caveDecisionPerlinConfig;
  Json m_layerHeightVariationPerlinConfig;
  Json m_caveHeightVariationPerlinConfig;
  Json m_caveFloorVariationPerlinConfig;

  int m_worldWidth;
  uint64_t m_seed;

  mutable HashLruCache<int, LayerPerlins> m_layerPerlinsCache;
  mutable HashLruCache<Vec2I, Sector> m_sectorCache;
};

}
