#pragma once

#include "StarConfig.hpp"
#include "StarLiquidTypes.hpp"
#include "StarSkyTypes.hpp"
#include "StarWeatherTypes.hpp"

import std;

namespace Star {

enum class WorldParametersType : std::uint8_t {
  TerrestrialWorldParameters,
  AsteroidsWorldParameters,
  FloatingDungeonWorldParameters
};
extern EnumMap<WorldParametersType> const WorldParametersTypeNames;

enum class BeamUpRule : std::uint8_t {
  Nowhere,
  Surface,
  Anywhere,
  AnywhereWithWarning
};
extern EnumMap<BeamUpRule> const BeamUpRuleNames;

enum class WorldEdgeForceRegionType : std::uint8_t {
  None,
  Top,
  Bottom,
  TopAndBottom
};
extern EnumMap<WorldEdgeForceRegionType> const WorldEdgeForceRegionTypeNames;

struct VisitableWorldParameters {
  VisitableWorldParameters() = default;
  VisitableWorldParameters(VisitableWorldParameters const& visitableWorldParameters) = default;
  explicit VisitableWorldParameters(Json const& store);

  virtual ~VisitableWorldParameters() = default;

  [[nodiscard]] virtual auto type() const -> WorldParametersType = 0;

  [[nodiscard]] virtual auto store() const -> Json;

  virtual void read(DataStream& ds);
  virtual void write(DataStream& ds) const;

  String typeName;
  float threatLevel{};
  Vec2U worldSize;
  float gravity{};
  bool airless{false};
  WeatherPool weatherPool;
  StringList environmentStatusEffects;
  std::optional<StringList> overrideTech;
  std::optional<List<Directives>> globalDirectives;
  BeamUpRule beamUpRule;
  bool disableDeathDrops{false};
  bool terraformed{false};
  WorldEdgeForceRegionType worldEdgeForceRegions{WorldEdgeForceRegionType::None};
};

struct TerrestrialWorldParameters : VisitableWorldParameters {
  struct TerrestrialRegion {
    String biome;

    String blockSelector;
    String fgCaveSelector;
    String bgCaveSelector;
    String fgOreSelector;
    String bgOreSelector;
    String subBlockSelector;

    LiquidId caveLiquid{};
    float caveLiquidSeedDensity{};

    LiquidId oceanLiquid{};
    int oceanLiquidLevel{};

    bool encloseLiquids{false};
    bool fillMicrodungeons{false};
  };

  struct TerrestrialLayer {
    int layerMinHeight;
    int layerBaseHeight;

    StringList dungeons;
    int dungeonXVariance;

    TerrestrialRegion primaryRegion;
    TerrestrialRegion primarySubRegion;

    List<TerrestrialRegion> secondaryRegions;
    List<TerrestrialRegion> secondarySubRegions;

    Vec2F secondaryRegionSizeRange;
    Vec2F subRegionSizeRange;
  };

  TerrestrialWorldParameters() = default;
  TerrestrialWorldParameters(TerrestrialWorldParameters const& terrestrialWorldParameters) = default;
  explicit TerrestrialWorldParameters(Json const& store);

  auto operator=(TerrestrialWorldParameters const& terrestrialWorldParameters) -> TerrestrialWorldParameters&;

  [[nodiscard]] auto type() const -> WorldParametersType override;

  [[nodiscard]] auto store() const -> Json override;

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String primaryBiome;
  LiquidId primarySurfaceLiquid{};
  String sizeName;
  float hueShift{};

  SkyColoring skyColoring;
  float dayLength{};

  Json blockNoiseConfig;
  Json blendNoiseConfig;
  float blendSize{};

  TerrestrialLayer spaceLayer{};
  TerrestrialLayer atmosphereLayer{};
  TerrestrialLayer surfaceLayer{};
  TerrestrialLayer subsurfaceLayer{};
  List<TerrestrialLayer> undergroundLayers;
  TerrestrialLayer coreLayer{};
};

struct AsteroidsWorldParameters : VisitableWorldParameters {
  AsteroidsWorldParameters();
  explicit AsteroidsWorldParameters(Json const& store);

  [[nodiscard]] auto type() const -> WorldParametersType override;

  [[nodiscard]] auto store() const -> Json override;

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  int asteroidTopLevel{};
  int asteroidBottomLevel{};
  float blendSize{};
  String asteroidBiome;
  Color ambientLightLevel;
};

struct FloatingDungeonWorldParameters : VisitableWorldParameters {
  FloatingDungeonWorldParameters() = default;
  explicit FloatingDungeonWorldParameters(Json const& store);

  [[nodiscard]] auto type() const -> WorldParametersType override;

  [[nodiscard]] auto store() const -> Json override;

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  int dungeonBaseHeight;
  int dungeonSurfaceHeight;
  int dungeonUndergroundLevel;
  String primaryDungeon;
  Color ambientLightLevel;
  std::optional<String> biome;
  std::optional<String> dayMusicTrack;
  std::optional<String> nightMusicTrack;
  std::optional<String> dayAmbientNoises;
  std::optional<String> nightAmbientNoises;
};

auto diskStoreVisitableWorldParameters(ConstPtr<VisitableWorldParameters> const& parameters) -> Json;
auto diskLoadVisitableWorldParameters(Json const& store) -> Ptr<VisitableWorldParameters>;

auto netStoreVisitableWorldParameters(ConstPtr<VisitableWorldParameters> const& parameters) -> ByteArray;
auto netLoadVisitableWorldParameters(ByteArray data) -> Ptr<VisitableWorldParameters>;

auto generateTerrestrialWorldParameters(String const& typeName, String const& sizeName, std::uint64_t seed) -> Ptr<TerrestrialWorldParameters>;
auto generateAsteroidsWorldParameters(std::uint64_t seed) -> Ptr<AsteroidsWorldParameters>;
auto generateFloatingDungeonWorldParameters(String const& dungeonWorldName) -> Ptr<FloatingDungeonWorldParameters>;

}// namespace Star
