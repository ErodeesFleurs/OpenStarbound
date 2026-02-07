#pragma once

#include "StarLine.hpp"
#include "StarList.hpp"

import std;

namespace Star {

// Axis aligned box that can be used as a bounding volume.
template <typename T, std::size_t N>
class Box {
public:
  using Coord = Vector<T, N>;
  using Line = Star::Line<T, N>;
  using LineIntersectResult = typename Line::IntersectResult;

  template <std::size_t P, typename T2 = void>
  using Enable2D = std::enable_if_t<P == 2 && N == P, T2>;

  struct IntersectResult {
    // Whether or not the two objects intersect
    bool intersects;
    // How much *this* box must be moved in order to make them not intersect
    // anymore
    Coord overlap;
    // Whether or not the intersection is touching only.  No overlap.
    bool glances;
  };

  static auto null() -> Box;
  static auto inf() -> Box;

  // Returns an integral aligned box that at least contains the given floating
  // point box.
  template <typename Box2>
  static auto integral(Box2 const& box) -> Box;

  // Returns an integral aligned box that is equal to the given box rounded to
  // the nearest whole number (does not necessarily contain the given box).
  template <typename Box2>
  static auto round(Box2 const& box) -> Box;

  template <typename... TN>
  static auto boundBoxOf(TN const&... list) -> Box;

  template <typename Collection>
  static auto boundBoxOfPoints(Collection const& collection) -> Box;

  static auto withSize(Coord const& min, Coord const& size) -> Box;
  static auto withCenter(Coord const& center, Coord const& size) -> Box;

  Box();
  Box(Coord const& min, Coord const& max);
  Box(Box const& b);
  auto operator=(Box const& b) -> Box&;

  template <typename T2>
  explicit Box(Box<T2, N> const& b);

  // Is equal to null()
  [[nodiscard]] auto isNull() const -> bool;

  // One or more dimensions are of negative magnitude
  [[nodiscard]] auto isNegative() const -> bool;

  // One or more dimensions are of zero or negative magnitude
  [[nodiscard]] auto isEmpty() const -> bool;

  // Sets the bounding box equal to one containing the given bounding box and
  // the current one.
  void combine(Box const& box);
  auto combined(Box const& box) const -> Box;

  // Sets the bounding box equal to one containing the current bounding box and
  // the given point.
  void combine(Coord const& point);
  auto combined(Coord const& point) const -> Box;

  // Sets the bounding box equal to the intersection of this one and the given
  // one.  If there is no intersection than the box becomes negative in that
  // dimension.
  void limit(Box const& box);
  auto limited(Box const& box) -> Box;

  // If any range has a min < max, swap them to make it non-null.
  void makePositive();

  // Sets any empty (or negative) dimensions in the bounding box to the
  // corresponding range in the given bounding box.  If the bounding box is not
  // empty in any dimension, then this has no effect.
  void rangeSetIfEmpty(Box const& b);

  auto size() const -> Coord;
  auto size(std::size_t dim) const -> T;

  // Sets bound box to the minimum bound box necessary to both have the given
  // aspect ratio and contain the current bounding box.
  void setAspect(Coord as, bool shrink = false);

  void makeCube();

  auto center() const -> Coord;
  void setCenter(Coord const& c);

  void translate(Coord const& c);
  auto translated(Coord const& c) const -> Box;

  // Translate the Box the minimum distance so that it includes the given point
  void translateToInclude(Coord const& coord, Coord const& padding = Coord());

  auto range(std::size_t dim) const -> Vector<T, 2>;
  void setRange(std::size_t dim, Vector<T, 2> v);
  void combineRange(std::size_t dim, Vector<T, 2> v);
  void limitRange(std::size_t dim, Vector<T, 2> v);

  // Expand from center.
  void expand(T factor);
  auto expanded(T factor) const -> Box;

  // Expand from center.
  void expand(Coord const& factor);
  auto expanded(Vector<T, N> const& factor) const -> Box;

  // Scale around origin.
  void scale(T factor);
  auto scaled(T factor) const -> Box;

  // Scale around origin.
  void scale(Coord const& factor);
  auto scaled(Vector<T, N> const& factor) const -> Box;

  // Increase all dimensions by a constant amount on all sides
  void pad(T amount);
  auto padded(T amount) const -> Box;

  // Increase all dimensions by a constant amount
  void pad(Coord const& amount);
  auto padded(Vector<T, N> const& amount) const -> Box;

  // Opposite of pad
  void trim(T amount);
  auto trimmed(T amount) const -> Box;

  // Opposite of pad
  void trim(Coord const& amount);
  auto trimmed(Vector<T, N> const& amount) const -> Box;

  // Flip around some dimension (may make box have negative volume)
  void flip(std::size_t dimension);
  auto flipped(std::size_t dimension) const -> Box;

  auto min() const -> Coord const&;
  auto max() const -> Coord const&;

  auto min() -> Coord&;
  auto max() -> Coord&;

  void setMin(Coord const& c);
  void setMax(Coord const& c);

  auto volume() const -> T;
  auto overlap(Box const& b) const -> Box;

  auto intersection(Box const& b) const -> IntersectResult;
  auto intersects(Box const& b, bool includeEdges = true) const -> bool;

  auto contains(Coord const& p, bool includeEdges = true) const -> bool;
  auto contains(Box const& b, bool includeEdges = true) const -> bool;

  // A version of contains that includes the min edges but not the max edges,
  // useful to select based on adjoining boxes without overlap.
  auto belongs(Coord const& p) const -> bool;

  auto containsEpsilon(Coord const& p, unsigned epsilons = 2) const -> bool;
  auto containsEpsilon(Box const& b, unsigned epsilons = 2) const -> bool;

  auto operator==(Box const& ref) const -> bool;
  auto operator!=(Box const& ref) const -> bool;

  // Find Coord inside box nearest to
  auto nearestCoordTo(Coord const& c) const -> Coord;

  // Find the coord in normalized space for this rect, so that 0 is the minimum
  // and 1 is the maximum.
  auto normal(Coord const& coord) const -> Coord;

  // The invers of normal, find the real space position of this normalized
  // coordinate.
  auto eval(Coord const& normalizedCoord) const -> Coord;

  // 2D Only

  // Slightly different to make ctor work
  template <std::size_t P = N, class = Enable2D<P>>
  Box(T minx, T miny, T maxx, T maxy);

  template <std::size_t P = N>
  auto xMin() const -> Enable2D<P, T>;
  template <std::size_t P = N>
  auto xMax() const -> Enable2D<P, T>;
  template <std::size_t P = N>
  auto yMin() const -> Enable2D<P, T>;
  template <std::size_t P = N>
  auto yMax() const -> Enable2D<P, T>;

  template <std::size_t P = N>
  auto setXMin(T xMin) -> Enable2D<P>;
  template <std::size_t P = N>
  auto setXMax(T xMax) -> Enable2D<P>;
  template <std::size_t P = N>
  auto setYMin(T yMin) -> Enable2D<P>;
  template <std::size_t P = N>
  auto setYMax(T yMax) -> Enable2D<P>;

  template <std::size_t P = N>
  auto width() const -> Enable2D<P, T>;
  template <std::size_t P = N>
  auto height() const -> Enable2D<P, T>;

  template <std::size_t P = N>
  auto translate(T x, T y) -> Enable2D<P, void>;
  template <std::size_t P = N>
  auto translateToInclude(T x, T y, T xPadding = 0, T yPadding = 0) -> Enable2D<P, void>;
  template <std::size_t P = N>
  auto scale(T x, T y) -> Enable2D<P, void>;
  template <std::size_t P = N>
  auto expand(T x, T y) -> Enable2D<P, void>;
  template <std::size_t P = N>
  auto flipHorizontal() -> Enable2D<P, void>;
  template <std::size_t P = N>
  auto flipVertical() -> Enable2D<P, void>;

  template <std::size_t P = N>
  auto edges() const -> Enable2D<P, Array<Line, 4>>;
  template <std::size_t P = N>
  auto intersects(Line const& l) const -> Enable2D<P, bool>;
  template <std::size_t P = N>
  auto intersectsCircle(Coord const& position, T radius) const -> Enable2D<P, bool>;
  template <std::size_t P = N>
  auto edgeIntersection(Line const& l) const -> Enable2D<P, LineIntersectResult>;

  // Returns a list of areas that are in this rect but not in the given rect.
  // Extra Credit: Implement this method for arbitrary dimensions.
  template <std::size_t P = N>
  auto subtract(Box const& rect) const -> Enable2D<P, List<Box>>;

protected:
  template <typename... TN>
  static void combineThings(Box& b, Coord const& point, TN const&... rest);

  template <typename... TN>
  static void combineThings(Box& b, Box const& box, TN const&... rest);

  template <typename... TN>
  static void combineThings(Box& b);

  Coord m_min;
  Coord m_max;
};

template <typename T, std::size_t N>
auto operator<<(std::ostream& os, Box<T, N> const& box) -> std::ostream&;

template <typename T>
using Rect = Box<T, 2>;

using RectI = Rect<int>;
using RectU = Rect<unsigned>;
using RectF = Rect<float>;
using RectD = Rect<double>;

template <typename T, std::size_t N>
auto Box<T, N>::null() -> Box<T, N> {
  return Box(Coord::filled(std::numeric_limits<T>::max()), Coord::filled(std::numeric_limits<T>::lowest()));
}

template <typename T, std::size_t N>
auto Box<T, N>::inf() -> Box<T, N> {
  return Box(Coord::filled(std::numeric_limits<T>::lowest()), Coord::filled(std::numeric_limits<T>::max()));
}

template <typename T, std::size_t N>
template <typename Box2>
auto Box<T, N>::integral(Box2 const& box) -> Box<T, N> {
  return Box(Coord::floor(box.min()), Coord::ceil(box.max()));
}

template <typename T, std::size_t N>
template <typename Box2>
auto Box<T, N>::round(Box2 const& box) -> Box<T, N> {
  return Box(Coord::round(box.min()), Coord::round(box.max()));
}

template <typename T, std::size_t N>
template <typename... TN>
auto Box<T, N>::boundBoxOf(TN const&... list) -> Box<T, N> {
  Box b = null();
  combineThings(b, list...);
  return b;
}

template <typename T, std::size_t N>
template <typename Collection>
auto Box<T, N>::boundBoxOfPoints(Collection const& collection) -> Box<T, N> {
  Box b = null();
  for (auto const& point : collection)
    b.combine(Coord(point));
  return b;
}

template <typename T, std::size_t N>
auto Box<T, N>::withSize(Coord const& min, Coord const& size) -> Box<T, N> {
  return Box(min, min + size);
}

template <typename T, std::size_t N>
auto Box<T, N>::withCenter(Coord const& center, Coord const& size) -> Box<T, N> {
  return Box(center - size / 2, center + size / 2);
}

template <typename T, std::size_t N>
Box<T, N>::Box() = default;

template <typename T, std::size_t N>
Box<T, N>::Box(Coord const& min, Coord const& max)
    : m_min(min), m_max(max) {}

template <typename T, std::size_t N>
Box<T, N>::Box(Box const& b)
    : m_min(b.min()), m_max(b.max()) {}

template <typename T, std::size_t N>
auto Box<T, N>::operator=(Box<T, N> const& b) -> Box<T, N>& = default;

template <typename T, std::size_t N>
template <typename T2>
Box<T, N>::Box(Box<T2, N> const& b)
    : m_min(b.min()), m_max(b.max()) {}

template <typename T, std::size_t N>
auto Box<T, N>::isNull() const -> bool {
  return m_min == Coord::filled(std::numeric_limits<T>::max())
    && m_max == Coord::filled(std::numeric_limits<T>::lowest());
}

template <typename T, std::size_t N>
auto Box<T, N>::isNegative() const -> bool {
  for (std::size_t i = 0; i < N; ++i) {
    if (m_max[i] < m_min[i])
      return true;
  }
  return false;
}

template <typename T, std::size_t N>
auto Box<T, N>::isEmpty() const -> bool {
  for (std::size_t i = 0; i < N; ++i) {
    if (m_max[i] <= m_min[i])
      return true;
  }
  return false;
}

template <typename T, std::size_t N>
void Box<T, N>::combine(Box const& box) {
  m_min = box.m_min.piecewiseMin(m_min);
  m_max = box.m_max.piecewiseMax(m_max);
}

template <typename T, std::size_t N>
auto Box<T, N>::combined(Box const& box) const -> Box<T, N> {
  auto b = *this;
  b.combine(box);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::combine(Coord const& point) {
  m_min = m_min.piecewiseMin(point);
  m_max = m_max.piecewiseMax(point);
}

template <typename T, std::size_t N>
auto Box<T, N>::combined(Coord const& point) const -> Box<T, N> {
  auto b = *this;
  b.combine(point);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::limit(Box const& box) {
  m_min = m_min.piecewiseMax(box.m_min);
  m_max = m_max.piecewiseMin(box.m_max);
}

template <typename T, std::size_t N>
auto Box<T, N>::limited(Box const& box) -> Box<T, N> {
  auto b = *this;
  b.limit(box);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::makePositive() {
  for (std::size_t i = 0; i < N; ++i) {
    if (m_max[i] < m_min[i])
      std::swap(m_max[i], m_min[i]);
  }
}

template <typename T, std::size_t N>
void Box<T, N>::rangeSetIfEmpty(Box const& b) {
  for (std::size_t i = 0; i < N; ++i) {
    if (m_max[i] <= m_min[i])
      setRange(i, b.range(i));
  }
}

template <typename T, std::size_t N>
void Box<T, N>::makeCube() {
  setAspect(Coord::filled(1));
}

template <typename T, std::size_t N>
auto Box<T, N>::size() const -> Coord {
  return m_max - m_min;
}

template <typename T, std::size_t N>
auto Box<T, N>::size(std::size_t dim) const -> T {
  return m_max[dim] - m_min[dim];
}

template <typename T, std::size_t N>
void Box<T, N>::setAspect(Coord as, bool shrink) {
  Coord nBox = (m_max - m_min).piecewiseDivide(as);
  Coord extent;
  if (shrink)
    extent = Coord::filled(nBox.min());
  else
    extent = Coord::filled(nBox.max());
  extent = extent.piecewiseMult(as);
  Coord center = (m_max + m_min) / 2;
  m_max = center + extent / 2;
  m_min = center - extent / 2;
}

template <typename T, std::size_t N>
auto Box<T, N>::center() const -> Coord {
  return (m_min + m_max) / 2;
}

template <typename T, std::size_t N>
void Box<T, N>::setCenter(Coord const& c) {
  translate(c - center());
}

template <typename T, std::size_t N>
void Box<T, N>::translate(Coord const& c) {
  m_min += c;
  m_max += c;
}

template <typename T, std::size_t N>
auto Box<T, N>::translated(Coord const& c) const -> Box<T, N> {
  auto b = *this;
  b.translate(c);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::translateToInclude(Coord const& coord, Coord const& padding) {
  Coord translation;
  for (std::size_t i = 0; i < N; ++i) {
    if (coord[i] < m_min[i] + padding[i])
      translation[i] = coord[i] - m_min[i] - padding[i];
    else if (coord[i] > m_max[i] - padding[i])
      translation[i] = coord[i] - m_max[i] + padding[i];
  }
  translate(translation);
}

template <typename T, std::size_t N>
auto Box<T, N>::range(std::size_t dim) const -> Vector<T, 2> {
  return Coord(m_min[dim], m_max[dim]);
}

template <typename T, std::size_t N>
void Box<T, N>::setRange(std::size_t dim, Vector<T, 2> v) {
  m_min[dim] = v[0];
  m_max[dim] = v[1];
}

template <typename T, std::size_t N>
void Box<T, N>::combineRange(std::size_t dim, Vector<T, 2> v) {
  m_min[dim] = std::min(m_min[dim], v[0]);
  m_max[dim] = std::max(m_max[dim], v[1]);
}

template <typename T, std::size_t N>
void Box<T, N>::limitRange(std::size_t dim, Vector<T, 2> v) {
  m_min[dim] = std::max(m_min[dim], v[0]);
  m_max[dim] = std::min(m_max[dim], v[1]);
}

template <typename T, std::size_t N>
void Box<T, N>::expand(T factor) {
  for (std::size_t i = 0; i < N; ++i) {
    auto rng = range(i);
    T center = rng.sum() / 2;
    T newDist = (rng[1] - rng[0]) * factor;
    setRange(i, Coord(center - newDist / 2, center + newDist / 2));
  }
}

template <typename T, std::size_t N>
auto Box<T, N>::expanded(T factor) const -> Box<T, N> {
  auto b = *this;
  b.expand(factor);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::expand(Coord const& factor) {
  for (std::size_t i = 0; i < N; ++i) {
    auto rng = range(i);
    T center = rng.sum() / 2;
    T newDist = (rng[1] - rng[0]) * factor[i];
    setRange(i, Coord(center - newDist / 2, center + newDist / 2));
  }
}

template <typename T, std::size_t N>
auto Box<T, N>::expanded(Coord const& factor) const -> Box<T, N> {
  auto b = *this;
  b.expand(factor);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::scale(T factor) {
  for (std::size_t i = 0; i < N; ++i)
    setRange(i, range(i) * factor);
}

template <typename T, std::size_t N>
auto Box<T, N>::scaled(T factor) const -> Box<T, N> {
  auto b = *this;
  b.scale(factor);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::scale(Coord const& factor) {
  for (std::size_t i = 0; i < N; ++i)
    setRange(i, range(i) * factor[i]);
}

template <typename T, std::size_t N>
auto Box<T, N>::scaled(Coord const& factor) const -> Box<T, N> {
  auto b = *this;
  b.scale(factor);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::pad(T amount) {
  for (std::size_t i = 0; i < N; ++i) {
    m_min[i] -= amount;
    m_max[i] += amount;
  }
}

template <typename T, std::size_t N>
auto Box<T, N>::padded(T amount) const -> Box<T, N> {
  auto b = *this;
  b.pad(amount);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::pad(Coord const& amount) {
  for (std::size_t i = 0; i < N; ++i) {
    m_min[i] -= amount[i];
    m_max[i] += amount[i];
  }
}

template <typename T, std::size_t N>
auto Box<T, N>::padded(Coord const& amount) const -> Box<T, N> {
  auto b = *this;
  b.pad(amount);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::trim(T amount) {
  pad(-amount);
}

template <typename T, std::size_t N>
auto Box<T, N>::trimmed(T amount) const -> Box<T, N> {
  auto b = *this;
  b.trim(amount);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::trim(Coord const& amount) {
  pad(-amount);
}

template <typename T, std::size_t N>
auto Box<T, N>::trimmed(Coord const& amount) const -> Box<T, N> {
  auto b = *this;
  b.trim(amount);
  return b;
}

template <typename T, std::size_t N>
void Box<T, N>::flip(std::size_t dimension) {
  std::swap(m_min[dimension], m_max[dimension]);
}

template <typename T, std::size_t N>
auto Box<T, N>::flipped(std::size_t dimension) const -> Box<T, N> {
  auto b = *this;
  b.flip(dimension);
  return b;
}

template <typename T, std::size_t N>
auto Box<T, N>::normal(Coord const& coord) const -> Coord {
  return (coord - m_min).piecewiseDivide(m_max - m_min);
}

template <typename T, std::size_t N>
auto Box<T, N>::eval(Coord const& normalizedCoord) const -> Coord {
  return normalizedCoord.piecewiseMultiply(m_max - m_min) + m_min;
}

template <typename T, std::size_t N>
auto Box<T, N>::min() const -> Coord const& {
  return m_min;
}

template <typename T, std::size_t N>
auto Box<T, N>::max() const -> Coord const& {
  return m_max;
}

template <typename T, std::size_t N>
auto Box<T, N>::min() -> Coord& {
  return m_min;
}

template <typename T, std::size_t N>
auto Box<T, N>::max() -> Coord& {
  return m_max;
}

template <typename T, std::size_t N>
void Box<T, N>::setMin(Coord const& c) {
  m_min = c;
}

template <typename T, std::size_t N>
void Box<T, N>::setMax(Coord const& c) {
  m_max = c;
}

template <typename T, std::size_t N>
auto Box<T, N>::volume() const -> T {
  return size().product();
}

template <typename T, std::size_t N>
auto Box<T, N>::overlap(Box const& b) const -> Box {
  Box result = *this;
  for (std::size_t i = 0; i < N; ++i) {
    result.m_min[i] = std::max(result.m_min[i], b.m_min[i]);
    result.m_max[i] = std::min(result.m_max[i], b.m_max[i]);
  }
  return result;
}

template <typename T, std::size_t N>
auto Box<T, N>::intersection(Box const& b) const -> IntersectResult {
  IntersectResult res;

  T overlap = std::numeric_limits<T>::max();
  std::size_t dim = 0;
  bool negative = false;
  for (std::size_t i = 0; i < N; ++i) {
    if (m_max[i] - b.m_min[i] < overlap) {
      overlap = m_max[i] - b.m_min[i];
      dim = i;
      negative = true;
    }
    if (b.m_max[i] - m_min[i] < overlap) {
      overlap = b.m_max[i] - m_min[i];
      dim = i;
      negative = false;
    }
  }

  res.overlap = Coord();
  if (overlap > 0) {
    res.intersects = true;
    res.overlap[dim] = overlap;
  } else {
    res.intersects = false;
    res.overlap[dim] = -overlap;
  }

  if (negative)
    res.overlap = -res.overlap;

  if (res.overlap == Coord()) {
    res.glances = true;
  } else {
    res.glances = false;
  }

  return res;
}

template <typename T, std::size_t N>
auto Box<T, N>::intersects(Box const& b, bool includeEdges) const -> bool {
  for (std::size_t i = 0; i < N; ++i) {
    if (includeEdges) {
      if (m_max[i] < b.m_min[i] || b.m_max[i] < m_min[i])
        return false;
    } else {
      if (m_max[i] <= b.m_min[i] || b.m_max[i] <= m_min[i])
        return false;
    }
  }
  return true;
}

template <typename T, std::size_t N>
auto Box<T, N>::contains(Coord const& p, bool includeEdges) const -> bool {
  for (std::size_t i = 0; i < N; ++i) {
    if (includeEdges) {
      if (p[i] < m_min[i] || p[i] > m_max[i])
        return false;
    } else {
      if (p[i] <= m_min[i] || p[i] >= m_max[i])
        return false;
    }
  }
  return true;
}

template <typename T, std::size_t N>
auto Box<T, N>::contains(Box const& b, bool includeEdges) const -> bool {
  return contains(b.min(), includeEdges) && contains(b.max(), includeEdges);
}

template <typename T, std::size_t N>
auto Box<T, N>::belongs(Coord const& p) const -> bool {
  for (std::size_t i = 0; i < N; ++i) {
    if (p[i] < m_min[i] || p[i] >= m_max[i])
      return false;
  }

  return true;
}

template <typename T, std::size_t N>
auto Box<T, N>::containsEpsilon(Coord const& p, unsigned epsilons) const -> bool {
  for (std::size_t i = 0; i < N; ++i) {
    if (p[i] < m_min[i] || p[i] > m_max[i])
      return false;
    if (nearEqual(p[i], m_min[i], epsilons) || nearEqual(p[i], m_max[i], epsilons))
      return false;
  }
  return true;
}

template <typename T, std::size_t N>
auto Box<T, N>::containsEpsilon(Box const& b, unsigned epsilons) const -> bool {
  return containsEpsilon(b.min(), epsilons) && containsEpsilon(b.max(), epsilons);
}

template <typename T, std::size_t N>
auto Box<T, N>::operator==(Box const& ref) const -> bool {
  return m_min == ref.m_min && m_max == ref.m_max;
}

template <typename T, std::size_t N>
auto Box<T, N>::operator!=(Box const& ref) const -> bool {
  return m_min != ref.m_min || m_max != ref.m_max;
}

template <typename T, std::size_t N>
template <typename... TN>
void Box<T, N>::combineThings(Box& b, Coord const& point, TN const&... rest) {
  b.combine(point);
  combineThings(b, rest...);
}

template <typename T, std::size_t N>
template <typename... TN>
void Box<T, N>::combineThings(Box& b, Box const& box, TN const&... rest) {
  b.combine(box);
  combineThings(b, rest...);
}

template <typename T, std::size_t N>
template <typename... TN>
void Box<T, N>::combineThings(Box&) {}

template <typename T, std::size_t N>
auto operator<<(std::ostream& os, Box<T, N> const& box) -> std::ostream& {
  os << "Box{min:" << box.min() << " max:" << box.max() << "}";
  return os;
}

template <typename T, std::size_t N>
template <std::size_t P, class>
Box<T, N>::Box(T minx, T miny, T maxx, T maxy)
    : Box(Coord(minx, miny), Coord(maxx, maxy)) {}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::xMin() const -> Enable2D<P, T> {
  return min()[0];
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::xMax() const -> Enable2D<P, T> {
  return max()[0];
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::yMin() const -> Enable2D<P, T> {
  return min()[1];
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::yMax() const -> Enable2D<P, T> {
  return max()[1];
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::setXMin(T xMin) -> Enable2D<P> {
  m_min[0] = xMin;
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::setXMax(T xMax) -> Enable2D<P> {
  m_max[0] = xMax;
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::setYMin(T yMin) -> Enable2D<P> {
  m_min[1] = yMin;
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::setYMax(T yMax) -> Enable2D<P> {
  m_max[1] = yMax;
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::width() const -> Enable2D<P, T> {
  return size(0);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::height() const -> Enable2D<P, T> {
  return size(1);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::translate(T x, T y) -> Enable2D<P, void> {
  translate(Coord(x, y));
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::translateToInclude(T x, T y, T xPadding, T yPadding) -> Enable2D<P, void> {
  translateToInclude(Coord(x, y), Coord(xPadding, yPadding));
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::scale(T x, T y) -> Enable2D<P, void> {
  scale(Coord(x, y));
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::expand(T x, T y) -> Enable2D<P, void> {
  expand(Coord(x, y));
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::flipHorizontal() -> Enable2D<P, void> {
  flip(0);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::flipVertical() -> Enable2D<P, void> {
  flip(1);
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::edges() const -> Enable2D<P, Array<Line, 4>> {
  Array<Line, 4> res;
  res[0] = {min(), {min()[0], max()[1]}};
  res[1] = {min(), {max()[0], min()[1]}};
  res[2] = {{min()[0], max()[1]}, max()};
  res[3] = {{max()[0], min()[1]}, max()};
  return res;
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::intersects(Line const& l) const -> Enable2D<P, bool> {
  if (contains(l.min()) || contains(l.max()))
    return true;

  for (auto i : edges()) {
    if (l.intersects(i))
      return true;
  }
  return false;
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::intersectsCircle(Coord const& position, T radius) const -> Enable2D<P, bool> {
  if (contains(position))
    return true;
  for (auto const& e : edges()) {
    if (e.distanceTo(position) <= radius)
      return true;
  }
  return false;
}

// returns the closest intersection point (from l.min())
template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::edgeIntersection(Line const& l) const -> Enable2D<P, LineIntersectResult> {
  Array<LineIntersectResult, 4> candidates;
  std::size_t numCandidates = 0;

  for (auto i : edges()) {
    auto res = l.intersection(i);
    if (res.intersects)
      candidates[numCandidates++] = res;
  }

  // How glancing is determined
  // There are a few possibilities
  // if candidates is empty then no intersection, easy
  // if there is only one candidate then there are two possibilities, glancing
  // or not
  //   But! if an endpoint is inside the rect, not just on the edge then it's
  //   false
  // if there are two candidates and at least one of them is not glancing then
  // false
  // if there are two candidates and at they're both glancing then there's a few
  // possibilities
  //   first, the line cuts through the corner, we can detect this by seeing if
  //   the point is in the
  //   box but not on the edge
  //   second, the line cuts across the corner, this case is true
  //   third, the line coincides with one of the sides, this case is also true.
  // if there are 3 candidates then two cases
  //   first, the line coincides with one of the sides, and glances off of the
  //   other two, true
  //   second, the line cuts through a corner and reaches the far side, false
  //   we can tell these apart by determining if any intersections coincide
  // if there are 4 candidates then the only possible case is false (cutting
  // through both corners
  if (numCandidates != 0) {
    std::sort(candidates.ptr(),
              candidates.ptr() + numCandidates,
              [&](LineIntersectResult const& a, LineIntersectResult const& b) -> auto { return a.t < b.t; });
    if (numCandidates == 1) {
      if (contains(l.min(), false) || contains(l.max(), false)) {
        candidates[0].glances = false;
      }
    } else if (numCandidates == 2) {
      if (contains(l.min(), false) || contains(l.max(), false)) {
        candidates[0].glances = false;
      } else if (contains(l.min()) && !candidates[1].glances) {
        candidates[0].glances = false;
      }
      if (candidates[1].coincides) {// If we coincide on either consider it true
        candidates[0].coincides = true;
      }
    } else if (numCandidates == 3) {
      if (candidates[0].coincides || candidates[1].coincides || candidates[2].coincides) {
        candidates[0].glances = true;
        candidates[0].coincides = true;
      } else {
        candidates[0].glances = false;
      }
    } else {
      candidates[0].glances = false;
      candidates[0].coincides = false;
    }

    return candidates[0];
  } else {
    return LineIntersectResult();
  }
}

template <typename T, std::size_t N>
template <std::size_t P>
auto Box<T, N>::subtract(Box const& rect) const -> Enable2D<P, List<Box>> {
  List<Box> regions;

  auto overlap = Box::overlap(rect);
  if (overlap.isEmpty()) {
    // If this rect doesn't overlap at all with the subtracted one, obviously
    // the entire rect is new territory.
    regions.append(*this);
  } else {
    // If there is overlap with this rect, we need to add the left, bottom,
    // right, and top sections.  These can overlap at the corners, so the left
    // and right sections will take the lower / upper left and lower / upper
    // right corners, and the top and bottom will be limited to the width of
    // the overlap section.

    if (xMin() < overlap.xMin())
      regions.append(Box(xMin(), yMin(), overlap.xMin(), yMax()));

    if (overlap.xMax() < xMax())
      regions.append(Box(overlap.xMax(), yMin(), xMax(), yMax()));

    if (yMin() < overlap.yMin())
      regions.append(Box(rect.xMin(), yMin(), rect.xMax(), overlap.yMin()));

    if (overlap.yMax() < yMax())
      regions.append(Box(rect.xMin(), overlap.yMax(), rect.xMax(), yMax()));
  }

  return regions;
}

template <typename T, std::size_t N>
auto Box<T, N>::nearestCoordTo(Coord const& c) const -> Coord {
  Coord result = c;
  for (std::size_t i = 0; i < N; ++i)
    result[i] = clamp(result[i], m_min[i], m_max[i]);
  return result;
}

}// namespace Star

template <typename T, std::size_t N>
struct std::formatter<Star::Box<T, N>> : Star::ostream_formatter {};
