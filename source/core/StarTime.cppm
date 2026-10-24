module;
import std;
import star.string;

namespace Star {

namespace Time {
  double timeSinceEpoch();
  std::int64_t millisecondsSinceEpoch();

  double monotonicTime();
  std::int64_t monotonicMilliseconds();
  std::int64_t monotonicMicroseconds();

  // Pretty print a duration of time (In days, hours, minutes, seconds, and milliseconds)
  String printDuration(double time);

  // Pretty print a given date and time
  String printDateAndTime(std::int64_t epochTicks, String format = "<year>-<month>-<day> <hours>:<minutes>:<seconds>.<millis>");
  String printCurrentDateAndTime(String format = "<year>-<month>-<day> <hours>:<minutes>:<seconds>.<millis>");

  // Native system_clock ticks since the Unix epoch; use epochTickFrequency()
  // rather than assuming a fixed tick unit.
  std::int64_t epochTicks();
  // Epoch ticks per second, static throughout application lifetime.
  std::int64_t epochTickFrequency();

  // Native steady_clock ticks since an unspecified time before program start;
  // use monotonicTickFrequency() rather than assuming a fixed tick unit.
  std::int64_t monotonicTicks();
  // Monotonic ticks per second, static throughout application lifetime.
  std::int64_t monotonicTickFrequency();

  double ticksToSeconds(std::int64_t ticks, std::int64_t tickFrequency);
  std::int64_t ticksToMilliseconds(std::int64_t ticks, std::int64_t tickFrequency);
  std::int64_t ticksToMicroseconds(std::int64_t ticks, std::int64_t tickFrequency);
  std::int64_t secondsToTicks(double seconds, std::int64_t tickFrequency);
  std::int64_t millisecondsToTicks(std::int64_t milliseconds, std::int64_t tickFrequency);
  std::int64_t microsecondsToTicks(std::int64_t microseconds, std::int64_t tickFrequency);
}

// Keeps track of elapsed real time since a given moment.  Guaranteed
// monotonically increasing and thread safe.
class Clock {
public:
  explicit Clock();
  explicit Clock(bool start);

  Clock(Clock const& clock);

  Clock& operator=(Clock const& clock);

  // Resets clock to 0 time
  void reset();

  void stop();
  void start();

  bool running() const;

  double time() const;
  std::int64_t milliseconds() const;

  // Override actual elapsed time with the given time.
  void setTime(double time);
  void setMilliseconds(std::int64_t millis);

  // Warp the clock backwards or forwards
  void adjustTime(double timeAdjustment);
  void adjustMilliseconds(std::int64_t millisAdjustment);

private:
  void updateElapsed() const;

  mutable std::recursive_mutex m_mutex;
  mutable std::chrono::steady_clock::duration m_elapsed;
  mutable std::optional<std::chrono::steady_clock::time_point> m_lastUpdate;
  bool m_running;
};

// An instance of Clock that counts down a given amount of time
class Timer : private Clock {
public:
  static Timer withTime(double timeLeft, bool start = true);
  static Timer withMilliseconds(std::int64_t millis, bool start = true);

  // Constructs a stopped timer whose time is up.
  Timer();
  Timer(Timer const& timer);
  Timer& operator=(Timer const& timer);

  // Start the timer with the given time left.
  void restart(double timeLeft);
  void restartWithMilliseconds(std::int64_t millisecondsLeft);

  // Time remaining on the timer.  If negative is true, will return negative
  // time values after the timer is up, if false it stops at zero.
  double timeLeft(bool negative = false) const;
  std::int64_t millisecondsLeft(bool negative = false) const;

  // Is the time remaining <= 0.0?
  bool timeUp() const;

  using Clock::stop;
  using Clock::start;
  using Clock::running;
};

}

export module star.time;

export namespace Star {
  using ::Star::Clock;
  using ::Star::Timer;
}

export namespace Star::Time {
  using ::Star::Time::timeSinceEpoch;
  using ::Star::Time::millisecondsSinceEpoch;
  using ::Star::Time::monotonicTime;
  using ::Star::Time::monotonicMilliseconds;
  using ::Star::Time::monotonicMicroseconds;
  using ::Star::Time::printDuration;
  using ::Star::Time::printDateAndTime;
  using ::Star::Time::printCurrentDateAndTime;
  using ::Star::Time::epochTicks;
  using ::Star::Time::epochTickFrequency;
  using ::Star::Time::monotonicTicks;
  using ::Star::Time::monotonicTickFrequency;
  using ::Star::Time::ticksToSeconds;
  using ::Star::Time::ticksToMilliseconds;
  using ::Star::Time::ticksToMicroseconds;
  using ::Star::Time::secondsToTicks;
  using ::Star::Time::millisecondsToTicks;
  using ::Star::Time::microsecondsToTicks;
}
