#include "StarNetElementGroup.hpp"

namespace Star {

void NetElementGroup::addNetElement(NetElement* element, bool propagateInterpolation) {
  assert(!m_elements.any([element](GroupElement const& groupElement) {
      return groupElement.element == element;
    }));

  element->initNetVersion(m_version);
  if (m_interpolationEnabled && propagateInterpolation)
    element->enableNetInterpolation(m_extrapolationHint);
  m_elements.append(GroupElement{element, propagateInterpolation});


  for (VersionNumber i = 0; i < (CurrentStreamVersion + 1); i++) {
    if (element->checkWithRules(NetCompatibilityRules(i)))
      m_elementCounts[i]++;
  }
}

void NetElementGroup::clearNetElements() {
  m_elementCounts.clear();
  m_elements.clear();
}

void NetElementGroup::initNetVersion(NetElementVersion const* version) {
  m_version = version;
  for (auto& groupElement : m_elements)
    groupElement.element->initNetVersion(m_version);
}

void NetElementGroup::netStore(DataStream& ds, NetCompatibilityRules rules) const {
  if (!checkWithRules(rules)) return;
  for (auto& groupElement : m_elements)
    if (groupElement.element->checkWithRules(rules))
      groupElement.element->netStore(ds, rules);
}

void NetElementGroup::netLoad(DataStream& ds, NetCompatibilityRules rules) {
  if (!checkWithRules(rules)) return;
  for (auto& groupElement : m_elements)
    if (groupElement.element->checkWithRules(rules))
      groupElement.element->netLoad(ds, rules);
}

void NetElementGroup::enableNetInterpolation(float extrapolationHint) {
  m_interpolationEnabled = true;
  m_extrapolationHint = extrapolationHint;
  for (auto& groupElement : m_elements) {
    if (groupElement.propagateInterpolation)
      groupElement.element->enableNetInterpolation(extrapolationHint);
  }
}

void NetElementGroup::disableNetInterpolation() {
  m_interpolationEnabled = false;
  m_extrapolationHint = 0;
  for (auto& groupElement : m_elements) {
    if (groupElement.propagateInterpolation)
      groupElement.element->disableNetInterpolation();
  }
}

void NetElementGroup::tickNetInterpolation(float dt) {
  if (m_interpolationEnabled) {
    for (auto& groupElement : m_elements)
      groupElement.element->tickNetInterpolation(dt);
  }
}

bool NetElementGroup::writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules) const {
  if (!checkWithRules(rules)) return false;

  auto expectedSize = m_elementCounts.maybe(rules.version()).value(m_elements.size());

  if (expectedSize == 0) {
    return false;
  } else if (expectedSize == 1) {
    for (auto& groupElement : m_elements) {
      if (groupElement.element->checkWithRules(rules)) {
        return groupElement.element->writeNetDelta(ds, fromVersion, rules);
      }
    }
  } else {
    bool deltaWritten = false;
    uint64_t i = 0;
    m_buffer.setStreamCompatibilityVersion(rules);
    for (auto& groupElement : m_elements) {
      if (i > expectedSize)
        break;
      if (!groupElement.element->checkWithRules(rules))
        continue;
      ++i;

      if (groupElement.element->writeNetDelta(m_buffer, fromVersion, rules)) {
        deltaWritten = true;
        ds.writeVlqU(i);
        ds.writeBytes(m_buffer.data());
        m_buffer.clear();
      }
    }
    if (deltaWritten)
      ds.writeVlqU(0);
    return deltaWritten;
  }
  return false;
}

void NetElementGroup::readNetDelta(DataStream& ds, float interpolationTime, NetCompatibilityRules rules) {
  if (!checkWithRules(rules))
    return;

  auto expectedSize = m_elementCounts.maybe(rules.version()).value(m_elements.size());

  if (expectedSize == 0) {
    throw IOException("readNetDelta called on empty NetElementGroup");
  } else if (expectedSize == 1) {
    for (auto& groupElement : m_elements)
      if (groupElement.element->checkWithRules(rules)) {
        groupElement.element->readNetDelta(ds, interpolationTime, rules);
        break;
      }
  } else {
    uint64_t readIndex = ds.readVlqU();
    uint64_t i = 0;
    uint64_t offset = 0;
    for (auto& groupElement : m_elements) {
      if (i > expectedSize)
        break;
      if (!groupElement.element->checkWithRules(rules)) {
        offset++;
        continue;
      }
      if (readIndex == 0 || readIndex - 1 > i) {
        if (m_interpolationEnabled)
          m_elements[i + offset].element->blankNetDelta(interpolationTime);
      } else if (readIndex - 1 == i) {
        m_elements[i + offset].element->readNetDelta(ds, interpolationTime, rules);
        readIndex = ds.readVlqU();
      } else {
        throw IOException("group indexes out of order in NetElementGroup::readNetDelta");
      }
      ++i;
    }
  }
}

void NetElementGroup::blankNetDelta(float interpolationTime) {
  if (m_interpolationEnabled) {
    for (auto& groupElement : m_elements)
      groupElement.element->blankNetDelta(interpolationTime);
  }
}

}
