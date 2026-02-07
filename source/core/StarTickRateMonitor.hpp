#pragma once

namespace Star {

// Monitors the rate at which 'tick()' is called in wall-clock seconds.
class TickRateMonitor {
public:
  // 'window' controls the dropoff at which 'rate' will approach zero if tick
  // is not called, measured in seconds.
  TickRateMonitor(double window);

  [[nodiscard]] auto window() const -> double;

  // Resets to a zero tick-rate state
  void reset();

  // Ticks the given number of times, returns the current rate.
  auto tick(unsigned count = 1) -> double;

  // Returns the rate as of the *current* time, not the time of the last tick.
  [[nodiscard]] auto rate() const -> double;

private:
  void dropOff(double currentTime);

  double m_window;
  double m_lastTick;
  double m_ticks;
};

// Helps tick at as close as possible to a given tick rate
class TickRateApproacher {
public:
  TickRateApproacher(double targetTickRate, double window);

  // The TickRateMonitor window influences how long the TickRateApproacher will
  // try and speed up or slow down the tick rate to match the target tick rate.
  // It should be chosen so that it is not so short that the actual target rate
  // drifts, but not too long so that the rate returns to normal quickly enough
  // with outliers.
  [[nodiscard]] auto window() const -> double;
  // Setting the window to a new value will reset the TickRateApproacher
  void setWindow(double window);

  [[nodiscard]] auto targetTickRate() const -> double;
  void setTargetTickRate(double targetTickRate);

  // Resets such that the current tick rate is assumed to be perfectly at the
  // target.
  void reset();

  auto tick(unsigned count = 1) -> double;
  [[nodiscard]] auto rate() const -> double;

  // How many ticks we currently should perform, so that if each tick happened
  // instantly, we would be as close to the target tick rate as possible.  If
  // we are ahead, may be negative.
  auto ticksBehind() -> double;

  // The negative of ticksBehind, is positive for how many ticks ahead we
  // currently are.
  auto ticksAhead() -> double;

  // How much spare time we have until the tick rate will begin to be behind
  // the target tick rate.
  auto spareTime() -> double;

private:
  TickRateMonitor m_tickRateMonitor;
  double m_targetTickRate;
};

};// namespace Star
