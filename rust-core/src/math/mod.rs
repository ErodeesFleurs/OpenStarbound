//! Mathematical types for Starbound Core
//!
//! This module provides vector and geometric types that are compatible
//! with the C++ Star::Vector and Star::Box types.

mod rect;
mod vector;

pub use rect::{Rect, RectF, RectI};
pub use vector::{Vec2, Vec2F, Vec2I, Vec3, Vec3B, Vec3F, Vec3I, Vec4, Vec4B, Vec4F, Vec4I};

/// Mathematical constants
pub mod constants {
    /// Pi constant
    pub const PI: f64 = std::f64::consts::PI;
    /// Pi as f32
    pub const PI_F32: f32 = std::f32::consts::PI;
    /// Two times Pi
    pub const TWO_PI: f64 = 2.0 * PI;
    /// Half Pi
    pub const HALF_PI: f64 = PI / 2.0;
}
