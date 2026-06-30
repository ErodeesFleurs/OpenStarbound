#pragma once

#include "StarAssets.hpp"
#include "StarJson.hpp"

namespace Star {

class InterpolationTracker {
public:
  InterpolationTracker(Json config = Json());
  InterpolationTracker(AssetsConstPtr assets, Json config);

  // Should interpolation be enabled on entities at all?  If this is false,
  // extrapolationHint and interpolationLead will always return 0.
  [[nodiscard]] bool interpolationEnabled() const;
  [[nodiscard]] unsigned extrapolationHint() const;

  // Time in-between entity updates
  [[nodiscard]] float entityUpdateDelta() const;

  void receiveTimeUpdate(double remoteTime);
  void update(double newLocalTime);

  // Lead time that incoming interpolated data as of this moment should be
  // marked for.  If interpolation is disabled, this is always 0.0
  [[nodiscard]] float interpolationLeadTime() const;

private:
  bool m_interpolationEnabled;
  float m_entityUpdateDelta;
  double m_timeLead;
  unsigned m_extrapolationHint;
  double m_timeTrackFactor;
  double m_timeMaxDistance;

  double m_currentTime = 0.0;
  Maybe<double> m_lastTimeUpdate;
  Maybe<double> m_predictedTime;
};

}
