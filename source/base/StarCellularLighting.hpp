#pragma once

#include "StarCellularLightArray.hpp"
#include "StarEither.hpp"
#include "StarException.hpp"
#include "StarImage.hpp"
#include "StarJson.hpp"
#include "StarRect.hpp"

import std;

namespace Star {

using LightmapException = ExceptionDerived<"LightmapException">;

class Lightmap {
public:
  Lightmap();
  Lightmap(unsigned width, unsigned height);
  Lightmap(Lightmap const& lightMap);
  Lightmap(Lightmap&& lightMap) noexcept;

  auto operator=(Lightmap const& lightMap) -> Lightmap&;
  auto operator=(Lightmap&& lightMap) noexcept -> Lightmap&;

  explicit operator ImageView();

  void set(unsigned x, unsigned y, float v);
  void set(unsigned x, unsigned y, Vec3F const& v);
  void add(unsigned x, unsigned y, Vec3F const& v);
  [[nodiscard]] auto get(unsigned x, unsigned y) const -> Vec3F;

  [[nodiscard]] auto empty() const -> bool;

  [[nodiscard]] auto size() const -> Vec2U;
  [[nodiscard]] auto width() const -> unsigned;
  [[nodiscard]] auto height() const -> unsigned;
  auto data() -> float*;

private:
  [[nodiscard]] auto len() const -> std::size_t;

  std::unique_ptr<float[]> m_data;
  unsigned m_width;
  unsigned m_height;
};

inline void Lightmap::set(unsigned x, unsigned y, float v) {
  if (x >= m_width || y >= m_height) {
    throw LightmapException(strf("[{}, {}] out of range in Lightmap::set", x, y));
    return;
  }
  float* ptr = m_data.get() + (y * m_width * 3 + x * 3);
  ptr[0] = ptr[1] = ptr[2] = v;
}

inline void Lightmap::set(unsigned x, unsigned y, Vec3F const& v) {
  if (x >= m_width || y >= m_height) {
    throw LightmapException(strf("[{}, {}] out of range in Lightmap::set", x, y));
    return;
  }
  float* ptr = m_data.get() + (y * m_width * 3 + x * 3);
  ptr[0] = v.x();
  ptr[1] = v.y();
  ptr[2] = v.z();
}

inline void Lightmap::add(unsigned x, unsigned y, Vec3F const& v) {
  if (x >= m_width || y >= m_height) {
    throw LightmapException(strf("[{}, {}] out of range in Lightmap::add", x, y));
    return;
  }
  float* ptr = m_data.get() + (y * m_width * 3 + x * 3);
  ptr[0] += v.x();
  ptr[1] += v.y();
  ptr[2] += v.z();
}

inline auto Lightmap::get(unsigned x, unsigned y) const -> Vec3F {
  if (x >= m_width || y >= m_height) {
    throw LightmapException(strf("[{}, {}] out of range in Lightmap::get", x, y));
    return {};
  }
  float* ptr = m_data.get() + (y * m_width * 3 + x * 3);
  return Vec3F{ptr[0], ptr[1], ptr[2]};
}

inline auto Lightmap::empty() const -> bool {
  return m_width == 0 || m_height == 0;
}

inline auto Lightmap::size() const -> Vec2U {
  return Vec2U{m_width, m_height};
}

inline auto Lightmap::width() const -> unsigned {
  return m_width;
}

inline auto Lightmap::height() const -> unsigned {
  return m_height;
}

inline auto Lightmap::data() -> float* {
  return m_data.get();
}

inline auto Lightmap::len() const -> std::size_t {
  return m_width * m_height * 3;
}

// Produce lighting values from an integral cellular grid.  Allows for floating
// positional point and cellular light sources, as well as pre-lighting cells
// individually.
class CellularLightingCalculator {
public:
  explicit CellularLightingCalculator(bool monochrome = false);

  using Cell = ColoredCellularLightArray::Cell;

  void setMonochrome(bool monochrome);

  void setParameters(Json const& config);

  // Call 'begin' to start a calculation for the given region
  void begin(RectI const& queryRegion);

  // Once begin is called, this will return the region that could possibly
  // affect the target calculation region.  All lighting values should be set
  // for the given calculation region before calling 'calculate'.
  [[nodiscard]] auto calculationRegion() const -> RectI;

  auto baseIndexFor(Vec2I const& position) -> std::size_t;

  void setCellIndex(std::size_t cellIndex, Vec3F const& light, bool obstacle);

  void addSpreadLight(Vec2F const& position, Vec3F const& light);
  void addPointLight(Vec2F const& position, Vec3F const& light, float beam, float beamAngle, float beamAmbience, bool asSpread = false);

  // Finish the calculation, and put the resulting color data in the given
  // output image.  The image will be reset to the size of the region given in
  // the call to 'begin', and formatted as RGB24.
  void calculate(Image& output);
  // Same as above, but the color data in a float buffer instead.
  void calculate(Lightmap& output);

  void setupImage(Image& image, PixelFormat format = PixelFormat::RGB24) const;

private:
  Json m_config;
  bool m_monochrome;
  Either<ColoredCellularLightArray, ScalarCellularLightArray> m_lightArray;
  RectI m_queryRegion;
  RectI m_calculationRegion;
};

// Produce light intensity values using the same algorithm as
// CellularLightingCalculator.  Only calculates a single point at a time, and
// uses scalar lights with no color calculation.
class CellularLightIntensityCalculator {
public:
  using Cell = ScalarCellularLightArray::Cell;

  void setParameters(Json const& config);

  void begin(Vec2F const& queryPosition);

  [[nodiscard]] auto calculationRegion() const -> RectI;

  void setCell(Vec2I const& position, Cell const& cell);
  void setCellColumn(Vec2I const& position, Cell const* cells, std::size_t count);

  void addSpreadLight(Vec2F const& position, float light);
  void addPointLight(Vec2F const& position, float light, float beam, float beamAngle, float beamAmbience);

  auto calculate() -> float;

private:
  ScalarCellularLightArray m_lightArray;
  Vec2F m_queryPosition;
  RectI m_queryRegion;
  ;
  RectI m_calculationRegion;
};

inline auto CellularLightingCalculator::baseIndexFor(Vec2I const& position) -> std::size_t {
  return (position[0] - m_calculationRegion.xMin()) * m_calculationRegion.height() + position[1] - m_calculationRegion.yMin();
}

inline void CellularLightingCalculator::setCellIndex(std::size_t cellIndex, Vec3F const& light, bool obstacle) {
  if (m_monochrome) {
    m_lightArray.right().cellAtIndex(cellIndex) = ScalarCellularLightArray::Cell{.light = light.sum() / 3, .obstacle = obstacle};
  } else {
    m_lightArray.left().cellAtIndex(cellIndex) = ColoredCellularLightArray::Cell{.light = light, .obstacle = obstacle};
  }
}

}// namespace Star
