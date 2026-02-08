#pragma once

#include "StarInterpolation.hpp"
#include "StarLogging.hpp"
#include "StarLruCache.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

// Implementation of DeCasteljau Algorithm for Bezier Curves
template <typename DataT, std::size_t Dimension, std::size_t Order, class PointT = Vector<DataT, Dimension>>
class Spline : public Array<PointT, Order + 1> {
public:
  using PointData = Array<PointT, Order + 1>;

  template <typename... T>
  Spline(PointT const& e1, T const&... rest)
      : PointData(e1, rest...) {
    m_pointCache.setMaxSize(1000);
    m_lengthCache.setMaxSize(1000);
  }

  Spline() : PointData(PointData::filled(PointT())) {
    m_pointCache.setMaxSize(1000);
    m_lengthCache.setMaxSize(1000);
  }

  auto pointAt(float t) const -> PointT {
    float u = std::clamp<float>(t, 0, 1);
    if (u != t) {
      t = u;
      Logger::warn("Passed out of range time to Spline::pointAt");
    }

    if (auto p = m_pointCache.ptr(t))
      return *p;

    PointData intermediates(*this);
    PointData temp;
    for (std::size_t order = Order + 1; order > 1; order--) {
      for (std::size_t i = 1; i < order; i++) {
        temp[i - 1] = lerp(t, intermediates[i - 1], intermediates[i]);
      }
      intermediates = std::move(temp);
    }

    m_pointCache.set(t, intermediates[0]);
    return intermediates[0];
  }

  auto tangentAt(float t) const -> PointT {
    float u = std::clamp<float>(t, 0, 1);
    if (u != t) {
      t = u;
      Logger::warn("Passed out of range time to Spline::tangentAt");
    }

    // constructs a hodograph and returns pointAt
    Spline<DataT, Dimension, Order - 1> hodograph;
    for (std::size_t i = 0; i < Order; i++) {
      hodograph[i] = ((*this)[i + 1] - (*this)[i]) * Order;
    }
    return hodograph.pointAt(t);
  }

  auto length(float begin = 0, float end = 1, std::size_t subdivisions = 100) const -> DataT {
    if (!(begin <= 1 && begin >= 0 && end <= 1 && end >= 0 && begin <= end)) {
      Logger::warn("Passed invalid range to Spline::length");
      return 0;
    }

    if (!begin) {
      if (auto p = m_lengthCache.ptr(end))
        return *p;
    }

    DataT res = 0;
    PointT previousPoint = pointAt(begin);
    for (std::size_t i = 1; i <= subdivisions; i++) {
      PointT currentPoint = pointAt(i / subdivisions * (end - begin));
      res += (currentPoint - previousPoint).magnitude();
      previousPoint = currentPoint;
    }

    if (!begin)
      m_lengthCache.set(end, res);

    return res;
  }

  auto arcLenPara(float u, DataT epsilon = .01) const -> float {
    if (u == 0)
      return 0;
    if (u == 1)
      return 1;
    u = clamp<float>(u, 0, 1);
    if (u == 0 || u == 1) {
      Logger::warn("Passed out of range time to Spline::arcLenPara");
      return u;
    }
    DataT targetLength = length() * u;
    float t = .5;
    float lower = 0;
    float upper = 1;
    DataT approxLen = length(0, t);
    while (targetLength - approxLen > epsilon || targetLength - approxLen < -epsilon) {
      if (targetLength > approxLen) {
        lower = t;
      } else {
        upper = t;
      }
      t = (upper - lower) * .5 + lower;
      approxLen = length(0, t);
    }
    return t;
  }

  auto origin() -> PointT& {
    m_pointCache.clear();
    m_lengthCache.clear();
    return (*this)[0];
  }

  auto origin() const -> PointT const& {
    return (*this)[0];
  }

  auto dest() -> PointT& {
    m_pointCache.clear();
    m_lengthCache.clear();
    return (*this)[Order];
  }

  auto dest() const -> PointT const& {
    return (*this)[Order];
  }

  auto operator[](std::size_t index) -> PointT& {
    m_pointCache.clear();
    m_lengthCache.clear();
    return PointData::operator[](index);
  }

  auto operator[](std::size_t index) const -> PointT const& {
    return PointData::operator[](index);
  }

protected:
  mutable LruCache<float, PointT> m_pointCache;
  mutable LruCache<float, DataT> m_lengthCache;
};

using CSplineF = Spline<float, 2, 3, Vec2F>;

}// namespace Star
