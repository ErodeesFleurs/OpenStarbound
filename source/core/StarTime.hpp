#pragma once

#include "StarThread.hpp"

import std;

namespace Star {


namespace Time {
  auto timeSinceEpoch() -> double;
  auto millisecondsSinceEpoch() -> std::int64_t;

  auto monotonicTime() -> double;
  auto monotonicMilliseconds() -> std::int64_t;
  auto monotonicMicroseconds() -> std::int64_t;

  // Pretty print a duration of time (In days, hours, minutes, seconds, and milliseconds)
  auto printDuration(double time) -> String;

  // Pretty print a given date and time
  auto printDateAndTime(std::int64_t epochTicks, String format = "<year>-<month>-<day> <hours>:<minutes>:<seconds>.<millis>") -> String;
  auto printCurrentDateAndTime(String format = "<year>-<month>-<day> <hours>:<minutes>:<seconds>.<millis>") -> String;

  // Ticks since unix epoch
  auto epochTicks() -> std::int64_t;
  // Epoch ticks per second, static throughout application lifetime.
  auto epochTickFrequency() -> std::int64_t;

  // Ticks since unspecified time before program start
  auto monotonicTicks() -> std::int64_t;
  // Monotonic ticks per second, static throughout application lifetime.
  auto monotonicTickFrequency() -> std::int64_t;

  auto ticksToSeconds(std::int64_t ticks, std::int64_t tickFrequency) -> double;
  auto ticksToMilliseconds(std::int64_t ticks, std::int64_t tickFrequency) -> std::int64_t;
  auto ticksToMicroseconds(std::int64_t ticks, std::int64_t tickFrequency) -> std::int64_t;
  auto secondsToTicks(double seconds, std::int64_t tickFrequency) -> std::int64_t;
  auto millisecondsToTicks(std::int64_t milliseconds, std::int64_t tickFrequency) -> std::int64_t;
  auto microsecondsToTicks(std::int64_t microseconds, std::int64_t tickFrequency) -> std::int64_t;
}

// Keeps track of elapsed real time since a given moment.  Guaranteed
// monotonically increasing and thread safe.
class Clock {
public:
  explicit Clock(bool start = true);

  Clock(Clock const& clock);

  auto operator=(Clock const& clock) -> Clock&;

  // Resets clock to 0 time
  void reset();

  void stop();
  void start();

  auto running() const -> bool;

  auto time() const -> double;
  auto milliseconds() const -> std::int64_t;

  // Override actual elapsed time with the given time.
  void setTime(double time);
  void setMilliseconds(std::int64_t millis);

  // Warp the clock backwards or forwards
  void adjustTime(double timeAdjustment);
  void adjustMilliseconds(std::int64_t millisAdjustment);

private:
  void updateElapsed() const;

  mutable RecursiveMutex m_mutex;
  mutable std::int64_t m_elapsedTicks;
  mutable std::optional<std::int64_t> m_lastTicks;
  bool m_running;
};

// An instance of Clock that counts down a given amount of time
class Timer : private Clock {
public:
  static auto withTime(double timeLeft, bool start = true) -> Timer;
  static auto withMilliseconds(std::int64_t millis, bool start = true) -> Timer;

  // Constructs a stopped timer whose time is up.
  Timer();
  Timer(Timer const& timer);
  auto operator=(Timer const& timer) -> Timer&;

  // Start the timer with the given time left.
  void restart(double timeLeft);
  void restartWithMilliseconds(std::int64_t millisecondsLeft);

  // Time remaining on the timer.  If negative is true, will return negative
  // time values after the timer is up, if false it stops at zero.
  auto timeLeft(bool negative = false) const -> double;
  auto millisecondsLeft(bool negative = false) const -> std::int64_t;

  // Is the time remaining <= 0.0?
  auto timeUp() const -> bool;

  using Clock::stop;
  using Clock::start;
  using Clock::running;
};

}
