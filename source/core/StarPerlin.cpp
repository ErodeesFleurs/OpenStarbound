#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.perlin;

namespace Star {

EnumMap<PerlinType> const PerlinTypeNames{
  {PerlinType::Uninitialized, "uninitialized"},
  {PerlinType::Perlin, "perlin"},
  {PerlinType::Billow, "billow"},
  {PerlinType::RidgedMulti, "ridgedMulti"},
};

}
