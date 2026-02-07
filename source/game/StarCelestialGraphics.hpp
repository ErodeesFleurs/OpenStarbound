#pragma once

#include "StarCelestialDatabase.hpp"
#include "StarConfig.hpp"

import std;

namespace Star {

class CelestialDatabase;

// Functions for generating and drawing worlds from a celestial database.
// Guards against drawing unloaded celestial objects, will return empty if no
// information is returned from the celestial database.
//
// Drawing methods return the stack of images to draw and the scale to draw
// them at.
class CelestialGraphics {
public:
  // Some static versions of drawing functions are given that do not require an
  // active CelestialDatabasePtr to draw.

  static auto drawSystemPlanetaryObject(CelestialParameters const& celestialParameters) -> List<std::pair<String, float>>;
  static auto drawSystemCentralBody(CelestialParameters const& celestialParameters) -> List<std::pair<String, float>>;

  // Specify the shadowing parameters in order to use the shadowing
  // information from that body instead of the primary one.
  static auto drawWorld(
    CelestialParameters const& celestialParameters, std::optional<CelestialParameters> const& shadowingParameters = {}) -> List<std::pair<String, float>>;
  static auto worldHorizonImages(CelestialParameters const& celestialParameters) -> List<std::pair<String, String>>;
  static auto worldRadialPosition(CelestialParameters const& celestialParameters) -> int;

  // Each orbiting body will occupy a unique orbital slot, but to give
  // graphical diversity, will also fit into exactly one radial slot for
  // display purposes.  The range of radial numbers is [0, RadialPosiitons)
  static auto planetRadialPositions() -> int;
  static auto satelliteRadialPositions() -> int;

  static auto drawSystemTwinkle(Ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& system, double twinkleTime) -> List<std::pair<String, float>>;

  // Returns the small graphic for the given planetary object appropriate for a
  // system-level view.
  static auto drawSystemPlanetaryObject(Ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate) -> List<std::pair<String, float>>;
  static auto drawSystemCentralBody(Ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate) -> List<std::pair<String, float>>;

  // Returns the graphics appropriate to draw an entire world (planetary object
  // or satellite object) in a map view.  Shadows the satellite the same as
  // its parent planetary object.
  static auto drawWorld(Ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate) -> List<std::pair<String, float>>;

  // Draw all of the left and right image pairs
  // for all the layers for the
  // world horizon.
  static auto worldHorizonImages(Ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate) -> List<std::pair<String, String>>;

  static int worldRadialPosition(Ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate);

private:
};

}// namespace Star
