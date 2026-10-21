import std;
import star.json;
import star.json_extra;
import star.bi_map;
import star.random;
import star.perlin;
import star.weighted_pool;
import star.id_map;
import star.set;
import star.casting;
import star.data_stream;
import star.game_types;
import star.material_types;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.strong_typedef;
import star.color;

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
import star.parallax;
import star.ambient;
import star.root;
import star.assets;

import star.algorithm;
namespace Star {

BiomePlaceables::BiomePlaceables() {
  grassMod = NoModId;
  grassModDensity = 0.0f;
  ceilingGrassMod = NoModId;
  ceilingGrassModDensity = 0.0f;
}

BiomePlaceables::BiomePlaceables(Assets const& assets, Json const& variant) {
  grassMod = variant.getInt("grassMod");
  grassModDensity = variant.getFloat("grassModDensity");
  ceilingGrassMod = variant.getInt("ceilingGrassMod");
  ceilingGrassModDensity = variant.getFloat("ceilingGrassModDensity");
  itemDistributions = variant.getArray("itemDistributions").transformed([&assets](Json const& item) {
    return BiomeItemDistribution(assets, item);
  });
}

Json BiomePlaceables::toJson() const {
  return JsonObject{
    {"grassMod", grassMod},
    {"grassModDensity", grassModDensity},
    {"ceilingGrassMod", ceilingGrassMod},
    {"ceilingGrassModDensity", ceilingGrassModDensity},
    {"itemDistributions", itemDistributions.transformed(std::mem_fn(&BiomeItemDistribution::toJson))}
  };
}

std::optional<TreeVariant> BiomePlaceables::firstTreeType() const {
  for (auto const& itemDistribution : itemDistributions) {
    for (auto const& biomeItem : itemDistribution.allItems()) {
      if (auto treePair = std::get_if<TreePair>(&biomeItem))
        return treePair->first;
    }
  }
  return {};
}

Biome::Biome() {
  mainBlock = EmptyMaterialId;
  hueShift = 0.0f;
  materialHueShift = MaterialHue();
}

Biome::Biome(Root& root, Json const& store) : Biome() {
  baseName = store.getString("baseName");
  description = store.getString("description");

  mainBlock = store.getUInt("mainBlock");
  subBlocks = store.getArray("subBlocks").transformed([](Json const& v) -> MaterialId { return v.toUInt(); });
  ores =
      store.getArray("ores").transformed([](Json const& v) { return std::pair<ModId, float>(v.getUInt(0), v.getFloat(1)); });
  hueShift = store.getFloat("hueShift");
  materialHueShift = store.getUInt("materialHueShift");

  auto assets = root.assets();
  surfacePlaceables = BiomePlaceables(*assets, store.get("surfacePlaceables"));
  undergroundPlaceables = BiomePlaceables(*assets, store.get("undergroundPlaceables"));

  if (auto config = store.opt("spawnProfile"))
    spawnProfile = SpawnProfile(*config);

  if (auto config = store.opt("parallax"))
    parallax = std::make_shared<Parallax>(root, *config);

  if (auto config = store.opt("ambientNoises"))
    ambientNoises = std::make_shared<AmbientNoisesDescription>(*config);
  if (auto config = store.opt("musicTrack"))
    musicTrack = std::make_shared<AmbientNoisesDescription>(*config);
}

Json Biome::toJson() const {
  return JsonObject{{"baseName", baseName},
      {"description", description},
      {"mainBlock", mainBlock},
      {"subBlocks", subBlocks.transformed(construct<Json>())},
      {"ores",
          ores.transformed([](std::pair<ModId, float> const& p) -> Json {
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
