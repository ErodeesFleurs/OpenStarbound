#pragma once

#include "StarPoly.hpp"

import std;

namespace Star {

// Utility class for dealing with the non-euclidean nature of the World.
// Handles the surprisingly complex job of deciding intersections and splitting
// geometry across the world wrap boundary.
class WorldGeometry {
public:
  // A null WorldGeometry will have diff / wrap methods etc be the normal
  // euclidean variety.
  WorldGeometry();
  WorldGeometry(unsigned width, unsigned height);
  WorldGeometry(Vec2U const& size);

  auto isNull() -> bool;

  auto operator==(WorldGeometry const& other) const -> bool;
  auto operator!=(WorldGeometry const& other) const -> bool;

  [[nodiscard]] auto width() const -> unsigned;
  [[nodiscard]] auto height() const -> unsigned;
  [[nodiscard]] auto size() const -> Vec2U;

  // Wrap given point back into world space by wrapping x
  [[nodiscard]] auto xwrap(int x) const -> int;
  [[nodiscard]] auto xwrap(float x) const -> float;
  // Only wraps x component.
  [[nodiscard]] auto xwrap(Vec2F const& pos) const -> Vec2F;
  [[nodiscard]] auto xwrap(Vec2I const& pos) const -> Vec2I;

  // y value is clamped to be in the range [0, height)
  [[nodiscard]] auto yclamp(float y) const -> float;

  // Wraps and clamps position
  [[nodiscard]] auto limit(Vec2F const& pos) const -> Vec2F;

  [[nodiscard]] auto crossesWrap(float xMin, float xMax) const -> bool;

  // Do these two inexes point to the same location
  [[nodiscard]] auto equal(Vec2I const& p1, Vec2I const& p2) const -> bool;

  // Same as wrap, returns unsigned type.
  [[nodiscard]] auto index(int x) const -> unsigned;
  [[nodiscard]] auto index(Vec2I const& i) const -> Vec2U;

  // returns right only distance from x2 to x1 (or x1 - x2).  Always positive.
  [[nodiscard]] auto pdiff(int x1, int x2) const -> int;

  // Shortest difference between two given points.  Always returns diff on the
  // "side" that x1 is on.
  [[nodiscard]] auto diff(float x1, float x2) const -> float;
  [[nodiscard]] auto diff(int x1, int x2) const -> int;

  // Same but for 2d vectors
  [[nodiscard]] auto diff(Vec2F const& p1, Vec2F const& p2) const -> Vec2F;
  [[nodiscard]] auto diff(Vec2I const& p1, Vec2I const& p2) const -> Vec2I;

  // Midpoint of the shortest line connecting two points.
  [[nodiscard]] auto midpoint(Vec2F const& p1, Vec2F const& p2) const -> Vec2F;

  [[nodiscard]] auto xDiffFunction() const -> std::function<float(float, float)>;
  [[nodiscard]] auto diffFunction() const -> std::function<Vec2F(Vec2F, Vec2F)>;
  [[nodiscard]] auto xLerpFunction(std::optional<float> discontinuityThreshold = {}) const -> std::function<float(float, float, float)>;
  [[nodiscard]] auto lerpFunction(std::optional<float> discontinuityThreshold = {}) const -> std::function<Vec2F(float, Vec2F, Vec2F)>;

  // Wrapping functions are not guaranteed to work for objects larger than
  // worldWidth / 2.  Bad things can happen.

  // Split the given Rect across world boundaries.
  [[nodiscard]] auto splitRect(RectF const& bbox) const -> StaticList<RectF, 2>;
  // Split the given Rect after translating it by position.
  [[nodiscard]] auto splitRect(RectF bbox, Vec2F const& position) const -> StaticList<RectF, 2>;

  [[nodiscard]] auto splitRect(RectI bbox) const -> StaticList<RectI, 2>;

  // Same but for Line
  [[nodiscard]] auto splitLine(Line2F line, bool preserveDirection = false) const -> StaticList<Line2F, 2>;
  [[nodiscard]] auto splitLine(Line2F line, Vec2F const& position, bool preserveDirection = false) const -> StaticList<Line2F, 2>;

  // Same but for Poly
  [[nodiscard]] auto splitPoly(PolyF const& poly) const -> StaticList<PolyF, 2>;
  [[nodiscard]] auto splitPoly(PolyF poly, Vec2F const& position) const -> StaticList<PolyF, 2>;

  // Split a horizontal region of the world across the world wrap point.
  [[nodiscard]] auto splitXRegion(Vec2I const& xRegion) const -> StaticList<Vec2I, 2>;
  [[nodiscard]] auto splitXRegion(Vec2F const& xRegion) const -> StaticList<Vec2F, 2>;

  [[nodiscard]] auto rectContains(RectF const& rect1, Vec2F const& pos) const -> bool;
  [[nodiscard]] auto rectIntersectsRect(RectF const& rect1, RectF const& rect2) const -> bool;
  [[nodiscard]] auto rectOverlap(RectF const& rect1, RectF const& rect2) const -> RectF;
  [[nodiscard]] auto polyContains(PolyF const& poly, Vec2F const& pos) const -> bool;
  [[nodiscard]] auto polyOverlapArea(PolyF const& poly1, PolyF const& poly2) const -> float;

  [[nodiscard]] auto lineIntersectsRect(Line2F const& line, RectF const& rect) const -> bool;
  [[nodiscard]] auto lineIntersectsPoly(Line2F const& line, PolyF const& poly) const -> bool;
  [[nodiscard]] auto polyIntersectsPoly(PolyF const& poly1, PolyF const& poly2) const -> bool;

  [[nodiscard]] auto rectIntersectsCircle(RectF const& rect, Vec2F const& center, float radius) const -> bool;
  [[nodiscard]] auto lineIntersectsCircle(Line2F const& line, Vec2F const& center, float radius) const -> bool;

  [[nodiscard]] auto lineIntersectsPolyAt(Line2F const& line, PolyF const& poly) const -> std::optional<Vec2F>;

  // Returns the distance from a point to any part of the given poly
  [[nodiscard]] auto polyDistance(PolyF const& poly, Vec2F const& point) const -> float;

  // Produces a point that is on the same "side" of the world as the source point.
  [[nodiscard]] auto nearestTo(int source, int target) const -> int;
  [[nodiscard]] auto nearestTo(float source, float target) const -> float;
  [[nodiscard]] auto nearestTo(Vec2I const& source, Vec2I const& target) const -> Vec2I;
  [[nodiscard]] auto nearestTo(Vec2F const& source, Vec2F const& target) const -> Vec2F;

  [[nodiscard]] auto nearestCoordInBox(RectF const& box, Vec2F const& pos) const -> Vec2F;
  [[nodiscard]] auto diffToNearestCoordInBox(RectF const& box, Vec2F const& pos) const -> Vec2F;

private:
  Vec2U m_size;
};

inline WorldGeometry::WorldGeometry()
    : m_size(Vec2U()) {}

inline WorldGeometry::WorldGeometry(unsigned width, unsigned height)
    : m_size(width, height) {}

inline WorldGeometry::WorldGeometry(Vec2U const& size)
    : m_size(size) {}

inline auto WorldGeometry::isNull() -> bool {
  return m_size == Vec2U();
}

inline auto WorldGeometry::operator==(WorldGeometry const& other) const -> bool {
  return m_size == other.m_size;
}

inline auto WorldGeometry::operator!=(WorldGeometry const& other) const -> bool {
  return m_size != other.m_size;
}

inline auto WorldGeometry::width() const -> unsigned {
  return m_size[0];
}

inline auto WorldGeometry::height() const -> unsigned {
  return m_size[1];
}

inline auto WorldGeometry::size() const -> Vec2U {
  return m_size;
}

inline auto WorldGeometry::xwrap(int x) const -> int {
  if (m_size[0] == 0)
    return x;
  else
    return pmod<int>(x, m_size[0]);
}

inline auto WorldGeometry::xwrap(float x) const -> float {
  if (m_size[0] == 0)
    return x;
  else
    return pfmod<float>(x, m_size[0]);
}

inline auto WorldGeometry::xwrap(Vec2F const& pos) const -> Vec2F {
  return {xwrap(pos[0]), pos[1]};
}

inline auto WorldGeometry::xwrap(Vec2I const& pos) const -> Vec2I {
  return {xwrap(pos[0]), pos[1]};
}

inline auto WorldGeometry::yclamp(float y) const -> float {
  return clamp<float>(y, 0, std::nextafter(m_size[1], 0.0f));
}

inline auto WorldGeometry::limit(Vec2F const& pos) const -> Vec2F {
  return {xwrap(pos[0]), yclamp(pos[1])};
}

inline auto WorldGeometry::crossesWrap(float xMin, float xMax) const -> bool {
  return xwrap(xMax) < xwrap(xMin);
}

inline auto WorldGeometry::equal(Vec2I const& p1, Vec2I const& p2) const -> bool {
  return index(p1) == index(p2);
}

inline auto WorldGeometry::index(int x) const -> unsigned {
  return (unsigned)xwrap(x);
}

inline auto WorldGeometry::index(Vec2I const& i) const -> Vec2U {
  return {static_cast<std::uint32_t>(xwrap(i[0])), i[1]};
}

inline auto WorldGeometry::pdiff(int x1, int x2) const -> int {
  if (m_size[0] == 0)
    return x1 - x2;
  else
    return pmod<int>(x1 - x2, m_size[0]);
}

inline auto WorldGeometry::diff(float x1, float x2) const -> float {
  if (m_size[0] == 0)
    return x1 - x2;
  else
    return wrapDiffF<float>(x1, x2, m_size[0]);
}

inline auto WorldGeometry::diff(int x1, int x2) const -> int {
  if (m_size[0] == 0)
    return x1 - x2;
  else
    return wrapDiff<int>(x1, x2, m_size[0]);
}

inline auto WorldGeometry::diff(Vec2F const& p1, Vec2F const& p2) const -> Vec2F {
  float xdiff = diff(p1[0], p2[0]);
  return {xdiff, p1[1] - p2[1]};
}

inline auto WorldGeometry::diff(Vec2I const& p1, Vec2I const& p2) const -> Vec2I {
  int xdiff = diff(p1[0], p2[0]);
  return {xdiff, p1[1] - p2[1]};
}

inline auto WorldGeometry::midpoint(Vec2F const& p1, Vec2F const& p2) const -> Vec2F {
  return xwrap(diff(p1, p2) / 2 + p2);
}

inline auto WorldGeometry::nearestTo(int source, int target) const -> int {
  if (std::abs(target - source) < (int)(m_size[0] / 2))
    return target;
  else
    return diff(target, source) + source;
}

inline auto WorldGeometry::nearestTo(float source, float target) const -> float {
  if (std::abs(target - source) < (float)(m_size[0] / 2))
    return target;
  else
    return diff(target, source) + source;
}

inline auto WorldGeometry::nearestTo(Vec2I const& source, Vec2I const& target) const -> Vec2I {
  return {nearestTo(source[0], target[0]), target[1]};
}

inline auto WorldGeometry::nearestTo(Vec2F const& source, Vec2F const& target) const -> Vec2F {
  return {nearestTo(source[0], target[0]), target[1]};
}

}// namespace Star
