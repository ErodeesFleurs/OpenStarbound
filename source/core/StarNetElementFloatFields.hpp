#pragma once

#include "StarException.hpp"
#include "StarInterpolation.hpp"
#include "StarNetElement.hpp"

import std;

namespace Star {

using StepStreamException = ExceptionDerived<"StepStreamException">;

template <typename T>
class NetElementFloating : public NetElement {
public:
  auto get() const -> T;
  void set(T value);

  // If a fixed point base is given, then instead of transmitting the value as
  // a float, it is transmitted as a VLQ of the value divided by the fixed
  // point base.  Any NetElementFloating that is transmitted to must also have
  // the same fixed point base set.
  void setFixedPointBase(std::optional<T> fixedPointBase = {});

  // If interpolation is enabled on the NetStepStates parent, and an
  // interpolator is set, then on steps in between data points this will be
  // used to interpolate this value.  It is not necessary that senders and
  // receivers both have matching interpolation functions, or any interpolation
  // functions at all.
  void setInterpolator(std::function<T(T, T, T)> interpolator);

  void initNetVersion(NetElementVersion const* version = nullptr) override;

  // Values are never interpolated, but they will be delayed for the given
  // interpolationTime.
  void enableNetInterpolation(float extrapolationHint = 0.0f) override;
  void disableNetInterpolation() override;
  void tickNetInterpolation(float dt) override;

  void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
  void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

  auto writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules = {}) const -> bool override;
  void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  void blankNetDelta(float interpolationTime = 0.0f) override;

private:
  void writeValue(DataStream& ds, T t) const;
  auto readValue(DataStream& ds) const -> T;

  auto interpolate() const -> T;

  std::optional<T> m_fixedPointBase;
  NetElementVersion const* m_netVersion = nullptr;
  std::uint64_t m_latestUpdateVersion = 0;
  T m_value = T();

  std::function<T(T, T, T)> m_interpolator;
  float m_extrapolation = 0.0f;
  std::optional<Deque<std::pair<float, T>>> m_interpolationDataPoints;
};

using NetElementFloat = NetElementFloating<float>;
using NetElementDouble = NetElementFloating<double>;

template <typename T>
auto NetElementFloating<T>::get() const -> T {
  return m_value;
}

template <typename T>
void NetElementFloating<T>::set(T value) {
  if (m_value != value) {
    // Only mark the step as updated here if it actually would change the
    // transmitted value.
    if (!m_fixedPointBase || round(m_value / *m_fixedPointBase) != round(value / *m_fixedPointBase))
      m_latestUpdateVersion = m_netVersion ? m_netVersion->current() : 0;

    m_value = value;

    if (m_interpolationDataPoints) {
      m_interpolationDataPoints->clear();
      m_interpolationDataPoints->append({0.0f, m_value});
    }
  }
}

template <typename T>
void NetElementFloating<T>::setFixedPointBase(std::optional<T> fixedPointBase) {
  m_fixedPointBase = fixedPointBase;
}

template <typename T>
void NetElementFloating<T>::setInterpolator(std::function<T(T, T, T)> interpolator) {
  m_interpolator = std::move(interpolator);
}

template <typename T>
void NetElementFloating<T>::initNetVersion(NetElementVersion const* version) {
  m_netVersion = version;
  m_latestUpdateVersion = 0;
}

template <typename T>
void NetElementFloating<T>::enableNetInterpolation(float extrapolationHint) {
  m_extrapolation = extrapolationHint;
  if (!m_interpolationDataPoints) {
    m_interpolationDataPoints.emplace();
    m_interpolationDataPoints->append({0.0f, m_value});
  }
}

template <typename T>
void NetElementFloating<T>::disableNetInterpolation() {
  if (m_interpolationDataPoints) {
    m_value = m_interpolationDataPoints->last().second;
    m_interpolationDataPoints.reset();
  }
}

template <typename T>
void NetElementFloating<T>::tickNetInterpolation(float dt) {
  if (m_interpolationDataPoints) {
    for (auto& p : *m_interpolationDataPoints)
      p.first -= dt;

    while (m_interpolationDataPoints->size() > 2 && (*m_interpolationDataPoints)[1].first <= 0.0f)
      m_interpolationDataPoints->removeFirst();

    m_value = interpolate();
  }
}

template <typename T>
void NetElementFloating<T>::netStore(DataStream& ds, NetCompatibilityRules rules) const {
  if (!checkWithRules(rules))
    return;
  if (m_interpolationDataPoints)
    writeValue(ds, m_interpolationDataPoints->last().second);
  else
    writeValue(ds, m_value);
}

template <typename T>
void NetElementFloating<T>::netLoad(DataStream& ds, NetCompatibilityRules rules) {
  if (!checkWithRules(rules))
    return;
  m_value = readValue(ds);
  m_latestUpdateVersion = m_netVersion ? m_netVersion->current() : 0;
  if (m_interpolationDataPoints) {
    m_interpolationDataPoints->clear();
    m_interpolationDataPoints->append({0.0f, m_value});
  }
}

template <typename T>
auto NetElementFloating<T>::writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules) const -> bool {
  if (!checkWithRules(rules))
    return false;
  if (m_latestUpdateVersion < fromVersion)
    return false;

  if (m_interpolationDataPoints)
    writeValue(ds, m_interpolationDataPoints->last().second);
  else
    writeValue(ds, m_value);

  return true;
}

template <typename T>
void NetElementFloating<T>::readNetDelta(DataStream& ds, float interpolationTime, [[maybe_unused]] NetCompatibilityRules rules) {
  T t = readValue(ds);

  m_latestUpdateVersion = m_netVersion ? m_netVersion->current() : 0;
  if (m_interpolationDataPoints) {
    if (interpolationTime < m_interpolationDataPoints->last().first)
      m_interpolationDataPoints->clear();
    m_interpolationDataPoints->append({interpolationTime, t});
    m_value = interpolate();
  } else {
    m_value = t;
  }
}

template <typename T>
void NetElementFloating<T>::blankNetDelta(float interpolationTime) {
  if (m_interpolationDataPoints) {
    auto lastPoint = m_interpolationDataPoints->last();
    float lastTime = lastPoint.first;
    lastPoint.first = interpolationTime;
    if (interpolationTime < lastTime)
      *m_interpolationDataPoints = {lastPoint};
    else
      m_interpolationDataPoints->append(lastPoint);

    m_value = interpolate();
  }
}

template <typename T>
void NetElementFloating<T>::writeValue(DataStream& ds, T t) const {
  if (m_fixedPointBase)
    ds.writeVlqI(round(t / *m_fixedPointBase));
  else
    ds.write(t);
}

template <typename T>
auto NetElementFloating<T>::readValue(DataStream& ds) const -> T {
  T t;
  if (m_fixedPointBase)
    t = ds.readVlqI() * *m_fixedPointBase;
  else
    ds.read(t);
  return t;
}

template <typename T>
auto NetElementFloating<T>::interpolate() const -> T {
  auto& dataPoints = *m_interpolationDataPoints;

  float ipos = inverseLinearInterpolateUpper(dataPoints.begin(), dataPoints.end(), 0.0f, [](float lhs, auto const& rhs) -> auto { return lhs < rhs.first; }, [](auto const& dataPoint) -> auto { return dataPoint.first; });
  auto bound = getBound2(ipos, dataPoints.size(), BoundMode::Extrapolate);

  if (m_interpolator) {
    auto const& minPoint = dataPoints[bound.i0];
    auto const& maxPoint = dataPoints[bound.i1];

    // If step separation is less than 1.0, don't normalize extrapolation to
    // the very small step difference, because this can result in large jumps
    // during jitter.
    float stepDist = max(maxPoint.first - minPoint.first, 1.0f);
    float offset = clamp<float>(bound.offset, 0.0f, 1.0f + m_extrapolation / stepDist);
    return m_interpolator(offset, minPoint.second, maxPoint.second);

  } else {
    if (bound.offset < 1.0f)
      return dataPoints[bound.i0].second;
    else
      return dataPoints[bound.i1].second;
  }
}

}// namespace Star
