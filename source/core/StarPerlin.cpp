import star.json;
import star.bi_map;
import star.random;
import star.perlin;

namespace Star {

extern EnumMap<PerlinType> const PerlinTypeNames{
  {PerlinType::Uninitialized, "uninitialized"},
  {PerlinType::Perlin, "perlin"},
  {PerlinType::Billow, "billow"},
  {PerlinType::RidgedMulti, "ridgedMulti"},
};

}
