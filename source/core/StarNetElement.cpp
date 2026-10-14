import std;
import star.data_stream;

import star.net_element;


namespace Star {

std::uint64_t NetElementVersion::current() const {
  return m_version;
}

std::uint64_t NetElementVersion::increment() {
  return ++m_version;
}

void NetElement::enableNetInterpolation(float) {}

void NetElement::disableNetInterpolation() {}

void NetElement::tickNetInterpolation(float) {}

void NetElement::blankNetDelta(float) {}

}
