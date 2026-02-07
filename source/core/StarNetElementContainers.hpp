#pragma once

#include "StarMap.hpp"
#include "StarNetElement.hpp"
#include "StarVariant.hpp"

import std;

namespace Star {

// NetElement map container that is more efficient than the naive serialization
// of an entire Map, because it delta encodes changes to save networking
// traffic.
template <typename BaseMap>
class NetElementMapWrapper : public NetElement, private BaseMap {
public:
  using iterator = typename BaseMap::iterator;
  using const_iterator = typename BaseMap::const_iterator;

  using key_type = typename BaseMap::key_type;
  using mapped_type = typename BaseMap::mapped_type;
  using value_type = typename BaseMap::value_type;

  void initNetVersion(NetElementVersion const* version = nullptr) override;

  void enableNetInterpolation(float extrapolationHint = 0.0f) override;
  void disableNetInterpolation() override;
  void tickNetInterpolation(float dt) override;

  void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
  void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

  [[nodiscard]] auto shouldWriteNetDelta(std::uint64_t fromVersion, NetCompatibilityRules rules = {}) const -> bool;
  auto writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules = {}) const -> bool override;
  void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  auto get(key_type const& key) const -> mapped_type const&;
  auto ptr(key_type const& key) const -> mapped_type const*;

  auto begin() const -> const_iterator;
  auto end() const -> const_iterator;

  using BaseMap::contains;
  using BaseMap::empty;
  using BaseMap::keys;
  using BaseMap::maybe;
  using BaseMap::pairs;
  using BaseMap::size;
  using BaseMap::value;
  using BaseMap::values;

  auto insert(value_type v) -> std::pair<const_iterator, bool>;
  auto insert(key_type k, mapped_type v) -> std::pair<const_iterator, bool>;

  void add(key_type k, mapped_type v);
  // Calling set with a matching key and value does not cause a delta to be
  // produced
  void set(key_type k, mapped_type v);
  // set requires that mapped_type implement operator==, push always generates
  // a delta and does not require mapped_type operator==
  void push(key_type k, mapped_type v);

  auto remove(key_type const& k) -> bool;

  auto erase(const_iterator i) -> const_iterator;

  auto take(key_type const& k) -> mapped_type;
  auto maybeTake(key_type const& k) -> std::optional<mapped_type>;

  void clear();

  auto baseMap() const -> BaseMap const&;
  void reset(BaseMap values);
  auto pullUpdated() -> bool;

  // Sets this map to contain the same keys / values as the given map.  All
  // values in this map not found in the given map are removed.  (Same as
  // reset, but with arbitrary map type).
  template <typename MapType>
  void setContents(MapType const& values);

  [[nodiscard]] auto changeDataLastVersion() const -> std::uint64_t;

private:
  // If a delta is written from further back than this many steps, the delta
  // will fall back to a full serialization of the entire state.
  static std::int64_t const MaxChangeDataVersions = 100;

  struct SetChange {
    key_type key;
    mapped_type value;
  };
  struct RemoveChange {
    key_type key;
  };
  struct ClearChange {};

  using ElementChange = Variant<SetChange, RemoveChange, ClearChange>;

  static void writeChange(DataStream& ds, ElementChange const& change);
  static auto readChange(DataStream& ds) -> ElementChange;

  void addChangeData(ElementChange change);

  void addPendingChangeData(ElementChange change, float interpolationTime);
  void applyChange(ElementChange change);

  Deque<std::pair<std::uint64_t, ElementChange>> m_changeData;
  Deque<std::pair<float, ElementChange>> m_pendingChangeData;
  NetElementVersion const* m_netVersion = nullptr;
  std::uint64_t m_changeDataLastVersion = 0;
  bool m_updated = false;
  bool m_interpolationEnabled = false;
};

template <typename Key, typename Value>
using NetElementMap = NetElementMapWrapper<Map<Key, Value>>;

template <typename Key, typename Value>
using NetElementHashMap = NetElementMapWrapper<HashMap<Key, Value>>;

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::initNetVersion(NetElementVersion const* version) {
  m_netVersion = version;

  m_changeData.clear();
  m_changeDataLastVersion = 0;

  for (auto& change : Star::take(m_pendingChangeData))
    applyChange(std::move(change.second));

  addChangeData(ClearChange());
  for (auto const& p : *this)
    addChangeData(SetChange{p.first, p.second});
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::enableNetInterpolation(float) {
  m_interpolationEnabled = true;
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::disableNetInterpolation() {
  m_interpolationEnabled = false;
  for (auto& change : Star::take(m_pendingChangeData))
    applyChange(std::move(change.second));
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::tickNetInterpolation(float dt) {
  for (auto& p : m_pendingChangeData)
    p.first -= dt;

  while (!m_pendingChangeData.empty() && m_pendingChangeData.first().first <= 0.0f)
    applyChange(m_pendingChangeData.takeFirst().second);
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::netStore(DataStream& ds, NetCompatibilityRules rules) const {
  if (!checkWithRules(rules))
    return;
  ds.writeVlqU(BaseMap::size() + m_pendingChangeData.size());
  for (auto const& pair : *this)
    writeChange(ds, SetChange{pair.first, pair.second});

  for (auto const& p : m_pendingChangeData)
    writeChange(ds, p.second);
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::netLoad(DataStream& ds, NetCompatibilityRules rules) {
  if (!checkWithRules(rules))
    return;
  m_changeData.clear();
  m_changeDataLastVersion = m_netVersion ? m_netVersion->current() : 0;
  m_pendingChangeData.clear();
  BaseMap::clear();

  addChangeData(ClearChange());

  std::uint64_t count = ds.readVlqU();
  for (std::uint64_t i = 0; i < count; ++i) {
    auto change = readChange(ds);
    addChangeData(change);
    applyChange(std::move(change));
  }

  m_updated = true;
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::shouldWriteNetDelta(std::uint64_t fromVersion, NetCompatibilityRules rules) const -> bool {
  if (!checkWithRules(rules))
    return false;
  if (fromVersion < m_changeDataLastVersion)
    return true;

  for (auto const& p : m_changeData)
    if (p.first >= fromVersion)
      return true;

  return false;
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules) const -> bool {
  if (!checkWithRules(rules))
    return false;
  bool deltaWritten = false;

  if (fromVersion < m_changeDataLastVersion) {
    deltaWritten = true;
    ds.writeVlqU(1);
    netStore(ds, rules);

  } else {
    for (auto const& p : m_changeData) {
      if (p.first >= fromVersion) {
        deltaWritten = true;
        ds.writeVlqU(2);
        writeChange(ds, p.second);
      }
    }
  }

  if (deltaWritten)
    ds.writeVlqU(0);

  return deltaWritten;
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::readNetDelta(DataStream& ds, float interpolationTime, NetCompatibilityRules rules) {
  if (!checkWithRules(rules))
    return;
  while (true) {
    std::uint64_t code = ds.readVlqU();
    if (code == 0) {
      break;
    } else if (code == 1) {
      netLoad(ds, rules);
    } else if (code == 2) {
      auto change = readChange(ds);
      addChangeData(change);

      if (m_interpolationEnabled && interpolationTime > 0.0f)
        addPendingChangeData(std::move(change), interpolationTime);
      else
        applyChange(std::move(change));
    } else {
      throw IOException("Improper delta code received in NetElementMapWrapper::readNetDelta");
    }
  }
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::get(key_type const& key) const -> mapped_type const& {
  return BaseMap::get(key);
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::ptr(key_type const& key) const -> mapped_type const* {
  return BaseMap::ptr(key);
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::begin() const -> const_iterator {
  return BaseMap::begin();
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::end() const -> const_iterator {
  return BaseMap::end();
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::insert(value_type v) -> std::pair<const_iterator, bool> {
  auto res = BaseMap::insert(v);
  if (res.second) {
    addChangeData(SetChange{std::move(v.first), std::move(v.second)});
    m_updated = true;
  }
  return res;
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::insert(key_type k, mapped_type v) -> std::pair<const_iterator, bool> {
  return insert(value_type(std::move(k), std::move(v)));
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::add(key_type k, mapped_type v) {
  if (!insert(value_type(std::move(k), std::move(v))).second)
    throw MapException::format("Entry with key '{}' already present.", outputAny(k));
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::set(key_type k, mapped_type v) {
  auto i = BaseMap::find(k);
  if (i != BaseMap::end()) {
    if (!(i->second == v)) {
      addChangeData(SetChange{std::move(k), v});
      i->second = std::move(v);
      m_updated = true;
    }
  } else {
    addChangeData(SetChange{k, v});
    BaseMap::insert(value_type(std::move(k), std::move(v)));
    m_updated = true;
  }
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::push(key_type k, mapped_type v) {
  auto i = BaseMap::find(k);
  if (i != BaseMap::end()) {
    addChangeData(SetChange(std::move(k), v));
    i->second = std::move(v);
  } else {
    addChangeData(SetChange(k, v));
    BaseMap::insert(value_type(std::move(k), std::move(v)));
  }
  m_updated = true;
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::remove(key_type const& k) -> bool {
  auto i = BaseMap::find(k);
  if (i != BaseMap::end()) {
    BaseMap::erase(i);
    addChangeData(RemoveChange{k});
    m_updated = true;
    return true;
  }
  return false;
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::erase(const_iterator i) -> const_iterator {
  addChangeData(RemoveChange(i->first));
  m_updated = true;
  return BaseMap::erase(i);
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::take(key_type const& k) -> mapped_type {
  auto i = BaseMap::find(k);
  if (i == BaseMap::end())
    throw MapException::format("Key '{}' not found in Map::take()", outputAny(k));
  auto m = std::move(i->second);
  erase(i);
  return m;
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::maybeTake(key_type const& k) -> std::optional<mapped_type> {
  auto i = BaseMap::find(k);
  if (i == BaseMap::end())
    return {};
  auto m = std::move(i->second);
  erase(i);
  return std::optional<mapped_type>(std::move(m));
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::clear() {
  if (!empty()) {
    addChangeData(ClearChange());
    m_updated = true;
    BaseMap::clear();
  }
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::baseMap() const -> BaseMap const& {
  return *this;
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::reset(BaseMap values) {
  for (auto const& p : *this) {
    if (!values.contains(p.first)) {
      addChangeData(RemoveChange{p.first});
      m_updated = true;
    }
  }

  for (auto const& p : values) {
    auto v = ptr(p.first);
    if (!v || !(*v == p.second)) {
      addChangeData(SetChange{p.first, p.second});
      m_updated = true;
    }
  }

  BaseMap::operator=(std::move(values));
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::pullUpdated() -> bool {
  return Star::take(m_updated);
}

template <typename BaseMap>
template <typename MapType>
void NetElementMapWrapper<BaseMap>::setContents(MapType const& values) {
  reset(BaseMap::from(values));
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::changeDataLastVersion() const -> std::uint64_t {
  return m_changeDataLastVersion;
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::writeChange(DataStream& ds, ElementChange const& change) {
  if (auto sc = change.template ptr<SetChange>()) {
    ds.write<std::uint8_t>(0);
    ds.write(sc->key);
    ds.write(sc->value);
  } else if (auto rc = change.template ptr<RemoveChange>()) {
    ds.write<std::uint8_t>(1);
    ds.write(rc->key);
  } else {
    ds.write<std::uint8_t>(2);
  }
}

template <typename BaseMap>
auto NetElementMapWrapper<BaseMap>::readChange(DataStream& ds) -> ElementChange {
  auto t = ds.read<std::uint8_t>();
  if (t == 0) {
    SetChange sc;
    ds.read(sc.key);
    ds.read(sc.value);
    return sc;
  } else if (t == 1) {
    RemoveChange rc;
    ds.read(rc.key);
    return rc;
  } else if (t == 2) {
    return ClearChange();
  } else {
    throw IOException("Improper type code received in NetElementMapWrapper::readChange");
  }
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::addChangeData(ElementChange change) {
  std::uint64_t currentVersion = m_netVersion ? m_netVersion->current() : 0;

  m_changeData.append({currentVersion, std::move(change)});

  m_changeDataLastVersion = std::max<std::int64_t>((std::int64_t)currentVersion - MaxChangeDataVersions, 0);
  while (!m_changeData.empty() && m_changeData.first().first < m_changeDataLastVersion)
    m_changeData.removeFirst();
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::addPendingChangeData(ElementChange change, float interpolationTime) {
  if (!m_pendingChangeData.empty() && interpolationTime < m_pendingChangeData.last().first) {
    for (auto& change : Star::take(m_pendingChangeData))
      applyChange(std::move(change.second));
  }
  m_pendingChangeData.append({interpolationTime, std::move(change)});
}

template <typename BaseMap>
void NetElementMapWrapper<BaseMap>::applyChange(ElementChange change) {
  if (auto set = change.template ptr<SetChange>())
    BaseMap::set(std::move(set->key), std::move(set->value));
  else if (auto remove = change.template ptr<RemoveChange>())
    BaseMap::remove(std::move(remove->key));
  else
    BaseMap::clear();
  m_updated = true;
}

}// namespace Star
