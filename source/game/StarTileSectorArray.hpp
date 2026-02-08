#pragma once

#include "StarConfig.hpp"
#include "StarRect.hpp"
#include "StarSectorArray2D.hpp"

import std;

namespace Star {

// Storage container for world tiles that understands the sector based
// non-euclidean nature of the World.
//
// All RectI regions in this class are assumed to be right/top exclusive, so
// each tile covered by a RectI region must be strictly contained within the
// region to be included.
template <typename TileT, unsigned SectorSizeT>
class TileSectorArray {
public:
  using Tile = TileT;
  static unsigned const SectorSize = SectorSizeT;

  using SectorArray = SectorArray2D<Tile, SectorSize>;
  using Sector = typename SectorArray::Sector;
  using Array = typename SectorArray::Array;

  TileSectorArray();
  TileSectorArray(Vec2U const& size, Tile defaultTile = Tile());

  void init(Vec2U const& size, Tile defaultTile = Tile());

  [[nodiscard]] auto size() const -> Vec2U;
  auto defaultTile() const -> Tile;

  // Returns true if this sector is within the size bounds, regardless of
  // loaded / unloaded status.
  auto sectorValid(Sector const& sector) const -> bool;

  auto sectorFor(Vec2I const& pos) const -> Sector;

  // Return all valid sectors within a given range, regardless of loaded /
  // unloaded status.
  auto validSectorsFor(RectI const& region) const -> List<Sector>;

  // Returns the region for this sector, which is SectorSize x SectorSize
  // large.
  auto sectorRegion(Sector const& sector) const -> RectI;

  // Returns adjacent sectors in any given integral movement, in sectors.
  auto adjacentSector(Sector const& sector, Vec2I const& sectorMovement) -> Sector;

  // Load a sector into the active sector array.
  void loadSector(Sector const& sector, Ptr<Array> array);
  // Load with a sector full of the default tile.
  void loadDefaultSector(Sector const& sector);
  // Make a copy of a sector
  auto copySector(Sector const& sector) -> Ptr<Array>;
  // Take a sector out of the sector array.
  auto unloadSector(Sector const& sector) -> Ptr<Array>;

  auto sectorLoaded(Sector sector) const -> bool;
  auto loadedSectors() const -> List<Sector>;
  [[nodiscard]] auto loadedSectorCount() const -> std::size_t;

  // Will return null if the sector is unloaded.
  auto sectorArray(Sector sector) const -> Array const*;
  auto sectorArray(Sector sector) -> Array*;

  [[nodiscard]] auto tileLoaded(Vec2I const& pos) const -> bool;

  auto tile(Vec2I const& pos) const -> Tile const&;

  // Will return nullptr if the position is invalid.
  auto modifyTile(Vec2I const& pos) -> Tile*;

  // Function signature here is (Vec2I const&, Tile const&).  Will be called
  // for the entire region, valid or not.  If tile positions are not valid,
  // they will be called with the defaultTile.
  template <typename Function>
  void tileEach(RectI const& region, Function&& function) const;

  // Behaves like tileEach, but gathers the results of calling the function into
  // a MultiArray
  template <typename Function>
  auto tileEachResult(RectI const& region, Function&& function) const -> MultiArray<std::invoke_result_t<Function, Vec2I, Tile>, 2>;

  // Fastest way to copy data from the tile array to a given target array.
  // Takes a multi-array and a region and a function, resizes the multi-array
  // to be the size of the given region, and then calls the given function on
  // each tile in the region with this signature:
  // function(MultiArray::Element& target, Vec2I const& position, Tile const& source);
  // Called with the defaultTile for out of range positions.
  template <typename MultiArray, typename Function>
  void tileEachTo(MultiArray& results, RectI const& region, Function&& function) const;

  // Function signature here is (Vec2I const&, Tile&).  If a tile position
  // within this range is not valid, the function *will not* be called for that
  // position.
  template <typename Function>
  void tileEval(RectI const& region, Function&& function);

  // Will not be called for parts of the region that are not valid positions.
  template <typename Function>
  void tileEachColumns(RectI const& region, Function&& function) const;
  template <typename Function>
  void tileEvalColumns(RectI const& region, Function&& function);

  // Searches for a tile that satisfies a given condition in a block-area.
  // Returns true on the first instance found.  Passed in function must accept
  // (Vec2I const&, Tile const&).
  template <typename Function>
  auto tileSatisfies(RectI const& region, Function&& function) const -> bool;
  // Same, but uses a radius of 'distance', which is inclusive on all sides.
  // In other words, calling tileSatisfies({0, 0}, 1, <func>) should be
  // equivalent to calling tileSatisfies({-1, -1}, {3, 3}, <func>).
  template <typename Function>
  auto tileSatisfies(Vec2I const& pos, unsigned distance, Function&& function) const -> bool;

private:
  struct SplitRect {
    RectI rect;
    int xOffset;
  };

  // function must return bool to continue iteration
  template <typename Function>
  auto tileEachAbortable(RectI const& region, Function&& function) const -> bool;

  // Splits rects along the world wrap line and wraps the x coordinate for each
  // rect into world space.  Also returns the integral x offset to transform
  // back into the input rect range.
  auto splitRect(RectI rect) const -> StaticList<SplitRect, 2>;

  // Clamp the rect to entirely within valid tile spaces in y dimension
  [[nodiscard]] auto yClampRect(RectI const& r) const -> RectI;

  Vec2U m_worldSize;
  Tile m_default;
  SectorArray m_tileSectors;
};

template <typename Tile, unsigned SectorSize>
unsigned const TileSectorArray<Tile, SectorSize>::SectorSize;

template <typename Tile, unsigned SectorSize>
TileSectorArray<Tile, SectorSize>::TileSectorArray() = default;

template <typename Tile, unsigned SectorSize>
TileSectorArray<Tile, SectorSize>::TileSectorArray(Vec2U const& size, Tile defaultTile) {
  init(size, std::move(defaultTile));
}

template <typename Tile, unsigned SectorSize>
void TileSectorArray<Tile, SectorSize>::init(Vec2U const& size, Tile defaultTile) {
  m_worldSize = size;
  // Initialize to enough sectors to fit world size at least.
  m_tileSectors.init((size[0] + SectorSize - 1) / SectorSize, (size[1] + SectorSize - 1) / SectorSize);
  m_default = std::move(defaultTile);
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::size() const -> Vec2U {
  return m_worldSize;
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::defaultTile() const -> Tile {
  return m_default;
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::sectorFor(Vec2I const& pos) const -> Sector {
  return m_tileSectors.sectorFor((unsigned)pmod<int>(pos[0], m_worldSize[0]), (unsigned)pos[1]);
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::sectorValid(Sector const& sector) const -> bool {
  return m_tileSectors.sectorValid(sector);
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::validSectorsFor(RectI const& region) const -> List<Sector> {
  List<Sector> sectors;
  for (auto const& split : splitRect(yClampRect(region))) {
    auto sectorRange = m_tileSectors.sectorRange(split.rect.xMin(), split.rect.yMin(), split.rect.width(), split.rect.height());
    sectors.reserve(sectors.size() + (sectorRange.max[0] - sectorRange.min[0]) * (sectorRange.max[1] - sectorRange.min[1]));
    for (std::size_t x = sectorRange.min[0]; x < sectorRange.max[0]; ++x) {
      for (std::size_t y = sectorRange.min[1]; y < sectorRange.max[1]; ++y)
        sectors.append({x, y});
    }
  }
  return sectors;
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::sectorRegion(Sector const& sector) const -> RectI {
  Vec2I sectorCorner(m_tileSectors.sectorCorner(sector));
  return RectI::withSize(sectorCorner, {std::min<int>(SectorSize, m_worldSize[0] - sectorCorner[0]), std::min<int>(SectorSize, m_worldSize[1] - sectorCorner[1])});
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::adjacentSector(Sector const& sector, Vec2I const& sectorMovement) -> Sector {
  // This works because the only smaller than SectorSize sectors are on the
  // world wrap point, and there is only one vertical line of them, but it's
  // very not-obvious that it works.
  Vec2I corner(m_tileSectors.sectorCorner(sector));
  corner += sectorMovement * SectorSize;
  return sectorFor(corner);
}

template <typename Tile, unsigned SectorSize>
void TileSectorArray<Tile, SectorSize>::loadSector(Sector const& sector, Ptr<Array> tile) {
  if (sectorValid(sector))
    m_tileSectors.loadSector(sector, std::move(tile));
}

template <typename Tile, unsigned SectorSize>
void TileSectorArray<Tile, SectorSize>::loadDefaultSector(Sector const& sector) {
  if (sectorValid(sector))
    m_tileSectors.loadSector(sector, std::make_unique<Array>(m_default));
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::copySector(Sector const& sector) -> Ptr<Array> {
  if (sectorValid(sector))
    return m_tileSectors.copySector(sector);
  else
    return {};
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::unloadSector(Sector const& sector) -> Ptr<Array> {
  if (sectorValid(sector))
    return m_tileSectors.takeSector(sector);
  else
    return {};
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::sectorLoaded(Sector sector) const -> bool {
  if (sectorValid(sector))
    return m_tileSectors.sectorLoaded(sector);
  else
    return false;
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::loadedSectors() const -> List<Sector> {
  return m_tileSectors.loadedSectors();
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::loadedSectorCount() const -> std::size_t {
  return m_tileSectors.loadedSectorCount();
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::sectorArray(Sector sector) const -> Array const* {
  if (sectorValid(sector))
    return m_tileSectors.sector(sector);
  else
    return nullptr;
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::sectorArray(Sector sector) -> Array* {
  if (sectorValid(sector))
    return m_tileSectors.sector(sector);
  else
    return nullptr;
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::tileLoaded(Vec2I const& pos) const -> bool {
  if (pos[1] < 0 || pos[1] >= (int)m_worldSize[1])
    return false;

  auto xind = (unsigned)pmod<int>(pos[0], m_worldSize[0]);
  auto yind = (unsigned)pos[1];

  return m_tileSectors.get(xind, yind) != nullptr;
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::tile(Vec2I const& pos) const -> Tile const& {
  if (pos[1] < 0 || pos[1] >= (int)m_worldSize[1])
    return m_default;

  auto xind = (unsigned)pmod<int>(pos[0], m_worldSize[0]);
  auto yind = (unsigned)pos[1];

  Tile const* tile = m_tileSectors.get(xind, yind);
  if (tile)
    return *tile;
  else
    return m_default;
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::modifyTile(Vec2I const& pos) -> Tile* {
  if (pos[1] < 0 || pos[1] >= (int)m_worldSize[1])
    return nullptr;

  auto xind = (unsigned)pmod<int>(pos[0], m_worldSize[0]);
  auto yind = (unsigned)pos[1];

  return m_tileSectors.get(xind, yind);
}

template <typename Tile, unsigned SectorSize>
template <typename Function>
void TileSectorArray<Tile, SectorSize>::tileEach(RectI const& region, Function&& function) const {
  tileEachAbortable(region,
                    [&](Vec2I const& pos, Tile const& tile) -> auto {
                      function(pos, tile);
                      return true;
                    });
}

template <typename Tile, unsigned SectorSize>
template <typename Function>
auto TileSectorArray<Tile, SectorSize>::tileEachResult(RectI const& region, Function&& function) const -> MultiArray<std::invoke_result_t<Function, Vec2I, Tile>, 2> {
  MultiArray<std::invoke_result_t<Function, Vec2I, Tile>, 2> res;
  tileEachTo(res, region, [&](auto& res, Vec2I const& pos, Tile const& tile) -> auto { res = function(pos, tile); });
  return res;
}

template <typename Tile, unsigned SectorSize>
template <typename MultiArray, typename Function>
void TileSectorArray<Tile, SectorSize>::tileEachTo(MultiArray& results, RectI const& region, Function&& function) const {
  if (region.isEmpty()) {
    results.setSize({0, 0});
    return;
  }

  int xArrayOffset = -region.xMin();
  int yArrayOffset = -region.yMin();
  results.setSize(Vec2S(region.size()));

  for (auto const& split : splitRect(region)) {
    auto clampedRect = yClampRect(split.rect);
    if (!clampedRect.isEmpty()) {
      m_tileSectors.evalColumns(clampedRect.xMin(), clampedRect.yMin(), clampedRect.width(), clampedRect.height(), [&](std::size_t x, std::size_t y, Tile const* column, std::size_t columnSize) -> auto {
          std::size_t arrayColumnIndex = (x + split.xOffset + xArrayOffset) * results.size(1) + y + yArrayOffset;
          if (column) {
            for (std::size_t i = 0; i < columnSize; ++i)
              function(results.atIndex(arrayColumnIndex + i), Vec2I((int)x + split.xOffset, y + i), column[i]);
          } else {
            for (std::size_t i = 0; i < columnSize; ++i)
              function(results.atIndex(arrayColumnIndex + i), Vec2I((int)x + split.xOffset, y + i), m_default);
          }
          return true; }, true);
    }

    // Call with default tile for tiles outside of the y-range (to ensure that
    // every index in the rect gets called)
    for (int x = split.rect.xMin(); x < split.rect.xMax(); ++x) {
      for (int y = split.rect.yMin(); y < min<int>(split.rect.yMax(), 0); ++y)
        function(results(x + split.xOffset + xArrayOffset, y + yArrayOffset), Vec2I(x + split.xOffset, y), m_default);
    }

    for (int x = split.rect.xMin(); x < split.rect.xMax(); ++x) {
      for (int y = max<int>(split.rect.yMin(), m_worldSize[1]); y < split.rect.yMax(); ++y)
        function(results(x + split.xOffset + xArrayOffset, y + yArrayOffset), Vec2I(x + split.xOffset, y), m_default);
    }
  }
}

template <typename Tile, unsigned SectorSize>
template <typename Function>
void TileSectorArray<Tile, SectorSize>::tileEval(RectI const& region, Function&& function) {
  for (auto const& split : splitRect(region)) {
    auto clampedRect = yClampRect(split.rect);
    if (!clampedRect.isEmpty()) {
      // If non-const variant, do not call function if tile not loaded (pass
      // false to evalEmpty in sector array)
      auto fwrapper = [&](unsigned x, unsigned y, Tile* tile) -> auto {
        function(Vec2I((int)x + split.xOffset, (int)y), *tile);
        return true;
      };
      m_tileSectors.eval(clampedRect.xMin(), clampedRect.yMin(), clampedRect.width(), clampedRect.height(), fwrapper, false);
    }
  }
}

template <typename Tile, unsigned SectorSize>
template <typename Function>
void TileSectorArray<Tile, SectorSize>::tileEachColumns(RectI const& region, Function&& function) const {
  const_cast<TileSectorArray*>(this)->tileEvalColumns(
    region, [&](Vec2I const& pos, Tile* tiles, std::size_t size) -> auto { function(pos, (Tile const*)tiles, size); });
}

template <typename Tile, unsigned SectorSize>
template <typename Function>
void TileSectorArray<Tile, SectorSize>::tileEvalColumns(RectI const& region, Function&& function) {
  for (auto const& split : splitRect(region)) {
    auto clampedRect = yClampRect(split.rect);
    if (!clampedRect.isEmpty()) {
      auto fwrapper = [&](std::size_t x, std::size_t y, Tile* column, std::size_t columnSize) -> auto {
        function(Vec2I((int)x + split.xOffset, (int)y), column, columnSize);
        return true;
      };
      m_tileSectors.evalColumns(clampedRect.xMin(), clampedRect.yMin(), clampedRect.width(), clampedRect.height(), fwrapper, false);
    }
  }
}

template <typename Tile, unsigned SectorSize>
template <typename Function>
auto TileSectorArray<Tile, SectorSize>::tileSatisfies(Vec2I const& pos, unsigned distance, Function&& function) const -> bool {
  return tileSatisfies(RectI::withSize(pos - Vec2I::filled(distance), Vec2I::filled(distance * 2 + 1)), function);
}

template <typename Tile, unsigned SectorSize>
template <typename Function>
auto TileSectorArray<Tile, SectorSize>::tileSatisfies(RectI const& region, Function&& function) const -> bool {
  return !tileEachAbortable(region, [&](Vec2I const& pos, Tile const& tile) -> auto { return !function(pos, tile); });
}

template <typename Tile, unsigned SectorSize>
template <typename Function>
auto TileSectorArray<Tile, SectorSize>::tileEachAbortable(RectI const& region, Function&& function) const -> bool {
  for (auto const& split : splitRect(region)) {
    auto clampedRect = yClampRect(split.rect);
    if (!clampedRect.isEmpty()) {
      // If const variant, call function with default tile if not loaded.
      auto fwrapper = [&](unsigned x, unsigned y, Tile const* tile) -> auto {
        if (!tile)
          tile = &m_default;
        return function(Vec2I((int)x + split.xOffset, y), *tile);
      };
      if (!m_tileSectors.eval(clampedRect.xMin(), clampedRect.yMin(), clampedRect.width(), clampedRect.height(), fwrapper, true))
        return false;
    }

    // Call with default tile for tiles outside of the y-range (to ensure
    // that every index in the rect gets called)
    for (int x = split.rect.xMin(); x < split.rect.xMax(); ++x) {
      for (int y = split.rect.yMin(); y < min<int>(split.rect.yMax(), 0); ++y) {
        if (!function(Vec2I(x + split.xOffset, y), m_default))
          return false;
      }
    }

    for (int x = split.rect.xMin(); x < split.rect.xMax(); ++x) {
      for (int y = max<int>(split.rect.yMin(), m_worldSize[1]); y < split.rect.yMax(); ++y)
        if (!function(Vec2I(x + split.xOffset, y), m_default))
          return false;
    }
  }
  return true;
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::splitRect(RectI rect) const -> StaticList<SplitRect, 2> {
  // TODO: Offset here does not support rects outside of -m_worldSize[0] to 2 * m_worldSize[0]!

  // any rect at least the width of the world is equivalent to a rect that spans the width of the world exactly
  if (rect.width() >= (int)m_worldSize[0])
    return {SplitRect{RectI(0, rect.yMin(), m_worldSize[0], rect.yMax()), 0}};

  if (rect.isEmpty())
    return {};

  int width = rect.width();
  int xMin = pmod<int>(rect.xMin(), m_worldSize[0]);
  int xOffset = rect.xMin() - xMin;
  rect.setXMin(xMin);
  rect.setXMax(xMin + width);

  if (rect.xMin() < (int)m_worldSize[0] && rect.xMax() > (int)m_worldSize[0]) {
    return {
      SplitRect{RectI(rect.xMin(), rect.yMin(), m_worldSize[0], rect.yMax()), xOffset},
      SplitRect{RectI(0, rect.yMin(), rect.xMax() - m_worldSize[0], rect.yMax()), xOffset + (int)m_worldSize[0]}};
  } else {
    return {SplitRect{rect, xOffset}};
  }
}

template <typename Tile, unsigned SectorSize>
auto TileSectorArray<Tile, SectorSize>::yClampRect(RectI const& r) const -> RectI {
  return {r.xMin(), clamp<int>(r.yMin(), 0, m_worldSize[1]), r.xMax(), clamp<int>(r.yMax(), 0, m_worldSize[1])};
}

}// namespace Star
