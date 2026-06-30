#include "StarCelestialCoordinate.hpp"
#include "StarCelestialTypes.hpp"
#include "StarJsonExtra.hpp"
#include "StarLexicalCast.hpp"
#include "StarStaticRandom.hpp"
#include "StarDataStreamExtra.hpp"

namespace Star {

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

[[nodiscard]] bool CelestialCoordinate::isNull() const {
  return m_location == Vec3I() && m_planetaryOrbitNumber == 0 && m_satelliteOrbitNumber == 0;
}

[[nodiscard]] bool CelestialCoordinate::isSystem() const {
  return !isNull() && m_planetaryOrbitNumber == 0;
}

[[nodiscard]] bool CelestialCoordinate::isPlanetaryBody() const {
  return !isNull() && m_planetaryOrbitNumber != 0 && m_satelliteOrbitNumber == 0;
}

[[nodiscard]] bool CelestialCoordinate::isSatelliteBody() const {
  return !isNull() && m_planetaryOrbitNumber != 0 && m_satelliteOrbitNumber != 0;
}

[[nodiscard]] Vec3I CelestialCoordinate::location() const {
  return m_location;
}

[[nodiscard]] CelestialCoordinate CelestialCoordinate::system() const {
  if (isNull())
    throw CelestialException("CelestialCoordinate::system() called on null coordinate");
  return CelestialCoordinate(m_location);
}

[[nodiscard]] CelestialCoordinate CelestialCoordinate::planet() const {
  if (isPlanetaryBody())
    return *this;
  if (isSatelliteBody())
    return CelestialCoordinate(m_location, m_planetaryOrbitNumber);
  throw CelestialException("CelestialCoordinate::planet() called on null or system coordinate type");
}

[[nodiscard]] int CelestialCoordinate::orbitNumber() const {
  if (isSatelliteBody())
    return m_satelliteOrbitNumber;
  if (isPlanetaryBody())
    return m_planetaryOrbitNumber;
  if (isSystem())
    return 0;
  throw CelestialException("CelestialCoordinate::orbitNumber() called on null coordinate");
}

[[nodiscard]] CelestialCoordinate CelestialCoordinate::parent() const {
  if (isSatelliteBody())
    return CelestialCoordinate(m_location, m_planetaryOrbitNumber);
  if (isPlanetaryBody())
    return CelestialCoordinate(m_location);
  throw CelestialException("CelestialCoordinate::parent() called on null or system coordinate");
}

[[nodiscard]] CelestialCoordinate CelestialCoordinate::child(int orbitNumber) const {
  if (isSystem())
    return CelestialCoordinate(m_location, orbitNumber);
  if (isPlanetaryBody())
    return CelestialCoordinate(m_location, m_planetaryOrbitNumber, orbitNumber);
  throw CelestialException("CelestialCoordinate::child called on null or satellite coordinate");
}

[[nodiscard]] Json CelestialCoordinate::toJson() const {
  if (isNull()) {
    return Json();
  } else {
    return JsonObject{{"location", jsonFromVec3I(m_location)},
        {"planet", m_planetaryOrbitNumber},
        {"satellite", m_satelliteOrbitNumber}};
  }
}

[[nodiscard]] String CelestialCoordinate::id() const {
  return toString(*this);
}

[[nodiscard]] double CelestialCoordinate::distance(CelestialCoordinate const& rhs) const {
  return Vec2D(m_location[0] - rhs.m_location[0], m_location[1] - rhs.m_location[1]).magnitude();
}

[[nodiscard]] String CelestialCoordinate::filename() const {
  return id().replace(":", "_");
}

[[nodiscard]] CelestialCoordinate::operator bool() const {
  return !isNull();
}

std::ostream& operator<<(std::ostream& os, CelestialCoordinate const& coord) {
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

DataStream& operator>>(DataStream& ds, CelestialCoordinate& coordinate) {
  ds.read(coordinate.m_location);
  ds.read(coordinate.m_planetaryOrbitNumber);
  ds.read(coordinate.m_satelliteOrbitNumber);

  return ds;
}

DataStream& operator<<(DataStream& ds, CelestialCoordinate const& coordinate) {
  ds.write(coordinate.m_location);
  ds.write(coordinate.m_planetaryOrbitNumber);
  ds.write(coordinate.m_satelliteOrbitNumber);

  return ds;
}

}
