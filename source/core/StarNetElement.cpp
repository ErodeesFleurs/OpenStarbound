#include "StarNetElement.hpp"

import std;

namespace Star {

auto NetElementVersion::current() const -> std::uint64_t {
  return m_version;
}

auto NetElementVersion::increment() -> std::uint64_t {
  return ++m_version;
}

void NetElement::enableNetInterpolation(float) {}

void NetElement::disableNetInterpolation() {}

void NetElement::tickNetInterpolation(float) {}

void NetElement::blankNetDelta(float) {}

}
