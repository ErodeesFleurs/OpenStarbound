#include "StarNetElementBasicFields.hpp"

import std;

namespace Star {

void NetElementSize::readData(DataStream& ds, std::size_t& v) const {
  std::uint64_t s = ds.readVlqU();
  if (s == 0)
    v = std::numeric_limits<std::size_t>::max();
  else
    v = s - 1;
}

void NetElementSize::writeData(DataStream& ds, std::size_t const& v) const {
  if (v == std::numeric_limits<std::size_t>::max())
    ds.writeVlqU(0);
  else
    ds.writeVlqU(v + 1);
}

void NetElementBool::readData(DataStream& ds, bool& v) const {
  ds.read(v);
}

void NetElementBool::writeData(DataStream& ds, bool const& v) const {
  ds.write(v);
}

void NetElementEvent::trigger() {
  set(get() + 1);
}

auto NetElementEvent::pullOccurrences() -> std::uint64_t {
  std::uint64_t occurrences = get();
  std::uint64_t unchecked = occurrences - m_pulledOccurrences;
  m_pulledOccurrences = occurrences;
  return unchecked;
}

auto NetElementEvent::pullOccurred() -> bool {
  return pullOccurrences() != 0;
}

void NetElementEvent::ignoreOccurrences() {
  m_pulledOccurrences = get();
}

void NetElementEvent::setIgnoreOccurrencesOnNetLoad(bool ignoreOccurrencesOnNetLoad) {
  m_ignoreOccurrencesOnNetLoad = ignoreOccurrencesOnNetLoad;
}

void NetElementEvent::netLoad(DataStream& ds, NetCompatibilityRules rules) {
  NetElementUInt::netLoad(ds, rules);
  if (m_ignoreOccurrencesOnNetLoad)
    ignoreOccurrences();
}

void NetElementEvent::updated() {
  NetElementBasicField::updated();
  if (m_pulledOccurrences > get())
    m_pulledOccurrences = get();
}

}// namespace Star
