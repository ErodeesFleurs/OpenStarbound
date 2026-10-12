#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarInterpolation.hpp"
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
