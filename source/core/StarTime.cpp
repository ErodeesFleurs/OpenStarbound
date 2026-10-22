import std;
import star.string;
import star.format;
import star.thread;
import star.time;
import star.math_common;
import star.lexical_cast;

namespace Star {
namespace Time {

double timeSinceEpoch() {
  return ticksToSeconds(epochTicks(), epochTickFrequency());
}

std::int64_t millisecondsSinceEpoch() {
  return ticksToMilliseconds(epochTicks(), epochTickFrequency());
}

double monotonicTime() {
  return ticksToSeconds(monotonicTicks(), monotonicTickFrequency());
}

std::int64_t monotonicMilliseconds() {
  return ticksToMilliseconds(monotonicTicks(), monotonicTickFrequency());
}

std::int64_t monotonicMicroseconds() {
  return ticksToMicroseconds(monotonicTicks(), monotonicTickFrequency());
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
  for (auto const* component : {&hours, &minutes, &seconds, &milliseconds}) {
    if (component->empty())
      continue;
    if (!result.empty())
      result += ", ";
    result += *component;
  }
  return result;
}

String printCurrentDateAndTime(String format) {
  return printDateAndTime(epochTicks(), std::move(format));
}

double ticksToSeconds(std::int64_t ticks, std::int64_t tickFrequency) {
  return ticks / (double)tickFrequency;
}

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
  m_elapsedTicks = 0;
  m_running = false;
  if (start)
    Clock::start();
}

Clock::Clock(Clock const& clock) {
  operator=(clock);
}

Clock& Clock::operator=(Clock const& clock) {
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

bool Clock::running() const {
  RecursiveMutexLocker locker(m_mutex);
  return m_running;
}

double Clock::time() const {
  RecursiveMutexLocker locker(m_mutex);
  updateElapsed();
  return Time::ticksToSeconds(m_elapsedTicks, Time::monotonicTickFrequency());
}

std::int64_t Clock::milliseconds() const {
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
