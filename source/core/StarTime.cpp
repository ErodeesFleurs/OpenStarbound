#include "StarTime.hpp"
#include "StarFormat.hpp"

#ifdef STAR_SYSTEM_WINDOWS
#include <ctime>
#else
#include <time.h>
#endif

import std;

namespace Star {

namespace Time {
// Use nanoseconds as the internal representation for ticks for high resolution and cross-platform consistency.
static constexpr std::int64_t TicksPerSecond = 1'000'000'000;

auto timeSinceEpoch() -> double {
  return ticksToSeconds(epochTicks(), epochTickFrequency());
}

auto millisecondsSinceEpoch() -> std::int64_t {
  return ticksToMilliseconds(epochTicks(), epochTickFrequency());
}

auto monotonicTime() -> double {
  return ticksToSeconds(monotonicTicks(), monotonicTickFrequency());
}

auto monotonicMilliseconds() -> std::int64_t {
  return ticksToMilliseconds(monotonicTicks(), monotonicTickFrequency());
}

auto monotonicMicroseconds() -> std::int64_t {
  return ticksToMicroseconds(monotonicTicks(), monotonicTickFrequency());
}

auto printDuration(double time) -> String {
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

auto printDateAndTime(std::int64_t epochTicks, String format) -> String {
  auto seconds = epochTicks / epochTickFrequency();
  auto millis = (epochTicks % epochTickFrequency()) / (epochTickFrequency() / 1000);

  auto t = (std::time_t)seconds;
  std::tm ptm;
#ifdef STAR_SYSTEM_WINDOWS
  if (localtime_s(&ptm, &t) != 0)
    return "Error formatting date";
#else
  if (localtime_r(&t, &ptm) == nullptr)
    return "Error formatting date";
#endif

  return format.replaceTags(StringMap<String>{
    {"year", strf("{:04d}", ptm.tm_year + 1900)},
    {"month", strf("{:02d}", ptm.tm_mon + 1)},
    {"day", strf("{:02d}", ptm.tm_mday)},
    {"hours", strf("{:02d}", ptm.tm_hour)},
    {"minutes", strf("{:02d}", ptm.tm_min)},
    {"seconds", strf("{:02d}", ptm.tm_sec)},
    {"millis", strf("{:03d}", millis)}});
}

auto printCurrentDateAndTime(String format) -> String {
  return printDateAndTime(epochTicks(), format);
}

auto epochTicks() -> std::int64_t {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

auto epochTickFrequency() -> std::int64_t {
  return TicksPerSecond;
}

auto monotonicTicks() -> std::int64_t {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

auto monotonicTickFrequency() -> std::int64_t {
  return TicksPerSecond;
}

auto ticksToSeconds(std::int64_t ticks, std::int64_t tickFrequency) -> double {
  return ticks / (double)tickFrequency;
}

auto ticksToMilliseconds(std::int64_t ticks, std::int64_t tickFrequency) -> std::int64_t {
  std::int64_t ticksPerMs = tickFrequency / 1000;
  if (ticksPerMs == 0)
    return 0;
  return (ticks + ticksPerMs / 2) / ticksPerMs;
}

auto ticksToMicroseconds(std::int64_t ticks, std::int64_t tickFrequency) -> std::int64_t {
  std::int64_t ticksPerUs = tickFrequency / 1000000;
  if (ticksPerUs == 0)
    return 0;
  return (ticks + ticksPerUs / 2) / ticksPerUs;
}

auto secondsToTicks(double seconds, std::int64_t tickFrequency) -> std::int64_t {
  return std::round(seconds * tickFrequency);
}

auto millisecondsToTicks(std::int64_t milliseconds, std::int64_t tickFrequency) -> std::int64_t {
  return milliseconds * (tickFrequency / 1000);
}

auto microsecondsToTicks(std::int64_t microseconds, std::int64_t tickFrequency) -> std::int64_t {
  return microseconds * (tickFrequency / 1000000);
}
}// namespace Time

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
  RecursiveMutexLocker locker(m_mutex);
  RecursiveMutexLocker otherLocker(clock.m_mutex);
  m_elapsedTicks = clock.m_elapsedTicks;
  m_lastTicks = clock.m_lastTicks;
  m_running = clock.m_running;

  return *this;
}

void Clock::reset() {
  RecursiveMutexLocker locker(m_mutex);
  m_elapsedTicks = 0;
  if (m_running)
    m_lastTicks = Time::monotonicTicks();
  else
    m_lastTicks.reset();
}

void Clock::stop() {
  RecursiveMutexLocker locker(m_mutex);
  updateElapsed();
  m_lastTicks.reset();
  m_running = false;
}

void Clock::start() {
  RecursiveMutexLocker locker(m_mutex);
  if (!m_running) {
    m_running = true;
    m_lastTicks = Time::monotonicTicks();
  }
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
  m_elapsedTicks = Time::secondsToTicks(time, Time::monotonicTickFrequency());
  if (m_running)
    m_lastTicks = Time::monotonicTicks();
}

void Clock::setMilliseconds(std::int64_t millis) {
  RecursiveMutexLocker locker(m_mutex);
  m_elapsedTicks = Time::millisecondsToTicks(millis, Time::monotonicTickFrequency());
  if (m_running)
    m_lastTicks = Time::monotonicTicks();
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

}// namespace Star
