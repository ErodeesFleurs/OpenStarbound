//! Biome types compatible with C++ Star::Biome
//!
//! This module provides biome-related types for world generation.

use crate::types::material_types::{MaterialHue, MaterialId, ModId, NO_MOD_ID, NULL_MATERIAL_ID};
use serde::{Deserialize, Serialize};

/// Biome item distribution type.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct BiomeItemDistribution {
    /// Distribution mode
    #[serde(default)]
    pub mode: String,
    /// Item type
    #[serde(default)]
    pub item_type: String,
    /// Distribution parameters
    #[serde(default)]
    pub parameters: serde_json::Value,
}

impl Default for BiomeItemDistribution {
    fn default() -> Self {
        Self {
            mode: "distribution".to_string(),
            item_type: String::new(),
            parameters: serde_json::Value::Null,
        }
    }
}

/// Tree variant for biome generation.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct TreeVariant {
    /// Tree stem name
    #[serde(default)]
    pub stem_name: String,
    /// Tree foliage name
    #[serde(default)]
    pub foliage_name: String,
    /// Stem hue shift
    #[serde(default)]
    pub stem_hue_shift: f32,
    /// Foliage hue shift
    #[serde(default)]
    pub foliage_hue_shift: f32,
}

impl Default for TreeVariant {
    fn default() -> Self {
        Self {
            stem_name: String::new(),
            foliage_name: String::new(),
            stem_hue_shift: 0.0,
            foliage_hue_shift: 0.0,
        }
    }
}

/// Biome placeable items configuration.
///
/// Matches C++ `BiomePlaceables` struct.
#[derive(Debug, Clone, PartialEq, Default, Serialize, Deserialize)]
pub struct BiomePlaceables {
    /// Grass mod for surface
    #[serde(default)]
    pub grass_mod: ModId,
    /// Grass mod density
    #[serde(default)]
    pub grass_mod_density: f32,
    /// Ceiling grass mod
    #[serde(default)]
    pub ceiling_grass_mod: ModId,
    /// Ceiling grass mod density
    #[serde(default)]
    pub ceiling_grass_mod_density: f32,
    /// Item distributions
    #[serde(default)]
    pub item_distributions: Vec<BiomeItemDistribution>,
}

impl BiomePlaceables {
    /// Creates new empty placeables.
    pub fn new() -> Self {
        Self::default()
    }

    /// Creates placeables from JSON.
    pub fn from_json(json: &serde_json::Value) -> Option<Self> {
        serde_json::from_value(json.clone()).ok()
    }

    /// Converts to JSON.
    pub fn to_json(&self) -> serde_json::Value {
        serde_json::to_value(self).unwrap_or(serde_json::Value::Null)
    }

    /// Gets the first tree type if any distributions contain trees.
    pub fn first_tree_type(&self) -> Option<TreeVariant> {
        for dist in &self.item_distributions {
            if dist.item_type == "tree" || dist.mode == "tree" {
                if let Ok(tree) = serde_json::from_value::<TreeVariant>(dist.parameters.clone()) {
                    return Some(tree);
                }
            }
        }
        None
    }
}

/// Ambient noise description.
#[derive(Debug, Clone, PartialEq, Default, Serialize, Deserialize)]
pub struct AmbientNoisesDescription {
    /// Day noises
    #[serde(default)]
    pub day: Vec<String>,
    /// Night noises
    #[serde(default)]
    pub night: Vec<String>,
}

impl AmbientNoisesDescription {
    /// Creates new empty ambient noises.
    pub fn new() -> Self {
        Self::default()
    }

    /// Creates from JSON.
    pub fn from_json(json: &serde_json::Value) -> Option<Self> {
        serde_json::from_value(json.clone()).ok()
    }

    /// Converts to JSON.
    pub fn to_json(&self) -> serde_json::Value {
        serde_json::to_value(self).unwrap_or(serde_json::Value::Null)
    }
}

/// Spawn profile for biome spawning.
#[derive(Debug, Clone, PartialEq, Default, Serialize, Deserialize)]
pub struct SpawnProfile {
    /// Monster spawn groups
    #[serde(default)]
    pub groups: Vec<SpawnGroup>,
}

/// Spawn group definition.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct SpawnGroup {
    /// Spawn pool name
    #[serde(default)]
    pub pool: String,
    /// Spawn rate
    #[serde(default = "default_spawn_rate")]
    pub rate: f32,
}

fn default_spawn_rate() -> f32 {
    1.0
}

impl Default for SpawnGroup {
    fn default() -> Self {
        Self {
            pool: String::new(),
            rate: 1.0,
        }
    }
}

/// Complete biome definition.
///
/// Matches C++ `Biome` struct.
#[derive(Debug, Clone, PartialEq, Default, Serialize, Deserialize)]
pub struct Biome {
    /// Base biome name
    #[serde(default)]
    pub base_name: String,
    /// Description
    #[serde(default)]
    pub description: String,

    /// Main block material
    #[serde(default)]
    pub main_block: MaterialId,
    /// Sub-block materials
    #[serde(default)]
    pub sub_blocks: Vec<MaterialId>,
    /// Ores with commonality multipliers
    #[serde(default)]
    pub ores: Vec<(ModId, f32)>,

    /// Hue shift for biome
    #[serde(default)]
    pub hue_shift: f32,
    /// Material hue shift
    #[serde(default)]
    pub material_hue_shift: MaterialHue,

    /// Surface placeables
    #[serde(default)]
    pub surface_placeables: BiomePlaceables,
    /// Underground placeables
    #[serde(default)]
    pub underground_placeables: BiomePlaceables,

    /// Spawn profile
    #[serde(default)]
    pub spawn_profile: SpawnProfile,

    /// Parallax configuration name
    #[serde(default)]
    pub parallax: Option<String>,

    /// Ambient noises
    #[serde(default)]
    pub ambient_noises: Option<AmbientNoisesDescription>,
    /// Music track
    #[serde(default)]
    pub music_track: Option<AmbientNoisesDescription>,
}

impl Biome {
    /// Creates a new empty biome.
    pub fn new() -> Self {
        Self::default()
    }

    /// Creates a biome from JSON.
    pub fn from_json(json: &serde_json::Value) -> Option<Self> {
        serde_json::from_value(json.clone()).ok()
    }

    /// Converts to JSON.
    pub fn to_json(&self) -> serde_json::Value {
        serde_json::to_value(self).unwrap_or(serde_json::Value::Null)
    }

    /// Gets the first tree type from surface placeables.
    pub fn first_tree_type(&self) -> Option<TreeVariant> {
        self.surface_placeables.first_tree_type()
    }
}

/// Biome database entry.
#[derive(Debug, Clone)]
pub struct BiomeEntry {
    /// Biome name
    pub name: String,
    /// Biome data
    pub biome: Biome,
    /// Source path
    pub source_path: String,
}

/// Biome database for looking up biomes.
#[derive(Debug, Clone, Default)]
pub struct BiomeDatabase {
    biomes: std::collections::HashMap<String, BiomeEntry>,
}

impl BiomeDatabase {
    /// Creates a new empty database.
    pub fn new() -> Self {
        Self::default()
    }

    /// Adds a biome to the database.
    pub fn add(&mut self, name: String, biome: Biome, source_path: String) {
        self.biomes.insert(
            name.clone(),
            BiomeEntry {
                name,
                biome,
                source_path,
            },
        );
    }

    /// Gets a biome by name.
    pub fn get(&self, name: &str) -> Option<&BiomeEntry> {
        self.biomes.get(name)
    }

    /// Gets all biome names.
    pub fn names(&self) -> impl Iterator<Item = &str> {
        self.biomes.keys().map(|s| s.as_str())
    }

    /// Returns the number of biomes.
    pub fn len(&self) -> usize {
        self.biomes.len()
    }

    /// Returns true if empty.
    pub fn is_empty(&self) -> bool {
        self.biomes.is_empty()
    }
}

/// Biome placement parameters.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct BiomePlacement {
    /// Priority for placement
    #[serde(default)]
    pub priority: i32,
    /// Minimum depth
    #[serde(default)]
    pub min_depth: f32,
    /// Maximum depth
    #[serde(default = "default_max_depth")]
    pub max_depth: f32,
    /// Required layers
    #[serde(default)]
    pub layers: Vec<String>,
}

fn default_max_depth() -> f32 {
    f32::INFINITY
}

impl Default for BiomePlacement {
    fn default() -> Self {
        Self {
            priority: 0,
            min_depth: 0.0,
            max_depth: f32::INFINITY,
            layers: Vec::new(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_biome_placeables() {
        let placeables = BiomePlaceables::new();
        assert_eq!(placeables.grass_mod, NO_MOD_ID);
        assert_eq!(placeables.grass_mod_density, 0.0);
        assert!(placeables.item_distributions.is_empty());
    }

    #[test]
    fn test_biome_placeables_json() {
        let json = serde_json::json!({
            "grass_mod": 10,
            "grass_mod_density": 0.5,
            "item_distributions": []
        });

        let placeables = BiomePlaceables::from_json(&json).unwrap();
        assert_eq!(placeables.grass_mod, 10);
        assert_eq!(placeables.grass_mod_density, 0.5);

        let back = placeables.to_json();
        assert_eq!(back["grass_mod"], 10);
    }

    #[test]
    fn test_biome() {
        let mut biome = Biome::new();
        biome.base_name = "forest".to_string();
        biome.main_block = 100;
        biome.sub_blocks.push(101);
        biome.sub_blocks.push(102);

        assert_eq!(biome.base_name, "forest");
        assert_eq!(biome.main_block, 100);
        assert_eq!(biome.sub_blocks.len(), 2);
    }

    #[test]
    fn test_biome_json() {
        let json = serde_json::json!({
            "base_name": "desert",
            "description": "A hot desert biome",
            "main_block": 200,
            "hue_shift": 30.0
        });

        let biome = Biome::from_json(&json).unwrap();
        assert_eq!(biome.base_name, "desert");
        assert_eq!(biome.description, "A hot desert biome");
        assert_eq!(biome.main_block, 200);
        assert_eq!(biome.hue_shift, 30.0);
    }

    #[test]
    fn test_ambient_noises() {
        let noises = AmbientNoisesDescription {
            day: vec!["birds.ogg".to_string()],
            night: vec!["crickets.ogg".to_string()],
        };

        let json = noises.to_json();
        assert_eq!(json["day"][0], "birds.ogg");
        assert_eq!(json["night"][0], "crickets.ogg");
    }

    #[test]
    fn test_spawn_profile() {
        let profile = SpawnProfile {
            groups: vec![SpawnGroup {
                pool: "surface_monsters".to_string(),
                rate: 0.5,
            }],
        };

        assert_eq!(profile.groups.len(), 1);
        assert_eq!(profile.groups[0].pool, "surface_monsters");
        assert_eq!(profile.groups[0].rate, 0.5);
    }

    #[test]
    fn test_biome_database() {
        let mut db = BiomeDatabase::new();
        assert!(db.is_empty());

        let biome = Biome {
            base_name: "forest".to_string(),
            main_block: 100,
            ..Default::default()
        };

        db.add("forest".to_string(), biome, "/biomes/forest.biome".to_string());

        assert_eq!(db.len(), 1);
        assert!(!db.is_empty());

        let entry = db.get("forest").unwrap();
        assert_eq!(entry.name, "forest");
        assert_eq!(entry.biome.main_block, 100);
        assert_eq!(entry.source_path, "/biomes/forest.biome");
    }

    #[test]
    fn test_tree_variant() {
        let tree = TreeVariant {
            stem_name: "oak".to_string(),
            foliage_name: "oakleaves".to_string(),
            stem_hue_shift: 0.0,
            foliage_hue_shift: 10.0,
        };

        assert_eq!(tree.stem_name, "oak");
        assert_eq!(tree.foliage_name, "oakleaves");
        assert_eq!(tree.foliage_hue_shift, 10.0);
    }

    #[test]
    fn test_biome_placement() {
        let placement = BiomePlacement::default();
        assert_eq!(placement.priority, 0);
        assert_eq!(placement.min_depth, 0.0);
        assert!(placement.max_depth.is_infinite());
    }
}
