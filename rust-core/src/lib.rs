//! # Starbound Core
//!
//! Core types and utilities for the OpenStarbound Rust implementation.
//! This crate provides fundamental data structures that are binary-compatible
//! with the C++ implementation to enable incremental migration.
//!
//! ## Modules
//!
//! - `math` - Vector, Rect, and other mathematical types
//! - `types` - Core game types like Color, Json, Uuid, ByteArray, Random, Perlin, Network
//! - `serialization` - Binary serialization compatible with C++ format
//! - `error` - Error types and handling

pub mod error;
pub mod math;
pub mod serialization;
pub mod types;

// Re-export commonly used types
pub use error::{Error, Result};
pub use math::{Rect, RectF, RectI, Vec2, Vec2F, Vec2I, Vec3, Vec3F, Vec3I, Vec4, Vec4B, Vec4F};
pub use types::{
    base64_decode, base64_encode, global_random, hex_decode, hex_encode, sha256, sha256_hex,
    sha256_str, AssetPath, ByteArray, Color, HostAddress, HostAddressWithPort, Json, JsonType,
    NetworkMode, Perlin, PerlinF, PerlinType, RandomSource, Sha256Hasher, Uuid, SHA256_SIZE,
    UUID_SIZE,
};

/// Version information for compatibility checking
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_version() {
        assert!(!VERSION.is_empty());
    }
}
