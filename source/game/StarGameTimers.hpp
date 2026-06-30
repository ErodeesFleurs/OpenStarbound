#pragma once

#include "StarGameTypes.hpp"
#include "StarJson.hpp"

namespace Star {

struct GameTimer {
  GameTimer() = default;
  explicit GameTimer(float time);

  float time{};
  float timer{};

  bool tick(float dt = GlobalTimestep); // returns true if time is up
  bool wrapTick(float dt = GlobalTimestep); // auto resets
  void reset();
  void setDone();
  void invert();

  [[nodiscard]] bool ready() const;
  [[nodiscard]] float percent() const;
};

DataStream& operator>>(DataStream& ds, GameTimer& gt);
DataStream& operator<<(DataStream& ds, GameTimer const& gt);

struct SlidingWindow {
  SlidingWindow() = default;
  SlidingWindow(float windowSize, size_t resolution, float initialValue);

  GameTimer sampleTimer;
  float windowSize = 1.0f;
  size_t resolution = 1;

  float currentMin = 0.0f;
  float currentMax = 0.0f;
  float currentAverage = 0.0f;

  size_t currentIndex = 0;
  std::vector<float> window;

  void reset(float initialValue);
  void update(function<float()> sampleFunction);
  void update(float newValue);
  void processUpdate(float newValue);

  [[nodiscard]] float min();
  [[nodiscard]] float max();
  [[nodiscard]] float average();
};

// Keeps long term track of elapsed time based on epochTime.
class EpochTimer {
public:
  EpochTimer() = default;
  explicit EpochTimer(Json json);

  [[nodiscard]] Json toJson() const;

  void update(double newEpochTime);

  [[nodiscard]] double elapsedTime() const;
  void setElapsedTime(double elapsedTime);

  friend DataStream& operator>>(DataStream& ds, EpochTimer& et);
  friend DataStream& operator<<(DataStream& ds, EpochTimer const& et);

private:
  Maybe<double> m_lastSeenEpochTime;
  double m_elapsedTime = 0.0;
};

}
