#pragma once

#include "StarThread.hpp"

namespace Star {

class Clock;
using ClockPtr = SharedPtr<Clock>;

namespace Time {
  [[nodiscard]] double timeSinceEpoch();
  [[nodiscard]] int64_t millisecondsSinceEpoch();

  [[nodiscard]] double monotonicTime();
  [[nodiscard]] int64_t monotonicMilliseconds();
  [[nodiscard]] int64_t monotonicMicroseconds();

  // Pretty print a duration of time (In days, hours, minutes, seconds, and milliseconds)
  [[nodiscard]] String printDuration(double time);

  // Pretty print a given date and time
  [[nodiscard]] String printDateAndTime(int64_t epochTicks, String format = "<year>-<month>-<day> <hours>:<minutes>:<seconds>.<millis>");
  [[nodiscard]] String printCurrentDateAndTime(String format = "<year>-<month>-<day> <hours>:<minutes>:<seconds>.<millis>");

  // Ticks since unix epoch
  [[nodiscard]] int64_t epochTicks();
  // Epoch ticks per second, static throughout application lifetime.
  [[nodiscard]] int64_t epochTickFrequency();

  // Ticks since unspecified time before program start
  [[nodiscard]] int64_t monotonicTicks();
  // Monotonic ticks per second, static throughout application lifetime.
  [[nodiscard]] int64_t monotonicTickFrequency();

  [[nodiscard]] double ticksToSeconds(int64_t ticks, int64_t tickFrequency);
  [[nodiscard]] int64_t ticksToMilliseconds(int64_t ticks, int64_t tickFrequency);
  [[nodiscard]] int64_t ticksToMicroseconds(int64_t ticks, int64_t tickFrequency);
  [[nodiscard]] int64_t secondsToTicks(double seconds, int64_t tickFrequency);
  [[nodiscard]] int64_t millisecondsToTicks(int64_t milliseconds, int64_t tickFrequency);
  [[nodiscard]] int64_t microsecondsToTicks(int64_t microseconds, int64_t tickFrequency);
}

// Keeps track of elapsed real time since a given moment.  Guaranteed
// monotonically increasing and thread safe.
class Clock {
public:
  explicit Clock(bool start = true);

  Clock(Clock const& clock);

  Clock& operator=(Clock const& clock);

  // Resets clock to 0 time
  void reset();

  void stop();
  void start();

  [[nodiscard]] bool running() const;

  [[nodiscard]] double time() const;
  [[nodiscard]] int64_t milliseconds() const;

  // Override actual elapsed time with the given time.
  void setTime(double time);
  void setMilliseconds(int64_t millis);

  // Warp the clock backwards or forwards
  void adjustTime(double timeAdjustment);
  void adjustMilliseconds(int64_t millisAdjustment);

private:
  void updateElapsed() const;

  mutable RecursiveMutex m_mutex;
  mutable int64_t m_elapsedTicks = 0;
  mutable Maybe<int64_t> m_lastTicks;
  bool m_running = false;
};

// An instance of Clock that counts down a given amount of time
class Timer : private Clock {
public:
  [[nodiscard]] static Timer withTime(double timeLeft, bool start = true);
  [[nodiscard]] static Timer withMilliseconds(int64_t millis, bool start = true);

  // Constructs a stopped timer whose time is up.
  Timer();
  Timer(Timer const& timer);
  Timer& operator=(Timer const& timer);

  // Start the timer with the given time left.
  void restart(double timeLeft);
  void restartWithMilliseconds(int64_t millisecondsLeft);

  // Time remaining on the timer.  If negative is true, will return negative
  // time values after the timer is up, if false it stops at zero.
  [[nodiscard]] double timeLeft(bool negative = false) const;
  [[nodiscard]] int64_t millisecondsLeft(bool negative = false) const;

  // Is the time remaining <= 0.0?
  [[nodiscard]] bool timeUp() const;

  using Clock::stop;
  using Clock::start;
  using Clock::running;
};

}
