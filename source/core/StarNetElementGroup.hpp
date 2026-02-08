#pragma once

#include "StarDataStreamDevices.hpp"
#include "StarNetElement.hpp"

import std;

namespace Star {

// A static group of NetElements that itself is a NetElement and serializes
// changes based on the order in which elements are added.  All participants
// must externally add elements of the correct type in the correct order.
class NetElementGroup : public NetElement {
public:
  NetElementGroup() = default;

  NetElementGroup(NetElementGroup const&) = delete;
  auto operator=(NetElementGroup const&) -> NetElementGroup& = delete;

  // Add an element to the group.
  void addNetElement(NetElement* element, bool propagateInterpolation = true);

  // Removes all previously added elements
  void clearNetElements();

  void initNetVersion(NetElementVersion const* version = nullptr) override;

  void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
  void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

  void enableNetInterpolation(float extrapolationHint = 0.0f) override;
  void disableNetInterpolation() override;
  void tickNetInterpolation(float dt) override;

  auto writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules = {}) const -> bool override;
  void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  void blankNetDelta(float interpolationTime) override;

  auto netVersion() const -> NetElementVersion const*;
  auto netInterpolationEnabled() const -> bool;
  auto netExtrapolationHint() const -> float;

private:
  List<std::pair<NetElement*, bool>> m_elements;
  NetElementVersion const* m_version = nullptr;
  bool m_interpolationEnabled = false;
  float m_extrapolationHint = 0.0f;

  HashMap<VersionNumber, std::size_t> m_elementCounts;

  mutable DataStreamBuffer m_buffer;
};

inline auto NetElementGroup::netVersion() const -> NetElementVersion const* {
  return m_version;
}

inline auto NetElementGroup::netInterpolationEnabled() const -> bool {
  return m_interpolationEnabled;
}

inline auto NetElementGroup::netExtrapolationHint() const -> float {
  return m_extrapolationHint;
}

}// namespace Star
