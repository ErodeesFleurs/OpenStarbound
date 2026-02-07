#pragma once

#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

using CelestialException = ExceptionDerived<"CelestialException">;

// Specifies coordinates to either a planetary system, a planetary body, or a
// satellite around such a planetary body.  The terms here are meant to be very
// generic, a "planetary body" could be an asteroid field, or a ship, or
// anything in orbit around the center of mass of a specific planetary system.
// The terms are really simply meant as a hierarchy of orbits.
//
// No validity checking is done here, any coordinate to any body whether it
// exists in a specific universe or not can be expressed.  isNull() simply
// checks whether the coordinate is the result of the empty constructor, not
// whether the coordinate points to a valid object or not.
class CelestialCoordinate {
public:
  // Creates the null CelestialCoordinate
  CelestialCoordinate();
  CelestialCoordinate(Vec3I location, int planetaryOrbitNumber = 0, int satelliteOrbitNumber = 0);
  explicit CelestialCoordinate(Json const& json);

  // Is this coordanate the null coordinate?
  [[nodiscard]] auto isNull() const -> bool;

  // Does this coordinate point to an entire planetary system?
  [[nodiscard]] auto isSystem() const -> bool;
  // Is this world a body whose "designated gravity buddy" is the center of a
  // planetary system?
  [[nodiscard]] auto isPlanetaryBody() const -> bool;
  // Is this world a body which orbits around a planetary body?
  [[nodiscard]] auto isSatelliteBody() const -> bool;

  [[nodiscard]] auto location() const -> Vec3I;

  // Returns just the system coordinate portion of this celestial coordinate.
  [[nodiscard]] auto system() const -> CelestialCoordinate;

  // Returns just the planet portion of this celestial coordinate, throws
  // exception if this is a system coordinate.
  [[nodiscard]] auto planet() const -> CelestialCoordinate;

  // Returns the orbit number for this body.  Returns 0 for system coordinates.
  [[nodiscard]] auto orbitNumber() const -> int;

  // Returns the system for a planet or the planet for a satellite.  If this is
  // a system coordinate, throws an exception.
  [[nodiscard]] auto parent() const -> CelestialCoordinate;

  // Returns a coordinate to a child object at the given orbit number.  If the
  // orbit number is 0, returns *this, otherwise if this is a satellite throws
  // an exception.
  [[nodiscard]] auto child(int orbitNumber) const -> CelestialCoordinate;

  // Stores coordinate in json form that can be used to reconstruct it.
  [[nodiscard]] auto toJson() const -> Json;

  // Returns coordinate in a parseable String format.
  [[nodiscard]] auto id() const -> String;

  // Returns a fakey fake distance
  [[nodiscard]] auto distance(CelestialCoordinate const& rhs) const -> double;

  // Returns a slightly different string format than id(), which is still in an
  // accepted format, but more appropriate for filenames.
  [[nodiscard]] auto filename() const -> String;

  // Returns true if not null
  explicit operator bool() const;

  auto operator<(CelestialCoordinate const& rhs) const -> bool;
  auto operator==(CelestialCoordinate const& rhs) const -> bool;
  auto operator!=(CelestialCoordinate const& rhs) const -> bool;

  // Prints in the same format accepted by parser.  Each coordinate is unique.
  friend auto operator<<(std::ostream& os, CelestialCoordinate const& coord) -> std::ostream&;

  friend auto operator>>(DataStream& ds, CelestialCoordinate& coordinate) -> DataStream&;
  friend auto operator<<(DataStream& ds, CelestialCoordinate const& coordinate) -> DataStream&;

private:
  Vec3I m_location;
  int m_planetaryOrbitNumber;
  int m_satelliteOrbitNumber;
};

}// namespace Star

template <>
struct std::formatter<Star::CelestialCoordinate> : Star::ostream_formatter {};
