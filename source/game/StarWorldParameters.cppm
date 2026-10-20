module;
import std;
import star.string;
import star.json;
import star.byte_array;
import star.variant;

import star.vector;
import star.game_types;
import star.liquid_types;
import star.color;
import star.bi_map;
import star.sky_types;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.animation;
import star.particle;
import star.weather_types;

import star.force_regions;

namespace Star {
class Root;

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

  virtual WorldParametersType type() const = 0;

  virtual Json store() const;

  virtual void read(DataStream& ds);
  virtual void write(DataStream& ds) const;

  String typeName;
  float threatLevel{};
  Vec2U worldSize;
  float gravity{};
  bool airless{false};
  WeatherPool weatherPool;
  StringList environmentStatusEffects;
  Maybe<StringList> overrideTech;
  Maybe<List<Directives>> globalDirectives;
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

  TerrestrialWorldParameters &operator=(TerrestrialWorldParameters const& terrestrialWorldParameters);

  WorldParametersType type() const override;

  Json store() const override;

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

  WorldParametersType type() const override;

  Json store() const override;

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

  WorldParametersType type() const override;

  Json store() const override;

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  int dungeonBaseHeight;
  int dungeonSurfaceHeight;
  int dungeonUndergroundLevel;
  String primaryDungeon;
  Color ambientLightLevel;
  Maybe<String> biome;
  Maybe<String> dayMusicTrack;
  Maybe<String> nightMusicTrack;
  Maybe<String> dayAmbientNoises;
  Maybe<String> nightAmbientNoises;
};

Json diskStoreVisitableWorldParameters(std::shared_ptr<VisitableWorldParameters const> const& parameters);
std::shared_ptr<VisitableWorldParameters> diskLoadVisitableWorldParameters(Json const& store);

ByteArray netStoreVisitableWorldParameters(std::shared_ptr<VisitableWorldParameters const> const& parameters);
std::shared_ptr<VisitableWorldParameters> netLoadVisitableWorldParameters(ByteArray data);

std::shared_ptr<TerrestrialWorldParameters> generateTerrestrialWorldParameters(Root& root, String const& typeName, String const& sizeName, std::uint64_t seed);
std::shared_ptr<AsteroidsWorldParameters> generateAsteroidsWorldParameters(Root& root, std::uint64_t seed);
std::shared_ptr<FloatingDungeonWorldParameters> generateFloatingDungeonWorldParameters(Root& root, String const& dungeonWorldName);

}

export module star.world_parameters;

export namespace Star {
  using ::Star::WorldParametersType;
  using ::Star::WorldParametersTypeNames;
  using ::Star::BeamUpRule;
  using ::Star::BeamUpRuleNames;
  using ::Star::WorldEdgeForceRegionType;
  using ::Star::WorldEdgeForceRegionTypeNames;
  using ::Star::diskStoreVisitableWorldParameters;
  using ::Star::diskLoadVisitableWorldParameters;
  using ::Star::netStoreVisitableWorldParameters;
  using ::Star::netLoadVisitableWorldParameters;
  using ::Star::generateTerrestrialWorldParameters;
  using ::Star::generateAsteroidsWorldParameters;
  using ::Star::generateFloatingDungeonWorldParameters;
  using ::Star::VisitableWorldParameters;
  using ::Star::TerrestrialWorldParameters;
  using ::Star::AsteroidsWorldParameters;
  using ::Star::FloatingDungeonWorldParameters;
}
