//! # Starbound Core
//!
//! Core types and utilities for the OpenStarbound Rust implementation.
//! This crate provides fundamental data structures that are binary-compatible
//! with the C++ implementation to enable incremental migration.
//!
//! ## Modules
//!
//! - `math` - Vector, Rect, and other mathematical types
//! - `types` - Core game types like Color, Json, Uuid, ByteArray, Random, Perlin, Network, Lua, Database
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
    base64_decode, base64_encode, btree, compress, global_random, hex_decode, hex_encode, net,
    sha256, sha256_hex, sha256_str, uncompress, AssetPath, AsyncWorkerPool, AtomicCounter, BiMap,
    BTreeDatabase, BTreeSha256Database, Buffer, ByteArray, CaseSensitivity, Color, CompressionLevel,
    ConditionVariable, DeviceIO, Either, FileDevice, FileInfo, FileSystem, FileType, HostAddress,
    HostAddressWithPort, Image, ImageView, IOMode, Json, JsonType, LruCache, LuaCallbacks,
    LuaContext, LuaEngine, LuaExceptionKind, LuaFunctionRef, LuaProfileEntry, LuaTableRef,
    LuaThreadRef, LuaThreadStatus, LuaUserDataRef, LuaValue, LuaVariadic, LuaWrappedFunction,
    MemoryDevice, NetCompatibilityRules, NetElementBase, NetElementBool,
    NetElementFloat, NetElementGroup, NetElementInt, NetElementString, NetElementVersion,
    NetworkMode, Perlin, PerlinF, PerlinType, PixelFormat, RandomSource, ReadersWriterLock,
    Sha256Hasher, SocketMode, SpinLock, SyncBTreeDatabase, TaskHandle, TcpServer, TcpSocket,
    Thread, ThreadFunction, TtlCache, UdpServer, UdpSocket, Uuid, VersionNumber, WorkerPool,
    ANY_VERSION, HIGH_COMPRESSION, LOW_COMPRESSION, MAX_UDP_DATA, MEDIUM_COMPRESSION, SHA256_SIZE,
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
