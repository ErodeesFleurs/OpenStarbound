//! Core types for Starbound
//!
//! This module provides core game types like Color, Json, Uuid, and other fundamental structures.

pub mod asset_path;
pub mod bimap;
pub mod btree_db;
mod byte_array;
mod color;
pub mod compression;
pub mod either;
mod encode;
pub mod file;
mod host_address;
pub mod image;
mod json;
pub mod lru_cache;
pub mod lua;
pub mod net_element;
mod perlin;
mod random;
mod sha256;
pub mod socket;
pub mod string_util;
pub mod thread;
mod uuid;
pub mod worker_pool;

pub use asset_path::AssetPath;
pub use bimap::BiMap;
pub use btree_db::{BTreeDatabase, BTreeSha256Database, DeviceIO, MemoryDevice, SyncBTreeDatabase};
pub use byte_array::ByteArray;
pub use color::Color;
pub use compression::{compress, uncompress, CompressionLevel, HIGH_COMPRESSION, LOW_COMPRESSION, MEDIUM_COMPRESSION};
pub use either::Either;
pub use encode::{base64_decode, base64_encode, hex_decode, hex_encode};
pub use file::{Buffer, FileDevice, FileInfo, FileSystem, FileType, IOMode};
pub use host_address::{HostAddress, HostAddressWithPort, NetworkMode};
pub use image::{Image, ImageView, PixelFormat, Vec3B, Vec4B};
pub use json::{Json, JsonType};
pub use lru_cache::{LruCache, TtlCache};
pub use lua::{
    LuaCallbacks, LuaContext, LuaEngine, LuaExceptionKind, LuaFunctionRef, LuaProfileEntry,
    LuaTableRef, LuaThreadRef, LuaThreadStatus, LuaUserDataRef, LuaValue, LuaVariadic,
    LuaWrappedFunction,
};
pub use net_element::{
    NetCompatibilityRules, NetElementBase, NetElementBool, NetElementFloat,
    NetElementGroup, NetElementInt, NetElementString, NetElementVersion, VersionNumber,
    ANY_VERSION,
};
pub use perlin::{Perlin, PerlinF, PerlinType};
pub use random::RandomSource;
pub use sha256::{sha256, sha256_hex, sha256_str, Sha256Hasher, SHA256_SIZE};
pub use socket::{SocketMode, TcpServer, TcpSocket, UdpServer, UdpSocket, MAX_UDP_DATA};
pub use string_util::CaseSensitivity;
pub use thread::{AtomicCounter, ConditionVariable, ReadersWriterLock, SpinLock, Thread, ThreadFunction};
pub use uuid::{Uuid, UUID_SIZE};
pub use worker_pool::{AsyncWorkerPool, TaskHandle, WorkerPool};

/// Global random number functions
pub mod global_random {
    pub use super::random::random::*;
}

/// Re-export btree database types
pub mod btree {
    pub use super::btree_db::*;
}

/// Re-export network element types
pub mod net {
    pub use super::net_element::*;
}
