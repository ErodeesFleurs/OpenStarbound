#pragma once

#include "StarDataStreamExtra.hpp"
#include "StarIdMap.hpp"
#include "StarNetElement.hpp"
#include "StarStrongTypedef.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

// A dynamic group of NetElements that manages creation and destruction of
// individual elements, that is itself a NetElement.  Element changes are not
// delayed by the interpolation delay, they will always happen immediately, but
// this does not inhibit the Elements themselves from handling their own delta
// update delays normally.
template <typename Element>
class NetElementDynamicGroup : public NetElement {
public:
  using ElementPtr = shared_ptr<Element>;
  using ElementId = uint32_t;
  static constexpr ElementId NullElementId = 0;

  NetElementDynamicGroup() = default;

  NetElementDynamicGroup(NetElementDynamicGroup const&) = delete;
  NetElementDynamicGroup& operator=(NetElementDynamicGroup const&) = delete;

  // Must not call addNetElement / removeNetElement when being used as a slave,
  // id errors will result.
  void setElementFactory(function<ElementPtr()> elementFactory);
  ElementId addNetElement(ElementPtr element);
  void removeNetElement(ElementId id);

  // Remove all elements
  void clearNetElements();

  List<ElementId> netElementIds() const;
  ElementPtr getNetElement(ElementId id) const;

  List<ElementPtr> netElements() const;

  void initNetVersion(NetElementVersion const* version = nullptr) override;

  // Values are never interpolated, but they will be delayed for the given
  // interpolationTime.
  void enableNetInterpolation(float extrapolationHint = 0.0f) override;
  void disableNetInterpolation() override;
  void tickNetInterpolation(float dt) override;

  void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
  void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

  bool writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules = {}) const override;
  void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  void blankNetDelta(float interpolationTime = 0.0f) override;

private:
  // If a delta is written from further back than this many versions, the delta
  // will fall back to a full serialization of the entire state.
  static constexpr int64_t MaxChangeDataVersions = 100;

  using ElementRemovalType = ElementId;
  using ElementAdditionType = pair<ElementId, ByteArray>;

  using ElementReset = StrongTypedef<Empty, struct ElementResetTag>;
  using ElementRemoval = StrongTypedefBuiltin<ElementRemovalType, struct ElementRemovalTag>;
  using ElementAddition = StrongTypedef<ElementAdditionType, struct ElementAdditionTag>;

  using ElementChange = Variant<ElementReset, ElementRemoval, ElementAddition>;

  using ElementMap = IdMap<ElementId, ElementPtr>;

  void addChangeData(ElementChange change);

  void readyElement(ElementPtr const& element);

  NetElementVersion const* m_netVersion = nullptr;
  bool m_interpolationEnabled = false;
  float m_extrapolationHint = 0.0f;

  ElementMap m_idMap = ElementMap(1, highest<ElementId>());
  function<ElementPtr()> m_elementFactory = []() { return make_shared<Element>(); };

  struct VersionedElementChange {
    uint64_t version;
    ElementChange change;
  };
  Deque<VersionedElementChange> m_changeData;
  uint64_t m_changeDataLastVersion = 0;

  mutable DataStreamBuffer m_buffer;
  mutable HashSet<ElementId> m_receivedDeltaIds;
};

template <typename Element>
void NetElementDynamicGroup<Element>::setElementFactory(function<ElementPtr()> elementFactory) {
  requireDependency(elementFactory, "NetElementDynamicGroup", "element factory");
  m_elementFactory = std::move(elementFactory);
}

template <typename Element>
auto NetElementDynamicGroup<Element>::addNetElement(ElementPtr element) -> ElementId {
  readyElement(element);
  auto id = m_idMap.add(std::move(element));

  addChangeData(ElementAddition(id, {}));// we will write the data stream once we know the rules for the one receiving

  return id;
}

template <typename Element>
void NetElementDynamicGroup<Element>::removeNetElement(ElementId id) {
  m_idMap.remove(id);
  addChangeData(ElementRemoval{id});
}

template <typename Element>
void NetElementDynamicGroup<Element>::clearNetElements() {
  for (auto const& id : netElementIds())
    removeNetElement(id);
}

template <typename Element>
auto NetElementDynamicGroup<Element>::netElementIds() const -> List<ElementId> {
  return m_idMap.keys();
}

template <typename Element>
auto NetElementDynamicGroup<Element>::getNetElement(ElementId id) const -> ElementPtr {
  return m_idMap.get(id);
}

template <typename Element>
auto NetElementDynamicGroup<Element>::netElements() const -> List<ElementPtr> {
  return m_idMap.values();
}

template <typename Element>
void NetElementDynamicGroup<Element>::initNetVersion(NetElementVersion const* version) {
  m_netVersion = version;
  m_changeData.clear();
  m_changeDataLastVersion = 0;

  addChangeData(ElementReset());
  for (auto& [elementId, element] : m_idMap) {
    element->initNetVersion(m_netVersion);
    addChangeData(ElementAddition(elementId, {}));// we will write the data stream once we know the rules for the one receiving
  }
}

template <typename Element>
void NetElementDynamicGroup<Element>::enableNetInterpolation(float extrapolationHint) {
  m_interpolationEnabled = true;
  m_extrapolationHint = extrapolationHint;
  for (auto& [elementId, element] : m_idMap)
    element->enableNetInterpolation(extrapolationHint);
}

template <typename Element>
void NetElementDynamicGroup<Element>::disableNetInterpolation() {
  m_interpolationEnabled = false;
  m_extrapolationHint = 0.0f;
  for (auto& [elementId, element] : m_idMap)
    element->disableNetInterpolation();
}

template <typename Element>
void NetElementDynamicGroup<Element>::tickNetInterpolation(float dt) {
  for (auto& [elementId, element] : m_idMap)
    element->tickNetInterpolation(dt);
}

template <typename Element>
void NetElementDynamicGroup<Element>::netStore(DataStream& ds, NetCompatibilityRules rules) const {
  if (!checkWithRules(rules))
    return;
  ds.writeVlqU(m_idMap.size());

  m_buffer.setStreamCompatibilityVersion(rules);
  for (auto& [elementId, element] : m_idMap) {
    ds.writeVlqU(elementId);
    element->netStore(m_buffer, rules);
    ds.write(m_buffer.data());
    m_buffer.clear();
  }
}

template <typename Element>
void NetElementDynamicGroup<Element>::netLoad(DataStream& ds, NetCompatibilityRules rules) {
  if (!checkWithRules(rules))
    return;
  m_changeData.clear();
  m_changeDataLastVersion = m_netVersion ? m_netVersion->current() : 0;
  m_idMap.clear();

  addChangeData(ElementReset());

  uint64_t count = ds.readVlqU();

  for (uint64_t i = 0; i < count; ++i) {
    ElementId id = ds.readVlqU();
    DataStreamBuffer storeBuffer(ds.read<ByteArray>());

    ElementPtr element = m_elementFactory();
    element->netLoad(storeBuffer, rules);
    readyElement(element);

    m_idMap.add(id, std::move(element));
    addChangeData(ElementAddition(id, storeBuffer.takeData()));
  }
}

template <typename Element>
bool NetElementDynamicGroup<Element>::writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules) const {
  if (!checkWithRules(rules))
    return false;
  if (fromVersion < m_changeDataLastVersion) {
    ds.write<bool>(true);
    netStore(ds, rules);
    return true;

  } else {
    bool deltaWritten = false;
    auto willWrite = [&]() {
      if (!deltaWritten) {
        deltaWritten = true;
        ds.write<bool>(false);
      }
    };

    for (auto const& changeData : m_changeData) {
      if (changeData.version >= fromVersion) {
        auto const& change = changeData.change;
        if (ElementAddition const* elementAddition = change.template ptr<ElementAddition>()) {
          ElementId id = elementAddition->first;
          if (shared_ptr<Element> const* element = m_idMap.ptr(id)) {
            willWrite();
            ds.writeVlqU(1);
            DataStreamBuffer storeBuffer;
            element->get()->netStore(storeBuffer, rules);
            ds.write(ElementChange(ElementAddition(id, storeBuffer.takeData())));
          }
        } else {
          willWrite();
          ds.writeVlqU(1);
          ds.write(change);
        }
      }
    }

    m_buffer.setStreamCompatibilityVersion(rules);
    for (auto& [elementId, element] : m_idMap) {
      if (element->writeNetDelta(m_buffer, fromVersion, rules)) {
        willWrite();
        ds.writeVlqU(elementId + 1);
        ds.writeBytes(m_buffer.data());
        m_buffer.clear();
      }
    }

    if (deltaWritten)
      ds.writeVlqU(0);

    return deltaWritten;
  }
}

template <typename Element>
void NetElementDynamicGroup<Element>::readNetDelta(DataStream& ds, float interpolationTime, NetCompatibilityRules rules) {
  if (!checkWithRules(rules))
    return;
  bool isFull = ds.read<bool>();
  if (isFull) {
    netLoad(ds, rules);
  } else {
    while (true) {
      uint64_t code = ds.readVlqU();
      if (code == 0) {
        break;
      }
      if (code == 1) {
        auto changeUpdate = ds.read<ElementChange>();
        addChangeData(changeUpdate);

        if (changeUpdate.template is<ElementReset>()) {
          m_idMap.clear();
        } else if (auto addition = changeUpdate.template ptr<ElementAddition>()) {
          auto& [elementId, elementData] = *addition;
          ElementPtr element = m_elementFactory();
          DataStreamBuffer storeBuffer(std::move(elementData));
          element->netLoad(storeBuffer, rules);
          readyElement(element);
          m_idMap.add(elementId, std::move(element));
        } else if (auto removal = changeUpdate.template ptr<ElementRemoval>()) {
          m_idMap.remove(*removal);
        }
      } else {
        ElementId elementId = code - 1;
        auto const& element = m_idMap.get(elementId);
        element->readNetDelta(ds, interpolationTime, rules);
        if (m_interpolationEnabled)
          m_receivedDeltaIds.add(elementId);
      }
    }

    if (m_interpolationEnabled) {
      for (auto& [elementId, element] : m_idMap) {
        if (!m_receivedDeltaIds.contains(elementId))
          element->blankNetDelta(interpolationTime);
      }

      m_receivedDeltaIds.clear();
    }
  }
}

template <typename Element>
void NetElementDynamicGroup<Element>::blankNetDelta(float interpolationTime) {
  if (m_interpolationEnabled) {
    for (auto& [elementId, element] : m_idMap)
      element->blankNetDelta(interpolationTime);
  }
}

template <typename Element>
void NetElementDynamicGroup<Element>::addChangeData(ElementChange change) {
  uint64_t currentVersion = m_netVersion ? m_netVersion->current() : 0;
  starAssert(m_changeData.empty() || m_changeData.last().version <= currentVersion);

  m_changeData.append(VersionedElementChange{currentVersion, std::move(change)});

  m_changeDataLastVersion = max<int64_t>(static_cast<int64_t>(currentVersion) - MaxChangeDataVersions, 0);
  while (!m_changeData.empty() && m_changeData.first().version < m_changeDataLastVersion)
    m_changeData.removeFirst();
}

template <typename Element>
void NetElementDynamicGroup<Element>::readyElement(ElementPtr const& element) {
  element->initNetVersion(m_netVersion);
  if (m_interpolationEnabled)
    element->enableNetInterpolation(m_extrapolationHint);
  else
    element->disableNetInterpolation();
}

}// namespace Star
