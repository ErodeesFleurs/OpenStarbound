#pragma once

#include "StarRect.hpp"

import std;

namespace Star {

template <typename DataType>
class Polygon {
public:
  using Vertex = Vector<DataType, 2>;
  using Line = Star::Line<DataType, 2>;
  using Rect = Star::Box<DataType, 2>;

  struct IntersectResult {
    // Whether or not the two objects intersect
    bool intersects;
    // How much *this* poly must be moved in order to make them not intersect
    // anymore
    Vertex overlap;
  };

  struct LineIntersectResult {
    // Point of intersection
    Vertex point;
    // t value at the point of intersection of the line that was checked
    DataType along;
    // Side that the line first intersected, if the line starts inside the
    // polygon, this will not be set.
    std::optional<std::size_t> intersectedSide;
  };

  using VertexList = List<Vertex>;
  using iterator = typename VertexList::iterator;
  using const_iterator = typename VertexList::const_iterator;

  static auto convexHull(VertexList points) -> Polygon;
  static auto clip(Polygon inputPoly, Polygon convexClipPoly) -> Polygon;

  // Creates a null polygon
  Polygon();
  Polygon(Polygon const& rhs);
  Polygon(Polygon&& rhs);

  template <typename DataType2>
  explicit Polygon(Box<DataType2, 2> const& rect);

  template <typename DataType2>
  explicit Polygon(Polygon<DataType2> const& p2);

  // This seems weird, but it isn't.  SAT intersection works perfectly well
  // with one Poly having only a single vertex.
  explicit Polygon(Vertex const& coord);

  // When specifying a polygon using this constructor the list should be in
  // counterclockwise order.
  explicit Polygon(VertexList const& vertexes);

  Polygon(std::initializer_list<Vertex> vertexes);

  [[nodiscard]] auto isNull() const -> bool;

  [[nodiscard]] auto isConvex() const -> bool;
  [[nodiscard]] auto convexArea() const -> float;

  void deduplicateVertexes(float maxDistance);

  void add(Vertex const& a);
  void remove(std::size_t i);

  void clear();

  auto vertexes() const -> VertexList const&;
  auto vertexes() -> VertexList&;

  [[nodiscard]] auto sides() const -> std::size_t;

  auto side(std::size_t i) const -> Line;

  auto distance(Vertex const& c) const -> DataType;

  void translate(Vertex const& c);

  void setCenter(Vertex const& c);

  void rotate(DataType a, Vertex const& c = Vertex());

  void scale(Vertex const& s, Vertex const& c = Vertex());
  void scale(DataType s, Vertex const& c = Vertex());

  void flipHorizontal(DataType horizontalPos = DataType());
  void flipVertical(DataType verticalPos = DataType());

  template <typename DataType2>
  void transform(Matrix3<DataType2> const& transMat);

  auto operator[](std::size_t i) const -> Vertex const&;
  auto operator[](std::size_t i) -> Vertex&;

  auto operator==(Polygon const& rhs) const -> bool;

  auto operator=(Polygon const& rhs) -> Polygon&;
  auto operator=(Polygon&& rhs) -> Polygon&;

  auto begin() -> iterator;
  auto begin() const -> const_iterator;

  auto end() -> iterator;
  auto end() const -> const_iterator;

  // vertex and normal wrap around so that i can never be out of range.
  auto vertex(std::size_t i) const -> Vertex const&;
  auto normal(std::size_t i) const -> Vertex;

  auto center() const -> Vertex;

  // a point in the volume, within min and max y, moved downwards to be a half
  // width from the bottom (if that point is within a half width from the
  // top, center() is returned)
  auto bottomCenter() const -> Vertex;

  auto boundBox() const -> Rect;

  // Determine winding number of the given point.
  auto windingNumber(Vertex const& p) const -> int;

  auto contains(Vertex const& p) const -> bool;

  // Normal SAT intersection finding the shortest separation of two convex
  // polys.
  auto satIntersection(Polygon const& p) const -> IntersectResult;

  // A directional version of a SAT intersection that will only separate
  // parallel to the given direction.  If choseSign is true, then the
  // separation can occur either with the given direction or opposite it, but
  // still parallel.  If it is false, separation will always occur in the given
  // direction only.
  auto directionalSatIntersection(Polygon const& p, Vertex const& direction, bool chooseSign) const -> IntersectResult;

  // Returns the closest intersection with the poly, if any.
  auto lineIntersection(Line const& l) const -> std::optional<LineIntersectResult>;

  auto intersects(Polygon const& p) const -> bool;
  auto intersects(Line const& l) const -> bool;

private:
  // i must be between 0 and m_vertexes.size() - 1
  auto sideAt(std::size_t i) const -> Line;

  VertexList m_vertexes;
};

template <typename DataType>
auto operator<<(std::ostream& os, Polygon<DataType> const& poly) -> std::ostream&;

using PolyI = Polygon<int>;
using PolyF = Polygon<float>;
using PolyD = Polygon<double>;

template <typename DataType>
auto Polygon<DataType>::convexHull(VertexList points) -> Polygon<DataType> {
  if (points.empty())
    return {};

  auto cross = [](Vertex o, Vertex a, Vertex b) -> auto {
    return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0]);
  };
  sort(points);

  VertexList lower;
  for (auto const& point : points) {
    while (lower.size() >= 2 && cross(lower[lower.size() - 2], lower[lower.size() - 1], point) <= 0)
      lower.removeLast();
    lower.append(point);
  }

  VertexList upper;
  for (auto const& point : reverseIterate(points)) {
    while (upper.size() >= 2 && cross(upper[upper.size() - 2], upper[upper.size() - 1], point) <= 0)
      upper.removeLast();
    upper.append(point);
  }

  upper.removeLast();
  lower.removeLast();
  lower.appendAll(take(upper));
  return Polygon<DataType>(std::move(lower));
}

template <typename DataType>
auto Polygon<DataType>::clip(Polygon inputPoly, Polygon convexClipPoly) -> Polygon<DataType> {
  if (inputPoly.sides() == 0)
    return inputPoly;

  auto insideEdge = [](Line const& edge, Vertex const& p) -> auto {
    return ((edge.max() - edge.min()) ^ (p - edge.min())) > 0;
  };

  VertexList outputVertexes = take(inputPoly.m_vertexes);
  for (std::size_t i = 0; i < convexClipPoly.sides(); ++i) {
    if (outputVertexes.empty())
      break;

    Line clipEdge = convexClipPoly.sideAt(i);
    VertexList inputVertexes = take(outputVertexes);
    Vertex s = inputVertexes.last();
    for (Vertex e : inputVertexes) {
      if (insideEdge(clipEdge, e)) {
        if (!insideEdge(clipEdge, s))
          outputVertexes.append(clipEdge.intersection(Line(s, e)).point);
        outputVertexes.append(e);
      } else if (insideEdge(clipEdge, s)) {
        outputVertexes.append(clipEdge.intersection(Line(s, e)).point);
      }
      s = e;
    }
  }

  return Polygon(std::move(outputVertexes));
}

template <typename DataType>
Polygon<DataType>::Polygon() = default;

template <typename DataType>
Polygon<DataType>::Polygon(Polygon const& rhs)
    : m_vertexes(rhs.m_vertexes) {}

template <typename DataType>
Polygon<DataType>::Polygon(Polygon&& rhs)
    : m_vertexes(std::move(rhs.m_vertexes)) {}

template <typename DataType>
template <typename DataType2>
Polygon<DataType>::Polygon(Box<DataType2, 2> const& rect) {
  m_vertexes = {
    Vertex(rect.min()), Vertex(rect.max()[0], rect.min()[1]), Vertex(rect.max()), Vertex(rect.min()[0], rect.max()[1])};
}

template <typename DataType>
template <typename DataType2>
Polygon<DataType>::Polygon(Polygon<DataType2> const& p2) {
  for (auto const& v : p2)
    m_vertexes.push_back(Vertex(v));
}

template <typename DataType>
Polygon<DataType>::Polygon(Vertex const& coord) {
  m_vertexes.push_back(coord);
}

template <typename DataType>
Polygon<DataType>::Polygon(VertexList const& vertexes)
    : m_vertexes(vertexes) {}

template <typename DataType>
Polygon<DataType>::Polygon(std::initializer_list<Vertex> vertexes)
    : m_vertexes(vertexes) {}

template <typename DataType>
auto Polygon<DataType>::isNull() const -> bool {
  return m_vertexes.empty();
}

template <typename DataType>
auto Polygon<DataType>::isConvex() const -> bool {
  if (sides() < 2)
    return true;

  for (unsigned i = 0; i < sides(); ++i) {
    if ((side(i + 1).diff() ^ side(i).diff()) > 0)
      return false;
  }

  return true;
}

template <typename DataType>
auto Polygon<DataType>::convexArea() const -> float {
  float area = 0.0f;
  for (std::size_t i = 0; i < m_vertexes.size(); ++i) {
    Vertex const& v1 = m_vertexes[i];
    Vertex const& v2 = i == m_vertexes.size() - 1 ? m_vertexes[0] : m_vertexes[i + 1];
    area += 0.5f * (v1[0] * v2[1] - v1[1] * v2[0]);
  }
  return area;
}

template <typename DataType>
void Polygon<DataType>::deduplicateVertexes(float maxDistance) {
  if (m_vertexes.empty())
    return;

  float distSquared = square(maxDistance);
  VertexList newVertexes = {m_vertexes[0]};
  for (std::size_t i = 1; i < m_vertexes.size(); ++i) {
    if (vmagSquared(m_vertexes[i] - newVertexes.last()) > distSquared)
      newVertexes.append(m_vertexes[i]);
  }

  if (vmagSquared(newVertexes.first() - newVertexes.last()) <= distSquared)
    newVertexes.removeLast();

  m_vertexes = std::move(newVertexes);
}

template <typename DataType>
void Polygon<DataType>::add(Vertex const& a) {
  m_vertexes.push_back(a);
}

template <typename DataType>
void Polygon<DataType>::remove(std::size_t i) {
  auto it = begin() + i % sides();
  m_vertexes.erase(it);
}

template <typename DataType>
void Polygon<DataType>::clear() {
  m_vertexes.clear();
}

template <typename DataType>
auto Polygon<DataType>::vertexes() const -> typename Polygon<DataType>::VertexList const& {
  return m_vertexes;
}

template <typename DataType>
auto Polygon<DataType>::vertexes() -> typename Polygon<DataType>::VertexList& {
  return m_vertexes;
}

template <typename DataType>
auto Polygon<DataType>::sides() const -> std::size_t {
  return m_vertexes.size();
}

template <typename DataType>
auto Polygon<DataType>::side(std::size_t i) const -> typename Polygon<DataType>::Line {
  return sideAt(i % m_vertexes.size());
}

template <typename DataType>
auto Polygon<DataType>::distance(Vertex const& c) const -> DataType {
  if (contains(c))
    return 0;

  DataType dist = highest<DataType>();
  for (std::size_t i = 0; i < m_vertexes.size(); ++i)
    dist = std::min(dist, sideAt(i).distanceTo(c));

  return dist;
}

template <typename DataType>
void Polygon<DataType>::translate(Vertex const& c) {
  for (auto& v : m_vertexes)
    v += c;
}

template <typename DataType>
void Polygon<DataType>::setCenter(Vertex const& c) {
  translate(c - center());
}

template <typename DataType>
void Polygon<DataType>::rotate(DataType a, Vertex const& c) {
  for (auto& v : m_vertexes)
    v = (v - c).rotate(a) + c;
}

template <typename DataType>
void Polygon<DataType>::scale(Vertex const& s, Vertex const& c) {
  for (auto& v : m_vertexes)
    v = vmult((v - c), s) + c;
}

template <typename DataType>
void Polygon<DataType>::scale(DataType s, Vertex const& c) {
  scale(Vertex::filled(s), c);
}

template <typename DataType>
void Polygon<DataType>::flipHorizontal(DataType horizontalPos) {
  scale(Vertex(-1, 1), Vertex(horizontalPos, 0));
  // Reverse vertexes to make sure poly remains counter-clockwise after flip.
  std::reverse(m_vertexes.begin(), m_vertexes.end());
}

template <typename DataType>
void Polygon<DataType>::flipVertical(DataType verticalPos) {
  scale(Vertex(1, -1), Vertex(0, verticalPos));
  // Reverse vertexes to make sure poly remains counter-clockwise after flip.
  std::reverse(m_vertexes.begin(), m_vertexes.end());
}

template <typename DataType>
template <typename DataType2>
void Polygon<DataType>::transform(Matrix3<DataType2> const& transMat) {
  for (auto& v : m_vertexes)
    v = transMat.transformVec2(v);
}

template <typename DataType>
auto Polygon<DataType>::operator[](std::size_t i) const -> typename Polygon<DataType>::Vertex const& {
  return m_vertexes[i];
}

template <typename DataType>
auto Polygon<DataType>::operator[](std::size_t i) -> typename Polygon<DataType>::Vertex& {
  return m_vertexes[i];
}

template <typename DataType>
auto Polygon<DataType>::operator==(Polygon<DataType> const& rhs) const -> bool {
  return m_vertexes == rhs.m_vertexes;
}

template <typename DataType>
auto Polygon<DataType>::operator=(Polygon const& rhs) -> Polygon<DataType>& = default;

template <typename DataType>
auto Polygon<DataType>::operator=(Polygon&& rhs) -> Polygon<DataType>& {
  m_vertexes = std::move(rhs.m_vertexes);
  return *this;
}

template <typename DataType>
auto Polygon<DataType>::begin() -> typename Polygon<DataType>::iterator {
  return m_vertexes.begin();
}

template <typename DataType>
auto Polygon<DataType>::begin() const -> typename Polygon<DataType>::const_iterator {
  return m_vertexes.begin();
}

template <typename DataType>
auto Polygon<DataType>::end() -> typename Polygon<DataType>::iterator {
  return m_vertexes.end();
}

template <typename DataType>
auto Polygon<DataType>::end() const -> typename Polygon<DataType>::const_iterator {
  return m_vertexes.end();
}

template <typename DataType>
auto Polygon<DataType>::vertex(std::size_t i) const -> typename Polygon<DataType>::Vertex const& {
  return m_vertexes[i % m_vertexes.size()];
}

template <typename DataType>
auto Polygon<DataType>::normal(std::size_t i) const -> typename Polygon<DataType>::Vertex {
  Vertex diff = side(i).diff();

  if (diff == Vertex())
    return Vertex();

  return diff.rot90().normalized();
}

template <typename DataType>
auto Polygon<DataType>::center() const -> typename Polygon<DataType>::Vertex {
  return std::accumulate(m_vertexes.begin(), m_vertexes.end(), Vertex()) / (DataType)m_vertexes.size();
}

template <typename DataType>
auto Polygon<DataType>::bottomCenter() const -> typename Polygon<DataType>::Vertex {
  if (m_vertexes.size() == 0)
    return Vertex();
  Polygon<DataType>::Vertex center = std::accumulate(m_vertexes.begin(), m_vertexes.end(), Vertex()) / (DataType)m_vertexes.size();
  Polygon<DataType>::Vertex bottomLeft = *std::min_element(m_vertexes.begin(), m_vertexes.end());
  Polygon<DataType>::Vertex topRight = *std::max_element(m_vertexes.begin(), m_vertexes.end());
  Polygon<DataType>::Vertex size = topRight - bottomLeft;
  if (size.x() > size.y())
    return center;
  return Polygon<DataType>::Vertex(center.x(), bottomLeft.y() + size.x() / 2.0f);
}

template <typename DataType>
auto Polygon<DataType>::boundBox() const -> Rect {
  auto bounds = Rect::null();
  for (auto const& v : m_vertexes)
    bounds.combine(v);
  return bounds;
}

template <typename DataType>
auto Polygon<DataType>::windingNumber(Vertex const& p) const -> int {

  auto isLeft = [](Vertex const& p0, Vertex const& p1, Vertex const& p2) -> auto {
    return ((p1[0] - p0[0]) * (p2[1] - p0[1]) - (p2[0] - p0[0]) * (p1[1] - p0[1]));
  };

  // the winding number counter
  int wn = 0;

  // loop through all edges of the polygon
  for (std::size_t i = 0; i < m_vertexes.size(); ++i) {
    auto const& first = m_vertexes[i];
    auto const& second = i == m_vertexes.size() - 1 ? m_vertexes[0] : m_vertexes[i + 1];

    // start y <= p[1]
    if (first[1] <= p[1]) {
      if (second[1] > p[1]) {
        // an upward crossing
        if (isLeft(first, second, p) > 0) {
          // p left of edge
          // have a valid up intersect
          ++wn;
        }
      }
    } else {
      // start y > p[1] (no test needed)
      if (second[1] <= p[1]) {
        // a downward crossing
        if (isLeft(first, second, p) < 0) {
          // p right of edge
          // have a valid down intersect
          --wn;
        }
      }
    }
  }

  return wn;
}

template <typename DataType>
auto Polygon<DataType>::contains(Vertex const& p) const -> bool {
  return windingNumber(p) != 0;
}

template <typename DataType>
auto Polygon<DataType>::satIntersection(Polygon const& p) const -> typename Polygon<DataType>::IntersectResult {
  // "Accumulates" the shortest separating distance and axis of this poly and
  // the given poly, after projecting all the vertexes of each poly onto a
  // given axis.  Used by SAT intersection, meant to be called with each tested
  // axis.
  auto accumSeparator = [this](Polygon const& p, Vertex const& axis, DataType& shortestOverlap, Vertex& finalSepDir) -> auto {
    DataType myProjectionLow = std::numeric_limits<DataType>::max();
    DataType targetProjectionHigh = std::numeric_limits<DataType>::lowest();

    for (auto const& v : m_vertexes) {
      DataType p = axis[0] * v[0] + axis[1] * v[1];
      if (p < myProjectionLow)
        myProjectionLow = p;
    }

    for (auto const& v : p.m_vertexes) {
      DataType p = axis[0] * v[0] + axis[1] * v[1];
      if (p > targetProjectionHigh)
        targetProjectionHigh = p;
    }

    float overlap = targetProjectionHigh - myProjectionLow;
    if (overlap < shortestOverlap) {
      shortestOverlap = overlap;
      finalSepDir = axis;
    }
  };

  DataType overlap = std::numeric_limits<DataType>::max();
  Vertex separatingDir = Vertex();

  if (!m_vertexes.empty()) {
    Vertex pv = m_vertexes[m_vertexes.size() - 1];
    for (auto const& v : m_vertexes) {
      Vertex sideNormal = pv - v;
      if (sideNormal != Vertex()) {
        sideNormal = sideNormal.rot90().normalized();
        accumSeparator(p, -sideNormal, overlap, separatingDir);
      }
      pv = v;
    }
  }

  if (!p.m_vertexes.empty()) {
    Vertex pv = p.m_vertexes[p.m_vertexes.size() - 1];
    for (auto const& v : p.m_vertexes) {
      Vertex sideNormal = pv - v;
      if (sideNormal != Vertex()) {
        sideNormal = sideNormal.rot90().normalized();
        accumSeparator(p, sideNormal, overlap, separatingDir);
      }
      pv = v;
    }
  }

  IntersectResult isect;
  isect.intersects = (overlap > 0);
  isect.overlap = separatingDir * overlap;

  return isect;
}

template <typename DataType>
auto Polygon<DataType>::directionalSatIntersection(
  Polygon const& p, Vertex const& direction, bool chooseSign) const -> typename Polygon<DataType>::IntersectResult {
  // A "directional" version of accumSeparator, that when intersecting only
  // ever tries to separate in the given direction.
  auto directionalAccumSeparator = [this](Polygon const& p, Vertex axis, DataType& shortestOverlap,
                                          Vertex const& separatingDir, Vertex& finalSepDir, bool chooseDir) -> auto {
    DataType myProjectionLow = std::numeric_limits<DataType>::max();
    DataType targetProjectionHigh = std::numeric_limits<DataType>::lowest();

    for (auto const& v : m_vertexes) {
      DataType p = axis[0] * v[0] + axis[1] * v[1];
      if (p < myProjectionLow)
        myProjectionLow = p;
    }

    for (auto const& v : p.m_vertexes) {
      DataType p = axis[0] * v[0] + axis[1] * v[1];
      if (p > targetProjectionHigh)
        targetProjectionHigh = p;
    }

    float overlap = targetProjectionHigh - myProjectionLow;

    // Separation was found, skip the rest of the method.
    if (overlap <= 0) {
      if (overlap < shortestOverlap) {
        shortestOverlap = overlap;
        finalSepDir = axis;
      }
      return;
    }

    DataType axisDot = separatingDir * axis;

    // Now, if we don't have separation and the axis is perpendicular to
    // requested, we can do nothing, return.
    if (axisDot == 0)
      return;

    // Separate along the given separating direction enough to separate as
    // determined by this axis.
    DataType projOverlap = overlap / axisDot;
    if (chooseDir) {
      DataType absProjOverlap = (projOverlap >= 0) ? projOverlap : -projOverlap;
      if (absProjOverlap < shortestOverlap) {
        shortestOverlap = absProjOverlap;
        finalSepDir = separatingDir * (projOverlap / absProjOverlap);
      }
    } else if (projOverlap >= 0) {
      if (projOverlap < shortestOverlap) {
        shortestOverlap = projOverlap;
        finalSepDir = separatingDir;
      }
    }
  };

  DataType overlap = std::numeric_limits<DataType>::max();
  Vertex separatingDir = Vertex();

  if (!m_vertexes.empty()) {
    Vertex pv = m_vertexes[m_vertexes.size() - 1];
    for (auto const& v : m_vertexes) {
      Vertex sideNormal = pv - v;
      if (sideNormal != Vertex()) {
        sideNormal = sideNormal.rot90().normalized();
        directionalAccumSeparator(p, -sideNormal, overlap, direction, separatingDir, chooseSign);
      }
      pv = v;
    }
  }

  if (!p.m_vertexes.empty()) {
    Vertex pv = p.m_vertexes[p.m_vertexes.size() - 1];
    for (auto const& v : p.m_vertexes) {
      Vertex sideNormal = pv - v;
      if (sideNormal != Vertex()) {
        sideNormal = sideNormal.rot90().normalized();
        directionalAccumSeparator(p, sideNormal, overlap, direction, separatingDir, chooseSign);
      }
      pv = v;
    }
  }

  IntersectResult isect;
  isect.intersects = (overlap > 0);
  isect.overlap = separatingDir * overlap;

  return isect;
}

template <typename DataType>
auto Polygon<DataType>::lineIntersection(Line const& l) const -> std::optional<LineIntersectResult> {
  if (contains(l.min()))
    return LineIntersectResult{l.min(), DataType(0), {}};

  std::optional<LineIntersectResult> nearestIntersection;
  for (std::size_t i = 0; i < m_vertexes.size(); ++i) {
    auto intersection = l.intersection(sideAt(i));
    if (intersection.intersects) {
      if (!nearestIntersection || intersection.t < nearestIntersection->along)
        nearestIntersection = LineIntersectResult{intersection.point, intersection.t, i};
    }
  }
  return nearestIntersection;
}

template <typename DataType>
auto Polygon<DataType>::intersects(Polygon const& p) const -> bool {
  return satIntersection(p).intersects;
}

template <typename DataType>
auto Polygon<DataType>::intersects(Line const& l) const -> bool {
  if (contains(l.min()) || contains(l.max()))
    return true;

  for (std::size_t i = 0; i < m_vertexes.size(); ++i) {
    if (l.intersects(sideAt(i)))
      return true;
  }

  return false;
}

template <typename DataType>
auto Polygon<DataType>::sideAt(std::size_t i) const -> Line {
  if (i == m_vertexes.size() - 1)
    return Line(m_vertexes[i], m_vertexes[0]);
  else
    return Line(m_vertexes[i], m_vertexes[i + 1]);
}

template <typename DataType>
auto operator<<(std::ostream& os, Polygon<DataType> const& poly) -> std::ostream& {
  os << "[Poly: ";
  for (auto i = poly.begin(); i != poly.end(); ++i) {
    if (i != poly.begin())
      os << ", ";
    os << *i;
  }
  os << "]";
  return os;
}

}// namespace Star
