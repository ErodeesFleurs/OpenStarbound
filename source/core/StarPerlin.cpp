import std;
import star.format;
import star.json;
import star.bi_map;
import star.random;
import star.perlin;

namespace Star {

extern std::expected<EnumMap<PerlinType>, StarException> const PerlinTypeNames = EnumMap<PerlinType>::create({
  {PerlinType::Uninitialized, "uninitialized"},
  {PerlinType::Perlin, "perlin"},
  {PerlinType::Billow, "billow"},
  {PerlinType::RidgedMulti, "ridgedMulti"},
});

}
