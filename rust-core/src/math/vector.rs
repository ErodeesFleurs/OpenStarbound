//! Vector types compatible with C++ Star::Vector
//!
//! This module provides N-dimensional vector types that match the binary
//! layout of the C++ implementation for FFI compatibility.

use serde::{Deserialize, Serialize};
use std::fmt;
use std::ops::{Add, AddAssign, Div, DivAssign, Index, IndexMut, Mul, MulAssign, Neg, Sub, SubAssign};

/// A generic N-dimensional vector type
///
/// The `data` field is public to enable direct array initialization for const contexts
/// and to maintain C ABI compatibility via `#[repr(C)]`.
#[derive(Clone, Copy, PartialEq, Eq, Hash)]
#[repr(C)]
pub struct Vec<T, const N: usize> {
    /// The underlying array data. Public for const initialization and FFI compatibility.
    pub data: [T; N],
}

// Manual Default implementation
impl<T: Copy + Default, const N: usize> Default for Vec<T, N> {
    fn default() -> Self {
        Self {
            data: [T::default(); N],
        }
    }
}

// Manual Serialize implementation for common sizes
impl<T: Serialize, const N: usize> Serialize for Vec<T, N> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        use serde::ser::SerializeSeq;
        let mut seq = serializer.serialize_seq(Some(N))?;
        for elem in &self.data {
            seq.serialize_element(elem)?;
        }
        seq.end()
    }
}

// Manual Deserialize implementation
impl<'de, T: Deserialize<'de> + Copy + Default, const N: usize> Deserialize<'de> for Vec<T, N> {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        struct VecVisitor<T, const N: usize>(std::marker::PhantomData<T>);

        impl<'de, T: Deserialize<'de> + Copy + Default, const N: usize> serde::de::Visitor<'de>
            for VecVisitor<T, N>
        {
            type Value = Vec<T, N>;

            fn expecting(&self, formatter: &mut std::fmt::Formatter) -> std::fmt::Result {
                write!(formatter, "a sequence of {} elements", N)
            }

            fn visit_seq<A>(self, mut seq: A) -> Result<Self::Value, A::Error>
            where
                A: serde::de::SeqAccess<'de>,
            {
                let mut data = [T::default(); N];
                for (i, item) in data.iter_mut().enumerate() {
                    *item = seq
                        .next_element()?
                        .ok_or_else(|| serde::de::Error::invalid_length(i, &self))?;
                }
                Ok(Vec { data })
            }
        }

        deserializer.deserialize_seq(VecVisitor(std::marker::PhantomData))
    }
}

// Type aliases matching C++ typedefs
pub type Vec2<T> = Vec<T, 2>;
pub type Vec3<T> = Vec<T, 3>;
pub type Vec4<T> = Vec<T, 4>;

pub type Vec2I = Vec2<i32>;
pub type Vec2F = Vec2<f32>;
pub type Vec2D = Vec2<f64>;
pub type Vec2B = Vec2<u8>;

pub type Vec3I = Vec3<i32>;
pub type Vec3F = Vec3<f32>;
pub type Vec3D = Vec3<f64>;
pub type Vec3B = Vec3<u8>;

pub type Vec4I = Vec4<i32>;
pub type Vec4F = Vec4<f32>;
pub type Vec4D = Vec4<f64>;
pub type Vec4B = Vec4<u8>;

impl<T: Copy + Default, const N: usize> Vec<T, N> {
    /// Create a new vector with all elements set to the given value
    pub fn filled(value: T) -> Self {
        Self {
            data: [value; N],
        }
    }

    /// Create a new zero vector
    pub fn zero() -> Self
    where
        T: Default,
    {
        Self::default()
    }

    /// Get a reference to the underlying array
    pub fn as_array(&self) -> &[T; N] {
        &self.data
    }

    /// Get a mutable reference to the underlying array
    pub fn as_array_mut(&mut self) -> &mut [T; N] {
        &mut self.data
    }

    /// Get the number of elements
    pub const fn len() -> usize {
        N
    }

    /// Check if the vector is empty (always false for N > 0)
    pub const fn is_empty() -> bool {
        N == 0
    }
}

// 2D Vector specific implementations
impl<T: Copy> Vec2<T> {
    /// Create a new 2D vector
    pub fn new(x: T, y: T) -> Self {
        Self { data: [x, y] }
    }

    /// Get x component
    pub fn x(&self) -> T {
        self.data[0]
    }

    /// Get y component
    pub fn y(&self) -> T {
        self.data[1]
    }

    /// Set x component
    pub fn set_x(&mut self, x: T) {
        self.data[0] = x;
    }

    /// Set y component
    pub fn set_y(&mut self, y: T) {
        self.data[1] = y;
    }
}

// 3D Vector specific implementations
impl<T: Copy> Vec3<T> {
    /// Create a new 3D vector
    pub fn new(x: T, y: T, z: T) -> Self {
        Self { data: [x, y, z] }
    }

    /// Get x component
    pub fn x(&self) -> T {
        self.data[0]
    }

    /// Get y component
    pub fn y(&self) -> T {
        self.data[1]
    }

    /// Get z component
    pub fn z(&self) -> T {
        self.data[2]
    }

    /// Set x component
    pub fn set_x(&mut self, x: T) {
        self.data[0] = x;
    }

    /// Set y component
    pub fn set_y(&mut self, y: T) {
        self.data[1] = y;
    }

    /// Set z component
    pub fn set_z(&mut self, z: T) {
        self.data[2] = z;
    }

    /// Convert to Vec2 by dropping z
    pub fn vec2(&self) -> Vec2<T> {
        Vec2::new(self.x(), self.y())
    }
}

// 4D Vector specific implementations
impl<T: Copy> Vec4<T> {
    /// Create a new 4D vector
    pub fn new(x: T, y: T, z: T, w: T) -> Self {
        Self { data: [x, y, z, w] }
    }

    /// Get x component
    pub fn x(&self) -> T {
        self.data[0]
    }

    /// Get y component
    pub fn y(&self) -> T {
        self.data[1]
    }

    /// Get z component
    pub fn z(&self) -> T {
        self.data[2]
    }

    /// Get w component
    pub fn w(&self) -> T {
        self.data[3]
    }

    /// Set x component
    pub fn set_x(&mut self, x: T) {
        self.data[0] = x;
    }

    /// Set y component
    pub fn set_y(&mut self, y: T) {
        self.data[1] = y;
    }

    /// Set z component
    pub fn set_z(&mut self, z: T) {
        self.data[2] = z;
    }

    /// Set w component
    pub fn set_w(&mut self, w: T) {
        self.data[3] = w;
    }

    /// Convert to Vec2 by dropping z and w
    pub fn vec2(&self) -> Vec2<T> {
        Vec2::new(self.x(), self.y())
    }

    /// Convert to Vec3 by dropping w
    pub fn vec3(&self) -> Vec3<T> {
        Vec3::new(self.x(), self.y(), self.z())
    }
}

// Float-specific operations
macro_rules! impl_float_ops {
    ($($t:ty),*) => {
        $(
            impl<const N: usize> Vec<$t, N> {
                /// Calculate the squared magnitude of the vector
                pub fn magnitude_squared(&self) -> $t {
                    self.data.iter().map(|&x| x * x).sum()
                }

                /// Calculate the magnitude of the vector
                pub fn magnitude(&self) -> $t {
                    self.magnitude_squared().sqrt()
                }

                /// Normalize the vector (returns a unit vector)
                pub fn normalized(&self) -> Self {
                    let m = self.magnitude();
                    if m != 0.0 {
                        *self / m
                    } else {
                        *self
                    }
                }

                /// Normalize the vector in place
                pub fn normalize(&mut self) {
                    *self = self.normalized();
                }

                /// Dot product with another vector
                pub fn dot(&self, other: &Self) -> $t {
                    self.data.iter().zip(other.data.iter()).map(|(&a, &b)| a * b).sum()
                }

                /// Calculate angle between two vectors (in radians)
                pub fn angle_between(&self, other: &Self) -> $t {
                    (self.normalized().dot(&other.normalized())).acos()
                }

                /// Piecewise minimum
                pub fn piecewise_min(&self, other: &Self) -> Self {
                    let mut result = *self;
                    for i in 0..N {
                        result.data[i] = self.data[i].min(other.data[i]);
                    }
                    result
                }

                /// Piecewise maximum
                pub fn piecewise_max(&self, other: &Self) -> Self {
                    let mut result = *self;
                    for i in 0..N {
                        result.data[i] = self.data[i].max(other.data[i]);
                    }
                    result
                }

                /// Piecewise multiply
                pub fn piecewise_multiply(&self, other: &Self) -> Self {
                    let mut result = *self;
                    for i in 0..N {
                        result.data[i] *= other.data[i];
                    }
                    result
                }

                /// Piecewise divide
                pub fn piecewise_divide(&self, other: &Self) -> Self {
                    let mut result = *self;
                    for i in 0..N {
                        result.data[i] /= other.data[i];
                    }
                    result
                }

                /// Floor each component
                pub fn floor(&self) -> Self {
                    let mut result = *self;
                    for i in 0..N {
                        result.data[i] = self.data[i].floor();
                    }
                    result
                }

                /// Ceil each component
                pub fn ceil(&self) -> Self {
                    let mut result = *self;
                    for i in 0..N {
                        result.data[i] = self.data[i].ceil();
                    }
                    result
                }

                /// Round each component
                pub fn round(&self) -> Self {
                    let mut result = *self;
                    for i in 0..N {
                        result.data[i] = self.data[i].round();
                    }
                    result
                }

                /// Absolute value of each component
                pub fn abs(&self) -> Self {
                    let mut result = *self;
                    for i in 0..N {
                        result.data[i] = self.data[i].abs();
                    }
                    result
                }

                /// Clamp each component
                pub fn clamp(&self, min: $t, max: $t) -> Self {
                    let mut result = *self;
                    for i in 0..N {
                        result.data[i] = self.data[i].max(min).min(max);
                    }
                    result
                }

                /// Sum of all components
                pub fn sum(&self) -> $t {
                    self.data.iter().sum()
                }

                /// Product of all components
                pub fn product(&self) -> $t {
                    self.data.iter().product()
                }

                /// Minimum component
                pub fn min_component(&self) -> $t {
                    self.data.iter().cloned().fold(<$t>::INFINITY, <$t>::min)
                }

                /// Maximum component
                pub fn max_component(&self) -> $t {
                    self.data.iter().cloned().fold(<$t>::NEG_INFINITY, <$t>::max)
                }
            }

            // 2D specific float operations
            impl Vec2<$t> {
                /// Create a unit vector from an angle (in radians)
                pub fn with_angle(angle: $t, magnitude: $t) -> Self {
                    Self::new(angle.cos() * magnitude, angle.sin() * magnitude)
                }

                /// Get the angle of this vector (in radians, range [-pi, pi])
                pub fn angle(&self) -> $t {
                    self.y().atan2(self.x())
                }

                /// Rotate the vector by an angle (in radians)
                pub fn rotate(&self, angle: $t) -> Self {
                    let cos_a = angle.cos();
                    let sin_a = angle.sin();
                    Self::new(
                        self.x() * cos_a - self.y() * sin_a,
                        self.x() * sin_a + self.y() * cos_a,
                    )
                }

                /// Rotate 90 degrees counter-clockwise
                pub fn rot90(&self) -> Self {
                    Self::new(-self.y(), self.x())
                }

                /// Convert to polar coordinates (angle, magnitude)
                pub fn to_polar(&self) -> Self {
                    Self::new(self.angle(), self.magnitude())
                }

                /// Convert from polar to cartesian
                pub fn to_cartesian(&self) -> Self {
                    Self::new(self.y() * self.x().cos(), self.y() * self.x().sin())
                }

                /// Cross product (returns scalar for 2D)
                pub fn cross(&self, other: &Self) -> $t {
                    self.x() * other.y() - self.y() * other.x()
                }
            }

            // 3D specific float operations
            impl Vec3<$t> {
                /// Cross product
                pub fn cross(&self, other: &Self) -> Self {
                    Self::new(
                        self.y() * other.z() - self.z() * other.y(),
                        self.z() * other.x() - self.x() * other.z(),
                        self.x() * other.y() - self.y() * other.x(),
                    )
                }

                /// Triple scalar product
                pub fn triple_scalar_product(a: &Self, b: &Self, c: &Self) -> $t {
                    a.dot(&b.cross(c))
                }
            }
        )*
    };
}

impl_float_ops!(f32, f64);

// Indexing operations
impl<T, const N: usize> Index<usize> for Vec<T, N> {
    type Output = T;

    fn index(&self, index: usize) -> &Self::Output {
        &self.data[index]
    }
}

impl<T, const N: usize> IndexMut<usize> for Vec<T, N> {
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        &mut self.data[index]
    }
}

// Arithmetic operations
impl<T: Add<Output = T> + Copy, const N: usize> Add for Vec<T, N> {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        let mut result = self;
        for i in 0..N {
            result.data[i] = self.data[i] + rhs.data[i];
        }
        result
    }
}

impl<T: Add<Output = T> + Copy, const N: usize> AddAssign for Vec<T, N> {
    fn add_assign(&mut self, rhs: Self) {
        *self = *self + rhs;
    }
}

impl<T: Sub<Output = T> + Copy, const N: usize> Sub for Vec<T, N> {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self::Output {
        let mut result = self;
        for i in 0..N {
            result.data[i] = self.data[i] - rhs.data[i];
        }
        result
    }
}

impl<T: Sub<Output = T> + Copy, const N: usize> SubAssign for Vec<T, N> {
    fn sub_assign(&mut self, rhs: Self) {
        *self = *self - rhs;
    }
}

impl<T: Mul<Output = T> + Copy, const N: usize> Mul<T> for Vec<T, N> {
    type Output = Self;

    fn mul(self, rhs: T) -> Self::Output {
        let mut result = self;
        for i in 0..N {
            result.data[i] = self.data[i] * rhs;
        }
        result
    }
}

impl<T: Mul<Output = T> + Copy, const N: usize> MulAssign<T> for Vec<T, N> {
    fn mul_assign(&mut self, rhs: T) {
        *self = *self * rhs;
    }
}

impl<T: Div<Output = T> + Copy, const N: usize> Div<T> for Vec<T, N> {
    type Output = Self;

    fn div(self, rhs: T) -> Self::Output {
        let mut result = self;
        for i in 0..N {
            result.data[i] = self.data[i] / rhs;
        }
        result
    }
}

impl<T: Div<Output = T> + Copy, const N: usize> DivAssign<T> for Vec<T, N> {
    fn div_assign(&mut self, rhs: T) {
        *self = *self / rhs;
    }
}

impl<T: Neg<Output = T> + Copy, const N: usize> Neg for Vec<T, N> {
    type Output = Self;

    fn neg(self) -> Self::Output {
        let mut result = self;
        for i in 0..N {
            result.data[i] = -self.data[i];
        }
        result
    }
}

// Display implementation
impl<T: fmt::Display, const N: usize> fmt::Display for Vec<T, N> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "(")?;
        for (i, val) in self.data.iter().enumerate() {
            if i > 0 {
                write!(f, ", ")?;
            }
            write!(f, "{}", val)?;
        }
        write!(f, ")")
    }
}

impl<T: fmt::Debug, const N: usize> fmt::Debug for Vec<T, N> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_tuple("Vec").field(&self.data).finish()
    }
}

// From array
impl<T: Copy, const N: usize> From<[T; N]> for Vec<T, N> {
    fn from(data: [T; N]) -> Self {
        Self { data }
    }
}

// Into array
impl<T: Copy, const N: usize> From<Vec<T, N>> for [T; N] {
    fn from(vec: Vec<T, N>) -> Self {
        vec.data
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_vec2_creation() {
        let v = Vec2F::new(1.0, 2.0);
        assert_eq!(v.x(), 1.0);
        assert_eq!(v.y(), 2.0);
    }

    #[test]
    fn test_vec3_creation() {
        let v = Vec3F::new(1.0, 2.0, 3.0);
        assert_eq!(v.x(), 1.0);
        assert_eq!(v.y(), 2.0);
        assert_eq!(v.z(), 3.0);
    }

    #[test]
    fn test_vec4_creation() {
        let v = Vec4F::new(1.0, 2.0, 3.0, 4.0);
        assert_eq!(v.x(), 1.0);
        assert_eq!(v.y(), 2.0);
        assert_eq!(v.z(), 3.0);
        assert_eq!(v.w(), 4.0);
    }

    #[test]
    fn test_vec2_operations() {
        let v1 = Vec2F::new(1.0, 2.0);
        let v2 = Vec2F::new(3.0, 4.0);

        // Addition
        let sum = v1 + v2;
        assert_eq!(sum.x(), 4.0);
        assert_eq!(sum.y(), 6.0);

        // Subtraction
        let diff = v2 - v1;
        assert_eq!(diff.x(), 2.0);
        assert_eq!(diff.y(), 2.0);

        // Scalar multiplication
        let scaled = v1 * 2.0;
        assert_eq!(scaled.x(), 2.0);
        assert_eq!(scaled.y(), 4.0);
    }

    #[test]
    fn test_vec2_magnitude() {
        let v = Vec2F::new(3.0, 4.0);
        assert!((v.magnitude() - 5.0).abs() < 1e-6);
    }

    #[test]
    fn test_vec2_normalize() {
        let v = Vec2F::new(3.0, 4.0);
        let n = v.normalized();
        assert!((n.magnitude() - 1.0).abs() < 1e-6);
    }

    #[test]
    fn test_vec2_dot() {
        let v1 = Vec2F::new(1.0, 2.0);
        let v2 = Vec2F::new(3.0, 4.0);
        assert!((v1.dot(&v2) - 11.0).abs() < 1e-6);
    }

    #[test]
    fn test_vec2_cross() {
        let v1 = Vec2F::new(1.0, 0.0);
        let v2 = Vec2F::new(0.0, 1.0);
        assert!((v1.cross(&v2) - 1.0).abs() < 1e-6);
    }

    #[test]
    fn test_vec3_cross() {
        let v1 = Vec3F::new(1.0, 0.0, 0.0);
        let v2 = Vec3F::new(0.0, 1.0, 0.0);
        let cross = v1.cross(&v2);
        assert!((cross.z() - 1.0).abs() < 1e-6);
    }

    #[test]
    fn test_vec2_rotate() {
        let v = Vec2F::new(1.0, 0.0);
        let rotated = v.rotate(std::f32::consts::FRAC_PI_2);
        assert!(rotated.x().abs() < 1e-6);
        assert!((rotated.y() - 1.0).abs() < 1e-6);
    }

    #[test]
    fn test_display() {
        let v = Vec2F::new(1.0, 2.0);
        assert_eq!(format!("{}", v), "(1, 2)");
    }

    #[test]
    fn test_index() {
        let v = Vec3F::new(1.0, 2.0, 3.0);
        assert_eq!(v[0], 1.0);
        assert_eq!(v[1], 2.0);
        assert_eq!(v[2], 3.0);
    }

    #[test]
    fn test_from_array() {
        let v: Vec3F = [1.0, 2.0, 3.0].into();
        assert_eq!(v.x(), 1.0);
        assert_eq!(v.y(), 2.0);
        assert_eq!(v.z(), 3.0);
    }

    #[test]
    fn test_piecewise_operations() {
        let v1 = Vec2F::new(1.0, 4.0);
        let v2 = Vec2F::new(3.0, 2.0);

        let min = v1.piecewise_min(&v2);
        assert_eq!(min.x(), 1.0);
        assert_eq!(min.y(), 2.0);

        let max = v1.piecewise_max(&v2);
        assert_eq!(max.x(), 3.0);
        assert_eq!(max.y(), 4.0);
    }
}
