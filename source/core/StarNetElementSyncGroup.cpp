import std;
import star.format;
import star.data_stream;

import star.net_element_group;
import star.net_element_sync_group;

import star.net_compatibility;
namespace Star {

std::expected<void, StarException> NetElementSyncGroup::enableNetInterpolation(float extrapolationHint) {
  { auto result = NetElementGroup::enableNetInterpolation(extrapolationHint); if (!result) return std::unexpected(std::move(result.error())); }
  if (m_hasRecentChanges)
    { auto result = netElementsNeedLoad(false); if (!result) return std::unexpected(std::move(result.error())); }
  return {};
}

std::expected<void, StarException> NetElementSyncGroup::disableNetInterpolation() {
  { auto result = NetElementGroup::disableNetInterpolation(); if (!result) return std::unexpected(std::move(result.error())); }
  if (m_hasRecentChanges)
    { auto result = netElementsNeedLoad(false); if (!result) return std::unexpected(std::move(result.error())); }
  return {};
}

std::expected<void, StarException> NetElementSyncGroup::tickNetInterpolation(float dt) {
  { auto result = NetElementGroup::tickNetInterpolation(dt); if (!result) return std::unexpected(std::move(result.error())); }
  if (m_hasRecentChanges) {
    m_recentDeltaTime -= dt;
    if (netInterpolationEnabled())
      { auto result = netElementsNeedLoad(false); if (!result) return std::unexpected(std::move(result.error())); }

    if (m_recentDeltaTime < 0.0f && m_recentDeltaWasBlank)
      m_hasRecentChanges = false;
  }
  return {};
}

std::expected<void, StarException> NetElementSyncGroup::netStore(DataStream& ds, NetCompatibilityRules rules) const {
  if (!checkWithRules(rules)) return {};
  { auto result = const_cast<NetElementSyncGroup*>(this)->netElementsNeedStore(); if (!result) return std::unexpected(std::move(result.error())); }
  return NetElementGroup::netStore(ds, rules);
}

std::expected<void, StarException> NetElementSyncGroup::netLoad(DataStream& ds, NetCompatibilityRules rules) {
  if (!checkWithRules(rules)) return {};
  { auto result = NetElementGroup::netLoad(ds, rules); if (!result) return std::unexpected(std::move(result.error())); }
  { auto result = netElementsNeedLoad(true); if (!result) return std::unexpected(std::move(result.error())); }
  return {};
}

std::expected<bool, StarException> NetElementSyncGroup::writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules) const {
  if (!checkWithRules(rules)) return false;
  { auto result = const_cast<NetElementSyncGroup*>(this)->netElementsNeedStore(); if (!result) return std::unexpected(std::move(result.error())); }
  return NetElementGroup::writeNetDelta(ds, fromVersion, rules);
}

std::expected<void, StarException> NetElementSyncGroup::readNetDelta(DataStream& ds, float interpolationTime, NetCompatibilityRules rules) {
  if (!checkWithRules(rules)) return {};
  { auto result = NetElementGroup::readNetDelta(ds, interpolationTime, rules); if (!result) return std::unexpected(std::move(result.error())); }

  m_hasRecentChanges = true;
  m_recentDeltaTime = interpolationTime;
  m_recentDeltaWasBlank = false;

  { auto result = netElementsNeedLoad(false); if (!result) return std::unexpected(std::move(result.error())); }
  return {};
}

std::expected<void, StarException> NetElementSyncGroup::blankNetDelta(float interpolationTime) {
  { auto result = NetElementGroup::blankNetDelta(interpolationTime); if (!result) return std::unexpected(std::move(result.error())); }

  if (!m_recentDeltaWasBlank) {
    m_recentDeltaTime = interpolationTime;
    m_recentDeltaWasBlank = true;
  }

  if (m_hasRecentChanges && netInterpolationEnabled())
    { auto result = netElementsNeedLoad(false); if (!result) return std::unexpected(std::move(result.error())); }
  return {};
}

std::expected<void, StarException> NetElementSyncGroup::netElementsNeedLoad(bool) { return {}; }

std::expected<void, StarException> NetElementSyncGroup::netElementsNeedStore() { return {}; }

void NetElementCallbackGroup::setNeedsLoadCallback(std::function<std::expected<void, StarException>(bool)> needsLoadCallback) {
  m_netElementsNeedLoad = std::move(needsLoadCallback);
}

void NetElementCallbackGroup::setNeedsStoreCallback(std::function<std::expected<void, StarException>()> needsStoreCallback) {
  m_netElementsNeedStore = std::move(needsStoreCallback);
}

std::expected<void, StarException> NetElementCallbackGroup::netElementsNeedLoad(bool load) {
  if (m_netElementsNeedLoad)
    return m_netElementsNeedLoad(load);
  return {};
}

std::expected<void, StarException> NetElementCallbackGroup::netElementsNeedStore() {
  if (m_netElementsNeedStore)
    return m_netElementsNeedStore();
  return {};
}

}
