#include "StarCelestialCoordinate.hpp"
#include "StarJsonExtra.hpp"
#include "StarLexicalCast.hpp"

import std;

namespace Star {

CelestialCoordinate::CelestialCoordinate() : m_planetaryOrbitNumber(0), m_satelliteOrbitNumber(0) {}

CelestialCoordinate::CelestialCoordinate(Vec3I location, int planetaryOrbitNumber, int satelliteOrbitNumber)
    : m_location(std::move(location)),
      m_planetaryOrbitNumber(planetaryOrbitNumber),
      m_satelliteOrbitNumber(satelliteOrbitNumber) {}

CelestialCoordinate::CelestialCoordinate(Json const& variant) : CelestialCoordinate() {
  if (variant.isType(Json::Type::String)) {
    String id = variant.toString();
    if (!id.empty() && !id.equalsIgnoreCase("null")) {
      try {
        auto plist = id.splitAny(" _:");

        m_location[0] = lexicalCast<int>(plist.at(0));
        m_location[1] = lexicalCast<int>(plist.at(1));
        m_location[2] = lexicalCast<int>(plist.at(2));

        if (plist.size() > 3)
          m_planetaryOrbitNumber = lexicalCast<int>(plist.at(3));
        if (plist.size() > 4)
          m_satelliteOrbitNumber = lexicalCast<int>(plist.at(4));

        if (m_planetaryOrbitNumber <= 0)
          throw CelestialException(strf("Planetary body number out of range in '{}'", id));
        if (m_satelliteOrbitNumber < 0)
          throw CelestialException(strf("Satellite body number out of range in '{}'", id));
      } catch (StarException const& e) {
        throw CelestialException(strf("Error parsing CelestialCoordinate from '{}'", id), e);
      }
    }
  } else if (variant.isType(Json::Type::Object)) {
    m_location = jsonToVec3I(variant.get("location"));
    m_planetaryOrbitNumber = variant.getInt("planet", 0);
    m_satelliteOrbitNumber = variant.getInt("satellite", 0);
  } else if (!variant.isNull()) {
    throw CelestialException(
      strf("Improper variant type {} trying to convert to SystemCoordinate", variant.typeName()));
  }
}

auto CelestialCoordinate::isNull() const -> bool {
  return m_location == Vec3I() && m_planetaryOrbitNumber == 0 && m_satelliteOrbitNumber == 0;
}

auto CelestialCoordinate::isSystem() const -> bool {
  return !isNull() && m_planetaryOrbitNumber == 0;
}

auto CelestialCoordinate::isPlanetaryBody() const -> bool {
  return !isNull() && m_planetaryOrbitNumber != 0 && m_satelliteOrbitNumber == 0;
}

auto CelestialCoordinate::isSatelliteBody() const -> bool {
  return !isNull() && m_planetaryOrbitNumber != 0 && m_satelliteOrbitNumber != 0;
}

auto CelestialCoordinate::location() const -> Vec3I {
  return m_location;
}

auto CelestialCoordinate::system() const -> CelestialCoordinate {
  if (isNull())
    throw CelestialException("CelestialCoordinate::system() called on null coordinate");
  return {m_location};
}

auto CelestialCoordinate::planet() const -> CelestialCoordinate {
  if (isPlanetaryBody())
    return *this;
  if (isSatelliteBody())
    return {m_location, m_planetaryOrbitNumber};
  throw CelestialException("CelestialCoordinate::planet() called on null or system coordinate type");
}

auto CelestialCoordinate::orbitNumber() const -> int {
  if (isSatelliteBody())
    return m_satelliteOrbitNumber;
  if (isPlanetaryBody())
    return m_planetaryOrbitNumber;
  if (isSystem())
    return 0;
  throw CelestialException("CelestialCoordinate::orbitNumber() called on null coordinate");
}

auto CelestialCoordinate::parent() const -> CelestialCoordinate {
  if (isSatelliteBody())
    return {m_location, m_planetaryOrbitNumber};
  if (isPlanetaryBody())
    return {m_location};
  throw CelestialException("CelestialCoordinate::parent() called on null or system coordinate");
}

auto CelestialCoordinate::child(int orbitNumber) const -> CelestialCoordinate {
  if (isSystem())
    return {m_location, orbitNumber};
  if (isPlanetaryBody())
    return {m_location, m_planetaryOrbitNumber, orbitNumber};
  throw CelestialException("CelestialCoordinate::child called on null or satellite coordinate");
}

auto CelestialCoordinate::toJson() const -> Json {
  if (isNull()) {
    return {};
  } else {
    return JsonObject{{"location", jsonFromVec3I(m_location)},
                      {"planet", m_planetaryOrbitNumber},
                      {"satellite", m_satelliteOrbitNumber}};
  }
}

auto CelestialCoordinate::id() const -> String {
  return toString(*this);
}

auto CelestialCoordinate::distance(CelestialCoordinate const& rhs) const -> double {
  return Vec2D(m_location[0] - rhs.m_location[0], m_location[1] - rhs.m_location[1]).magnitude();
}

auto CelestialCoordinate::filename() const -> String {
  return id().replace(":", "_");
}

CelestialCoordinate::operator bool() const {
  return !isNull();
}

auto CelestialCoordinate::operator<(CelestialCoordinate const& rhs) const -> bool {
  return tie(m_location, m_planetaryOrbitNumber, m_satelliteOrbitNumber)
    < tie(rhs.m_location, rhs.m_planetaryOrbitNumber, rhs.m_satelliteOrbitNumber);
}

auto CelestialCoordinate::operator==(CelestialCoordinate const& rhs) const -> bool {
  return tie(m_location, m_planetaryOrbitNumber, m_satelliteOrbitNumber)
    == tie(rhs.m_location, rhs.m_planetaryOrbitNumber, rhs.m_satelliteOrbitNumber);
}

auto CelestialCoordinate::operator!=(CelestialCoordinate const& rhs) const -> bool {
  return tie(m_location, m_planetaryOrbitNumber, m_satelliteOrbitNumber)
    != tie(rhs.m_location, rhs.m_planetaryOrbitNumber, rhs.m_satelliteOrbitNumber);
}

auto operator<<(std::ostream& os, CelestialCoordinate const& coord) -> std::ostream& {
  if (coord.isNull()) {
    os << "null";
  } else {
    format(os, "{}:{}:{}", coord.m_location[0], coord.m_location[1], coord.m_location[2]);

    if (coord.m_planetaryOrbitNumber) {
      format(os, ":{}", coord.m_planetaryOrbitNumber);
      if (coord.m_satelliteOrbitNumber)
        format(os, ":{}", coord.m_satelliteOrbitNumber);
    }
  }

  return os;
}

auto operator>>(DataStream& ds, CelestialCoordinate& coordinate) -> DataStream& {
  ds.read(coordinate.m_location);
  ds.read(coordinate.m_planetaryOrbitNumber);
  ds.read(coordinate.m_satelliteOrbitNumber);

  return ds;
}

auto operator<<(DataStream& ds, CelestialCoordinate const& coordinate) -> DataStream& {
  ds.write(coordinate.m_location);
  ds.write(coordinate.m_planetaryOrbitNumber);
  ds.write(coordinate.m_satelliteOrbitNumber);

  return ds;
}

}// namespace Star
