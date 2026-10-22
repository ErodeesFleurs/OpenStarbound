module;
import std;
import star.format;
import star.string;
import star.json;

import star.celestial_coordinate;
import star.game_types;
import star.color;
import star.bi_map;
import star.sky_types;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;

namespace Star {

class CelestialDatabase;
class Root;

// Functions for generating and drawing worlds from a celestial database.
// Guards against drawing unloaded celestial objects, will return empty if no
// information is returned from the celestial database.
//
// Drawing methods return the stack of images to draw and the scale to draw
// them at.
class CelestialGraphics {
public:
  // Some static versions of drawing functions are given that do not require an
  // active CelestialDatabase to draw.

  [[nodiscard]] static std::expected<List<std::pair<String, float>>, StarException> drawSystemPlanetaryObject(CelestialParameters const& celestialParameters);
  [[nodiscard]] static std::expected<List<std::pair<String, float>>, StarException> drawSystemCentralBody(CelestialParameters const& celestialParameters);

  // Specify the shadowing parameters in order to use the shadowing
  // information from that body instead of the primary one.
  [[nodiscard]] static std::expected<List<std::pair<String, float>>, StarException> drawWorld(
      Root& root, CelestialParameters const& celestialParameters, std::optional<CelestialParameters> const& shadowingParameters = {});
  [[nodiscard]] static std::expected<List<std::pair<String, String>>, StarException> worldHorizonImages(Root& root, CelestialParameters const& celestialParameters);
  [[nodiscard]] static std::expected<int, StarException> worldRadialPosition(Root& root, CelestialParameters const& celestialParameters);

  // Each orbiting body will occupy a unique orbital slot, but to give
  // graphical diversity, will also fit into exactly one radial slot for
  // display purposes.  The range of radial numbers is [0, RadialPosiitons)
  [[nodiscard]] static std::expected<int, StarException> planetRadialPositions(Root& root);
  [[nodiscard]] static std::expected<int, StarException> satelliteRadialPositions(Root& root);

  [[nodiscard]] static std::expected<List<std::pair<String, float>>, StarException> drawSystemTwinkle(Root& root, std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& system, double twinkleTime);

  // Returns the small graphic for the given planetary object appropriate for a
  // system-level view.
  [[nodiscard]] static std::expected<List<std::pair<String, float>>, StarException> drawSystemPlanetaryObject(std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate);
  [[nodiscard]] static std::expected<List<std::pair<String, float>>, StarException> drawSystemCentralBody(std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate);

  // Returns the graphics appropriate to draw an entire world (planetary object
  // or satellite object) in a map view.  Shadows the satellite the same as
  // its parent planetary object.
  [[nodiscard]] static std::expected<List<std::pair<String, float>>, StarException> drawWorld(Root& root, std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate);

  // Draw all of the left and right image pairs for all the layers for the
  // world horizon.
  [[nodiscard]] static std::expected<List<std::pair<String, String>>, StarException> worldHorizonImages(Root& root, std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate);

  [[nodiscard]] static std::expected<int, StarException> worldRadialPosition(Root& root, std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate);

private:
};

}

export module star.celestial_graphics;

export namespace Star {
  using ::Star::CelestialGraphics;
  using ::Star::CelestialDatabase;
}
