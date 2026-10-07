#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.perlin;
import star.weighted_pool;
#include "StarBiMap.hpp"
#include "StarIdMap.hpp"
#include "StarSet.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarStrongTypedef.hpp"
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarBiMap.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.spawn_type_database;
import star.biome;
import star.tile_damage;
import star.plant_database;
import star.parallax;
import star.ambient;

namespace Star {

BiomePlaceables::BiomePlaceables() {
  grassMod = NoModId;
  grassModDensity = 0.0f;
  ceilingGrassMod = NoModId;
  ceilingGrassModDensity = 0.0f;
}

BiomePlaceables::BiomePlaceables(Json const& variant) {
  grassMod = variant.getInt("grassMod");
  grassModDensity = variant.getFloat("grassModDensity");
  ceilingGrassMod = variant.getInt("ceilingGrassMod");
  ceilingGrassModDensity = variant.getFloat("ceilingGrassModDensity");
  itemDistributions = variant.getArray("itemDistributions").transformed(construct<BiomeItemDistribution>());
}

Json BiomePlaceables::toJson() const {
  return JsonObject{
    {"grassMod", grassMod},
    {"grassModDensity", grassModDensity},
    {"ceilingGrassMod", ceilingGrassMod},
    {"ceilingGrassModDensity", ceilingGrassModDensity},
    {"itemDistributions", itemDistributions.transformed(mem_fn(&BiomeItemDistribution::toJson))}
  };
}

Maybe<TreeVariant> BiomePlaceables::firstTreeType() const {
  for (auto const& itemDistribution : itemDistributions) {
    for (auto const& biomeItem : itemDistribution.allItems()) {
      if (biomeItem.is<TreePair>())
        return biomeItem.get<TreePair>().first;
    }
  }
  return {};
}

Biome::Biome() {
  mainBlock = EmptyMaterialId;
  hueShift = 0.0f;
  materialHueShift = MaterialHue();
}

Biome::Biome(Json const& store) : Biome() {
  baseName = store.getString("baseName");
  description = store.getString("description");

  mainBlock = store.getUInt("mainBlock");
  subBlocks = store.getArray("subBlocks").transformed([](Json const& v) -> MaterialId { return v.toUInt(); });
  ores =
      store.getArray("ores").transformed([](Json const& v) { return pair<ModId, float>(v.getUInt(0), v.getFloat(1)); });
  hueShift = store.getFloat("hueShift");
  materialHueShift = store.getUInt("materialHueShift");

  surfacePlaceables = BiomePlaceables(store.get("surfacePlaceables"));
  undergroundPlaceables = BiomePlaceables(store.get("undergroundPlaceables"));

  if (auto config = store.opt("spawnProfile"))
    spawnProfile = SpawnProfile(*config);

  if (auto config = store.opt("parallax"))
    parallax = make_shared<Parallax>(*config);

  if (auto config = store.opt("ambientNoises"))
    ambientNoises = make_shared<AmbientNoisesDescription>(*config);
  if (auto config = store.opt("musicTrack"))
    musicTrack = make_shared<AmbientNoisesDescription>(*config);
}

Json Biome::toJson() const {
  return JsonObject{{"baseName", baseName},
      {"description", description},
      {"mainBlock", mainBlock},
      {"subBlocks", subBlocks.transformed(construct<Json>())},
      {"ores",
          ores.transformed([](pair<ModId, float> const& p) -> Json {
            return JsonArray{p.first, p.second};
          })},
      {"hueShift", hueShift},
      {"materialHueShift", materialHueShift},
      {"surfacePlaceables", surfacePlaceables.toJson()},
      {"undergroundPlaceables", undergroundPlaceables.toJson()},
      {"spawnProfile", spawnProfile.toJson()},
      {"parallax", parallax ? parallax->store() : Json()},
      {"ambientNoises", ambientNoises ? ambientNoises->toJson() : Json()},
      {"musicTrack", musicTrack ? musicTrack->toJson() : Json()}};
}

}
