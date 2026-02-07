#pragma once

#include "StarPoly.hpp"
#include "StarRandom.hpp"
#include "StarTtlCache.hpp"

import std;

namespace Star {

// An "infinite" generator of points on a 2d plane, generated cell by cell with
// an upper and lower cell density range.  Each point is generated in a
// predictable way sector by sector, as long as the generator function is
// predictable and uses the RandomSource in a predictable way.  Useful for
// things like starfields, fields of debris, random object placement, etc.

template <typename PointData, typename DataType = float>
class Random2dPointGenerator {
public:
  using Poly = Star::Polygon<DataType>;
  using Point = Star::Vector<DataType, 2>;
  using Rect = Star::Rect<DataType>;
  using PointSet = List<std::pair<Point, PointData>>;
  Random2dPointGenerator(std::uint64_t seed, float cellSize, Vec2I const& densityRange);

  // Each point will in the area will be generated in a predictable order, and
  // if the callback uses the RandomSource in a predictable way, will generate
  // the same field for every call.
  template <typename PointCallback>
  auto generate(Poly const& area, PointCallback callback) -> PointSet;

private:
  HashTtlCache<Point, PointSet> m_cache;

  std::uint64_t m_seed;
  float m_cellSize;
  Vec2I m_densityRange;
};

template <typename PointData, typename DataType>
inline Random2dPointGenerator<PointData, DataType>::Random2dPointGenerator(std::uint64_t seed, float cellSize, Vec2I const& densityRange)
    : m_seed(seed), m_cellSize(cellSize), m_densityRange(densityRange) {}

template <typename PointData, typename DataType>
template <typename PointCallback>
auto Random2dPointGenerator<PointData, DataType>::generate(Poly const& area, PointCallback callback) -> PointSet {
  auto bound = area.boundBox();
  std::int64_t sectorXMin = std::floor(bound.xMin() / m_cellSize);
  std::int64_t sectorYMin = std::floor(bound.yMin() / m_cellSize);
  std::int64_t sectorXMax = std::ceil(bound.xMax() / m_cellSize);
  std::int64_t sectorYMax = std::ceil(bound.yMax() / m_cellSize);

  PointSet finalResult;
  RandomSource sectorRandomness;

  for (std::int64_t x = sectorXMin; x <= sectorXMax; ++x) {
    for (std::int64_t y = sectorYMin; y <= sectorYMax; ++y) {
      auto sector = Rect::withSize({x * m_cellSize, y * m_cellSize}, Point::filled(m_cellSize));
      if (!area.intersects(Poly(sector)))
        continue;

      finalResult.appendAll(m_cache.get(Point(x, y), [&](Point const&) -> auto {
        PointSet sectorResult;
        sectorRandomness.init(staticRandomU64(m_seed, x, y));
        unsigned max = sectorRandomness.randInt(m_densityRange[0], m_densityRange[1]);
        for (unsigned i = 0; i < max; ++i) {
          Point pointPos = Point(x + (DataType)sectorRandomness.randd(), y + (DataType)sectorRandomness.randd()) * m_cellSize;
          sectorResult.append(pair<Point, PointData>(pointPos, callback(sectorRandomness)));
        }

        return sectorResult;
      }));
    }
  }

  return finalResult;
}

}// namespace Star
