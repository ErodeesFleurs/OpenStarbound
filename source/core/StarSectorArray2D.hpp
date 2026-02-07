#pragma once

#include "StarMultiArray.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

// Holds a sparse 2d array of data based on sector size.  Meant to be used as a
// fast-as-possible sparse array.  Memory requiremenets are equal to the size
// of all loaded sectors PLUS pointer size * sectors wide * sectors high
template <typename ElementT, size_t SectorSize>
class SectorArray2D {
public:
  using Element = ElementT;
  using Sector = Vec2S;

  struct SectorRange {
    // Lower left sector
    Vec2S min;
    // Upper right sector *non-inclusive*
    Vec2S max;
  };

  struct Array {
    Array();
    Array(Element const& def);

    auto operator()(size_t x, size_t y) const -> Element const&;
    auto operator()(size_t x, size_t y) -> Element&;

    std::array<Element, SectorSize * SectorSize> elements;
  };
  using ArrayPtr = std::unique_ptr<Array>;

  using DynamicArray = MultiArray<Element, 2>;

  SectorArray2D();
  SectorArray2D(size_t numSectorsWide, size_t numSectorsHigh);

  void init(size_t numSectorsWide, size_t numSectorsHigh);

  // Total size of array elements
  [[nodiscard]] auto width() const -> std::size_t;
  [[nodiscard]] auto height() const -> std::size_t;

  // Is sector within width() and heigh()
  [[nodiscard]] auto sectorValid(Sector const& sector) const -> bool;

  // Returns the sector that contains the given point
  [[nodiscard]] auto sectorFor(size_t x, size_t y) const -> Sector;
  // Returns the sector range that contains the given rectangle
  auto sectorRange(size_t minX, size_t minY, size_t width, size_t height) const -> SectorRange;

  [[nodiscard]] auto sectorCorner(Sector const& id) const -> Vec2S;
  [[nodiscard]] auto hasSector(Sector const& id) const -> bool;

  [[nodiscard]] auto loadedSectors() const -> List<Sector>;
  [[nodiscard]] auto loadedSectorCount() const -> size_t;
  [[nodiscard]] auto sectorLoaded(Sector const& id) const -> bool;

  // Will return nullptr if sector is not loaded.
  auto sector(Sector const& id) -> Array*;
  auto sector(Sector const& id) const -> Array const*;

  void loadSector(Sector const& id, ArrayPtr array);
  auto copySector(Sector const& id) -> ArrayPtr;
  auto takeSector(Sector const& id) -> ArrayPtr;
  void discardSector(Sector const& id);

  // Will return nullptr if sector is not loaded.
  auto get(size_t x, size_t y) const -> Element const*;
  auto get(size_t x, size_t y) -> Element*;

  // Fast evaluate of elements in the given range.  If evalEmpty is true, then
  // function will be called even for unloaded sectors (with null pointer).
  // Function is called as function(size_t x, size_t y, Element* element).
  // Given function should return true to continue, false to stop.  Returns
  // false if any evaled functions return false.
  template <typename Function>
  auto eval(size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty = false) const -> bool;
  template <typename Function>
  auto eval(size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty = false) -> bool;

  // Individual sectors are stored column-major, so for speed, use this method
  // to get whole columns at a time.  If eval empty is true, function will be
  // called with for each empty column with the correct size information, but
  // the pointer will be null.  Function will be called as
  // function(size_t x, size_t y, Element* columnElements, size_t columnSize)
  // columnSize is guaranteed never to be greater than SectorSize.  Given
  // function should return true to continue, false to stop.  Returns false if
  // any evaled columns return false.
  template <typename Function>
  auto evalColumns(
    size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty = false) const -> bool;
  template <typename Function>
  auto evalColumns(size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty = false) -> bool;

private:
  using SectorArray = MultiArray<ArrayPtr, 2>;

  template <typename Function>
  auto evalPriv(size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty) -> bool;
  template <typename Function>
  auto evalColumnsPriv(size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty) -> bool;

  SectorArray m_sectors;
  HashSet<Sector> m_loadedSectors;
};

template <typename ElementT, size_t SectorSize>
SectorArray2D<ElementT, SectorSize>::Array::Array()
    : elements() {}

template <typename ElementT, size_t SectorSize>
SectorArray2D<ElementT, SectorSize>::Array::Array(Element const& def) {
  for (size_t i = 0; i < SectorSize * SectorSize; ++i)
    elements[i] = def;
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::Array::operator()(size_t x, size_t y) const -> ElementT const& {
  return elements[x * SectorSize + y];
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::Array::operator()(size_t x, size_t y) -> ElementT& {
  return elements[x * SectorSize + y];
}

template <typename ElementT, size_t SectorSize>
SectorArray2D<ElementT, SectorSize>::SectorArray2D() = default;

template <typename ElementT, size_t SectorSize>
SectorArray2D<ElementT, SectorSize>::SectorArray2D(size_t numSectorsWide, size_t numSectorsHigh) {
  init(numSectorsWide, numSectorsHigh);
}

template <typename ElementT, size_t SectorSize>
void SectorArray2D<ElementT, SectorSize>::init(size_t numSectorsWide, size_t numSectorsHigh) {
  m_sectors.clear();
  m_sectors.setSize(numSectorsWide, numSectorsHigh);
  m_loadedSectors.clear();
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::width() const -> size_t {
  return m_sectors.size(0) * SectorSize;
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::height() const -> size_t {
  return m_sectors.size(1) * SectorSize;
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::sectorValid(Sector const& sector) const -> bool {
  return sector[0] < m_sectors.size(0) && sector[1] < m_sectors.size(1);
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::sectorFor(size_t x, size_t y) const -> Sector {
  return {x / SectorSize, y / SectorSize};
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::sectorRange(size_t minX, size_t minY, size_t width, size_t height) const -> SectorRange {
  return {
    {minX / SectorSize, minY / SectorSize},
    {(minX + width + SectorSize - 1) / SectorSize, (minY + height + SectorSize - 1) / SectorSize}};
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::sectorCorner(Sector const& id) const -> Vec2S {
  return {id[0] * SectorSize, id[1] * SectorSize};
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::hasSector(Sector const& id) const -> bool {
  return (bool)m_sectors(id[0], id[1]);
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::loadedSectors() const -> List<Sector> {
  return m_loadedSectors.values();
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::loadedSectorCount() const -> size_t {
  return m_loadedSectors.size();
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::sectorLoaded(Sector const& id) const -> bool {
  return m_loadedSectors.contains(id);
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::sector(Sector const& id) -> Array* {
  return m_sectors(id[0], id[1]).get();
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::sector(Sector const& id) const -> Array const* {
  return m_sectors(id[0], id[1]).get();
}

template <typename ElementT, size_t SectorSize>
void SectorArray2D<ElementT, SectorSize>::loadSector(Sector const& id, ArrayPtr array) {
  auto& data = m_sectors(id[0], id[1]);
  data = std::move(array);
  if (data)
    m_loadedSectors.add(id);
  else
    m_loadedSectors.remove(id);
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::copySector(
  Sector const& id) -> typename SectorArray2D<ElementT, SectorSize>::ArrayPtr {
  if (auto const& array = m_sectors(id))
    return std::make_unique<Array>(*array);
  else
    return {};
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::takeSector(
  Sector const& id) -> typename SectorArray2D<ElementT, SectorSize>::ArrayPtr {
  ArrayPtr ret;
  m_loadedSectors.remove(id);
  std::swap(m_sectors(id[0], id[1]), ret);
  return ret;
}

template <typename ElementT, size_t SectorSize>
void SectorArray2D<ElementT, SectorSize>::discardSector(Sector const& id) {
  m_loadedSectors.remove(id);
  m_sectors(id[0], id[1]).reset();
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::get(
  size_t x, size_t y) const -> typename SectorArray2D<ElementT, SectorSize>::Element const* {
  Array* array = m_sectors(x / SectorSize, y / SectorSize).get();
  if (array) {
    return &(*array)(x % SectorSize, y % SectorSize);
  } else {
    return nullptr;
  }
}

template <typename ElementT, size_t SectorSize>
auto SectorArray2D<ElementT, SectorSize>::get(size_t x, size_t y) -> typename SectorArray2D<ElementT, SectorSize>::Element* {
  Array* array = m_sectors(x / SectorSize, y / SectorSize).get();
  if (array)
    return &(*array)(x % SectorSize, y % SectorSize);
  else
    return nullptr;
}

template <typename ElementT, size_t SectorSize>
template <typename Function>
auto SectorArray2D<ElementT, SectorSize>::eval(
  size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty) const -> bool {
  return const_cast<SectorArray2D*>(this)->evalPriv(minX, minY, width, height, std::forward<Function>(function), evalEmpty);
}

template <typename ElementT, size_t SectorSize>
template <typename Function>
auto SectorArray2D<ElementT, SectorSize>::eval(
  size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty) -> bool {
  return evalPriv(minX, minY, width, height, std::forward<Function>(function), evalEmpty);
}

template <typename ElementT, size_t SectorSize>
template <typename Function>
auto SectorArray2D<ElementT, SectorSize>::evalColumns(
  size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty) const -> bool {
  return const_cast<SectorArray2D*>(this)->evalColumnsPriv(
    minX, minY, width, height, std::forward<Function>(function), evalEmpty);
}

template <typename ElementT, size_t SectorSize>
template <typename Function>
auto SectorArray2D<ElementT, SectorSize>::evalColumns(
  size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty) -> bool {
  return evalColumnsPriv(minX, minY, width, height, std::forward<Function>(function), evalEmpty);
}

template <typename ElementT, size_t SectorSize>
template <typename Function>
auto SectorArray2D<ElementT, SectorSize>::evalPriv(
  size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty) -> bool {
  return evalColumnsPriv(minX, minY, width, height, [&function](size_t x, size_t y, Element* column, size_t columnSize) -> auto {
        for (size_t i = 0; i < columnSize; ++i) {
          if (column) {
            if (!function(x, y + i, column + i))
              return false;
          } else {
            if (!function(x, y + i, nullptr))
              return false;
          }
        }
        return true; }, evalEmpty);
}

template <typename ElementT, size_t SectorSize>
template <typename Function>
auto SectorArray2D<ElementT, SectorSize>::evalColumnsPriv(
  size_t minX, size_t minY, size_t width, size_t height, Function&& function, bool evalEmpty) -> bool {
  if (width == 0 || height == 0)
    return true;

  size_t maxX = minX + width;
  size_t maxY = minY + height;
  size_t minXSector = minX / SectorSize;
  size_t maxXSector = (maxX - 1) / SectorSize;

  size_t minYSector = minY / SectorSize;
  size_t maxYSector = (maxY - 1) / SectorSize;

  for (size_t xSector = minXSector; xSector <= maxXSector; ++xSector) {
    size_t minXi = 0;
    if (xSector == minXSector)
      minXi = minX % SectorSize;

    size_t maxXi = SectorSize - 1;
    if (xSector == maxXSector)
      maxXi = (maxX - 1) % SectorSize;

    for (size_t ySector = minYSector; ySector <= maxYSector; ++ySector) {
      Array* array = m_sectors(xSector, ySector).get();

      if (!array && !evalEmpty)
        continue;

      size_t minYi = 0;
      if (ySector == minYSector)
        minYi = minY % SectorSize;

      size_t maxYi = SectorSize - 1;
      if (ySector == maxYSector)
        maxYi = (maxY - 1) % SectorSize;

      size_t y_ = ySector * SectorSize;
      size_t x_ = xSector * SectorSize;
      if (!array) {
        for (size_t xi = minXi; xi <= maxXi; ++xi) {
          if (!function(xi + x_, minYi + y_, nullptr, maxYi - minYi + 1))
            return false;
        }
      } else {
        for (size_t xi = minXi; xi <= maxXi; ++xi) {
          if (!function(xi + x_, minYi + y_, &array->elements[xi * SectorSize + minYi], maxYi - minYi + 1))
            return false;
        }
      }
    }
  }

  return true;
}

}// namespace Star
