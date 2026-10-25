import std;
import star.string;
import star.format;
import star.time;
import star.thread;

namespace Star {
namespace Time {

std::int64_t epochTicks() {
  return std::chrono::system_clock::now().time_since_epoch().count();
}

std::int64_t epochTickFrequency() {
  static_assert(std::chrono::system_clock::period::num == 1);
  return std::chrono::system_clock::period::den;
}

std::int64_t monotonicTicks() {
  return std::chrono::steady_clock::now().time_since_epoch().count();
}

std::int64_t monotonicTickFrequency() {
  static_assert(std::chrono::steady_clock::period::num == 1);
  return std::chrono::steady_clock::period::den;
}

double timeSinceEpoch() {
  return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::int64_t millisecondsSinceEpoch() {
  auto elapsed = std::chrono::system_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed + std::chrono::microseconds(500)).count();
}

double monotonicTime() {
  return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

std::int64_t monotonicMilliseconds() {
  auto elapsed = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed + std::chrono::microseconds(500)).count();
}

std::int64_t monotonicMicroseconds() {
  auto elapsed = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(elapsed + std::chrono::nanoseconds(500)).count();
}

String printDuration(double time) {
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

  String result;
  for (auto component : {std::cref(hours), std::cref(minutes), std::cref(seconds), std::cref(milliseconds)}) {
    if (component.get().empty())
      continue;
    if (!result.empty())
      result += ", ";
    result += component.get();
  }
  return result;
}

String printCurrentDateAndTime(String format) {
  return printDateAndTime(epochTicks(), std::move(format));
}

double ticksToSeconds(std::int64_t ticks, std::int64_t tickFrequency) {
  return ticks / (double)tickFrequency;
}

// Retain the numeric API's half-unit bias followed by truncation toward zero,
// including its asymmetric rounding for negative countdown values.
std::int64_t ticksToMilliseconds(std::int64_t ticks, std::int64_t tickFrequency) {
  std::int64_t ticksPerMs = (tickFrequency + 500) / 1000;
  return (ticks + ticksPerMs / 2) / ticksPerMs;
}

std::int64_t ticksToMicroseconds(std::int64_t ticks, std::int64_t tickFrequency) {
  std::int64_t ticksPerUs = (tickFrequency + 500000) / 1000000;
  return (ticks + ticksPerUs / 2) / ticksPerUs;
}

std::int64_t secondsToTicks(double seconds, std::int64_t tickFrequency) {
  return std::round(seconds * tickFrequency);
}

std::int64_t millisecondsToTicks(std::int64_t milliseconds, std::int64_t tickFrequency) {
  return milliseconds * ((tickFrequency + 500) / 1000);
}

std::int64_t microsecondsToTicks(std::int64_t microseconds, std::int64_t tickFrequency) {
  return microseconds * ((tickFrequency + 500000) / 1000000);
}

}

Clock::Clock() : Clock(true) {}

Clock::Clock(bool start) {
  m_elapsed = std::chrono::steady_clock::duration::zero();
  m_running = false;
  if (start)
    Clock::start();
}

Clock::Clock(Clock const& clock) {
  operator=(clock);
}

Clock& Clock::operator=(Clock const& clock) {
  m_elapsed = clock.m_elapsed;
  m_lastUpdate = clock.m_lastUpdate;
  m_running = clock.m_running;

  return *this;
}

void Clock::reset() {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  updateElapsed();
  m_elapsed = std::chrono::steady_clock::duration::zero();
}

void Clock::stop() {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  m_lastUpdate.reset();
  m_running = false;
}

void Clock::start() {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  m_running = true;
  updateElapsed();
}

bool Clock::running() const {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  return m_running;
}

double Clock::time() const {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  updateElapsed();
  return std::chrono::duration<double>(m_elapsed).count();
}

std::int64_t Clock::milliseconds() const {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  updateElapsed();
  return std::chrono::duration_cast<std::chrono::milliseconds>(m_elapsed + std::chrono::microseconds(500)).count();
}

void Clock::setTime(double time) {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  updateElapsed();
  m_elapsed = std::chrono::steady_clock::duration(Time::secondsToTicks(time, Time::monotonicTickFrequency()));
}

void Clock::setMilliseconds(std::int64_t millis) {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  updateElapsed();
  m_elapsed = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(millis));
}

void Clock::adjustTime(double timeAdjustment) {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  setTime(std::max<double>(0.0, time() + timeAdjustment));
}

void Clock::adjustMilliseconds(std::int64_t millisAdjustment) {
  std::unique_lock locker(m_mutex, std::defer_lock);
  lockWithDiagnostics(locker);
  setMilliseconds(milliseconds() + millisAdjustment);
}

void Clock::updateElapsed() const {
  if (!m_running)
    return;

  auto currentTime = std::chrono::steady_clock::now();

  if (m_lastUpdate)
    m_elapsed += currentTime - *m_lastUpdate;

  m_lastUpdate = currentTime;
}

Timer Timer::withTime(double timeLeft, bool start) {
  Timer timer;
  timer.setTime(-timeLeft);
  if (start)
    timer.start();
  return timer;
}

Timer Timer::withMilliseconds(std::int64_t millis, bool start) {
  Timer timer;
  timer.setMilliseconds(-millis);
  if (start)
    timer.start();
  return timer;
}

Timer::Timer() : Clock(false) {
  setTime(0.0);
}

Timer::Timer(Timer const& timer)
  : Clock(timer) {}

Timer& Timer::operator=(Timer const& timer) {
  Clock::operator=(timer);
  return *this;
}

void Timer::restart(double timeLeft) {
  Clock::setTime(-timeLeft);
  Clock::start();
}

void Timer::restartWithMilliseconds(std::int64_t millisecondsLeft) {
  Clock::setMilliseconds(-millisecondsLeft);
  Clock::start();
}

double Timer::timeLeft(bool negative) const {
  double timeLeft = -Clock::time();
  if (!negative)
    timeLeft = std::max(0.0, timeLeft);
  return timeLeft;
}

std::int64_t Timer::millisecondsLeft(bool negative) const {
  std::int64_t millisLeft = -Clock::milliseconds();
  if (!negative)
    millisLeft = std::max<std::int64_t>(0, millisLeft);
  return millisLeft;
}

bool Timer::timeUp() const {
  return Clock::time() >= 0.0;
}

}
