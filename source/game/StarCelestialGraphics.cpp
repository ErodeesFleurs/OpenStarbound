import star.json_extra;
import star.json;
import star.ordered_map;
import star.either;
import star.variant;
import star.game_types;
import star.liquid_types;
import star.color;
import star.bi_map;
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
import star.casting;
import star.lexical_cast;
import star.format;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.optional;
import star.string;
import star.version;

import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;
import star.celestial_graphics;
import star.tile_damage;
import star.plant_database;
import star.parallax;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.terrain_database;
import star.liquids_database;
import star.celestial_database;
import star.biome_database;


import star.static_random;
namespace Star {

List<std::pair<String, float>> CelestialGraphics::drawSystemPlanetaryObject(CelestialParameters const& parameters) {
  return {{parameters.getParameter("smallImage").toString(), parameters.getParameter("smallImageScale").toFloat()}};
}

List<std::pair<String, float>> CelestialGraphics::drawSystemCentralBody(CelestialParameters const& parameters) {
  return {{parameters.getParameter("image").toString(), parameters.getParameter("imageScale").toFloat()}};
}

List<std::pair<String, float>> CelestialGraphics::drawWorld(
    Root& root, CelestialParameters const& celestialParameters, std::optional<CelestialParameters> const& overrideShadowParameters) {
  auto assets = root.assets();
  auto liquidsDatabase = root.liquidsDatabase();

  CelestialParameters shadowParameters = overrideShadowParameters.value_or(celestialParameters);

  String type = celestialParameters.getParameter("worldType").toString();

  List<std::pair<String, float>> layers;

  if (type == "Terrestrial") {
    auto terrestrialParameters = as<TerrestrialWorldParameters>(celestialParameters.visitableParameters());
    if (!terrestrialParameters)
      return {};

    auto gfxConfig = jsonMerge(assets->json("/celestial.config:terrestrialGraphics").get("default"),
        assets->json("/celestial.config:terrestrialGraphics").get(terrestrialParameters->typeName, JsonObject()));

    auto liquidImages = gfxConfig.getString("liquidImages", "");
    auto baseImages = gfxConfig.getString("baseImages", "");
    auto shadowImages = gfxConfig.getString("shadowImages", "");
    auto baseCount = gfxConfig.getInt("baseCount", 0);
    auto dynamicsImages = gfxConfig.getString("dynamicsImages", "");
    float imageScale = celestialParameters.getParameter("imageScale", 1.0f).toFloat();

    // If the planet has water, then draw the corresponding water image as the
    // base layer, otherwise use the bottom most mask image.
    if (terrestrialParameters->primarySurfaceLiquid != EmptyLiquidId && !liquidImages.empty()) {
      String liquidBaseImage = liquidImages.replace("<liquid>", liquidsDatabase->liquidName(terrestrialParameters->primarySurfaceLiquid));
      layers.append({std::move(liquidBaseImage), imageScale});
    } else {
      if (baseCount > 0) {
        String baseLayer = strf("{}?hueshift={}", baseImages.replace("<biome>",
            terrestrialParameters->primaryBiome).replace("<num>", toString(baseCount)), terrestrialParameters->hueShift);
        layers.append({std::move(baseLayer), imageScale});
      }
    }

    // Then draw all the biome layers
    for (int i = 0; i < baseCount; ++i) {
      String baseImage = baseImages.replace("<num>", toString(baseCount - i));
      String hueShiftString, dynamicMaskString;
      if (!dynamicsImages.empty())
        dynamicMaskString = "?addmask=" + dynamicsImages.replace("<num>", toString(celestialParameters.randomizeParameterRange(gfxConfig.getArray("dynamicsRange"), i).toInt()));
      if (terrestrialParameters->hueShift != 0)
        hueShiftString = strf("?hueshift={}", terrestrialParameters->hueShift);
      String layer = baseImage + hueShiftString + dynamicMaskString;
      layers.append({std::move(layer), imageScale});
    }

    if (!shadowImages.empty()) {
      String shadow = shadowImages.replace("<num>", toString(shadowParameters.randomizeParameterRange(gfxConfig.getArray("shadowNumber")).toInt()));
      layers.append({std::move(shadow), imageScale});
    }

  } else if (type == "Asteroids") {
    String maskImages = celestialParameters.getParameter("maskImages").toString();
    int maskCount = celestialParameters.getParameter("masks").toInt();
    String dynamicsImages = celestialParameters.getParameter("dynamicsImages").toString();
    float imageScale = celestialParameters.getParameter("imageScale", 1.0f).toFloat();

    for (int i = 0; i < maskCount; ++i) {
      String biomeMaskBase = maskImages.replace("<num>", toString(maskCount - i));
      String dynamicMask = dynamicsImages.replace("<num>", toString(celestialParameters.randomizeParameterRange("dynamicsRange", i).toInt()));
      String layer = strf("{}?addmask={}", biomeMaskBase, dynamicMask);
      layers.append({std::move(layer), imageScale});
    }

  } else if (type == "FloatingDungeon") {
    String image = celestialParameters.getParameter("image").toString();
    float imageScale = celestialParameters.getParameter("imageScale", 1.0f).toFloat();
    layers.append({std::move(image), imageScale});

    if (!celestialParameters.getParameter("dynamicsImages").toString().empty()) {
      String dynamicsImages = celestialParameters.getParameter("dynamicsImages", "").toString();
      String dynamicsImage = dynamicsImages.replace("<num>", toString(celestialParameters.randomizeParameterRange("dynamicsRange").toInt()));
      layers.append({std::move(dynamicsImage), imageScale});
    }

  } else if (type == "GasGiant") {
    auto gfxConfig = assets->json("/celestial.config:gasGiantGraphics");

    auto baseImage = gfxConfig.getString("baseImage", "");
    auto shadowImages = gfxConfig.getString("shadowImages", "");
    auto dynamicsImages = gfxConfig.getString("dynamicsImages", "");
    auto overlayImages = gfxConfig.getString("overlayImages", "");
    auto overlayCount = gfxConfig.getInt("overlayCount", 0);
    float imageScale = celestialParameters.getParameter("imageScale", 1.0f).toFloat();

    float hueShift = celestialParameters.randomizeParameterRange(gfxConfig.getArray("primaryHueShiftRange")).toFloat();
    if (!baseImage.empty())
      layers.append({strf("{}?hueshift={}", baseImage, hueShift), imageScale});

    if (!overlayImages.empty()) {
      for (int i = 0; i < overlayCount; ++i) {
        hueShift += celestialParameters.randomizeParameterRange(gfxConfig.getArray("hueShiftOffsetRange")).toFloat();
        String maskImage = dynamicsImages.replace("<num>", toString(celestialParameters.randomizeParameterRange(gfxConfig.getArray("dynamicsRange"), i).toInt()));
        String overlayImage = overlayImages.replace("<num>", toString(i));
        layers.append({strf("{}?hueshift={}?addmask={}", overlayImage, hueShift, maskImage), imageScale});
      }
    }

    if (!shadowImages.empty()) {
      String shadow = shadowImages.replace("<num>", toString(shadowParameters.randomizeParameterRange(gfxConfig.getArray("shadowNumber")).toInt()));
      layers.append({std::move(shadow), imageScale});
    }
  }

  return layers;
}

List<std::pair<String, String>> CelestialGraphics::worldHorizonImages(Root& root, CelestialParameters const& celestialParameters) {
  auto assets = root.assets();
  auto liquidsDatabase = root.liquidsDatabase();

  auto getLR = [](String const& base) -> std::pair<String, String> {
    return std::pair<String, String>(base.replace("<selector>", "l"), base.replace("<selector>", "r"));
  };

  String type = celestialParameters.getParameter("worldType").toString();

  List<std::pair<String, String>> res;

  if (type == "Terrestrial") {
    auto terrestrialParameters = as<TerrestrialWorldParameters>(celestialParameters.visitableParameters());
    if (!terrestrialParameters)
      return {};

    auto gfxConfig = jsonMerge(assets->json("/celestial.config:terrestrialHorizonGraphics").get("default"),
        assets->json("/celestial.config:terrestrialHorizonGraphics").get(terrestrialParameters->typeName, JsonObject()));

    String baseImages = gfxConfig.getString("baseImages");
    String atmoTextures = gfxConfig.getString("atmosphereTextures");
    String shadowTextures = gfxConfig.getString("shadowTextures");
    String maskTextures = gfxConfig.getString("maskTextures");
    String liquidTextures = gfxConfig.getString("liquidTextures");
    auto numMasks = jsonToVec2I(gfxConfig.get("maskRange"));
    auto maskPerPlanetRange = jsonToVec2I(gfxConfig.get("maskPerPlanetRange"));

    auto biomeHueShift = "?" + imageOperationToString(HueShiftImageOperation::hueShiftDegrees(terrestrialParameters->hueShift));

    if (terrestrialParameters->primarySurfaceLiquid != EmptyLiquidId) {
      auto seed = celestialParameters.seed();
      RandomSource rand(seed);

      int numPlanetMasks = rand.randInt(maskPerPlanetRange[0], maskPerPlanetRange[1]);
      List<int> masks;
      for (int i = 0; i < numPlanetMasks; ++i)
        masks.append(rand.randInt(numMasks[0], numMasks[1]));

      String liquidBase = liquidTextures.replace("<liquid>", liquidsDatabase->liquidName(terrestrialParameters->primarySurfaceLiquid));
      res.append(getLR(liquidBase));

      StringList planetMaskListL;
      StringList planetMaskListR;
      for (auto m : masks) {
        String base = maskTextures.replace("<mask>", toString(m));
        auto lr = getLR(base);
        planetMaskListL.append(lr.first);
        planetMaskListR.append(lr.second);
      }

      String leftMask, rightMask;
      if (!planetMaskListL.empty())
        leftMask = "?" + imageOperationToString(AlphaMaskImageOperation{AlphaMaskImageOperation::Additive, planetMaskListL, {0, 0}});
      if (!planetMaskListR.empty())
        rightMask = "?" + imageOperationToString(AlphaMaskImageOperation{AlphaMaskImageOperation::Additive, planetMaskListR, {0, 0}});

      auto toAppend = getLR(baseImages + biomeHueShift);
      res.append({toAppend.first + leftMask, toAppend.second + rightMask});
    } else {
      res.append(getLR(baseImages + biomeHueShift));
    }

    if (celestialParameters.getParameter("atmosphere", true).toBool())
      res.append(getLR(atmoTextures));

    res.append(getLR(shadowTextures));

  } else if (type == "Asteroids") {
    res.append(getLR(assets->json("/celestial.config:asteroidsHorizons").toString()));

  } else if (type == "FloatingDungeon") {
    auto dungeonParameters = as<FloatingDungeonWorldParameters>(celestialParameters.visitableParameters());
    auto dungeonHorizons = assets->json("/celestial.config:floatingDungeonHorizons");
    if (dungeonHorizons.contains(dungeonParameters->primaryDungeon))
      res.append(getLR(dungeonHorizons.get(dungeonParameters->primaryDungeon).toString()));
  }

  return res;
}

int CelestialGraphics::worldRadialPosition(Root& root, CelestialParameters const& parameters) {
  if (parameters.coordinate().isPlanetaryBody())
    return staticRandomU32(parameters.seed(), "RadialNumber") % planetRadialPositions(root);
  if (parameters.coordinate().isSatelliteBody())
    return staticRandomU32(parameters.seed(), "RadialNumber") % satelliteRadialPositions(root);
  return 0;
}

int CelestialGraphics::planetRadialPositions(Root& root) {
  return root.assets()->json("/celestial.config:planetRadialSlots").toInt();
}

int CelestialGraphics::satelliteRadialPositions(Root& root) {
  return root.assets()->json("/celestial.config:satelliteRadialSlots").toInt();
}

List<std::pair<String, float>> CelestialGraphics::drawSystemTwinkle(Root& root, std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& system, double time) {
  auto parameters = celestialDatabase->parameters(system);
  if (!parameters)
    return {};

  auto assets = root.assets();

  int twinkleFrameCount = assets->json("/celestial.config:twinkleFrames").toInt();
  float twinkleScale = assets->json("/celestial.config:twinkleScale").toFloat();
  String twinkleFrameset = parameters->getParameter("twinkleFrames").toString();
  float twinkleTime = parameters->randomizeParameterRange("twinkleTime").toFloat();
  String twinkleBackground = parameters->getParameter("twinkleBackground").toString();

  String twinkleFrame = strf("{}:{}", twinkleFrameset, (int)(std::fmod<double>(time / twinkleTime, 1.0f) * twinkleFrameCount));

  return {{std::move(twinkleBackground), 1.0f}, {std::move(twinkleFrame), twinkleScale}};
}

List<std::pair<String, float>> CelestialGraphics::drawSystemPlanetaryObject(std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate) {
  if (auto params = celestialDatabase->parameters(coordinate))
    return drawSystemPlanetaryObject(takeOptional(params));
  return {};
}

List<std::pair<String, float>> CelestialGraphics::drawSystemCentralBody(std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate) {
  if (auto params = celestialDatabase->parameters(coordinate))
    return drawSystemCentralBody(takeOptional(params));
  return {};
}

List<std::pair<String, float>> CelestialGraphics::drawWorld(Root& root, std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate) {
  auto params = celestialDatabase->parameters(coordinate);
  if (!params)
    return {};

  if (coordinate.isSatelliteBody())
    return drawWorld(root, takeOptional(params), celestialDatabase->parameters(coordinate.parent()));
  else
    return drawWorld(root, takeOptional(params), {});
}

List<std::pair<String, String>> CelestialGraphics::worldHorizonImages(Root& root, std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate) {
  if (auto params = celestialDatabase->parameters(coordinate))
    return worldHorizonImages(root, takeOptional(params));
  return {};
}

int CelestialGraphics::worldRadialPosition(Root& root, std::shared_ptr<CelestialDatabase> celestialDatabase, CelestialCoordinate const& coordinate) {
  if (auto params = celestialDatabase->parameters(coordinate))
    return worldRadialPosition(root, takeOptional(params));
  return 0;
}

}
