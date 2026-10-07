#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarEither.hpp"
#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarFile.hpp"
#include "StarLexicalCast.hpp"
#include "StarImage.hpp"
#include "StarString.hpp"
#include "StarOrderedSet.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarBlockAllocator.hpp"
import star.lru_cache;
#include "StarInterpolation.hpp"
import star.perlin;
#include "StarIdMap.hpp"
#include "StarSet.hpp"
#include "StarNetElementSystem.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarStrongTypedef.hpp"
#include "StarList.hpp"

import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;
import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;
import star.world_layout;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_damage;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.sky_parameters;
import star.world_template;


import star.celestial_database;

using namespace Star;

int main(int argc, char** argv) {
  try {
    RootLoader rootLoader({{}, {}, {}, LogLevel::Error, false, {}});

    rootLoader.setSummary("Generate a WorldTemplate and output the data in it to an image");

    rootLoader.addParameter("coordinate", "coordinate", OptionParser::Optional, "coordinate for the celestial world");
    rootLoader.addParameter("coordseed", "seed", OptionParser::Optional, "seed to use when selecting a random celestial world coordinate");
    rootLoader.addParameter("size", "size", OptionParser::Optional, "x,y size of the region to be rendered");
    rootLoader.addSwitch("weighting", "Output instead the region weighting at each point");
    rootLoader.addSwitch("weightingblocknoise", "apply layout block noise before outputting weighting");
    rootLoader.addSwitch("transition", "show biome transition regions");

    RootUPtr root;
    OptionParser::Options options;
    tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    CelestialMasterDatabasePtr celestialDatabase = make_shared<CelestialMasterDatabase>();

    Maybe<CelestialCoordinate> coordinate;
    if (!options.parameters["coordinate"].empty())
      coordinate = CelestialCoordinate(options.parameters["coordinate"].first());
    else if (!options.parameters["coordseed"].empty())
      coordinate = celestialDatabase->findRandomWorld(
          10, 50, {}, lexicalCast<uint64_t>(options.parameters["coordseed"].first()));
    else
      coordinate = celestialDatabase->findRandomWorld();

    if (!coordinate)
      throw StarException("Could not find world to generate, try again");

    coutf("Generating world with coordinate {}\n", *coordinate);

    WorldTemplate worldTemplate(*coordinate, celestialDatabase);
    auto size = worldTemplate.size();

    if (!options.parameters["size"].empty()) {
      auto regionSize = Vec2U(lexicalCast<unsigned>(options.parameters["size"].first().split(",")[0]),
          lexicalCast<unsigned>(options.parameters["size"].first().split(",")[1]));
      size = regionSize.piecewiseClamp(Vec2U(0, 0), size);
    } else if (size[0] > 1000) {
      size[0] = 1000;
    }

    coutf("Generating {} size image for world of type '{}'\n", size, worldTemplate.worldParameters()->typeName);
    auto outputImage = make_shared<Image>(size, PixelFormat::RGB24);

    Color groundColor = Color::rgb(255, 0, 0);
    Color caveColor = Color::rgb(128, 0, 0);
    Color blankColor = Color::rgb(0, 0, 0);

    for (size_t x = 0; x < size[0]; ++x) {
      for (size_t y = 0; y < size[1]; ++y) {
        if (options.switches.contains("weighting")) {
          auto layout = worldTemplate.worldLayout();
          Color color = Color::Black;
          Vec2I pos(x, y);
          if (options.switches.contains("weightingblocknoise")) {
            if (auto blockNoise = layout->blockNoise())
              pos = blockNoise->apply(pos, size);
          }
          auto weightings = layout->getWeighting(pos[0], pos[1]);
          for (auto const& weighting : weightings) {
            Color mixColor = Color::rgb(128, 0, 0);
            mixColor.setHue(staticRandomFloat((uint64_t)weighting.region));
            color = Color::rgbaf(color.toRgbaF() + mixColor.toRgbaF() * weighting.weight);
          }
          outputImage->set(x, y, color.toRgb());
        } else if (options.switches.contains("transition")) {
          auto blockInfo = worldTemplate.blockInfo(x, y);
          if (isRealMaterial(blockInfo.foreground)) {
            Color color = groundColor;
            color.setHue(blockInfo.biomeTransition ? 0 : 0.5f);
            outputImage->set(x, y, color.toRgb());
          } else if (isRealMaterial(blockInfo.background)) {
            Color color = caveColor;
            color.setHue(blockInfo.biomeTransition ? 0 : 0.5f);
            outputImage->set(x, y, color.toRgb());
          } else {
            outputImage->set(x, y, blankColor.toRgb());
          }
        } else {
          // Image y = 0 is the top, so reverse it for the world tile
          auto blockInfo = worldTemplate.blockInfo(x, y);
          if (isRealMaterial(blockInfo.foreground)) {
            Color color = groundColor;
            color.setHue(staticRandomFloat(blockInfo.foreground));
            color.setSaturation(staticRandomFloat(blockInfo.foregroundMod));
            outputImage->set(x, y, color.toRgb());
          } else if (isRealMaterial(blockInfo.background)) {
            Color color = caveColor;
            color.setHue(staticRandomFloat(blockInfo.background));
            color.setSaturation(staticRandomFloat(blockInfo.backgroundMod));
            outputImage->set(x, y, color.toRgb());
          } else {
            outputImage->set(x, y, blankColor.toRgb());
          }
        }
      }
    }

    outputImage->writePng(File::open("mapgen.png", IOMode::Write));
    return 0;
  } catch (std::exception const& e) {
    cerrf("exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
