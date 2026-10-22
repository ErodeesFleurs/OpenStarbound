import std;
import star.format;
import star.data_stream;

import star.net_element;


namespace Star {

std::uint64_t NetElementVersion::current() const {
  return m_version;
}

std::uint64_t NetElementVersion::increment() {
  return ++m_version;
}

std::expected<void, StarException> NetElement::enableNetInterpolation(float) { return {}; }

std::expected<void, StarException> NetElement::disableNetInterpolation() { return {}; }

std::expected<void, StarException> NetElement::tickNetInterpolation(float) { return {}; }

std::expected<void, StarException> NetElement::blankNetDelta(float) { return {}; }

}
