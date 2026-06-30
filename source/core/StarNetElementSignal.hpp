#pragma once

#include "StarNetElement.hpp"

namespace Star {

// NetElement that sends signals during delta writes that can be received by
// slaves.  It has no 'state', and nothing is sent during a store / load, and
// it only keeps past signals for a maximum number of versions.  Thus, it is
// not appropriate to use to send updates to long term states, only for event
// like things that are not harmful if missed.
template <typename Signal>
class NetElementSignal : public NetElement {
public:
  NetElementSignal(size_t maxSignalQueue = 32);

  void initNetVersion(observer_ptr<NetElementVersion const> version = nullptr) override;

  void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
  void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

  void enableNetInterpolation(float extrapolationHint = 0.0f) override;
  void disableNetInterpolation() override;
  void tickNetInterpolation(float dt) override;

  [[nodiscard]] bool writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules = {}) const override;
  void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void send(Signal signal);
  [[nodiscard]] List<Signal> receive();

private:
  struct SignalEntry {
    uint64_t version;
    Signal signal;
    bool received;
  };
  struct PendingSignal {
    float timeToSend;
    Signal signal;
  };

  size_t m_maxSignalQueue;
  observer_ptr<NetElementVersion const> m_netVersion = nullptr;
  bool m_netInterpolationEnabled = false;
  Deque<SignalEntry> m_signals;
  Deque<PendingSignal> m_pendingSignals;
};

template <typename Signal>
NetElementSignal<Signal>::NetElementSignal(size_t maxSignalQueue) {
  m_maxSignalQueue = maxSignalQueue;
}

template <typename Signal>
void NetElementSignal<Signal>::initNetVersion(observer_ptr<NetElementVersion const> version) {
  m_netVersion = version;
  m_signals.clear();
}

template <typename Signal>
void NetElementSignal<Signal>::netStore(DataStream&, NetCompatibilityRules) const {}

template <typename Signal>
void NetElementSignal<Signal>::netLoad(DataStream&, NetCompatibilityRules) {
}

template <typename Signal>
void NetElementSignal<Signal>::enableNetInterpolation(float) {
  m_netInterpolationEnabled = true;
}

template <typename Signal>
void NetElementSignal<Signal>::disableNetInterpolation() {
  m_netInterpolationEnabled = false;
  for (auto& pendingSignal : take(m_pendingSignals))
    send(std::move(pendingSignal.signal));
}

template <typename Signal>
void NetElementSignal<Signal>::tickNetInterpolation(float dt) {
  for (auto& pendingSignal : m_pendingSignals)
    pendingSignal.timeToSend -= dt;

  while (!m_pendingSignals.empty() && m_pendingSignals.first().timeToSend <= 0.0f) {
    auto pendingSignal = m_pendingSignals.takeFirst();
    send(std::move(pendingSignal.signal));
  }
}

template <typename Signal>
bool NetElementSignal<Signal>::writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules) const {
  if (!checkWithRules(rules)) return false;
  size_t numToWrite = 0;
  for (auto const& signalEntry : m_signals) {
    if (signalEntry.version >= fromVersion)
      ++numToWrite;
  }
  if (numToWrite == 0)
    return false;

  ds.writeVlqU(numToWrite);

  for (auto const& signalEntry : m_signals) {
    if (signalEntry.version >= fromVersion)
      ds.write(signalEntry.signal);
  }

  return true;
}

template <typename Signal>
void NetElementSignal<Signal>::readNetDelta(DataStream& ds, float interpolationTime, NetCompatibilityRules rules) {
  if (!checkWithRules(rules)) return;
  size_t numToRead = ds.readVlqU();
  for (size_t i = 0; i < numToRead; ++i) {
    Signal s;
    ds.read(s);
    if (m_netInterpolationEnabled && interpolationTime > 0.0f) {
      if (!m_pendingSignals.empty() && m_pendingSignals.last().timeToSend > interpolationTime) {
        for (auto& pendingSignal : take(m_pendingSignals))
          send(std::move(pendingSignal.signal));
      }
      m_pendingSignals.append(PendingSignal{interpolationTime, std::move(s)});
    } else {
      send(std::move(s));
    }
  }
}

template <typename Signal>
void NetElementSignal<Signal>::send(Signal signal) {
  m_signals.append({m_netVersion ? m_netVersion->current() : 0, signal, false});
  while (m_signals.size() > m_maxSignalQueue)
    m_signals.removeFirst();
}

template <typename Signal>
[[nodiscard]] List<Signal> NetElementSignal<Signal>::receive() {
  List<Signal> received;
  for (auto& signalEntry : m_signals) {
    if (!signalEntry.received) {
      received.append(signalEntry.signal);
      signalEntry.received = true;
    }
  }
  return received;
}

}
