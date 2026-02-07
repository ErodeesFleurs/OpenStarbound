#pragma once

#include "StarDataStreamDevices.hpp"
#include "StarNetElement.hpp"

import std;

namespace Star {

// Mixin for the NetElement that should be the top element for a network, wraps
// any NetElement class and manages the NetElementVersion.
template <typename BaseNetElement>
class NetElementTop : public BaseNetElement {
public:
  NetElementTop();

  // Writes the state update to the given DataStream then returns the version
  // code that should be passed to the next call to writeState.  If
  // 'fromVersion' is 0, then this is a full write for an initial read of a
  // slave NetElementTop.
  auto writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t>;
  // Reads a state produced by a call to writeState, optionally with the
  // interpolation delay time for the data contained in this state update.  If
  // the state is a full update rather than a delta, the interoplation delay
  // will be ignored.  Blank updates are not necessary to send to be read by
  // readState, *unless* extrapolation is enabled.  If extrapolation is
  // enabled, reading a blank update calls 'blankNetDelta' which is necessary
  // to not improperly extrapolate past the end of incoming deltas.
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {});

private:
  using BaseNetElement::blankNetDelta;
  using BaseNetElement::checkWithRules;
  using BaseNetElement::initNetVersion;
  using BaseNetElement::netLoad;
  using BaseNetElement::netStore;
  using BaseNetElement::readNetDelta;
  using BaseNetElement::writeNetDelta;

  NetElementVersion m_netVersion;
};

template <typename BaseNetElement>
NetElementTop<BaseNetElement>::NetElementTop() {
  BaseNetElement::initNetVersion(&m_netVersion);
}

template <typename BaseNetElement>
auto NetElementTop<BaseNetElement>::writeNetState(std::uint64_t fromVersion, NetCompatibilityRules rules) -> std::pair<ByteArray, std::uint64_t> {
  DataStreamBuffer ds;
  ds.setStreamCompatibilityVersion(rules);
  if (fromVersion == 0) {
    ds.write<bool>(true);
    BaseNetElement::netStore(ds, rules);
    return {ds.takeData(), m_netVersion.increment()};
  } else {
    ds.write<bool>(false);
    if (!BaseNetElement::writeNetDelta(ds, fromVersion, rules))
      return {ByteArray(), m_netVersion.current()};
    else
      return {ds.takeData(), m_netVersion.increment()};
  }
}

template <typename BaseNetElement>
void NetElementTop<BaseNetElement>::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  if (data.empty()) {
    BaseNetElement::blankNetDelta(interpolationTime);
  } else {
    DataStreamBuffer ds(std::move(data));
    ds.setStreamCompatibilityVersion(rules);
    if (ds.read<bool>())
      BaseNetElement::netLoad(ds, rules);
    else
      BaseNetElement::readNetDelta(ds, interpolationTime, rules);
  }
}

}// namespace Star
