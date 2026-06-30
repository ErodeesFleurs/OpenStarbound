#pragma once

#include "StarPoly.hpp"

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

  [[nodiscard]] bool isNull() const;

  [[nodiscard]] bool operator==(WorldGeometry const& other) const;
  [[nodiscard]] bool operator!=(WorldGeometry const& other) const;

  [[nodiscard]] unsigned width() const;
  [[nodiscard]] unsigned height() const;
  [[nodiscard]] Vec2U size() const;

  // Wrap given point back into world space by wrapping x
  [[nodiscard]] int xwrap(int x) const;
  [[nodiscard]] float xwrap(float x) const;
  // Only wraps x component.
  [[nodiscard]] Vec2F xwrap(Vec2F const& pos) const;
  [[nodiscard]] Vec2I xwrap(Vec2I const& pos) const;

  // y value is clamped to be in the range [0, height)
  [[nodiscard]] float yclamp(float y) const;

  // Wraps and clamps position
  [[nodiscard]] Vec2F limit(Vec2F const& pos) const;

  [[nodiscard]] bool crossesWrap(float xMin, float xMax) const;

  // Do these two inexes point to the same location
  [[nodiscard]] bool equal(Vec2I const& p1, Vec2I const& p2) const;

  // Same as wrap, returns unsigned type.
  [[nodiscard]] unsigned index(int x) const;
  [[nodiscard]] Vec2U index(Vec2I const& i) const;

  // returns right only distance from x2 to x1 (or x1 - x2).  Always positive.
  [[nodiscard]] int pdiff(int x1, int x2) const;

  // Shortest difference between two given points.  Always returns diff on the
  // "side" that x1 is on.
  [[nodiscard]] float diff(float x1, float x2) const;
  [[nodiscard]] int diff(int x1, int x2) const;

  // Same but for 2d vectors
  [[nodiscard]] Vec2F diff(Vec2F const& p1, Vec2F const& p2) const;
  [[nodiscard]] Vec2I diff(Vec2I const& p1, Vec2I const& p2) const;

  // Midpoint of the shortest line connecting two points.
  [[nodiscard]] Vec2F midpoint(Vec2F const& p1, Vec2F const& p2) const;

  [[nodiscard]] function<float(float, float)> xDiffFunction() const;
  [[nodiscard]] function<Vec2F(Vec2F, Vec2F)> diffFunction() const;
  [[nodiscard]] function<float(float, float, float)> xLerpFunction(Maybe<float> discontinuityThreshold = {}) const;
  [[nodiscard]] function<Vec2F(float, Vec2F, Vec2F)> lerpFunction(Maybe<float> discontinuityThreshold = {}) const;

  // Wrapping functions are not guaranteed to work for objects larger than
  // worldWidth / 2.  Bad things can happen.

  // Split the given Rect across world boundaries.
  [[nodiscard]] StaticList<RectF, 2> splitRect(RectF const& bbox) const;
  // Split the given Rect after translating it by position.
  [[nodiscard]] StaticList<RectF, 2> splitRect(RectF bbox, Vec2F const& position) const;

  [[nodiscard]] StaticList<RectI, 2> splitRect(RectI bbox) const;

  // Same but for Line
  [[nodiscard]] StaticList<Line2F, 2> splitLine(Line2F line, bool preserveDirection = false) const;
  [[nodiscard]] StaticList<Line2F, 2> splitLine(Line2F line, Vec2F const& position, bool preserveDirection = false) const;

  // Same but for Poly
  [[nodiscard]] StaticList<PolyF, 2> splitPoly(PolyF const& poly) const;
  [[nodiscard]] StaticList<PolyF, 2> splitPoly(PolyF poly, Vec2F const& position) const;

  // Split a horizontal region of the world across the world wrap point.
  [[nodiscard]] StaticList<Vec2I, 2> splitXRegion(Vec2I const& xRegion) const;
  [[nodiscard]] StaticList<Vec2F, 2> splitXRegion(Vec2F const& xRegion) const;

  [[nodiscard]] bool rectContains(RectF const& rect1, Vec2F const& pos) const;
  [[nodiscard]] bool rectIntersectsRect(RectF const& rect1, RectF const& rect2) const;
  [[nodiscard]] RectF rectOverlap(RectF const& rect1, RectF const& rect2) const;
  [[nodiscard]] bool polyContains(PolyF const& poly, Vec2F const& pos) const;
  [[nodiscard]] float polyOverlapArea(PolyF const& poly1, PolyF const& poly2) const;

  [[nodiscard]] bool lineIntersectsRect(Line2F const& line, RectF const& rect) const;
  [[nodiscard]] bool lineIntersectsPoly(Line2F const& line, PolyF const& poly) const;
  [[nodiscard]] bool polyIntersectsPoly(PolyF const& poly1, PolyF const& poly2) const;

  [[nodiscard]] bool rectIntersectsCircle(RectF const& rect, Vec2F const& center, float radius) const;
  [[nodiscard]] bool lineIntersectsCircle(Line2F const& line, Vec2F const& center, float radius) const;

  [[nodiscard]] Maybe<Vec2F> lineIntersectsPolyAt(Line2F const& line, PolyF const& poly) const;

  // Returns the distance from a point to any part of the given poly
  [[nodiscard]] float polyDistance(PolyF const& poly, Vec2F const& point) const;

  // Produces a point that is on the same "side" of the world as the source point.
  [[nodiscard]] int nearestTo(int source, int target) const;
  [[nodiscard]] float nearestTo(float source, float target) const;
  [[nodiscard]] Vec2I nearestTo(Vec2I const& source, Vec2I const& target) const;
  [[nodiscard]] Vec2F nearestTo(Vec2F const& source, Vec2F const& target) const;

  [[nodiscard]] Vec2F nearestCoordInBox(RectF const& box, Vec2F const& pos) const;
  [[nodiscard]] Vec2F diffToNearestCoordInBox(RectF const& box, Vec2F const& pos) const;

private:
  Vec2U m_size;
};

inline WorldGeometry::WorldGeometry()
  : m_size(Vec2U()) {}

inline WorldGeometry::WorldGeometry(unsigned width, unsigned height)
  : m_size(width, height) {}

inline WorldGeometry::WorldGeometry(Vec2U const& size)
  : m_size(size) {}

[[nodiscard]] inline bool WorldGeometry::isNull() const {
  return m_size == Vec2U();
}

[[nodiscard]] inline bool WorldGeometry::operator==(WorldGeometry const& other) const {
  return m_size == other.m_size;
}

[[nodiscard]] inline bool WorldGeometry::operator!=(WorldGeometry const& other) const {
  return m_size != other.m_size;
}

[[nodiscard]] inline unsigned WorldGeometry::width() const {
  return m_size[0];
}

[[nodiscard]] inline unsigned WorldGeometry::height() const {
  return m_size[1];
}

[[nodiscard]] inline Vec2U WorldGeometry::size() const {
  return m_size;
}

[[nodiscard]] inline int WorldGeometry::xwrap(int x) const {
  if (m_size[0] == 0)
    return x;
  else
    return pmod<int>(x, m_size[0]);
}

[[nodiscard]] inline float WorldGeometry::xwrap(float x) const {
  if (m_size[0] == 0)
    return x;
  else
    return pfmod<float>(x, m_size[0]);
}

[[nodiscard]] inline Vec2F WorldGeometry::xwrap(Vec2F const& pos) const {
  return {xwrap(pos[0]), pos[1]};
}

[[nodiscard]] inline Vec2I WorldGeometry::xwrap(Vec2I const& pos) const {
  return {xwrap(pos[0]), pos[1]};
}

[[nodiscard]] inline float WorldGeometry::yclamp(float y) const {
  return clamp<float>(y, 0, std::nextafter(m_size[1], 0.0f));
}

[[nodiscard]] inline Vec2F WorldGeometry::limit(Vec2F const& pos) const {
  return {xwrap(pos[0]), yclamp(pos[1])};
}

[[nodiscard]] inline bool WorldGeometry::crossesWrap(float xMin, float xMax) const {
  return xwrap(xMax) < xwrap(xMin);
}

[[nodiscard]] inline bool WorldGeometry::equal(Vec2I const& p1, Vec2I const& p2) const {
  return index(p1) == index(p2);
}

[[nodiscard]] inline unsigned WorldGeometry::index(int x) const {
  return static_cast<unsigned>(xwrap(x));
}

[[nodiscard]] inline Vec2U WorldGeometry::index(Vec2I const& i) const {
  return Vec2U(xwrap(i[0]), i[1]);
}

[[nodiscard]] inline int WorldGeometry::pdiff(int x1, int x2) const {
  if (m_size[0] == 0)
    return x1 - x2;
  else
    return pmod<int>(x1 - x2, m_size[0]);
}

[[nodiscard]] inline float WorldGeometry::diff(float x1, float x2) const {
  if (m_size[0] == 0)
    return x1 - x2;
  else
    return wrapDiffF<float>(x1, x2, m_size[0]);
}

[[nodiscard]] inline int WorldGeometry::diff(int x1, int x2) const {
  if (m_size[0] == 0)
    return x1 - x2;
  else
    return wrapDiff<int>(x1, x2, m_size[0]);
}

[[nodiscard]] inline Vec2F WorldGeometry::diff(Vec2F const& p1, Vec2F const& p2) const {
  float xdiff = diff(p1[0], p2[0]);
  return {xdiff, p1[1] - p2[1]};
}

[[nodiscard]] inline Vec2I WorldGeometry::diff(Vec2I const& p1, Vec2I const& p2) const {
  int xdiff = diff(p1[0], p2[0]);
  return {xdiff, p1[1] - p2[1]};
}

[[nodiscard]] inline Vec2F WorldGeometry::midpoint(Vec2F const& p1, Vec2F const& p2) const {
  return xwrap(diff(p1, p2) / 2 + p2);
}

[[nodiscard]] inline int WorldGeometry::nearestTo(int source, int target) const {
  if (abs(target - source) < (int)(m_size[0] / 2))
    return target;
  else
    return diff(target, source) + source;
}

[[nodiscard]] inline float WorldGeometry::nearestTo(float source, float target) const {
  if (abs(target - source) < (float)(m_size[0] / 2))
    return target;
  else
    return diff(target, source) + source;
}

[[nodiscard]] inline Vec2I WorldGeometry::nearestTo(Vec2I const& source, Vec2I const& target) const {
  return Vec2I(nearestTo(source[0], target[0]), target[1]);
}

[[nodiscard]] inline Vec2F WorldGeometry::nearestTo(Vec2F const& source, Vec2F const& target) const {
  return Vec2F(nearestTo(source[0], target[0]), target[1]);
}

}
