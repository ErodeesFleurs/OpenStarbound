#include "StarGameTimers.hpp"

#include "StarDataStreamExtra.hpp"
#include "StarJsonExtra.hpp"

import std;

namespace Star {

GameTimer::GameTimer() : time(), timer() {}

GameTimer::GameTimer(float time) : time(time) {
  reset();
}

auto GameTimer::tick(float dt) -> bool {
  timer = approach(0.0f, timer, dt);
  return timer == 0.0f;
}

auto GameTimer::ready() const -> bool {
  return timer == 0.0f;
}

auto GameTimer::wrapTick(float dt) -> bool {
  auto res = tick(dt);
  if (res)
    reset();
  return res;
}

void GameTimer::reset() {
  timer = time;
}

void GameTimer::setDone() {
  timer = 0.0f;
}

void GameTimer::invert() {
  timer = time - timer;
}

auto GameTimer::percent() const -> float {
  if (time)
    return timer / time;
  else
    return 0.0f;
}

auto operator>>(DataStream& ds, GameTimer& gt) -> DataStream& {
  ds >> gt.time;
  ds >> gt.timer;
  return ds;
}

auto operator<<(DataStream& ds, GameTimer const& gt) -> DataStream& {
  ds << gt.time;
  ds << gt.timer;
  return ds;
}

SlidingWindow::SlidingWindow() : windowSize(1.0f), resolution(1) {}

SlidingWindow::SlidingWindow(float windowSize, std::size_t resolution, float initialValue)
    : windowSize(windowSize), resolution(resolution) {
  sampleTimer = GameTimer(windowSize / resolution);
  window = std::vector<float>(resolution);
  reset(initialValue);
}

void SlidingWindow::reset(float initialValue) {
  sampleTimer.reset();
  currentIndex = 0;
  currentMin = initialValue;
  currentMax = initialValue;
  currentAverage = initialValue;
  std::ranges::fill(window, initialValue);
}

void SlidingWindow::update(std::function<float()> sampleFunction) {
  if (sampleTimer.wrapTick()) {
    processUpdate(sampleFunction());
  }
}

void SlidingWindow::update(float newValue) {
  if (sampleTimer.wrapTick()) {
    processUpdate(newValue);
  }
}

void SlidingWindow::processUpdate(float newValue) {
  ++currentIndex;
  currentIndex = currentIndex % resolution;
  window[currentIndex] = newValue;

  currentMin = std::numeric_limits<float>::max();
  currentMax = 0;
  float total = 0;
  for (float v : window) {
    total += v;
    currentMin = std::min(currentMin, v);
    currentMax = std::max(currentMax, v);
  }
  currentAverage = total / resolution;
}

auto SlidingWindow::min() -> float {
  return currentMin;
}

auto SlidingWindow::max() -> float {
  return currentMax;
}

auto SlidingWindow::average() -> float {
  return currentAverage;
}

EpochTimer::EpochTimer() : m_elapsedTime(0.0) {}

EpochTimer::EpochTimer(Json json) {
  m_lastSeenEpochTime = json.get("lastEpochTime").optDouble();
  m_elapsedTime = json.getDouble("elapsedTime");
}

auto EpochTimer::toJson() const -> Json {
  return JsonObject{{"lastEpochTime", jsonFromMaybe(m_lastSeenEpochTime)}, {"elapsedTime", m_elapsedTime}};
}

void EpochTimer::update(double newEpochTime) {
  if (!m_lastSeenEpochTime) {
    m_lastSeenEpochTime = newEpochTime;
  } else {
    // Don't allow elapsed time to go backwards in the case of the epoch time
    // being lost or wrong.
    double difference = newEpochTime - *m_lastSeenEpochTime;
    if (difference > 0)
      m_elapsedTime += difference;
    m_lastSeenEpochTime = newEpochTime;
  }
}

auto EpochTimer::elapsedTime() const -> double {
  return m_elapsedTime;
}

void EpochTimer::setElapsedTime(double elapsedTime) {
  m_elapsedTime = elapsedTime;
}

auto operator>>(DataStream& ds, EpochTimer& et) -> DataStream& {
  ds >> et.m_lastSeenEpochTime;
  ds >> et.m_elapsedTime;
  return ds;
}

auto operator<<(DataStream& ds, EpochTimer const& et) -> DataStream& {
  ds << et.m_lastSeenEpochTime;
  ds << et.m_elapsedTime;
  return ds;
}

}// namespace Star
