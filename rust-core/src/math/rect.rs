//! Rectangle/Box types compatible with C++ Star::Box/Rect
//!
//! This module provides N-dimensional axis-aligned box types that match
//! the binary layout of the C++ implementation for FFI compatibility.

use super::vector::Vec2;
use serde::{Deserialize, Serialize};
use std::fmt;

/// A generic N-dimensional axis-aligned box/rectangle
#[derive(Clone, Copy, PartialEq, Eq, Hash)]
#[repr(C)]
pub struct Box<T, const N: usize> {
    min: super::vector::Vec<T, N>,
    max: super::vector::Vec<T, N>,
}

// Manual Default implementation
impl<T: Copy + Default, const N: usize> Default for Box<T, N> {
    fn default() -> Self {
        Self {
            min: super::vector::Vec::default(),
            max: super::vector::Vec::default(),
        }
    }
}

// Manual Serialize implementation
impl<T: Serialize + Copy, const N: usize> Serialize for Box<T, N> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        use serde::ser::SerializeStruct;
        let mut state = serializer.serialize_struct("Box", 2)?;
        state.serialize_field("min", &self.min)?;
        state.serialize_field("max", &self.max)?;
        state.end()
    }
}

// Manual Deserialize implementation
impl<'de, T: Deserialize<'de> + Copy + Default, const N: usize> Deserialize<'de> for Box<T, N> {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        #[derive(Deserialize)]
        #[serde(field_identifier, rename_all = "lowercase")]
        enum Field {
            Min,
            Max,
        }

        struct BoxVisitor<T, const N: usize>(std::marker::PhantomData<T>);

        impl<'de, T: Deserialize<'de> + Copy + Default, const N: usize> serde::de::Visitor<'de>
            for BoxVisitor<T, N>
        {
            type Value = Box<T, N>;

            fn expecting(&self, formatter: &mut std::fmt::Formatter) -> std::fmt::Result {
                formatter.write_str("struct Box")
            }

            fn visit_seq<V>(self, mut seq: V) -> Result<Box<T, N>, V::Error>
            where
                V: serde::de::SeqAccess<'de>,
            {
                let min = seq
                    .next_element()?
                    .ok_or_else(|| serde::de::Error::invalid_length(0, &self))?;
                let max = seq
                    .next_element()?
                    .ok_or_else(|| serde::de::Error::invalid_length(1, &self))?;
                Ok(Box { min, max })
            }

            fn visit_map<V>(self, mut map: V) -> Result<Box<T, N>, V::Error>
            where
                V: serde::de::MapAccess<'de>,
            {
                let mut min = None;
                let mut max = None;
                while let Some(key) = map.next_key()? {
                    match key {
                        Field::Min => {
                            if min.is_some() {
                                return Err(serde::de::Error::duplicate_field("min"));
                            }
                            min = Some(map.next_value()?);
                        }
                        Field::Max => {
                            if max.is_some() {
                                return Err(serde::de::Error::duplicate_field("max"));
                            }
                            max = Some(map.next_value()?);
                        }
                    }
                }
                let min = min.ok_or_else(|| serde::de::Error::missing_field("min"))?;
                let max = max.ok_or_else(|| serde::de::Error::missing_field("max"))?;
                Ok(Box { min, max })
            }
        }

        const FIELDS: &[&str] = &["min", "max"];
        deserializer.deserialize_struct("Box", FIELDS, BoxVisitor(std::marker::PhantomData))
    }
}

// Type aliases matching C++ typedefs
pub type Rect<T> = Box<T, 2>;
pub type Box3<T> = Box<T, 3>;
pub type Box4<T> = Box<T, 4>;

pub type RectI = Rect<i32>;
pub type RectU = Rect<u32>;
pub type RectF = Rect<f32>;
pub type RectD = Rect<f64>;

pub type Box3I = Box3<i32>;
pub type Box3F = Box3<f32>;

impl<T: Copy + Default, const N: usize> Box<T, N> {
    /// Create a new box from min and max corners
    pub fn new(min: super::vector::Vec<T, N>, max: super::vector::Vec<T, N>) -> Self {
        Self { min, max }
    }

    /// Get the minimum corner
    pub fn min(&self) -> super::vector::Vec<T, N> {
        self.min
    }

    /// Get the maximum corner
    pub fn max(&self) -> super::vector::Vec<T, N> {
        self.max
    }

    /// Set the minimum corner
    pub fn set_min(&mut self, min: super::vector::Vec<T, N>) {
        self.min = min;
    }

    /// Set the maximum corner
    pub fn set_max(&mut self, max: super::vector::Vec<T, N>) {
        self.max = max;
    }
}

// 2D Rectangle specific implementations
impl<T: Copy + Default> Rect<T> {
    /// Create a rectangle from corner coordinates
    pub fn from_coords(x_min: T, y_min: T, x_max: T, y_max: T) -> Self {
        Self {
            min: Vec2::new(x_min, y_min),
            max: Vec2::new(x_max, y_max),
        }
    }

    /// Get x minimum
    pub fn x_min(&self) -> T {
        self.min.x()
    }

    /// Get y minimum
    pub fn y_min(&self) -> T {
        self.min.y()
    }

    /// Get x maximum
    pub fn x_max(&self) -> T {
        self.max.x()
    }

    /// Get y maximum
    pub fn y_max(&self) -> T {
        self.max.y()
    }

    /// Set x minimum
    pub fn set_x_min(&mut self, x: T) {
        self.min.set_x(x);
    }

    /// Set y minimum
    pub fn set_y_min(&mut self, y: T) {
        self.min.set_y(y);
    }

    /// Set x maximum
    pub fn set_x_max(&mut self, x: T) {
        self.max.set_x(x);
    }

    /// Set y maximum
    pub fn set_y_max(&mut self, y: T) {
        self.max.set_y(y);
    }
}

// Float-specific operations for all box types
macro_rules! impl_box_float_ops {
    ($($t:ty),*) => {
        $(
            impl<const N: usize> Box<$t, N> {
                /// Create a null box (min > max)
                pub fn null() -> Self {
                    Self {
                        min: super::vector::Vec::filled(<$t>::MAX),
                        max: super::vector::Vec::filled(<$t>::MIN),
                    }
                }

                /// Create an infinite box
                pub fn inf() -> Self {
                    Self {
                        min: super::vector::Vec::filled(<$t>::MIN),
                        max: super::vector::Vec::filled(<$t>::MAX),
                    }
                }

                /// Check if the box is null (min > max in any dimension)
                pub fn is_null(&self) -> bool {
                    for i in 0..N {
                        if self.min[i] > self.max[i] {
                            return true;
                        }
                    }
                    false
                }

                /// Check if the box is negative (has negative volume)
                pub fn is_negative(&self) -> bool {
                    for i in 0..N {
                        if self.max[i] < self.min[i] {
                            return true;
                        }
                    }
                    false
                }

                /// Check if the box is empty (zero or negative volume)
                pub fn is_empty(&self) -> bool {
                    for i in 0..N {
                        if self.max[i] <= self.min[i] {
                            return true;
                        }
                    }
                    false
                }

                /// Get the size of the box
                pub fn size(&self) -> super::vector::Vec<$t, N> {
                    self.max - self.min
                }

                /// Get the center of the box
                pub fn center(&self) -> super::vector::Vec<$t, N> {
                    (self.min + self.max) / 2.0
                }

                /// Get the volume (area for 2D)
                pub fn volume(&self) -> $t {
                    self.size().product()
                }

                /// Translate the box by a vector
                pub fn translate(&mut self, offset: super::vector::Vec<$t, N>) {
                    self.min = self.min + offset;
                    self.max = self.max + offset;
                }

                /// Get a translated copy of the box
                pub fn translated(&self, offset: super::vector::Vec<$t, N>) -> Self {
                    let mut result = *self;
                    result.translate(offset);
                    result
                }

                /// Set the center of the box
                pub fn set_center(&mut self, center: super::vector::Vec<$t, N>) {
                    let offset = center - self.center();
                    self.translate(offset);
                }

                /// Create a box with given size centered at origin
                pub fn with_size(min: super::vector::Vec<$t, N>, size: super::vector::Vec<$t, N>) -> Self {
                    Self {
                        min,
                        max: min + size,
                    }
                }

                /// Create a box with given center and size
                pub fn with_center(center: super::vector::Vec<$t, N>, size: super::vector::Vec<$t, N>) -> Self {
                    let half_size = size / 2.0;
                    Self {
                        min: center - half_size,
                        max: center + half_size,
                    }
                }

                /// Expand the box from center
                pub fn expand(&mut self, factor: $t) {
                    let center = self.center();
                    let half_size = self.size() / 2.0 * factor;
                    self.min = center - half_size;
                    self.max = center + half_size;
                }

                /// Get an expanded copy
                pub fn expanded(&self, factor: $t) -> Self {
                    let mut result = *self;
                    result.expand(factor);
                    result
                }

                /// Scale around origin
                pub fn scale(&mut self, factor: $t) {
                    self.min = self.min * factor;
                    self.max = self.max * factor;
                }

                /// Get a scaled copy
                pub fn scaled(&self, factor: $t) -> Self {
                    let mut result = *self;
                    result.scale(factor);
                    result
                }

                /// Pad the box by a constant amount on all sides
                pub fn pad(&mut self, amount: $t) {
                    for i in 0..N {
                        self.min[i] -= amount;
                        self.max[i] += amount;
                    }
                }

                /// Get a padded copy
                pub fn padded(&self, amount: $t) -> Self {
                    let mut result = *self;
                    result.pad(amount);
                    result
                }

                /// Trim the box (opposite of pad)
                pub fn trim(&mut self, amount: $t) {
                    self.pad(-amount);
                }

                /// Get a trimmed copy
                pub fn trimmed(&self, amount: $t) -> Self {
                    let mut result = *self;
                    result.trim(amount);
                    result
                }

                /// Combine with another box (union)
                pub fn combine(&mut self, other: &Self) {
                    self.min = self.min.piecewise_min(&other.min);
                    self.max = self.max.piecewise_max(&other.max);
                }

                /// Get a combined copy
                pub fn combined(&self, other: &Self) -> Self {
                    let mut result = *self;
                    result.combine(other);
                    result
                }

                /// Combine with a point
                pub fn combine_point(&mut self, point: super::vector::Vec<$t, N>) {
                    self.min = self.min.piecewise_min(&point);
                    self.max = self.max.piecewise_max(&point);
                }

                /// Limit to another box (intersection)
                pub fn limit(&mut self, other: &Self) {
                    self.min = self.min.piecewise_max(&other.min);
                    self.max = self.max.piecewise_min(&other.max);
                }

                /// Get a limited copy
                pub fn limited(&self, other: &Self) -> Self {
                    let mut result = *self;
                    result.limit(other);
                    result
                }

                /// Get the overlap with another box
                pub fn overlap(&self, other: &Self) -> Self {
                    self.limited(other)
                }

                /// Check if intersects with another box
                pub fn intersects(&self, other: &Self, include_edges: bool) -> bool {
                    for i in 0..N {
                        if include_edges {
                            if self.max[i] < other.min[i] || other.max[i] < self.min[i] {
                                return false;
                            }
                        } else if self.max[i] <= other.min[i] || other.max[i] <= self.min[i] {
                            return false;
                        }
                    }
                    true
                }

                /// Check if contains a point
                pub fn contains_point(&self, point: super::vector::Vec<$t, N>, include_edges: bool) -> bool {
                    for i in 0..N {
                        if include_edges {
                            if point[i] < self.min[i] || point[i] > self.max[i] {
                                return false;
                            }
                        } else if point[i] <= self.min[i] || point[i] >= self.max[i] {
                            return false;
                        }
                    }
                    true
                }

                /// Check if contains another box
                pub fn contains_box(&self, other: &Self, include_edges: bool) -> bool {
                    self.contains_point(other.min, include_edges) && self.contains_point(other.max, include_edges)
                }

                /// Find the nearest point in the box to a given point
                pub fn nearest_point_to(&self, point: super::vector::Vec<$t, N>) -> super::vector::Vec<$t, N> {
                    let mut result = point;
                    for i in 0..N {
                        result[i] = result[i].max(self.min[i]).min(self.max[i]);
                    }
                    result
                }

                /// Make the box positive (swap min/max if needed)
                pub fn make_positive(&mut self) {
                    for i in 0..N {
                        if self.max[i] < self.min[i] {
                            std::mem::swap(&mut self.min[i], &mut self.max[i]);
                        }
                    }
                }
            }

            // 2D specific float operations
            impl Rect<$t> {
                /// Get width
                pub fn width(&self) -> $t {
                    self.max.x() - self.min.x()
                }

                /// Get height
                pub fn height(&self) -> $t {
                    self.max.y() - self.min.y()
                }

                /// Flip horizontal
                pub fn flip_horizontal(&mut self) {
                    std::mem::swap(&mut self.min[0], &mut self.max[0]);
                }

                /// Flip vertical
                pub fn flip_vertical(&mut self) {
                    std::mem::swap(&mut self.min[1], &mut self.max[1]);
                }

                /// Get flipped horizontal copy
                pub fn flipped_horizontal(&self) -> Self {
                    let mut result = *self;
                    result.flip_horizontal();
                    result
                }

                /// Get flipped vertical copy
                pub fn flipped_vertical(&self) -> Self {
                    let mut result = *self;
                    result.flip_vertical();
                    result
                }
            }
        )*
    };
}

impl_box_float_ops!(f32, f64);

// Integer-specific operations
macro_rules! impl_box_int_ops {
    ($($t:ty),*) => {
        $(
            impl<const N: usize> Box<$t, N> {
                /// Create a null box
                pub fn null() -> Self {
                    Self {
                        min: super::vector::Vec::filled(<$t>::MAX),
                        max: super::vector::Vec::filled(<$t>::MIN),
                    }
                }

                /// Check if the box is null
                pub fn is_null(&self) -> bool {
                    for i in 0..N {
                        if self.min[i] > self.max[i] {
                            return true;
                        }
                    }
                    false
                }

                /// Check if the box is empty
                pub fn is_empty(&self) -> bool {
                    for i in 0..N {
                        if self.max[i] <= self.min[i] {
                            return true;
                        }
                    }
                    false
                }

                /// Get the size of the box
                pub fn size(&self) -> super::vector::Vec<$t, N> {
                    self.max - self.min
                }

                /// Check if contains a point
                pub fn contains_point(&self, point: super::vector::Vec<$t, N>, include_edges: bool) -> bool {
                    for i in 0..N {
                        if include_edges {
                            if point[i] < self.min[i] || point[i] > self.max[i] {
                                return false;
                            }
                        } else if point[i] <= self.min[i] || point[i] >= self.max[i] {
                            return false;
                        }
                    }
                    true
                }

                /// Check if intersects with another box
                pub fn intersects(&self, other: &Self, include_edges: bool) -> bool {
                    for i in 0..N {
                        if include_edges {
                            if self.max[i] < other.min[i] || other.max[i] < self.min[i] {
                                return false;
                            }
                        } else if self.max[i] <= other.min[i] || other.max[i] <= self.min[i] {
                            return false;
                        }
                    }
                    true
                }
            }

            // 2D specific integer operations
            impl Rect<$t> {
                /// Get width
                pub fn width(&self) -> $t {
                    self.max.x() - self.min.x()
                }

                /// Get height
                pub fn height(&self) -> $t {
                    self.max.y() - self.min.y()
                }
            }
        )*
    };
}

impl_box_int_ops!(i32, i64, u32, u64);

// Display implementation
impl<T: fmt::Display + Copy + Default, const N: usize> fmt::Display for Box<T, N> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Box{{min:{} max:{}}}", self.min, self.max)
    }
}

impl<T: fmt::Debug + Copy + Default, const N: usize> fmt::Debug for Box<T, N> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("Box")
            .field("min", &self.min)
            .field("max", &self.max)
            .finish()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_rect_creation() {
        let r = RectF::from_coords(0.0, 0.0, 10.0, 10.0);
        assert_eq!(r.x_min(), 0.0);
        assert_eq!(r.y_min(), 0.0);
        assert_eq!(r.x_max(), 10.0);
        assert_eq!(r.y_max(), 10.0);
    }

    #[test]
    fn test_rect_size() {
        let r = RectF::from_coords(0.0, 0.0, 10.0, 20.0);
        assert_eq!(r.width(), 10.0);
        assert_eq!(r.height(), 20.0);
    }

    #[test]
    fn test_rect_center() {
        let r = RectF::from_coords(0.0, 0.0, 10.0, 10.0);
        let center = r.center();
        assert_eq!(center.x(), 5.0);
        assert_eq!(center.y(), 5.0);
    }

    #[test]
    fn test_rect_contains_point() {
        let r = RectF::from_coords(0.0, 0.0, 10.0, 10.0);
        assert!(r.contains_point(Vec2::new(5.0, 5.0), true));
        assert!(r.contains_point(Vec2::new(0.0, 0.0), true));
        assert!(!r.contains_point(Vec2::new(-1.0, 5.0), true));
    }

    #[test]
    fn test_rect_intersects() {
        let r1 = RectF::from_coords(0.0, 0.0, 10.0, 10.0);
        let r2 = RectF::from_coords(5.0, 5.0, 15.0, 15.0);
        let r3 = RectF::from_coords(20.0, 20.0, 30.0, 30.0);

        assert!(r1.intersects(&r2, true));
        assert!(!r1.intersects(&r3, true));
    }

    #[test]
    fn test_rect_translate() {
        let mut r = RectF::from_coords(0.0, 0.0, 10.0, 10.0);
        r.translate(Vec2::new(5.0, 5.0));
        assert_eq!(r.x_min(), 5.0);
        assert_eq!(r.y_min(), 5.0);
        assert_eq!(r.x_max(), 15.0);
        assert_eq!(r.y_max(), 15.0);
    }

    #[test]
    fn test_rect_combine() {
        let mut r1 = RectF::from_coords(0.0, 0.0, 10.0, 10.0);
        let r2 = RectF::from_coords(5.0, 5.0, 15.0, 15.0);
        r1.combine(&r2);
        assert_eq!(r1.x_min(), 0.0);
        assert_eq!(r1.y_min(), 0.0);
        assert_eq!(r1.x_max(), 15.0);
        assert_eq!(r1.y_max(), 15.0);
    }

    #[test]
    fn test_rect_overlap() {
        let r1 = RectF::from_coords(0.0, 0.0, 10.0, 10.0);
        let r2 = RectF::from_coords(5.0, 5.0, 15.0, 15.0);
        let overlap = r1.overlap(&r2);
        assert_eq!(overlap.x_min(), 5.0);
        assert_eq!(overlap.y_min(), 5.0);
        assert_eq!(overlap.x_max(), 10.0);
        assert_eq!(overlap.y_max(), 10.0);
    }

    #[test]
    fn test_rect_volume() {
        let r = RectF::from_coords(0.0, 0.0, 10.0, 20.0);
        assert!((r.volume() - 200.0).abs() < 1e-6);
    }

    #[test]
    fn test_rect_with_center() {
        let r = RectF::with_center(Vec2::new(5.0, 5.0), Vec2::new(10.0, 10.0));
        assert_eq!(r.x_min(), 0.0);
        assert_eq!(r.y_min(), 0.0);
        assert_eq!(r.x_max(), 10.0);
        assert_eq!(r.y_max(), 10.0);
    }
}
