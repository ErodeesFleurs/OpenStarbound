//! Tile damage types compatible with C++ Star::TileDamage
//!
//! This module provides tile damage types for terrain damage and harvesting.

use crate::error::Result;
use crate::math::Vec2;
use crate::serialization::{DataReader, DataWriter, Readable, Writable};

/// Type of damage applied to tiles.
///
/// Matches C++ `TileDamageType` enum.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum TileDamageType {
    /// Damage that will not actually kill the target
    #[default]
    Protected = 0,
    /// Best at chopping down trees, things made of wood, etc.
    Plantish = 1,
    /// For digging / drilling through materials
    Blockish = 2,
    /// Gravity gun etc
    Beamish = 3,
    /// Penetrating damage done passively by explosions
    Explosive = 4,
    /// Can melt certain block types
    Fire = 5,
    /// Can "till" certain materials into others
    Tilling = 6,
}

impl TileDamageType {
    /// Returns whether this damage type is penetrating (doesn't require line of sight).
    pub fn is_penetrating(&self) -> bool {
        matches!(self, TileDamageType::Explosive)
    }

    /// Converts damage type from a string name.
    pub fn from_name(name: &str) -> Option<Self> {
        match name.to_lowercase().as_str() {
            "protected" => Some(TileDamageType::Protected),
            "plantish" => Some(TileDamageType::Plantish),
            "blockish" => Some(TileDamageType::Blockish),
            "beamish" => Some(TileDamageType::Beamish),
            "explosive" => Some(TileDamageType::Explosive),
            "fire" => Some(TileDamageType::Fire),
            "tilling" => Some(TileDamageType::Tilling),
            _ => None,
        }
    }

    /// Gets the name of this damage type.
    pub fn name(&self) -> &'static str {
        match self {
            TileDamageType::Protected => "Protected",
            TileDamageType::Plantish => "Plantish",
            TileDamageType::Blockish => "Blockish",
            TileDamageType::Beamish => "Beamish",
            TileDamageType::Explosive => "Explosive",
            TileDamageType::Fire => "Fire",
            TileDamageType::Tilling => "Tilling",
        }
    }
}

impl Readable for TileDamageType {
    fn read(reader: &mut DataReader) -> Result<Self> {
        let val = reader.read_u8()?;
        match val {
            0 => Ok(TileDamageType::Protected),
            1 => Ok(TileDamageType::Plantish),
            2 => Ok(TileDamageType::Blockish),
            3 => Ok(TileDamageType::Beamish),
            4 => Ok(TileDamageType::Explosive),
            5 => Ok(TileDamageType::Fire),
            6 => Ok(TileDamageType::Tilling),
            _ => Ok(TileDamageType::Protected), // Default to protected
        }
    }
}

impl Writable for TileDamageType {
    fn write(&self, writer: &mut DataWriter) -> Result<()> {
        writer.write_u8(*self as u8)
    }
}

/// Tile damage parameters for a single tile.
///
/// Matches C++ `TileDamage` struct.
#[derive(Debug, Clone, Copy, PartialEq, Default)]
#[repr(C)]
pub struct TileDamage {
    /// Type of damage
    pub damage_type: TileDamageType,
    /// Amount of damage
    pub amount: f32,
    /// Harvest level required
    pub harvest_level: u32,
}

impl TileDamage {
    /// Creates a new tile damage.
    pub fn new(damage_type: TileDamageType, amount: f32, harvest_level: u32) -> Self {
        Self {
            damage_type,
            amount,
            harvest_level,
        }
    }

    /// Creates protected damage (no actual damage).
    pub fn protected() -> Self {
        Self {
            damage_type: TileDamageType::Protected,
            amount: 0.0,
            harvest_level: 1,
        }
    }
}

impl Readable for TileDamage {
    fn read(reader: &mut DataReader) -> Result<Self> {
        let damage_type = TileDamageType::read(reader)?;
        let amount = reader.read_f32()?;
        let harvest_level = reader.read_var_u32()?;
        Ok(Self {
            damage_type,
            amount,
            harvest_level,
        })
    }
}

impl Writable for TileDamage {
    fn write(&self, writer: &mut DataWriter) -> Result<()> {
        self.damage_type.write(writer)?;
        writer.write_f32(self.amount)?;
        writer.write_var_u32(self.harvest_level)
    }
}

/// Damage parameters for tiles - defines how resistant a tile is to damage.
///
/// Matches C++ `TileDamageParameters` class.
#[derive(Debug, Clone)]
pub struct TileDamageParameters {
    /// Damage multipliers per type (type -> multiplier)
    damages: std::collections::HashMap<TileDamageType, f32>,
    /// Health recovered per second
    recovery_per_second: f32,
    /// Maximum time for damage effect display
    max_effect_time: f32,
    /// Total health of the tile
    total_health: f32,
    /// Required harvest level to damage
    required_harvest_level: u32,
}

impl Default for TileDamageParameters {
    fn default() -> Self {
        Self {
            damages: std::collections::HashMap::new(),
            recovery_per_second: 0.0,
            max_effect_time: 0.0,
            total_health: 1.0,
            required_harvest_level: 1,
        }
    }
}

impl TileDamageParameters {
    /// Creates new damage parameters with default values.
    pub fn new() -> Self {
        Self::default()
    }

    /// Creates damage parameters from config values.
    pub fn with_health(total_health: f32, required_harvest_level: u32) -> Self {
        Self {
            damages: std::collections::HashMap::new(),
            recovery_per_second: 0.0,
            max_effect_time: 1.0,
            total_health,
            required_harvest_level,
        }
    }

    /// Sets a damage multiplier for a specific damage type.
    pub fn set_damage(&mut self, damage_type: TileDamageType, multiplier: f32) {
        self.damages.insert(damage_type, multiplier);
    }

    /// Calculates the damage done by a damage source.
    pub fn damage_done(&self, damage: &TileDamage) -> f32 {
        if damage.harvest_level < self.required_harvest_level {
            return 0.0;
        }

        let multiplier = self.damages.get(&damage.damage_type).copied().unwrap_or(1.0);
        damage.amount * multiplier
    }

    /// Gets the recovery per second.
    pub fn recovery_per_second(&self) -> f32 {
        self.recovery_per_second
    }

    /// Sets the recovery per second.
    pub fn set_recovery_per_second(&mut self, recovery: f32) {
        self.recovery_per_second = recovery;
    }

    /// Gets the required harvest level.
    pub fn required_harvest_level(&self) -> u32 {
        self.required_harvest_level
    }

    /// Gets the maximum effect time.
    pub fn max_effect_time(&self) -> f32 {
        self.max_effect_time
    }

    /// Gets the total health.
    pub fn total_health(&self) -> f32 {
        self.total_health
    }

    /// Sums two damage parameters.
    pub fn sum(&self, other: &TileDamageParameters) -> TileDamageParameters {
        let mut result = self.clone();
        result.total_health += other.total_health;
        result.required_harvest_level = result.required_harvest_level.max(other.required_harvest_level);
        result
    }
}

impl Readable for TileDamageParameters {
    fn read(reader: &mut DataReader) -> Result<Self> {
        let num_damages = reader.read_var_u32()? as usize;
        let mut damages = std::collections::HashMap::new();
        for _ in 0..num_damages {
            let dt = TileDamageType::read(reader)?;
            let mult = reader.read_f32()?;
            damages.insert(dt, mult);
        }
        let recovery_per_second = reader.read_f32()?;
        let max_effect_time = reader.read_f32()?;
        let total_health = reader.read_f32()?;
        let required_harvest_level = reader.read_var_u32()?;
        Ok(Self {
            damages,
            recovery_per_second,
            max_effect_time,
            total_health,
            required_harvest_level,
        })
    }
}

impl Writable for TileDamageParameters {
    fn write(&self, writer: &mut DataWriter) -> Result<()> {
        writer.write_var_u32(self.damages.len() as u32)?;
        for (dt, mult) in &self.damages {
            dt.write(writer)?;
            writer.write_f32(*mult)?;
        }
        writer.write_f32(self.recovery_per_second)?;
        writer.write_f32(self.max_effect_time)?;
        writer.write_f32(self.total_health)?;
        writer.write_var_u32(self.required_harvest_level)
    }
}

/// Tracks the current damage status of a tile.
///
/// Matches C++ `TileDamageStatus` class.
#[derive(Debug, Clone, Copy, PartialEq, Default)]
#[repr(C)]
pub struct TileDamageStatus {
    /// Current damage as a percentage (0-1)
    damage_percentage: f32,
    /// Factor for calculating visual effect
    damage_effect_time_factor: f32,
    /// Whether tile was harvested (not destroyed)
    harvested: bool,
    /// Position of the damage source
    damage_source_position: Vec2<f32>,
    /// Type of damage applied
    damage_type: TileDamageType,
    /// Calculated effect percentage
    damage_effect_percentage: f32,
}

impl TileDamageStatus {
    /// Creates a new, healthy tile damage status.
    pub fn new() -> Self {
        Self::default()
    }

    /// Gets the current damage percentage.
    pub fn damage_percentage(&self) -> f32 {
        self.damage_percentage
    }

    /// Gets the damage effect percentage.
    pub fn damage_effect_percentage(&self) -> f32 {
        self.damage_effect_percentage
    }

    /// Gets the damage source position.
    pub fn source_position(&self) -> Vec2<f32> {
        self.damage_source_position
    }

    /// Gets the damage type.
    pub fn damage_type(&self) -> TileDamageType {
        self.damage_type
    }

    /// Resets the damage status.
    pub fn reset(&mut self) {
        *self = Self::default();
    }

    /// Applies damage to this tile.
    pub fn damage(
        &mut self,
        params: &TileDamageParameters,
        source_position: Vec2<f32>,
        damage: &TileDamage,
    ) {
        let done = params.damage_done(damage);
        if done <= 0.0 {
            return;
        }

        let health = params.total_health();
        if health <= 0.0 {
            return;
        }

        self.damage_percentage += done / health;
        self.damage_source_position = source_position;
        self.damage_type = damage.damage_type;
        self.damage_effect_time_factor = params.max_effect_time();
        self.update_damage_effect_percentage();

        if self.damage_percentage >= 1.0 {
            self.harvested = damage.damage_type != TileDamageType::Explosive;
        }
    }

    /// Recovers damage over time.
    pub fn recover(&mut self, params: &TileDamageParameters, dt: f32) {
        if self.dead() || self.damage_percentage <= 0.0 {
            return;
        }

        let health = params.total_health();
        if health <= 0.0 {
            return;
        }

        let recovery = (params.recovery_per_second() * dt) / health;
        self.damage_percentage = (self.damage_percentage - recovery).max(0.0);

        // Decay effect over time
        if self.damage_effect_time_factor > 0.0 {
            self.damage_effect_time_factor -= dt;
            self.update_damage_effect_percentage();
        }
    }

    /// Returns true if tile has no damage.
    pub fn healthy(&self) -> bool {
        self.damage_percentage <= 0.0
    }

    /// Returns true if tile has some damage but is not dead.
    pub fn damaged(&self) -> bool {
        self.damage_percentage > 0.0 && self.damage_percentage < 1.0
    }

    /// Returns true if tile was protected from damage.
    pub fn damage_protected(&self) -> bool {
        self.damage_type == TileDamageType::Protected
    }

    /// Returns true if tile is dead (damage >= 100%).
    pub fn dead(&self) -> bool {
        self.damage_percentage >= 1.0
    }

    /// Returns true if tile was harvested (vs destroyed).
    pub fn harvested(&self) -> bool {
        self.harvested
    }

    fn update_damage_effect_percentage(&mut self) {
        if self.damage_effect_time_factor > 0.0 {
            // Effect shows based on remaining time factor
            self.damage_effect_percentage = self.damage_percentage.min(1.0);
        } else {
            self.damage_effect_percentage = 0.0;
        }
    }
}

impl Readable for TileDamageStatus {
    fn read(reader: &mut DataReader) -> Result<Self> {
        let damage_percentage = reader.read_f32()?;
        let damage_effect_time_factor = reader.read_f32()?;
        let harvested = reader.read_bool()?;
        let x = reader.read_f32()?;
        let y = reader.read_f32()?;
        let damage_type = TileDamageType::read(reader)?;
        let damage_effect_percentage = reader.read_f32()?;
        Ok(Self {
            damage_percentage,
            damage_effect_time_factor,
            harvested,
            damage_source_position: Vec2::new(x, y),
            damage_type,
            damage_effect_percentage,
        })
    }
}

impl Writable for TileDamageStatus {
    fn write(&self, writer: &mut DataWriter) -> Result<()> {
        writer.write_f32(self.damage_percentage)?;
        writer.write_f32(self.damage_effect_time_factor)?;
        writer.write_bool(self.harvested)?;
        writer.write_f32(self.damage_source_position.x())?;
        writer.write_f32(self.damage_source_position.y())?;
        self.damage_type.write(writer)?;
        writer.write_f32(self.damage_effect_percentage)
    }
}

/// Generates a list of tile positions for an area brush.
pub fn tile_area_brush(range: f32, center_offset: Vec2<f32>, diameter_mode: bool) -> Vec<Vec2<i32>> {
    let mut result = Vec::new();
    let actual_range = if diameter_mode { range / 2.0 } else { range };
    let range_sq = actual_range * actual_range;
    let i_range = actual_range.ceil() as i32;

    for y in -i_range..=i_range {
        for x in -i_range..=i_range {
            let fx = x as f32 + center_offset.x();
            let fy = y as f32 + center_offset.y();
            if fx * fx + fy * fy <= range_sq {
                result.push(Vec2::new(x, y));
            }
        }
    }

    result
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tile_damage_type() {
        assert!(!TileDamageType::Blockish.is_penetrating());
        assert!(TileDamageType::Explosive.is_penetrating());

        assert_eq!(TileDamageType::from_name("blockish"), Some(TileDamageType::Blockish));
        assert_eq!(TileDamageType::from_name("EXPLOSIVE"), Some(TileDamageType::Explosive));
        assert_eq!(TileDamageType::from_name("invalid"), None);

        assert_eq!(TileDamageType::Plantish.name(), "Plantish");
    }

    #[test]
    fn test_tile_damage() {
        let damage = TileDamage::new(TileDamageType::Blockish, 10.0, 2);
        assert_eq!(damage.damage_type, TileDamageType::Blockish);
        assert_eq!(damage.amount, 10.0);
        assert_eq!(damage.harvest_level, 2);

        let protected = TileDamage::protected();
        assert_eq!(protected.damage_type, TileDamageType::Protected);
        assert_eq!(protected.amount, 0.0);
    }

    #[test]
    fn test_tile_damage_parameters() {
        let mut params = TileDamageParameters::with_health(100.0, 1);
        params.set_damage(TileDamageType::Blockish, 2.0);

        let blockish = TileDamage::new(TileDamageType::Blockish, 10.0, 1);
        assert_eq!(params.damage_done(&blockish), 20.0); // 10 * 2

        let low_level = TileDamage::new(TileDamageType::Blockish, 10.0, 0);
        assert_eq!(params.damage_done(&low_level), 0.0); // Harvest level too low
    }

    #[test]
    fn test_tile_damage_status() {
        let params = TileDamageParameters::with_health(100.0, 1);
        let mut status = TileDamageStatus::new();

        assert!(status.healthy());
        assert!(!status.damaged());
        assert!(!status.dead());

        let damage = TileDamage::new(TileDamageType::Blockish, 50.0, 1);
        status.damage(&params, Vec2::new(0.0, 0.0), &damage);

        assert!(!status.healthy());
        assert!(status.damaged());
        assert!(!status.dead());
        assert_eq!(status.damage_percentage(), 0.5);

        let damage2 = TileDamage::new(TileDamageType::Blockish, 60.0, 1);
        status.damage(&params, Vec2::new(0.0, 0.0), &damage2);

        assert!(status.dead());
        assert!(status.harvested());
    }

    #[test]
    fn test_tile_area_brush() {
        let brush = tile_area_brush(1.5, Vec2::new(0.0, 0.0), false);
        assert!(brush.contains(&Vec2::new(0, 0)));
        assert!(brush.contains(&Vec2::new(1, 0)));
        assert!(!brush.contains(&Vec2::new(2, 0)));
    }

    #[test]
    fn test_serialization() {
        // Test TileDamageType
        let dt = TileDamageType::Blockish;
        let mut writer = DataWriter::new();
        dt.write(&mut writer).unwrap();
        let mut reader = DataReader::new(writer.data());
        let dt2 = TileDamageType::read(&mut reader).unwrap();
        assert_eq!(dt, dt2);

        // Test TileDamage
        let damage = TileDamage::new(TileDamageType::Fire, 25.5, 3);
        let mut writer = DataWriter::new();
        damage.write(&mut writer).unwrap();
        let mut reader = DataReader::new(writer.data());
        let damage2 = TileDamage::read(&mut reader).unwrap();
        assert_eq!(damage.damage_type, damage2.damage_type);
        assert_eq!(damage.amount, damage2.amount);
        assert_eq!(damage.harvest_level, damage2.harvest_level);

        // Test TileDamageStatus
        let params = TileDamageParameters::with_health(100.0, 1);
        let mut status = TileDamageStatus::new();
        status.damage(&params, Vec2::new(1.5, 2.5), &TileDamage::new(TileDamageType::Blockish, 30.0, 1));

        let mut writer = DataWriter::new();
        status.write(&mut writer).unwrap();
        let mut reader = DataReader::new(writer.data());
        let status2 = TileDamageStatus::read(&mut reader).unwrap();
        assert_eq!(status.damage_percentage(), status2.damage_percentage());
        assert_eq!(status.damage_type(), status2.damage_type());
    }
}
