//! Liquid types for fluid simulation in the world.
//!
//! Compatible with C++ Star::LiquidTypes from StarLiquidTypes.hpp

use crate::serialization::{DataReader, DataWriter, Readable, Writable};
use crate::error::Result;
use std::io::{Read, Write};

/// Liquid identifier (u8).
pub type LiquidId = u8;

/// Empty liquid ID constant.
pub const EMPTY_LIQUID_ID: LiquidId = 0;

/// Represents the level of liquid in a tile.
#[derive(Debug, Clone, Copy, PartialEq)]
#[repr(C)]
pub struct LiquidLevel {
    /// The type of liquid.
    pub liquid: LiquidId,
    /// The level of liquid (0.0 to 1.0).
    pub level: f32,
}

impl Default for LiquidLevel {
    fn default() -> Self {
        Self {
            liquid: EMPTY_LIQUID_ID,
            level: 0.0,
        }
    }
}

impl LiquidLevel {
    /// Create a new liquid level.
    pub fn new(liquid: LiquidId, level: f32) -> Self {
        Self { liquid, level }
    }

    /// Create an empty liquid level.
    pub fn empty() -> Self {
        Self::default()
    }

    /// Check if this is empty (no liquid).
    pub fn is_empty(&self) -> bool {
        self.liquid == EMPTY_LIQUID_ID || self.level <= 0.0
    }

    /// Take a specified amount of liquid, returning what was taken.
    pub fn take(&mut self, amount: f32) -> LiquidLevel {
        let taken = amount.min(self.level);
        self.level -= taken;
        
        let result = LiquidLevel {
            liquid: self.liquid,
            level: taken,
        };
        
        if self.level <= 0.0 {
            self.liquid = EMPTY_LIQUID_ID;
            self.level = 0.0;
        }
        
        result
    }
}

/// Network update for liquid (compressed format).
#[derive(Debug, Clone, Copy, PartialEq)]
#[repr(C)]
pub struct LiquidNetUpdate {
    /// The type of liquid.
    pub liquid: LiquidId,
    /// The level as a byte (0-255 maps to 0.0-1.0).
    pub level: u8,
}

impl Default for LiquidNetUpdate {
    fn default() -> Self {
        Self {
            liquid: EMPTY_LIQUID_ID,
            level: 0,
        }
    }
}

impl LiquidNetUpdate {
    /// Create a new liquid net update.
    pub fn new(liquid: LiquidId, level: u8) -> Self {
        Self { liquid, level }
    }

    /// Convert to a LiquidLevel.
    pub fn to_liquid_level(&self) -> LiquidLevel {
        LiquidLevel {
            liquid: self.liquid,
            level: byte_to_float(self.level),
        }
    }

    /// Create from a LiquidLevel.
    pub fn from_liquid_level(level: &LiquidLevel) -> Self {
        Self {
            liquid: level.liquid,
            level: float_to_byte(level.level),
        }
    }
}

/// Stored liquid with pressure information.
#[derive(Debug, Clone, Copy, PartialEq)]
#[repr(C)]
pub struct LiquidStore {
    /// The type of liquid.
    pub liquid: LiquidId,
    /// The level of liquid (0.0 to 1.0).
    pub level: f32,
    /// The pressure of the liquid.
    pub pressure: f32,
    /// Whether this is a source (endless) block.
    pub source: bool,
}

impl Default for LiquidStore {
    fn default() -> Self {
        Self {
            liquid: EMPTY_LIQUID_ID,
            level: 0.0,
            pressure: 0.0,
            source: false,
        }
    }
}

impl LiquidStore {
    /// Create a new liquid store.
    pub fn new(liquid: LiquidId, level: f32, pressure: f32, source: bool) -> Self {
        Self {
            liquid,
            level,
            pressure,
            source,
        }
    }

    /// Create a filled liquid store.
    pub fn filled(liquid: LiquidId, level: f32, pressure: Option<f32>) -> Self {
        Self {
            liquid,
            level,
            pressure: pressure.unwrap_or(0.0),
            source: false,
        }
    }

    /// Create an endless (source) liquid store.
    pub fn endless(liquid: LiquidId, pressure: f32) -> Self {
        Self {
            liquid,
            level: 1.0,
            pressure,
            source: true,
        }
    }

    /// Get the liquid level.
    pub fn liquid_level(&self) -> LiquidLevel {
        LiquidLevel {
            liquid: self.liquid,
            level: self.level,
        }
    }

    /// Get the network update representation.
    pub fn net_update(&self) -> LiquidNetUpdate {
        LiquidNetUpdate {
            liquid: self.liquid,
            level: float_to_byte(self.level),
        }
    }

    /// Update the liquid with new values.
    /// Returns Some(update) if the level changed enough to notify.
    pub fn update(&mut self, liquid: LiquidId, level: f32, pressure: f32) -> Option<LiquidNetUpdate> {
        let old_net = self.net_update();
        
        self.liquid = liquid;
        self.level = level;
        self.pressure = pressure;
        
        let new_net = self.net_update();
        
        if old_net.liquid != new_net.liquid || old_net.level != new_net.level {
            Some(new_net)
        } else {
            None
        }
    }

    /// Take a specified amount of liquid, returning what was taken.
    pub fn take(&mut self, amount: f32) -> LiquidLevel {
        if self.source {
            // Source blocks don't lose liquid
            return LiquidLevel {
                liquid: self.liquid,
                level: amount.min(self.level),
            };
        }
        
        let taken = amount.min(self.level);
        self.level -= taken;
        
        let result = LiquidLevel {
            liquid: self.liquid,
            level: taken,
        };
        
        if self.level <= 0.0 {
            self.liquid = EMPTY_LIQUID_ID;
            self.level = 0.0;
            self.pressure = 0.0;
        }
        
        result
    }

    /// Check if this is empty (no liquid).
    pub fn is_empty(&self) -> bool {
        self.liquid == EMPTY_LIQUID_ID || self.level <= 0.0
    }
}

// Helper functions for byte/float conversion

/// Convert a byte (0-255) to a float (0.0-1.0).
#[inline]
pub fn byte_to_float(byte: u8) -> f32 {
    byte as f32 / 255.0
}

/// Convert a float (0.0-1.0) to a byte (0-255).
#[inline]
pub fn float_to_byte(float: f32) -> u8 {
    (float.clamp(0.0, 1.0) * 255.0) as u8
}

// Serialization implementations

impl Readable for LiquidLevel {
    fn read<R: Read>(reader: &mut DataReader<R>) -> Result<Self> {
        Ok(Self {
            liquid: reader.read_u8()?,
            level: reader.read_f32()?,
        })
    }
}

impl Writable for LiquidLevel {
    fn write<W: Write>(&self, writer: &mut DataWriter<W>) -> Result<()> {
        writer.write_u8(self.liquid)?;
        writer.write_f32(self.level)?;
        Ok(())
    }
}

impl Readable for LiquidNetUpdate {
    fn read<R: Read>(reader: &mut DataReader<R>) -> Result<Self> {
        Ok(Self {
            liquid: reader.read_u8()?,
            level: reader.read_u8()?,
        })
    }
}

impl Writable for LiquidNetUpdate {
    fn write<W: Write>(&self, writer: &mut DataWriter<W>) -> Result<()> {
        writer.write_u8(self.liquid)?;
        writer.write_u8(self.level)?;
        Ok(())
    }
}

impl Readable for LiquidStore {
    fn read<R: Read>(reader: &mut DataReader<R>) -> Result<Self> {
        Ok(Self {
            liquid: reader.read_u8()?,
            level: reader.read_f32()?,
            pressure: reader.read_f32()?,
            source: reader.read_bool()?,
        })
    }
}

impl Writable for LiquidStore {
    fn write<W: Write>(&self, writer: &mut DataWriter<W>) -> Result<()> {
        writer.write_u8(self.liquid)?;
        writer.write_f32(self.level)?;
        writer.write_f32(self.pressure)?;
        writer.write_bool(self.source)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_liquid_level_default() {
        let level = LiquidLevel::default();
        assert_eq!(level.liquid, EMPTY_LIQUID_ID);
        assert_eq!(level.level, 0.0);
        assert!(level.is_empty());
    }

    #[test]
    fn test_liquid_level_take() {
        let mut level = LiquidLevel::new(1, 0.8);
        let taken = level.take(0.3);
        
        assert_eq!(taken.liquid, 1);
        assert!((taken.level - 0.3).abs() < 0.001);
        assert!((level.level - 0.5).abs() < 0.001);
    }

    #[test]
    fn test_liquid_level_take_all() {
        let mut level = LiquidLevel::new(1, 0.5);
        let taken = level.take(1.0);
        
        assert_eq!(taken.liquid, 1);
        assert!((taken.level - 0.5).abs() < 0.001);
        assert!(level.is_empty());
    }

    #[test]
    fn test_liquid_net_update_conversion() {
        let level = LiquidLevel::new(5, 0.75);
        let update = LiquidNetUpdate::from_liquid_level(&level);
        
        assert_eq!(update.liquid, 5);
        assert_eq!(update.level, 191); // 0.75 * 255 ≈ 191
        
        let back = update.to_liquid_level();
        assert_eq!(back.liquid, 5);
        assert!((back.level - 0.749).abs() < 0.01);
    }

    #[test]
    fn test_liquid_store_filled() {
        let store = LiquidStore::filled(2, 0.9, Some(1.5));
        assert_eq!(store.liquid, 2);
        assert!((store.level - 0.9).abs() < 0.001);
        assert!((store.pressure - 1.5).abs() < 0.001);
        assert!(!store.source);
    }

    #[test]
    fn test_liquid_store_endless() {
        let store = LiquidStore::endless(3, 2.0);
        assert_eq!(store.liquid, 3);
        assert_eq!(store.level, 1.0);
        assert!((store.pressure - 2.0).abs() < 0.001);
        assert!(store.source);
    }

    #[test]
    fn test_liquid_store_source_take() {
        let mut store = LiquidStore::endless(1, 1.0);
        let taken = store.take(0.5);
        
        // Source doesn't lose liquid
        assert_eq!(store.level, 1.0);
        assert!((taken.level - 0.5).abs() < 0.001);
    }

    #[test]
    fn test_byte_float_conversion() {
        assert_eq!(byte_to_float(0), 0.0);
        assert!((byte_to_float(127) - 0.498).abs() < 0.01);
        assert!((byte_to_float(255) - 1.0).abs() < 0.001);
        
        assert_eq!(float_to_byte(0.0), 0);
        assert_eq!(float_to_byte(0.5), 127);
        assert_eq!(float_to_byte(1.0), 255);
    }

    #[test]
    fn test_liquid_level_serialization() {
        let original = LiquidLevel::new(5, 0.75);
        
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            original.write(&mut writer).unwrap();
        }
        
        let mut reader = DataReader::new(std::io::Cursor::new(buf));
        let read: LiquidLevel = reader.read().unwrap();
        
        assert_eq!(read.liquid, original.liquid);
        assert!((read.level - original.level).abs() < 0.001);
    }

    #[test]
    fn test_liquid_store_serialization() {
        let original = LiquidStore::new(3, 0.8, 1.5, true);
        
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            original.write(&mut writer).unwrap();
        }
        
        let mut reader = DataReader::new(std::io::Cursor::new(buf));
        let read: LiquidStore = reader.read().unwrap();
        
        assert_eq!(read.liquid, original.liquid);
        assert!((read.level - original.level).abs() < 0.001);
        assert!((read.pressure - original.pressure).abs() < 0.001);
        assert_eq!(read.source, original.source);
    }
}
