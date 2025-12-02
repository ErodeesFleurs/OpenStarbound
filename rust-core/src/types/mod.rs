//! Core types for Starbound
//!
//! This module provides core game types like Color, Json, Uuid, and other fundamental structures.

mod asset_path;
mod byte_array;
mod color;
mod json;
mod perlin;
mod random;
mod uuid;

pub use asset_path::AssetPath;
pub use byte_array::ByteArray;
pub use color::Color;
pub use json::{Json, JsonType};
pub use perlin::{Perlin, PerlinF, PerlinType};
pub use random::RandomSource;
pub use uuid::{Uuid, UUID_SIZE};

/// Global random number functions
pub mod global_random {
    pub use super::random::random::*;
}
