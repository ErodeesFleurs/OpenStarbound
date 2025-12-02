//! Collision types for physics and world interaction.
//!
//! Compatible with C++ Star::CollisionBlock from StarCollisionBlock.hpp

use crate::math::{Vec2F, Vec2I, RectF};
use crate::serialization::{DataReader, DataWriter, Readable, Writable};
use crate::error::Result;
use std::io::{Read, Write};
use std::fmt;

/// Collision kind enum.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Default)]
#[repr(u8)]
pub enum CollisionKind {
    /// Unloaded or un-generated tiles. Collides like Block but doesn't tile with it.
    #[default]
    Null = 0,
    /// No collision.
    None = 1,
    /// Platform collision (one-way).
    Platform = 2,
    /// Dynamic collision (moving objects).
    Dynamic = 3,
    /// Slippery collision (ice-like).
    Slippery = 4,
    /// Solid block collision.
    Block = 5,
}

impl CollisionKind {
    /// Parse from string name.
    pub fn from_name(name: &str) -> Option<Self> {
        match name.to_lowercase().as_str() {
            "null" => Some(Self::Null),
            "none" => Some(Self::None),
            "platform" => Some(Self::Platform),
            "dynamic" => Some(Self::Dynamic),
            "slippery" => Some(Self::Slippery),
            "block" => Some(Self::Block),
            _ => None,
        }
    }

    /// Get the string name.
    pub fn name(&self) -> &'static str {
        match self {
            Self::Null => "Null",
            Self::None => "None",
            Self::Platform => "Platform",
            Self::Dynamic => "Dynamic",
            Self::Slippery => "Slippery",
            Self::Block => "Block",
        }
    }

    /// Check if this is a solid collision.
    pub fn is_solid(&self) -> bool {
        matches!(self, Self::Null | Self::Dynamic | Self::Slippery | Self::Block)
    }
}

impl fmt::Display for CollisionKind {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.name())
    }
}

/// Tile collision override.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum TileCollisionOverride {
    /// No override.
    #[default]
    None = 0,
    /// Override to empty (no collision).
    Empty = 1,
    /// Override to platform.
    Platform = 2,
    /// Override to block.
    Block = 3,
}

impl TileCollisionOverride {
    /// Parse from string name.
    pub fn from_name(name: &str) -> Option<Self> {
        match name.to_lowercase().as_str() {
            "none" => Some(Self::None),
            "empty" => Some(Self::Empty),
            "platform" => Some(Self::Platform),
            "block" => Some(Self::Block),
            _ => None,
        }
    }

    /// Get the string name.
    pub fn name(&self) -> &'static str {
        match self {
            Self::None => "None",
            Self::Empty => "Empty",
            Self::Platform => "Platform",
            Self::Block => "Block",
        }
    }

    /// Convert to collision kind.
    pub fn to_collision_kind(&self) -> CollisionKind {
        match self {
            Self::None => CollisionKind::Null,
            Self::Empty => CollisionKind::None,
            Self::Platform => CollisionKind::Platform,
            Self::Block => CollisionKind::Block,
        }
    }
}

/// A set of collision kinds represented as a bitfield.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
pub struct CollisionSet {
    kinds: u8,
}

impl CollisionSet {
    /// Create an empty collision set.
    pub fn new() -> Self {
        Self { kinds: 0 }
    }

    /// Create a collision set from a slice of collision kinds.
    pub fn from_kinds(kinds: &[CollisionKind]) -> Self {
        let mut set = Self::new();
        for kind in kinds {
            set.insert(*kind);
        }
        set
    }

    /// Create the default collision set (Null, Slippery, Dynamic, Block).
    pub fn default_set() -> Self {
        Self::from_kinds(&[
            CollisionKind::Null,
            CollisionKind::Slippery,
            CollisionKind::Dynamic,
            CollisionKind::Block,
        ])
    }

    /// Create the block collision set (Block, Slippery).
    pub fn block_set() -> Self {
        Self::from_kinds(&[CollisionKind::Block, CollisionKind::Slippery])
    }

    /// Insert a collision kind into the set.
    pub fn insert(&mut self, kind: CollisionKind) {
        self.kinds |= Self::kind_bit(kind);
    }

    /// Remove a collision kind from the set.
    pub fn remove(&mut self, kind: CollisionKind) {
        self.kinds &= !Self::kind_bit(kind);
    }

    /// Check if the set contains a collision kind.
    pub fn contains(&self, kind: CollisionKind) -> bool {
        (self.kinds & Self::kind_bit(kind)) != 0
    }

    /// Check if empty.
    pub fn is_empty(&self) -> bool {
        self.kinds == 0
    }

    fn kind_bit(kind: CollisionKind) -> u8 {
        1 << ((kind as u8) + 1)
    }
}

/// Check if a collision kind collides with a collision set.
#[inline]
pub fn is_colliding(kind: CollisionKind, set: &CollisionSet) -> bool {
    set.contains(kind)
}

/// Check if a collision kind is solid colliding (using default set).
#[inline]
pub fn is_solid_colliding(kind: CollisionKind) -> bool {
    is_colliding(kind, &CollisionSet::default_set())
}

/// Returns the highest priority collision kind.
/// Priority: Block > Slippery > Dynamic > Platform > None > Null
#[inline]
pub fn max_collision(first: CollisionKind, second: CollisionKind) -> CollisionKind {
    if first > second { first } else { second }
}

/// A collision block representing a collidable space.
#[derive(Debug, Clone, PartialEq)]
pub struct CollisionBlock {
    /// The kind of collision.
    pub kind: CollisionKind,
    /// The tile space this block occupies.
    pub space: Vec2I,
    /// The polygon shape of the collision.
    pub poly: Vec<Vec2F>,
    /// Bounding rectangle of the polygon.
    pub poly_bounds: RectF,
}

impl CollisionBlock {
    /// Create a new collision block.
    pub fn new(kind: CollisionKind, space: Vec2I, poly: Vec<Vec2F>, poly_bounds: RectF) -> Self {
        Self {
            kind,
            space,
            poly,
            poly_bounds,
        }
    }

    /// Create a null collision block for a tile space.
    pub fn null_block(space: Vec2I) -> Self {
        let space_f = Vec2F::new(space.x() as f32, space.y() as f32);
        Self {
            kind: CollisionKind::Null,
            space,
            poly: vec![
                space_f,
                Vec2F::new(space_f.x() + 1.0, space_f.y()),
                Vec2F::new(space_f.x() + 1.0, space_f.y() + 1.0),
                Vec2F::new(space_f.x(), space_f.y() + 1.0),
            ],
            poly_bounds: RectF::with_size(space_f, Vec2F::new(1.0, 1.0)),
        }
    }

    /// Create a standard block collision for a tile space.
    pub fn block(space: Vec2I) -> Self {
        let space_f = Vec2F::new(space.x() as f32, space.y() as f32);
        Self {
            kind: CollisionKind::Block,
            space,
            poly: vec![
                space_f,
                Vec2F::new(space_f.x() + 1.0, space_f.y()),
                Vec2F::new(space_f.x() + 1.0, space_f.y() + 1.0),
                Vec2F::new(space_f.x(), space_f.y() + 1.0),
            ],
            poly_bounds: RectF::with_size(space_f, Vec2F::new(1.0, 1.0)),
        }
    }

    /// Check if a point is inside the collision polygon.
    pub fn contains_point(&self, point: Vec2F) -> bool {
        // First check bounding box
        if !self.poly_bounds.contains_point(point, true) {
            return false;
        }

        // Simple point-in-polygon test (ray casting)
        let n = self.poly.len();
        if n < 3 {
            return false;
        }

        let mut inside = false;
        let mut j = n - 1;
        for i in 0..n {
            let pi = &self.poly[i];
            let pj = &self.poly[j];
            
            if ((pi.y() > point.y()) != (pj.y() > point.y()))
                && (point.x() < (pj.x() - pi.x()) * (point.y() - pi.y()) / (pj.y() - pi.y()) + pi.x())
            {
                inside = !inside;
            }
            j = i;
        }
        inside
    }
}

// Serialization implementations

impl Readable for CollisionKind {
    fn read<R: Read>(reader: &mut DataReader<R>) -> Result<Self> {
        let value = reader.read_u8()?;
        Ok(match value {
            0 => CollisionKind::Null,
            1 => CollisionKind::None,
            2 => CollisionKind::Platform,
            3 => CollisionKind::Dynamic,
            4 => CollisionKind::Slippery,
            5 => CollisionKind::Block,
            _ => CollisionKind::Null,
        })
    }
}

impl Writable for CollisionKind {
    fn write<W: Write>(&self, writer: &mut DataWriter<W>) -> Result<()> {
        writer.write_u8(*self as u8)
    }
}

impl Readable for TileCollisionOverride {
    fn read<R: Read>(reader: &mut DataReader<R>) -> Result<Self> {
        let value = reader.read_u8()?;
        Ok(match value {
            0 => TileCollisionOverride::None,
            1 => TileCollisionOverride::Empty,
            2 => TileCollisionOverride::Platform,
            3 => TileCollisionOverride::Block,
            _ => TileCollisionOverride::None,
        })
    }
}

impl Writable for TileCollisionOverride {
    fn write<W: Write>(&self, writer: &mut DataWriter<W>) -> Result<()> {
        writer.write_u8(*self as u8)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_collision_kind_ordering() {
        assert!(CollisionKind::Block > CollisionKind::Slippery);
        assert!(CollisionKind::Slippery > CollisionKind::Dynamic);
        assert!(CollisionKind::Dynamic > CollisionKind::Platform);
        assert!(CollisionKind::Platform > CollisionKind::None);
        assert!(CollisionKind::None > CollisionKind::Null);
    }

    #[test]
    fn test_collision_kind_names() {
        assert_eq!(CollisionKind::from_name("Block"), Some(CollisionKind::Block));
        assert_eq!(CollisionKind::from_name("platform"), Some(CollisionKind::Platform));
        assert_eq!(CollisionKind::from_name("NONE"), Some(CollisionKind::None));
        assert_eq!(CollisionKind::from_name("invalid"), None);
    }

    #[test]
    fn test_collision_set() {
        let mut set = CollisionSet::new();
        assert!(set.is_empty());
        
        set.insert(CollisionKind::Block);
        assert!(set.contains(CollisionKind::Block));
        assert!(!set.contains(CollisionKind::Platform));
        
        set.insert(CollisionKind::Platform);
        assert!(set.contains(CollisionKind::Platform));
        
        set.remove(CollisionKind::Block);
        assert!(!set.contains(CollisionKind::Block));
    }

    #[test]
    fn test_collision_set_default() {
        let set = CollisionSet::default_set();
        assert!(set.contains(CollisionKind::Null));
        assert!(set.contains(CollisionKind::Slippery));
        assert!(set.contains(CollisionKind::Dynamic));
        assert!(set.contains(CollisionKind::Block));
        assert!(!set.contains(CollisionKind::None));
        assert!(!set.contains(CollisionKind::Platform));
    }

    #[test]
    fn test_max_collision() {
        assert_eq!(max_collision(CollisionKind::Block, CollisionKind::Platform), CollisionKind::Block);
        assert_eq!(max_collision(CollisionKind::None, CollisionKind::Dynamic), CollisionKind::Dynamic);
        assert_eq!(max_collision(CollisionKind::Null, CollisionKind::None), CollisionKind::None);
    }

    #[test]
    fn test_collision_block_null() {
        let block = CollisionBlock::null_block(Vec2I::new(5, 10));
        assert_eq!(block.kind, CollisionKind::Null);
        assert_eq!(block.space, Vec2I::new(5, 10));
        assert_eq!(block.poly.len(), 4);
    }

    #[test]
    fn test_collision_block_contains_point() {
        let block = CollisionBlock::block(Vec2I::new(0, 0));
        
        assert!(block.contains_point(Vec2F::new(0.5, 0.5)));
        assert!(block.contains_point(Vec2F::new(0.1, 0.1)));
        assert!(block.contains_point(Vec2F::new(0.9, 0.9)));
        assert!(!block.contains_point(Vec2F::new(1.5, 0.5)));
        assert!(!block.contains_point(Vec2F::new(-0.5, 0.5)));
    }

    #[test]
    fn test_tile_collision_override() {
        assert_eq!(TileCollisionOverride::Empty.to_collision_kind(), CollisionKind::None);
        assert_eq!(TileCollisionOverride::Platform.to_collision_kind(), CollisionKind::Platform);
        assert_eq!(TileCollisionOverride::Block.to_collision_kind(), CollisionKind::Block);
    }

    #[test]
    fn test_collision_kind_serialization() {
        for kind in [
            CollisionKind::Null,
            CollisionKind::None,
            CollisionKind::Platform,
            CollisionKind::Dynamic,
            CollisionKind::Slippery,
            CollisionKind::Block,
        ] {
            let mut buf = Vec::new();
            {
                let mut writer = DataWriter::new(&mut buf);
                kind.write(&mut writer).unwrap();
            }
            
            let mut reader = DataReader::new(std::io::Cursor::new(buf));
            let read: CollisionKind = reader.read().unwrap();
            
            assert_eq!(read, kind);
        }
    }
}
