#pragma once

#include "StarGameTypes.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

struct GameTimer {
  GameTimer();
  explicit GameTimer(float time);

  float time;
  float timer;

  auto tick(float dt = GlobalTimestep) -> bool;    // returns true if time is up
  auto wrapTick(float dt = GlobalTimestep) -> bool;// auto resets
  void reset();
  void setDone();
  void invert();

  [[nodiscard]] auto ready() const -> bool;
  [[nodiscard]] auto percent() const -> float;
};

auto operator>>(DataStream& ds, GameTimer& gt) -> DataStream&;
auto operator<<(DataStream& ds, GameTimer const& gt) -> DataStream&;

struct SlidingWindow {
  SlidingWindow();
  SlidingWindow(float windowSize, std::size_t resolution, float initialValue);

  GameTimer sampleTimer;
  float windowSize;
  std::size_t resolution;

  float currentMin;
  float currentMax;
  float currentAverage;

  std::size_t currentIndex;
  std::vector<float> window;

  void reset(float initialValue);
  void update(std::function<float()> sampleFunction);
  void update(float newValue);
  void processUpdate(float newValue);

  auto min() -> float;
  auto max() -> float;
  auto average() -> float;
};

// Keeps long term track of elapsed time based on epochTime.
class EpochTimer {
public:
  EpochTimer();
  explicit EpochTimer(Json json);

  [[nodiscard]] auto toJson() const -> Json;

  void update(double newEpochTime);

  [[nodiscard]] auto elapsedTime() const -> double;
  void setElapsedTime(double elapsedTime);

  friend auto operator>>(DataStream& ds, EpochTimer& et) -> DataStream&;
  friend auto operator<<(DataStream& ds, EpochTimer const& et) -> DataStream&;

private:
  std::optional<double> m_lastSeenEpochTime;
  double m_elapsedTime;
};

}// namespace Star
