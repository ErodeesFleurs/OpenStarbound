#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarXXHash.hpp"
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
#include "StarWeightedPool.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarString.hpp"
#include "StarOrderedSet.hpp"
#include "StarVersion.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarListener.hpp"
#include "StarLruCache.hpp"
#include "StarPerlin.hpp"
#include "StarIdMap.hpp"
#include "StarSet.hpp"
#include "StarNetElementSystem.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarStrongTypedef.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarMathCommon.hpp"
#include "StarArray.hpp"
#include "StarRpcPromise.hpp"
#include "StarSectorArray2D.hpp"
#include <functional>
#include "StarNetCompatibility.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarMap.hpp"
#include "StarRandom.hpp"
#include "StarBlockAllocator.hpp"

#include "StarLuaRoot.hpp"
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
import star.worker_pool;
import star.tile_sector_array;
import star.tile_modification;
import star.world;
import star.liquid_types;
import star.collision_generator;
import star.world_tiles;
import star.chat_types;
import star.uuid;
import star.warping;
import star.wiring;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.cellular_light_array;
import star.cellular_lighting;
import star.cellular_liquid;
import star.world_structure;
#include "StarLuaComponents.hpp"
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.world_render_data;
import star.world_client_state;
import star.interpolation_tracker;
import star.spawn_type_database;
import star.spawner;
import star.world_server;


import star.celestial_database;

using namespace Star;

int main(int argc, char** argv) {
  try {
    RootLoader rootLoader({{}, {}, {}, LogLevel::Error, false, {}});
    rootLoader.addParameter("coordinate", "coordinate", OptionParser::Optional, "world coordinate to test");
    rootLoader.addParameter("regions", "regions", OptionParser::Optional, "number of regions to generate, default 1000");
    rootLoader.addParameter("regionsize", "size", OptionParser::Optional, "width / height of each generation region, default 10");
    rootLoader.addParameter("reportevery", "report regions", OptionParser::Optional, "number of generation regions before each progress report, default 20");

    RootUPtr root;
    OptionParser::Options options;
    tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    coutf("Fully loading root...");
    root->fullyLoad();
    coutf(" done\n");

    CelestialMasterDatabase celestialDatabase;

    CelestialCoordinate coordinate;
    if (auto coordinateOption = options.parameters.maybe("coordinate")) {
      coordinate = CelestialCoordinate(coordinateOption->first());
    } else {
      coordinate = celestialDatabase.findRandomWorld(100, 50, [&](CelestialCoordinate const& coord) {
          return celestialDatabase.parameters(coord)->isVisitable();
        }).take();
    }

    unsigned regionsToGenerate = 1000;
    if (auto regionsOption = options.parameters.maybe("regions"))
      regionsToGenerate = lexicalCast<unsigned>(regionsOption->first());

    unsigned regionSize = 10;
    if (auto regionSizeOption = options.parameters.maybe("regionsize"))
      regionSize = lexicalCast<unsigned>(regionSizeOption->first());

    unsigned reportEvery = 20;
    if (auto reportEveryOption = options.parameters.maybe("reportevery"))
      reportEvery = lexicalCast<unsigned>(reportEveryOption->first());

    coutf("testing generation on coordinate {}\n", coordinate);

    auto worldParameters = celestialDatabase.parameters(coordinate).take();
    auto worldTemplate = make_shared<WorldTemplate>(worldParameters.visitableParameters(), SkyParameters(), worldParameters.seed());

    auto rand = RandomSource(worldTemplate->worldSeed());

    WorldServer worldServer(std::move(worldTemplate), File::ephemeralFile());
    Vec2U worldSize = worldServer.geometry().size();

    double start = Time::monotonicTime();
    double lastReport = Time::monotonicTime();

    coutf("Starting world generation for {} regions\n", regionsToGenerate);

    for (unsigned i = 0; i < regionsToGenerate; ++i) {
      if (i != 0 && i % reportEvery == 0) {
        float gps = reportEvery / (Time::monotonicTime() - lastReport);
        lastReport = Time::monotonicTime();
        coutf("[{}] {}s | Generatons Per Second: {}\n", i, Time::monotonicTime() - start, gps);
      }

      RectI region = RectI::withCenter(Vec2I(rand.randInt(0, worldSize[0]), rand.randInt(0, worldSize[1])), Vec2I::filled(regionSize));
      worldServer.generateRegion(region);
    }

    coutf("Finished generating {} regions with size {}x{} in world '{}' in {} seconds", regionsToGenerate, regionSize, regionSize, coordinate, Time::monotonicTime() - start);

    return 0;

  } catch (std::exception const& e) {
    cerrf("Exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
