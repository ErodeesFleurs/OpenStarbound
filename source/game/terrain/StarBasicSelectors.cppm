module;
import std;
import star.format;
import star.json;


export module star.terrain_basic;
import star.terrain_database;

export namespace Star {

struct ConstantSelector : TerrainSelector {
  static char const* const Name;

  [[nodiscard]] static std::expected<std::shared_ptr<ConstantSelector>, StarException> create(Json const& config, TerrainSelectorParameters const& parameters);
private:
  struct ConstructionKey { explicit ConstructionKey() = default; };
public:
  ConstantSelector(ConstructionKey, Json const& config, TerrainSelectorParameters const& parameters, float value);

  [[nodiscard]] std::expected<float, StarException> get(int x, int y) const override;

  float m_value;
};



struct FlatSurfaceSelector : TerrainSelector {
  static char const* const Name;

  [[nodiscard]] static std::expected<std::shared_ptr<FlatSurfaceSelector>, StarException> create(Json const& config, TerrainSelectorParameters const& parameters);
private:
  struct ConstructionKey { explicit ConstructionKey() = default; };
public:
  FlatSurfaceSelector(ConstructionKey, Json const& config, TerrainSelectorParameters const& parameters, float adjustment, float flip);

  [[nodiscard]] std::expected<float, StarException> get(int x, int y) const override;

  float surfaceLevel;
  float adjustment;
  float flip;
};

}

namespace Star {

char const* const ConstantSelector::Name = "constant";

std::expected<std::shared_ptr<ConstantSelector>, StarException> ConstantSelector::create(Json const& config, TerrainSelectorParameters const& parameters) {
  auto value = config.getFloat("value");
  if (!value)
    return std::unexpected(std::move(value.error()));
  return std::make_shared<ConstantSelector>(ConstructionKey{}, config, parameters, *value);
}

ConstantSelector::ConstantSelector(ConstructionKey, Json const& config, TerrainSelectorParameters const& parameters, float value)
  : TerrainSelector(Name, config, parameters), m_value(value) {}

std::expected<float, StarException> ConstantSelector::get(int, int) const {
  return m_value;
}



char const* const FlatSurfaceSelector::Name = "flatSurface";

std::expected<std::shared_ptr<FlatSurfaceSelector>, StarException> FlatSurfaceSelector::create(Json const& config, TerrainSelectorParameters const& parameters) {
  auto adjustment = config.getFloat("adjustment", 0);
  if (!adjustment)
    return std::unexpected(std::move(adjustment.error()));
  auto flip = config.getBool("flip", false);
  if (!flip)
    return std::unexpected(std::move(flip.error()));
  return std::make_shared<FlatSurfaceSelector>(ConstructionKey{}, config, parameters, *adjustment, *flip ? -1.0f : 1.0f);
}

FlatSurfaceSelector::FlatSurfaceSelector(ConstructionKey, Json const& config, TerrainSelectorParameters const& parameters, float adjustmentValue, float flipValue)
  : TerrainSelector(Name, config, parameters), surfaceLevel(parameters.baseHeight), adjustment(adjustmentValue), flip(flipValue) {}

std::expected<float, StarException> FlatSurfaceSelector::get(int, int y) const {
  return flip * (surfaceLevel - (y - adjustment));
}

}
