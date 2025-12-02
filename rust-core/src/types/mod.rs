//! Core types for Starbound
//!
//! This module provides core game types like Color, Json, Uuid, and other fundamental structures.

mod byte_array;
mod color;
mod json;
mod uuid;

pub use byte_array::ByteArray;
pub use color::Color;
pub use json::{Json, JsonType};
pub use uuid::{Uuid, UUID_SIZE};
