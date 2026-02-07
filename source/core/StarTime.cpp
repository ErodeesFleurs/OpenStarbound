#include "StarTime.hpp"

import std;

namespace Star {

auto Time::timeSinceEpoch() -> double {
  return ticksToSeconds(epochTicks(), epochTickFrequency());
}

auto Time::millisecondsSinceEpoch() -> std::int64_t {
  return ticksToMilliseconds(epochTicks(), epochTickFrequency());
}

auto Time::monotonicTime() -> double {
  return ticksToSeconds(monotonicTicks(), monotonicTickFrequency());
}

auto Time::monotonicMilliseconds() -> std::int64_t {
  return ticksToMilliseconds(monotonicTicks(), monotonicTickFrequency());
}

auto Time::monotonicMicroseconds() -> std::int64_t {
  return ticksToMicroseconds(monotonicTicks(), monotonicTickFrequency());
}

auto Time::printDuration(double time) -> String {
  String hours;
  String minutes;
  String seconds;
  String milliseconds;

  if (time >= 3600) {
    int numHours = (int)time / 3600;
    hours = strf("{} hour{}", numHours, numHours == 1 ? "" : "s");
  }
  if (time >= 60) {
    int numMinutes = (int)(time / 60) % 60;
    minutes = strf("{} minute{}", numMinutes, numMinutes == 1 ? "" : "s");
  }
  if (time >= 1) {
    int numSeconds = (int)time % 60;
    seconds = strf("{} second{}", numSeconds, numSeconds == 1 ? "" : "s");
  }

  int numMilliseconds = std::round(std::fmod(time, 1.0) * 1000);
  milliseconds = strf("{} millisecond{}", numMilliseconds, numMilliseconds == 1 ? "" : "s");

  return String::joinWith(", ", hours, minutes, seconds, milliseconds);
}

auto Time::printCurrentDateAndTime(String format) -> String {
  return printDateAndTime(epochTicks(), format);
}

auto Time::ticksToSeconds(std::int64_t ticks, std::int64_t tickFrequency) -> double {
  return ticks / (double)tickFrequency;
}

auto Time::ticksToMilliseconds(std::int64_t ticks, std::int64_t tickFrequency) -> std::int64_t {
  std::int64_t ticksPerMs = (tickFrequency + 500) / 1000;
  return (ticks + ticksPerMs / 2) / ticksPerMs;
}

auto Time::ticksToMicroseconds(std::int64_t ticks, std::int64_t tickFrequency) -> std::int64_t {
  std::int64_t ticksPerUs = (tickFrequency + 500000) / 1000000;
  return (ticks + ticksPerUs / 2) / ticksPerUs;
}

auto Time::secondsToTicks(double seconds, std::int64_t tickFrequency) -> std::int64_t {
  return std::round(seconds * tickFrequency);
}

auto Time::millisecondsToTicks(std::int64_t milliseconds, std::int64_t tickFrequency) -> std::int64_t {
  return milliseconds * ((tickFrequency + 500) / 1000);
}

auto Time::microsecondsToTicks(std::int64_t microseconds, std::int64_t tickFrequency) -> std::int64_t {
  return microseconds * ((tickFrequency + 500000) / 1000000);
}

Clock::Clock(bool start) {
  m_elapsedTicks = 0;
  m_running = false;
  if (start)
    Clock::start();
}

Clock::Clock(Clock const& clock) {
  operator=(clock);
}

auto Clock::operator=(Clock const& clock) -> Clock& {
  m_elapsedTicks = clock.m_elapsedTicks;
  m_lastTicks = clock.m_lastTicks;
  m_running = clock.m_running;

  return *this;
}

void Clock::reset() {
  RecursiveMutexLocker locker(m_mutex);
  updateElapsed();
  m_elapsedTicks = 0;
}

void Clock::stop() {
  RecursiveMutexLocker locker(m_mutex);
  m_lastTicks.reset();
  m_running = false;
}

void Clock::start() {
  RecursiveMutexLocker locker(m_mutex);
  m_running = true;
  updateElapsed();
}

auto Clock::running() const -> bool {
  RecursiveMutexLocker locker(m_mutex);
  return m_running;
}

auto Clock::time() const -> double {
  RecursiveMutexLocker locker(m_mutex);
  updateElapsed();
  return Time::ticksToSeconds(m_elapsedTicks, Time::monotonicTickFrequency());
}

auto Clock::milliseconds() const -> std::int64_t {
  RecursiveMutexLocker locker(m_mutex);
  updateElapsed();
  return Time::ticksToMilliseconds(m_elapsedTicks, Time::monotonicTickFrequency());
}

void Clock::setTime(double time) {
  RecursiveMutexLocker locker(m_mutex);
  updateElapsed();
  m_elapsedTicks = Time::secondsToTicks(time, Time::monotonicTickFrequency());
}

void Clock::setMilliseconds(std::int64_t millis) {
  RecursiveMutexLocker locker(m_mutex);
  updateElapsed();
  m_elapsedTicks = Time::millisecondsToTicks(millis, Time::monotonicTickFrequency());
}

void Clock::adjustTime(double timeAdjustment) {
  RecursiveMutexLocker locker(m_mutex);
  setTime(std::max<double>(0.0, time() + timeAdjustment));
}

void Clock::adjustMilliseconds(std::int64_t millisAdjustment) {
  RecursiveMutexLocker locker(m_mutex);
  setMilliseconds(milliseconds() + millisAdjustment);
}

void Clock::updateElapsed() const {
  if (!m_running)
    return;

  std::int64_t currentTicks = Time::monotonicTicks();

  if (m_lastTicks)
    m_elapsedTicks += (currentTicks - *m_lastTicks);

  m_lastTicks = currentTicks;
}

auto Timer::withTime(double timeLeft, bool start) -> Timer {
  Timer timer;
  timer.setTime(-timeLeft);
  if (start)
    timer.start();
  return timer;
}

auto Timer::withMilliseconds(std::int64_t millis, bool start) -> Timer {
  Timer timer;
  timer.setMilliseconds(-millis);
  if (start)
    timer.start();
  return timer;
}

Timer::Timer() : Clock(false) {
  setTime(0.0);
}

Timer::Timer(Timer const& timer) = default;

auto Timer::operator=(Timer const& timer) -> Timer& = default;

void Timer::restart(double timeLeft) {
  Clock::setTime(-timeLeft);
  Clock::start();
}

void Timer::restartWithMilliseconds(std::int64_t millisecondsLeft) {
  Clock::setMilliseconds(-millisecondsLeft);
  Clock::start();
}

auto Timer::timeLeft(bool negative) const -> double {
  double timeLeft = -Clock::time();
  if (!negative)
    timeLeft = std::max(0.0, timeLeft);
  return timeLeft;
}

auto Timer::millisecondsLeft(bool negative) const -> std::int64_t {
  std::int64_t millisLeft = -Clock::milliseconds();
  if (!negative)
    millisLeft = std::max<std::int64_t>(0, millisLeft);
  return millisLeft;
}

auto Timer::timeUp() const -> bool {
  return Clock::time() >= 0.0;
}

}
