//! World tile types compatible with C++ Star::WorldTiles
//!
//! This module provides tile structures for world terrain data.

use crate::error::Result;
use crate::serialization::{DataReader, DataWriter, Readable, Writable};
use crate::types::collision::{CollisionBlock, CollisionKind, CollisionSet};
use crate::types::game_types::{DungeonId, TileLayer, NO_DUNGEON_ID};
use crate::types::liquid_types::{LiquidId, LiquidLevel, LiquidNetUpdate, LiquidStore};
use crate::types::material_types::{
    MaterialColorVariant, MaterialHue, MaterialId, ModId, DEFAULT_MATERIAL_COLOR_VARIANT,
    NO_MOD_ID, NULL_MATERIAL_ID,
};
use crate::types::net_element::VersionNumber;
use crate::types::tile_damage::{TileDamageStatus, TileDamageType};

/// Biome index for block placement.
pub type BiomeIndex = u8;

/// Maximum number of collisions per space.
pub const MAX_COLLISIONS_PER_SPACE: usize = 8;

/// Base world tile structure.
///
/// Matches C++ `WorldTile` struct.
#[derive(Debug, Clone, PartialEq)]
#[repr(C)]
pub struct WorldTile {
    /// Foreground material ID
    pub foreground: MaterialId,
    /// Foreground hue shift
    pub foreground_hue_shift: MaterialHue,
    /// Foreground mod ID
    pub foreground_mod: ModId,
    /// Foreground mod hue shift
    pub foreground_mod_hue_shift: MaterialHue,
    /// Foreground color variant
    pub foreground_color_variant: MaterialColorVariant,

    /// Background material ID
    pub background: MaterialId,
    /// Background hue shift
    pub background_hue_shift: MaterialHue,
    /// Background mod ID
    pub background_mod: ModId,
    /// Background mod hue shift
    pub background_mod_hue_shift: MaterialHue,
    /// Background color variant
    pub background_color_variant: MaterialColorVariant,

    /// Collision kind for this tile
    pub collision: CollisionKind,

    /// Whether collision cache needs rebuild
    pub collision_cache_dirty: bool,
    /// Cached collision blocks
    pub collision_cache: Vec<CollisionBlock>,

    /// Block biome index
    pub block_biome_index: BiomeIndex,
    /// Environment biome index
    pub environment_biome_index: BiomeIndex,

    /// Whether this is a biome transition
    pub biome_transition: bool,

    /// Foreground damage status
    pub foreground_damage: TileDamageStatus,
    /// Background damage status
    pub background_damage: TileDamageStatus,

    /// Dungeon ID this block belongs to
    pub dungeon_id: DungeonId,
}

impl Default for WorldTile {
    fn default() -> Self {
        Self {
            foreground: NULL_MATERIAL_ID,
            foreground_hue_shift: MaterialHue::default(),
            foreground_mod: NO_MOD_ID,
            foreground_mod_hue_shift: MaterialHue::default(),
            foreground_color_variant: DEFAULT_MATERIAL_COLOR_VARIANT,
            background: NULL_MATERIAL_ID,
            background_hue_shift: MaterialHue::default(),
            background_mod: NO_MOD_ID,
            background_mod_hue_shift: MaterialHue::default(),
            background_color_variant: DEFAULT_MATERIAL_COLOR_VARIANT,
            collision: CollisionKind::Null,
            collision_cache_dirty: true,
            collision_cache: Vec::new(),
            block_biome_index: 0,
            environment_biome_index: 0,
            biome_transition: false,
            foreground_damage: TileDamageStatus::default(),
            background_damage: TileDamageStatus::default(),
            dungeon_id: NO_DUNGEON_ID,
        }
    }
}

impl WorldTile {
    /// Creates a new empty world tile.
    pub fn new() -> Self {
        Self::default()
    }

    /// Gets the material ID for a layer.
    pub fn material(&self, layer: TileLayer) -> MaterialId {
        match layer {
            TileLayer::Foreground => self.foreground,
            TileLayer::Background => self.background,
        }
    }

    /// Gets the mod ID for a layer.
    pub fn mod_id(&self, layer: TileLayer) -> ModId {
        match layer {
            TileLayer::Foreground => self.foreground_mod,
            TileLayer::Background => self.background_mod,
        }
    }

    /// Gets the material color variant for a layer.
    pub fn material_color(&self, layer: TileLayer) -> MaterialColorVariant {
        match layer {
            TileLayer::Foreground => self.foreground_color_variant,
            TileLayer::Background => self.background_color_variant,
        }
    }

    /// Gets the collision kind.
    pub fn get_collision(&self) -> CollisionKind {
        self.collision
    }

    /// Gets the material, hue, and color for a layer.
    pub fn material_and_color(&self, layer: TileLayer) -> (MaterialId, MaterialHue, MaterialColorVariant) {
        match layer {
            TileLayer::Foreground => (
                self.foreground,
                self.foreground_hue_shift,
                self.foreground_color_variant,
            ),
            TileLayer::Background => (
                self.background,
                self.background_hue_shift,
                self.background_color_variant,
            ),
        }
    }

    /// Checks if the layer is connectable to adjacent tiles.
    pub fn is_connectable(&self, layer: TileLayer, material_only: bool) -> bool {
        let mat = self.material(layer);
        if mat == NULL_MATERIAL_ID {
            return false;
        }
        if !material_only {
            return self.mod_id(layer) != NO_MOD_ID;
        }
        true
    }

    /// Checks if this tile is colliding with the given collision set.
    pub fn is_colliding(&self, collision_set: &CollisionSet) -> bool {
        collision_set.contains(self.collision)
    }
}

/// Server-side tile with additional data.
///
/// Matches C++ `ServerTile` struct.
#[derive(Debug, Clone, PartialEq)]
pub struct ServerTile {
    /// Base tile data
    pub base: WorldTile,
    /// Liquid storage
    pub liquid: LiquidStore,
    /// Root source for plants/objects
    pub root_source: Option<(i32, i32)>,
    /// Object collision (runtime calculated)
    pub object_collision: CollisionKind,
}

/// Current serialization version for server tiles.
pub const CURRENT_SERVER_TILE_VERSION: VersionNumber = 1;

impl Default for ServerTile {
    fn default() -> Self {
        Self {
            base: WorldTile::default(),
            liquid: LiquidStore::default(),
            root_source: None,
            object_collision: CollisionKind::None,
        }
    }
}

impl ServerTile {
    /// Creates a new empty server tile.
    pub fn new() -> Self {
        Self::default()
    }

    /// Checks if this tile is colliding with the given collision set.
    pub fn is_colliding(&self, collision_set: &CollisionSet) -> bool {
        collision_set.contains(self.get_collision())
    }

    /// Gets the effective collision kind.
    pub fn get_collision(&self) -> CollisionKind {
        // Object collision takes precedence if set
        if self.object_collision != CollisionKind::None {
            return crate::types::collision::max_collision(self.base.collision, self.object_collision);
        }
        self.base.collision
    }

    /// Updates the collision kind.
    pub fn update_collision(&mut self, kind: CollisionKind) -> bool {
        let old = self.base.collision;
        self.base.collision = kind;
        self.base.collision_cache_dirty = true;

        // If collision doesn't support liquid, destroy it
        if !kind.blocks_liquid() && !self.liquid.is_empty() {
            self.liquid.clear();
        }

        old != kind
    }

    /// Updates the object collision kind.
    pub fn update_object_collision(&mut self, kind: CollisionKind) -> bool {
        let old = self.object_collision;
        self.object_collision = kind;
        self.base.collision_cache_dirty = true;
        old != kind
    }

    /// Reads from a data stream.
    pub fn read(&mut self, reader: &mut DataReader, _version: VersionNumber) -> Result<()> {
        self.base.foreground = reader.read_u16()?;
        self.base.foreground_hue_shift = MaterialHue::from_raw(reader.read_u8()?);
        self.base.foreground_mod = reader.read_u16()?;
        self.base.foreground_mod_hue_shift = MaterialHue::from_raw(reader.read_u8()?);
        self.base.foreground_color_variant = reader.read_u8()?;

        self.base.background = reader.read_u16()?;
        self.base.background_hue_shift = MaterialHue::from_raw(reader.read_u8()?);
        self.base.background_mod = reader.read_u16()?;
        self.base.background_mod_hue_shift = MaterialHue::from_raw(reader.read_u8()?);
        self.base.background_color_variant = reader.read_u8()?;

        self.base.collision = CollisionKind::from_u8(reader.read_u8()?);

        self.base.block_biome_index = reader.read_u8()?;
        self.base.environment_biome_index = reader.read_u8()?;
        self.base.biome_transition = reader.read_bool()?;

        self.base.foreground_damage = TileDamageStatus::read(reader)?;
        self.base.background_damage = TileDamageStatus::read(reader)?;

        self.base.dungeon_id = reader.read_u16()?;

        self.liquid = LiquidStore::read(reader)?;

        let has_root = reader.read_bool()?;
        if has_root {
            let x = reader.read_i32()?;
            let y = reader.read_i32()?;
            self.root_source = Some((x, y));
        } else {
            self.root_source = None;
        }

        Ok(())
    }

    /// Writes to a data stream.
    pub fn write(&self, writer: &mut DataWriter) -> Result<()> {
        writer.write_u16(self.base.foreground)?;
        writer.write_u8(self.base.foreground_hue_shift.to_raw())?;
        writer.write_u16(self.base.foreground_mod)?;
        writer.write_u8(self.base.foreground_mod_hue_shift.to_raw())?;
        writer.write_u8(self.base.foreground_color_variant)?;

        writer.write_u16(self.base.background)?;
        writer.write_u8(self.base.background_hue_shift.to_raw())?;
        writer.write_u16(self.base.background_mod)?;
        writer.write_u8(self.base.background_mod_hue_shift.to_raw())?;
        writer.write_u8(self.base.background_color_variant)?;

        writer.write_u8(self.base.collision as u8)?;

        writer.write_u8(self.base.block_biome_index)?;
        writer.write_u8(self.base.environment_biome_index)?;
        writer.write_bool(self.base.biome_transition)?;

        self.base.foreground_damage.write(writer)?;
        self.base.background_damage.write(writer)?;

        writer.write_u16(self.base.dungeon_id)?;

        self.liquid.write(writer)?;

        if let Some((x, y)) = self.root_source {
            writer.write_bool(true)?;
            writer.write_i32(x)?;
            writer.write_i32(y)?;
        } else {
            writer.write_bool(false)?;
        }

        Ok(())
    }
}

/// Client-side tile with rendering-specific data.
///
/// Matches C++ `ClientTile` struct.
#[derive(Debug, Clone, PartialEq)]
pub struct ClientTile {
    /// Base tile data
    pub base: WorldTile,
    /// Whether background is light transparent
    pub background_light_transparent: bool,
    /// Whether foreground is light transparent
    pub foreground_light_transparent: bool,
    /// Liquid level
    pub liquid: LiquidLevel,
    /// Local gravity
    pub gravity: f32,
}

impl Default for ClientTile {
    fn default() -> Self {
        Self {
            base: WorldTile::default(),
            background_light_transparent: true,
            foreground_light_transparent: true,
            liquid: LiquidLevel::default(),
            gravity: 0.0,
        }
    }
}

impl ClientTile {
    /// Creates a new empty client tile.
    pub fn new() -> Self {
        Self::default()
    }
}

/// Network tile for client-server transfer.
///
/// Matches C++ `NetTile` struct.
#[derive(Debug, Clone, PartialEq, Default)]
#[repr(C)]
pub struct NetTile {
    pub background: MaterialId,
    pub background_hue_shift: MaterialHue,
    pub background_color_variant: MaterialColorVariant,
    pub background_mod: ModId,
    pub background_mod_hue_shift: MaterialHue,
    pub foreground: MaterialId,
    pub foreground_hue_shift: MaterialHue,
    pub foreground_color_variant: MaterialColorVariant,
    pub foreground_mod: ModId,
    pub foreground_mod_hue_shift: MaterialHue,
    pub collision: CollisionKind,
    pub block_biome_index: BiomeIndex,
    pub environment_biome_index: BiomeIndex,
    pub liquid: LiquidNetUpdate,
    pub dungeon_id: DungeonId,
}

impl NetTile {
    /// Creates a new empty net tile.
    pub fn new() -> Self {
        Self::default()
    }

    /// Creates a net tile from a world tile.
    pub fn from_world_tile(tile: &WorldTile, liquid: LiquidNetUpdate) -> Self {
        Self {
            background: tile.background,
            background_hue_shift: tile.background_hue_shift,
            background_color_variant: tile.background_color_variant,
            background_mod: tile.background_mod,
            background_mod_hue_shift: tile.background_mod_hue_shift,
            foreground: tile.foreground,
            foreground_hue_shift: tile.foreground_hue_shift,
            foreground_color_variant: tile.foreground_color_variant,
            foreground_mod: tile.foreground_mod,
            foreground_mod_hue_shift: tile.foreground_mod_hue_shift,
            collision: tile.collision,
            block_biome_index: tile.block_biome_index,
            environment_biome_index: tile.environment_biome_index,
            liquid,
            dungeon_id: tile.dungeon_id,
        }
    }
}

impl Readable for NetTile {
    fn read(reader: &mut DataReader) -> Result<Self> {
        Ok(Self {
            background: reader.read_u16()?,
            background_hue_shift: MaterialHue::from_raw(reader.read_u8()?),
            background_color_variant: reader.read_u8()?,
            background_mod: reader.read_u16()?,
            background_mod_hue_shift: MaterialHue::from_raw(reader.read_u8()?),
            foreground: reader.read_u16()?,
            foreground_hue_shift: MaterialHue::from_raw(reader.read_u8()?),
            foreground_color_variant: reader.read_u8()?,
            foreground_mod: reader.read_u16()?,
            foreground_mod_hue_shift: MaterialHue::from_raw(reader.read_u8()?),
            collision: CollisionKind::from_u8(reader.read_u8()?),
            block_biome_index: reader.read_u8()?,
            environment_biome_index: reader.read_u8()?,
            liquid: LiquidNetUpdate::read(reader)?,
            dungeon_id: reader.read_u16()?,
        })
    }
}

impl Writable for NetTile {
    fn write(&self, writer: &mut DataWriter) -> Result<()> {
        writer.write_u16(self.background)?;
        writer.write_u8(self.background_hue_shift.to_raw())?;
        writer.write_u8(self.background_color_variant)?;
        writer.write_u16(self.background_mod)?;
        writer.write_u8(self.background_mod_hue_shift.to_raw())?;
        writer.write_u16(self.foreground)?;
        writer.write_u8(self.foreground_hue_shift.to_raw())?;
        writer.write_u8(self.foreground_color_variant)?;
        writer.write_u16(self.foreground_mod)?;
        writer.write_u8(self.foreground_mod_hue_shift.to_raw())?;
        writer.write_u8(self.collision as u8)?;
        writer.write_u8(self.block_biome_index)?;
        writer.write_u8(self.environment_biome_index)?;
        self.liquid.write(writer)?;
        writer.write_u16(self.dungeon_id)
    }
}

/// Predicted tile state for client-side prediction.
///
/// Matches C++ `PredictedTile` struct.
#[derive(Debug, Clone, Default)]
pub struct PredictedTile {
    pub time: i64,
    pub background: Option<MaterialId>,
    pub background_hue_shift: Option<MaterialHue>,
    pub background_color_variant: Option<MaterialColorVariant>,
    pub background_mod: Option<ModId>,
    pub background_mod_hue_shift: Option<MaterialHue>,
    pub foreground: Option<MaterialId>,
    pub foreground_hue_shift: Option<MaterialHue>,
    pub foreground_color_variant: Option<MaterialColorVariant>,
    pub foreground_mod: Option<ModId>,
    pub foreground_mod_hue_shift: Option<MaterialHue>,
    pub liquid: Option<LiquidLevel>,
    pub collision: Option<CollisionKind>,
}

impl PredictedTile {
    /// Returns true if any predicted state is set.
    pub fn is_active(&self) -> bool {
        self.foreground.is_some()
            || self.foreground_mod.is_some()
            || self.foreground_hue_shift.is_some()
            || self.foreground_mod_hue_shift.is_some()
            || self.background.is_some()
            || self.background_mod.is_some()
            || self.background_hue_shift.is_some()
            || self.background_mod_hue_shift.is_some()
    }

    /// Applies predicted values to a tile.
    pub fn apply_to_world_tile(&self, tile: &mut WorldTile) {
        if let Some(fg) = self.foreground {
            tile.foreground = fg;
        }
        if let Some(fg_mod) = self.foreground_mod {
            tile.foreground_mod = fg_mod;
        }
        if let Some(hue) = self.foreground_hue_shift {
            tile.foreground_hue_shift = hue;
        }
        if let Some(hue) = self.foreground_mod_hue_shift {
            tile.foreground_mod_hue_shift = hue;
        }
        if let Some(bg) = self.background {
            tile.background = bg;
        }
        if let Some(bg_mod) = self.background_mod {
            tile.background_mod = bg_mod;
        }
        if let Some(hue) = self.background_hue_shift {
            tile.background_hue_shift = hue;
        }
        if let Some(hue) = self.background_mod_hue_shift {
            tile.background_mod_hue_shift = hue;
        }
    }
}

/// Render tile - minimal structure for rendering.
///
/// Matches C++ `RenderTile` struct. Layout matches C++ for fast hashing.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(C, packed)]
pub struct RenderTile {
    pub foreground: MaterialId,
    pub foreground_mod: ModId,
    pub background: MaterialId,
    pub background_mod: ModId,
    pub foreground_hue_shift: MaterialHue,
    pub foreground_mod_hue_shift: MaterialHue,
    pub foreground_color_variant: MaterialColorVariant,
    pub foreground_damage_type: TileDamageType,
    pub foreground_damage_level: u8,
    pub background_hue_shift: MaterialHue,
    pub background_mod_hue_shift: MaterialHue,
    pub background_color_variant: MaterialColorVariant,
    pub background_damage_type: TileDamageType,
    pub background_damage_level: u8,
    pub liquid_id: LiquidId,
    pub liquid_level: u8,
}

impl RenderTile {
    /// Creates a new empty render tile.
    pub fn new() -> Self {
        Self::default()
    }

    /// Creates a render tile from a world tile.
    pub fn from_world_tile(tile: &WorldTile, liquid_id: LiquidId, liquid_level: u8) -> Self {
        Self {
            foreground: tile.foreground,
            foreground_mod: tile.foreground_mod,
            background: tile.background,
            background_mod: tile.background_mod,
            foreground_hue_shift: tile.foreground_hue_shift,
            foreground_mod_hue_shift: tile.foreground_mod_hue_shift,
            foreground_color_variant: tile.foreground_color_variant,
            foreground_damage_type: tile.foreground_damage.damage_type(),
            foreground_damage_level: (tile.foreground_damage.damage_effect_percentage() * 255.0) as u8,
            background_hue_shift: tile.background_hue_shift,
            background_mod_hue_shift: tile.background_mod_hue_shift,
            background_color_variant: tile.background_color_variant,
            background_damage_type: tile.background_damage.damage_type(),
            background_damage_level: (tile.background_damage.damage_effect_percentage() * 255.0) as u8,
            liquid_id,
            liquid_level,
        }
    }

    /// Hashes terrain data into the provided hasher.
    pub fn hash_terrain<H: std::hash::Hasher>(&self, hasher: &mut H) {
        use std::hash::Hash;
        // Hash all terrain-relevant fields
        self.foreground.hash(hasher);
        self.foreground_mod.hash(hasher);
        self.background.hash(hasher);
        self.background_mod.hash(hasher);
        self.foreground_hue_shift.hash(hasher);
        self.foreground_mod_hue_shift.hash(hasher);
        self.foreground_color_variant.hash(hasher);
        self.foreground_damage_type.hash(hasher);
        self.foreground_damage_level.hash(hasher);
        self.background_hue_shift.hash(hasher);
        self.background_mod_hue_shift.hash(hasher);
        self.background_color_variant.hash(hasher);
        self.background_damage_type.hash(hasher);
        self.background_damage_level.hash(hasher);
    }

    /// Hashes liquid data into the provided hasher.
    pub fn hash_liquid<H: std::hash::Hasher>(&self, hasher: &mut H) {
        use std::hash::Hash;
        self.liquid_id.hash(hasher);
        self.liquid_level.hash(hasher);
    }
}

impl Readable for RenderTile {
    fn read(reader: &mut DataReader) -> Result<Self> {
        Ok(Self {
            foreground: reader.read_u16()?,
            foreground_mod: reader.read_u16()?,
            background: reader.read_u16()?,
            background_mod: reader.read_u16()?,
            foreground_hue_shift: MaterialHue::from_raw(reader.read_u8()?),
            foreground_mod_hue_shift: MaterialHue::from_raw(reader.read_u8()?),
            foreground_color_variant: reader.read_u8()?,
            foreground_damage_type: TileDamageType::read(reader)?,
            foreground_damage_level: reader.read_u8()?,
            background_hue_shift: MaterialHue::from_raw(reader.read_u8()?),
            background_mod_hue_shift: MaterialHue::from_raw(reader.read_u8()?),
            background_color_variant: reader.read_u8()?,
            background_damage_type: TileDamageType::read(reader)?,
            background_damage_level: reader.read_u8()?,
            liquid_id: reader.read_u8()?,
            liquid_level: reader.read_u8()?,
        })
    }
}

impl Writable for RenderTile {
    fn write(&self, writer: &mut DataWriter) -> Result<()> {
        writer.write_u16(self.foreground)?;
        writer.write_u16(self.foreground_mod)?;
        writer.write_u16(self.background)?;
        writer.write_u16(self.background_mod)?;
        writer.write_u8(self.foreground_hue_shift.to_raw())?;
        writer.write_u8(self.foreground_mod_hue_shift.to_raw())?;
        writer.write_u8(self.foreground_color_variant)?;
        self.foreground_damage_type.write(writer)?;
        writer.write_u8(self.foreground_damage_level)?;
        writer.write_u8(self.background_hue_shift.to_raw())?;
        writer.write_u8(self.background_mod_hue_shift.to_raw())?;
        writer.write_u8(self.background_color_variant)?;
        self.background_damage_type.write(writer)?;
        writer.write_u8(self.background_damage_level)?;
        writer.write_u8(self.liquid_id)?;
        writer.write_u8(self.liquid_level)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_world_tile_default() {
        let tile = WorldTile::new();
        assert_eq!(tile.foreground, NULL_MATERIAL_ID);
        assert_eq!(tile.background, NULL_MATERIAL_ID);
        assert_eq!(tile.collision, CollisionKind::Null);
        assert_eq!(tile.dungeon_id, NO_DUNGEON_ID);
    }

    #[test]
    fn test_world_tile_material_accessors() {
        let mut tile = WorldTile::new();
        tile.foreground = 100;
        tile.background = 200;

        assert_eq!(tile.material(TileLayer::Foreground), 100);
        assert_eq!(tile.material(TileLayer::Background), 200);
    }

    #[test]
    fn test_server_tile() {
        let tile = ServerTile::new();
        assert_eq!(tile.base.foreground, NULL_MATERIAL_ID);
        assert!(tile.liquid.is_empty());
        assert_eq!(tile.object_collision, CollisionKind::None);
    }

    #[test]
    fn test_server_tile_collision() {
        let mut tile = ServerTile::new();
        assert!(tile.update_collision(CollisionKind::Block));
        assert_eq!(tile.get_collision(), CollisionKind::Block);

        tile.update_object_collision(CollisionKind::Platform);
        // Block is "stronger" than Platform
        assert_eq!(tile.get_collision(), CollisionKind::Block);
    }

    #[test]
    fn test_client_tile() {
        let tile = ClientTile::new();
        assert!(tile.background_light_transparent);
        assert!(tile.foreground_light_transparent);
        assert_eq!(tile.gravity, 0.0);
    }

    #[test]
    fn test_net_tile_serialization() {
        let net_tile = NetTile {
            foreground: 100,
            foreground_hue_shift: MaterialHue::from_raw(50),
            foreground_color_variant: 1,
            foreground_mod: 10,
            foreground_mod_hue_shift: MaterialHue::default(),
            background: 200,
            background_hue_shift: MaterialHue::from_raw(25),
            background_color_variant: 2,
            background_mod: 20,
            background_mod_hue_shift: MaterialHue::default(),
            collision: CollisionKind::Block,
            block_biome_index: 1,
            environment_biome_index: 2,
            liquid: LiquidNetUpdate::default(),
            dungeon_id: 5,
        };

        let mut writer = DataWriter::new();
        net_tile.write(&mut writer).unwrap();

        let mut reader = DataReader::new(writer.data());
        let net_tile2 = NetTile::read(&mut reader).unwrap();

        assert_eq!(net_tile.foreground, net_tile2.foreground);
        assert_eq!(net_tile.background, net_tile2.background);
        assert_eq!(net_tile.collision, net_tile2.collision);
        assert_eq!(net_tile.dungeon_id, net_tile2.dungeon_id);
    }

    #[test]
    fn test_predicted_tile() {
        let mut predicted = PredictedTile::default();
        assert!(!predicted.is_active());

        predicted.foreground = Some(100);
        assert!(predicted.is_active());

        let mut tile = WorldTile::new();
        predicted.apply_to_world_tile(&mut tile);
        assert_eq!(tile.foreground, 100);
    }

    #[test]
    fn test_render_tile() {
        let tile = RenderTile::new();
        assert_eq!(tile.foreground, 0);
        assert_eq!(tile.liquid_level, 0);

        // Test serialization
        let mut writer = DataWriter::new();
        tile.write(&mut writer).unwrap();

        let mut reader = DataReader::new(writer.data());
        let tile2 = RenderTile::read(&mut reader).unwrap();
        assert_eq!(tile, tile2);
    }
}
