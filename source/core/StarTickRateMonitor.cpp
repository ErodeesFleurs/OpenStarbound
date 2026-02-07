#include "StarTickRateMonitor.hpp"
#include "StarTime.hpp"

import std;

namespace Star {

TickRateMonitor::TickRateMonitor(double window) : m_window(window) {
  reset();
}

auto TickRateMonitor::window() const -> double {
  return m_window;
}

void TickRateMonitor::reset() {
  m_lastTick = Time::monotonicTime() - m_window;
  m_ticks = 0;
}

auto TickRateMonitor::tick(unsigned count) -> double {
  double currentTime = Time::monotonicTime();

  if (m_lastTick > currentTime) {
    m_lastTick = currentTime - m_window;
    m_ticks = 0;
  } else if (m_lastTick < currentTime) {
    double timePast = currentTime - m_lastTick;
    double rate = m_ticks / m_window;
    m_ticks = std::max(0.0, m_ticks - timePast * rate);
    m_lastTick = currentTime;
  }

  m_ticks += count;

  return m_ticks / m_window;
}

auto TickRateMonitor::rate() const -> double {
  return TickRateMonitor(*this).tick(0);
}

TickRateApproacher::TickRateApproacher(double targetTickRate, double window)
  : m_tickRateMonitor(window), m_targetTickRate(targetTickRate) {}

auto TickRateApproacher::window() const -> double {
  return m_tickRateMonitor.window();
}

void TickRateApproacher::setWindow(double window) {
  if (window != m_tickRateMonitor.window()) {
    m_tickRateMonitor = TickRateMonitor(window);
    tick(m_targetTickRate * window);
  }
}

auto TickRateApproacher::targetTickRate() const -> double {
  return m_targetTickRate;
}

void TickRateApproacher::setTargetTickRate(double targetTickRate) {
  m_targetTickRate = targetTickRate;
}

void TickRateApproacher::reset() {
  setWindow(window());
}

auto TickRateApproacher::tick(unsigned count) -> double {
  return m_tickRateMonitor.tick(count);
}

auto TickRateApproacher::rate() const -> double {
  return m_tickRateMonitor.rate();
}

auto TickRateApproacher::ticksBehind() -> double {
  return (m_targetTickRate - m_tickRateMonitor.rate()) * window();
}

auto TickRateApproacher::ticksAhead() -> double {
  return -ticksBehind();
}

auto TickRateApproacher::spareTime() -> double {
  return ticksAhead() / m_targetTickRate;
}

}
