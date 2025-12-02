//! Core types for Starbound
//!
//! This module provides core game types like Color, Json, Uuid, and other fundamental structures.

pub mod asset_path;
pub mod bimap;
mod byte_array;
mod color;
pub mod compression;
pub mod either;
mod encode;
mod host_address;
mod json;
mod perlin;
mod random;
mod sha256;
pub mod string_util;
mod uuid;

pub use asset_path::AssetPath;
pub use bimap::BiMap;
pub use byte_array::ByteArray;
pub use color::Color;
pub use compression::{compress, uncompress, CompressionLevel, HIGH_COMPRESSION, LOW_COMPRESSION, MEDIUM_COMPRESSION};
pub use either::Either;
pub use encode::{base64_decode, base64_encode, hex_decode, hex_encode};
pub use host_address::{HostAddress, HostAddressWithPort, NetworkMode};
pub use json::{Json, JsonType};
pub use perlin::{Perlin, PerlinF, PerlinType};
pub use random::RandomSource;
pub use sha256::{sha256, sha256_hex, sha256_str, Sha256Hasher, SHA256_SIZE};
pub use string_util::CaseSensitivity;
pub use uuid::{Uuid, UUID_SIZE};

/// Global random number functions
pub mod global_random {
    pub use super::random::random::*;
}
