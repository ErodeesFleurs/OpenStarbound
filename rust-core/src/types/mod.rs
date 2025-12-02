//! Core types for Starbound
//!
//! This module provides core game types like Color, Json, Uuid, and other fundamental structures.

pub mod asset_path;
pub mod bimap;
pub mod btree_db;
mod byte_array;
mod color;
pub mod compression;
pub mod damage_types;
pub mod either;
mod encode;
pub mod file;
pub mod game_types;
mod host_address;
pub mod image;
mod json;
pub mod logging;
pub mod lru_cache;
pub mod lua;
pub mod net_element;
pub mod option_parser;
mod perlin;
mod random;
mod sha256;
pub mod socket;
pub mod string_util;
pub mod thread;
pub mod time;
mod uuid;
pub mod worker_pool;

pub use asset_path::AssetPath;
pub use bimap::BiMap;
pub use btree_db::{BTreeDatabase, BTreeSha256Database, DeviceIO, MemoryDevice, SyncBTreeDatabase};
pub use byte_array::ByteArray;
pub use color::Color;
pub use compression::{compress, uncompress, CompressionLevel, HIGH_COMPRESSION, LOW_COMPRESSION, MEDIUM_COMPRESSION};
pub use damage_types::{DamageType, EntityDamageTeam, HitType, TeamNumber, TeamType};
pub use either::Either;
pub use encode::{base64_decode, base64_encode, hex_decode, hex_encode};
pub use file::{Buffer, FileDevice, FileInfo, FileSystem, FileType, IOMode};
pub use game_types::{
    center_of_tile, connection_entity_space, connection_for_entity, direction_of,
    entity_id_in_space, get_angle_side, global_timescale, global_timestep, is_real_dungeon,
    numerical_direction, server_global_timestep, set_global_timescale, set_global_timestep,
    set_server_global_timestep, ConnectionId, Direction, DungeonId, EntityId, EntityMode,
    FireMode, Gender, MoveControlType, PortraitMode, Rarity, TileDamageResult, TileLayer,
    ToolHand, BIOME_MICRO_DUNGEON_ID, CONSTRUCTION_DUNGEON_ID, DESTROYED_BLOCK_DUNGEON_ID,
    FIRST_META_DUNGEON_ID, MAX_CLIENT_CONNECTION_ID, MAX_SERVER_ENTITY_ID,
    MIN_CLIENT_CONNECTION_ID, MIN_SERVER_ENTITY_ID, NO_DUNGEON_ID, NULL_ENTITY_ID,
    PROTECTED_ZERO_G_DUNGEON_ID, SERVER_CONNECTION_ID, SPAWN_DUNGEON_ID, SYSTEM_WORLD_TIMESTEP,
    TILE_PIXELS, WORLD_SECTOR_SIZE, ZERO_G_DUNGEON_ID,
};
pub use host_address::{HostAddress, HostAddressWithPort, NetworkMode};
pub use image::{Image, ImageView, PixelFormat, Vec3B, Vec4B};
pub use json::{Json, JsonType};
pub use logging::{FileLogSink, Line, LogLevel, LogMap, LogSink, LogText, Logger, Point, SpatialLogger, StdoutLogSink};
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
pub use option_parser::{OptionParser, Options, RequirementMode};
pub use perlin::{Perlin, PerlinF, PerlinType};
pub use random::RandomSource;
pub use sha256::{sha256, sha256_hex, sha256_str, Sha256Hasher, SHA256_SIZE};
pub use socket::{SocketMode, TcpServer, TcpSocket, UdpServer, UdpSocket, MAX_UDP_DATA};
pub use string_util::CaseSensitivity;
pub use thread::{AtomicCounter, ConditionVariable, ReadersWriterLock, SpinLock, Thread, ThreadFunction};
pub use time::{
    milliseconds_since_epoch, milliseconds_to_ticks, monotonic_microseconds, monotonic_milliseconds,
    monotonic_time, print_current_date_and_time, print_date_and_time, print_duration,
    seconds_to_ticks, ticks_to_microseconds, ticks_to_milliseconds, ticks_to_seconds,
    time_since_epoch, Clock, Timer,
};
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
