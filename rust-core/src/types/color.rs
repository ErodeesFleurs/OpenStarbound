//! Color type compatible with C++ Star::Color
//!
//! This module provides a Color type that matches the binary layout
//! and functionality of the C++ implementation.

use crate::math::{Vec3, Vec3F, Vec4, Vec4B, Vec4F};
use serde::{Deserialize, Serialize};
use std::fmt;
use std::str::FromStr;

/// RGBA Color stored as floating point values [0.0, 1.0]
#[derive(Clone, Copy, PartialEq, Serialize, Deserialize)]
#[repr(C)]
pub struct Color {
    data: Vec4F,
}

impl Default for Color {
    fn default() -> Self {
        Self::BLACK
    }
}

impl Color {
    // Named color constants matching C++
    pub const RED: Color = Color { data: Vec4F { data: [1.0, 0.0, 0.0, 1.0] } };
    pub const ORANGE: Color = Color { data: Vec4F { data: [1.0, 0.5, 0.0, 1.0] } };
    pub const YELLOW: Color = Color { data: Vec4F { data: [1.0, 1.0, 0.0, 1.0] } };
    pub const GREEN: Color = Color { data: Vec4F { data: [0.0, 1.0, 0.0, 1.0] } };
    pub const BLUE: Color = Color { data: Vec4F { data: [0.0, 0.0, 1.0, 1.0] } };
    pub const INDIGO: Color = Color { data: Vec4F { data: [0.29, 0.0, 0.51, 1.0] } };
    pub const VIOLET: Color = Color { data: Vec4F { data: [0.93, 0.51, 0.93, 1.0] } };
    pub const BLACK: Color = Color { data: Vec4F { data: [0.0, 0.0, 0.0, 1.0] } };
    pub const WHITE: Color = Color { data: Vec4F { data: [1.0, 1.0, 1.0, 1.0] } };
    pub const MAGENTA: Color = Color { data: Vec4F { data: [1.0, 0.0, 1.0, 1.0] } };
    pub const DARK_MAGENTA: Color = Color { data: Vec4F { data: [0.55, 0.0, 0.55, 1.0] } };
    pub const CYAN: Color = Color { data: Vec4F { data: [0.0, 1.0, 1.0, 1.0] } };
    pub const DARK_CYAN: Color = Color { data: Vec4F { data: [0.0, 0.55, 0.55, 1.0] } };
    pub const CORNFLOWER_BLUE: Color = Color { data: Vec4F { data: [0.39, 0.58, 0.93, 1.0] } };
    pub const GRAY: Color = Color { data: Vec4F { data: [0.5, 0.5, 0.5, 1.0] } };
    pub const LIGHT_GRAY: Color = Color { data: Vec4F { data: [0.75, 0.75, 0.75, 1.0] } };
    pub const DARK_GRAY: Color = Color { data: Vec4F { data: [0.25, 0.25, 0.25, 1.0] } };
    pub const DARK_GREEN: Color = Color { data: Vec4F { data: [0.0, 0.39, 0.0, 1.0] } };
    pub const PINK: Color = Color { data: Vec4F { data: [1.0, 0.75, 0.8, 1.0] } };
    pub const CLEAR: Color = Color { data: Vec4F { data: [0.0, 0.0, 0.0, 0.0] } };

    /// Create a color from RGBA float values [0.0, 1.0]
    pub const fn from_rgba_f32(r: f32, g: f32, b: f32, a: f32) -> Self {
        Self {
            data: Vec4F { data: [r, g, b, a] },
        }
    }

    /// Create a color from RGB float values [0.0, 1.0], with alpha = 1.0
    pub const fn from_rgb_f32(r: f32, g: f32, b: f32) -> Self {
        Self::from_rgba_f32(r, g, b, 1.0)
    }

    /// Create a color from RGBA byte values [0, 255]
    pub fn from_rgba_u8(r: u8, g: u8, b: u8, a: u8) -> Self {
        Self::from_rgba_f32(
            r as f32 / 255.0,
            g as f32 / 255.0,
            b as f32 / 255.0,
            a as f32 / 255.0,
        )
    }

    /// Create a color from RGB byte values [0, 255], with alpha = 255
    pub fn from_rgb_u8(r: u8, g: u8, b: u8) -> Self {
        Self::from_rgba_u8(r, g, b, 255)
    }

    /// Create a color from Vec4B
    pub fn from_vec4b(v: Vec4B) -> Self {
        Self::from_rgba_u8(v.x(), v.y(), v.z(), v.w())
    }

    /// Create a color from Vec4F
    pub fn from_vec4f(v: Vec4F) -> Self {
        Self::from_rgba_f32(v.x(), v.y(), v.z(), v.w())
    }

    /// Create a color from Vec3F with alpha = 1.0
    pub fn from_vec3f(v: Vec3F) -> Self {
        Self::from_rgb_f32(v.x(), v.y(), v.z())
    }

    /// Create a grayscale color from float [0.0, 1.0]
    pub fn gray_f32(g: f32) -> Self {
        Self::from_rgb_f32(g, g, g)
    }

    /// Create a grayscale color from byte [0, 255]
    pub fn gray_u8(g: u8) -> Self {
        Self::from_rgb_u8(g, g, g)
    }

    /// Create a color from HSV values
    /// h: hue [0.0, 1.0], s: saturation [0.0, 1.0], v: value [0.0, 1.0]
    pub fn from_hsv(h: f32, s: f32, v: f32) -> Self {
        Self::from_hsva(h, s, v, 1.0)
    }

    /// Create a color from HSVA values
    pub fn from_hsva(h: f32, s: f32, v: f32, a: f32) -> Self {
        let h = h * 6.0;
        let hi = h.floor() as i32 % 6;
        let f = h - h.floor();
        let p = v * (1.0 - s);
        let q = v * (1.0 - f * s);
        let t = v * (1.0 - (1.0 - f) * s);

        let (r, g, b) = match hi {
            0 => (v, t, p),
            1 => (q, v, p),
            2 => (p, v, t),
            3 => (p, q, v),
            4 => (t, p, v),
            _ => (v, p, q),
        };

        Self::from_rgba_f32(r, g, b, a)
    }

    /// Parse a color from hex string (supports #RGB, #RGBA, #RRGGBB, #RRGGBBAA)
    pub fn from_hex(s: &str) -> Result<Self, crate::error::Error> {
        let s = s.trim_start_matches('#');

        let (r, g, b, a) = match s.len() {
            3 => {
                let r = u8::from_str_radix(&s[0..1], 16).map_err(|e| crate::error::Error::Color(e.to_string()))? * 17;
                let g = u8::from_str_radix(&s[1..2], 16).map_err(|e| crate::error::Error::Color(e.to_string()))? * 17;
                let b = u8::from_str_radix(&s[2..3], 16).map_err(|e| crate::error::Error::Color(e.to_string()))? * 17;
                (r, g, b, 255u8)
            }
            4 => {
                let r = u8::from_str_radix(&s[0..1], 16).map_err(|e| crate::error::Error::Color(e.to_string()))? * 17;
                let g = u8::from_str_radix(&s[1..2], 16).map_err(|e| crate::error::Error::Color(e.to_string()))? * 17;
                let b = u8::from_str_radix(&s[2..3], 16).map_err(|e| crate::error::Error::Color(e.to_string()))? * 17;
                let a = u8::from_str_radix(&s[3..4], 16).map_err(|e| crate::error::Error::Color(e.to_string()))? * 17;
                (r, g, b, a)
            }
            6 => {
                let r = u8::from_str_radix(&s[0..2], 16).map_err(|e| crate::error::Error::Color(e.to_string()))?;
                let g = u8::from_str_radix(&s[2..4], 16).map_err(|e| crate::error::Error::Color(e.to_string()))?;
                let b = u8::from_str_radix(&s[4..6], 16).map_err(|e| crate::error::Error::Color(e.to_string()))?;
                (r, g, b, 255u8)
            }
            8 => {
                let r = u8::from_str_radix(&s[0..2], 16).map_err(|e| crate::error::Error::Color(e.to_string()))?;
                let g = u8::from_str_radix(&s[2..4], 16).map_err(|e| crate::error::Error::Color(e.to_string()))?;
                let b = u8::from_str_radix(&s[4..6], 16).map_err(|e| crate::error::Error::Color(e.to_string()))?;
                let a = u8::from_str_radix(&s[6..8], 16).map_err(|e| crate::error::Error::Color(e.to_string()))?;
                (r, g, b, a)
            }
            _ => {
                return Err(crate::error::Error::Color(format!(
                    "Invalid hex color length: {}",
                    s.len()
                )));
            }
        };

        Ok(Self::from_rgba_u8(r, g, b, a))
    }

    /// Create a color from a 32-bit unsigned integer (AARRGGBB format)
    pub fn from_uint32(v: u32) -> Self {
        let a = ((v >> 24) & 0xFF) as u8;
        let r = ((v >> 16) & 0xFF) as u8;
        let g = ((v >> 8) & 0xFF) as u8;
        let b = (v & 0xFF) as u8;
        Self::from_rgba_u8(r, g, b, a)
    }

    /// Create a color from temperature in Kelvin
    pub fn from_temperature(temp: f32) -> Self {
        let temp = temp.clamp(1000.0, 40000.0) / 100.0;

        let r = if temp <= 66.0 {
            255.0
        } else {
            let r = temp - 60.0;
            (329.698727446 * r.powf(-0.1332047592)).clamp(0.0, 255.0)
        };

        let g = if temp <= 66.0 {
            let g = temp;
            (99.4708025861 * g.ln() - 161.1195681661).clamp(0.0, 255.0)
        } else {
            let g = temp - 60.0;
            (288.1221695283 * g.powf(-0.0755148492)).clamp(0.0, 255.0)
        };

        let b = if temp >= 66.0 {
            255.0
        } else if temp <= 19.0 {
            0.0
        } else {
            let b = temp - 10.0;
            (138.5177312231 * b.ln() - 305.0447927307).clamp(0.0, 255.0)
        };

        Self::from_rgb_u8(r as u8, g as u8, b as u8)
    }

    // Getters as bytes
    pub fn red(&self) -> u8 {
        (self.data.x() * 255.0).clamp(0.0, 255.0) as u8
    }

    pub fn green(&self) -> u8 {
        (self.data.y() * 255.0).clamp(0.0, 255.0) as u8
    }

    pub fn blue(&self) -> u8 {
        (self.data.z() * 255.0).clamp(0.0, 255.0) as u8
    }

    pub fn alpha(&self) -> u8 {
        (self.data.w() * 255.0).clamp(0.0, 255.0) as u8
    }

    // Setters as bytes
    pub fn set_red(&mut self, r: u8) {
        self.data.set_x(r as f32 / 255.0);
    }

    pub fn set_green(&mut self, g: u8) {
        self.data.set_y(g as f32 / 255.0);
    }

    pub fn set_blue(&mut self, b: u8) {
        self.data.set_z(b as f32 / 255.0);
    }

    pub fn set_alpha(&mut self, a: u8) {
        self.data.set_w(a as f32 / 255.0);
    }

    // Getters as floats
    pub fn red_f(&self) -> f32 {
        self.data.x()
    }

    pub fn green_f(&self) -> f32 {
        self.data.y()
    }

    pub fn blue_f(&self) -> f32 {
        self.data.z()
    }

    pub fn alpha_f(&self) -> f32 {
        self.data.w()
    }

    // Setters as floats
    pub fn set_red_f(&mut self, r: f32) {
        self.data.set_x(r);
    }

    pub fn set_green_f(&mut self, g: f32) {
        self.data.set_y(g);
    }

    pub fn set_blue_f(&mut self, b: f32) {
        self.data.set_z(b);
    }

    pub fn set_alpha_f(&mut self, a: f32) {
        self.data.set_w(a);
    }

    /// Check if the color is fully transparent
    pub fn is_clear(&self) -> bool {
        self.alpha_f() == 0.0
    }

    /// Convert to 32-bit unsigned integer (AARRGGBB format)
    pub fn to_uint32(&self) -> u32 {
        ((self.alpha() as u32) << 24)
            | ((self.red() as u32) << 16)
            | ((self.green() as u32) << 8)
            | (self.blue() as u32)
    }

    /// Convert to Vec4B (RGBA bytes)
    pub fn to_rgba(&self) -> Vec4B {
        Vec4B::new(self.red(), self.green(), self.blue(), self.alpha())
    }

    /// Convert to Vec3B (RGB bytes)
    pub fn to_rgb(&self) -> Vec3<u8> {
        Vec3::new(self.red(), self.green(), self.blue())
    }

    /// Convert to Vec4F (RGBA floats)
    pub fn to_rgba_f(&self) -> Vec4F {
        self.data
    }

    /// Convert to Vec3F (RGB floats)
    pub fn to_rgb_f(&self) -> Vec3F {
        self.data.vec3()
    }

    /// Get the internal data
    pub fn data(&self) -> &Vec4F {
        &self.data
    }

    /// Convert to HSVA
    pub fn to_hsva(&self) -> Vec4F {
        let r = self.red_f();
        let g = self.green_f();
        let b = self.blue_f();

        let max = r.max(g).max(b);
        let min = r.min(g).min(b);
        let delta = max - min;

        let v = max;
        let s = if max == 0.0 { 0.0 } else { delta / max };

        let h = if delta == 0.0 {
            0.0
        } else if max == r {
            ((g - b) / delta).rem_euclid(6.0) / 6.0
        } else if max == g {
            ((b - r) / delta + 2.0) / 6.0
        } else {
            ((r - g) / delta + 4.0) / 6.0
        };

        Vec4F::new(h, s, v, self.alpha_f())
    }

    /// Get hue [0.0, 1.0]
    pub fn hue(&self) -> f32 {
        self.to_hsva().x()
    }

    /// Get saturation [0.0, 1.0]
    pub fn saturation(&self) -> f32 {
        self.to_hsva().y()
    }

    /// Get value [0.0, 1.0]
    pub fn value(&self) -> f32 {
        self.to_hsva().z()
    }

    /// Set hue
    pub fn set_hue(&mut self, hue: f32) {
        let hsva = self.to_hsva();
        *self = Self::from_hsva(hue, hsva.y(), hsva.z(), hsva.w());
    }

    /// Set saturation
    pub fn set_saturation(&mut self, saturation: f32) {
        let hsva = self.to_hsva();
        *self = Self::from_hsva(hsva.x(), saturation, hsva.z(), hsva.w());
    }

    /// Set value
    pub fn set_value(&mut self, value: f32) {
        let hsva = self.to_hsva();
        *self = Self::from_hsva(hsva.x(), hsva.y(), value, hsva.w());
    }

    /// Shift hue by the given amount (wraps around)
    pub fn hue_shift(&mut self, shift: f32) {
        let hsva = self.to_hsva();
        *self = Self::from_hsva((hsva.x() + shift).rem_euclid(1.0), hsva.y(), hsva.z(), hsva.w());
    }

    /// Fade toward black by the given amount [0.0, 1.0]
    pub fn fade(&mut self, amount: f32) {
        self.data.set_x(self.data.x() * (1.0 - amount));
        self.data.set_y(self.data.y() * (1.0 - amount));
        self.data.set_z(self.data.z() * (1.0 - amount));
    }

    /// Convert to hex string
    pub fn to_hex(&self) -> String {
        if self.alpha() == 255 {
            format!("{:02X}{:02X}{:02X}", self.red(), self.green(), self.blue())
        } else {
            format!(
                "{:02X}{:02X}{:02X}{:02X}",
                self.red(),
                self.green(),
                self.blue(),
                self.alpha()
            )
        }
    }

    /// Convert to linear color space
    pub fn to_linear(&self) -> Self {
        Self::from_rgba_f32(
            Self::srgb_to_linear(self.red_f()),
            Self::srgb_to_linear(self.green_f()),
            Self::srgb_to_linear(self.blue_f()),
            self.alpha_f(),
        )
    }

    /// Convert to sRGB color space
    pub fn to_srgb(&self) -> Self {
        Self::from_rgba_f32(
            Self::linear_to_srgb(self.red_f()),
            Self::linear_to_srgb(self.green_f()),
            Self::linear_to_srgb(self.blue_f()),
            self.alpha_f(),
        )
    }

    /// Get contrasting color (black or white)
    pub fn contrasting(&self) -> Self {
        let luminance = 0.299 * self.red_f() + 0.587 * self.green_f() + 0.114 * self.blue_f();
        if luminance > 0.5 {
            Self::BLACK
        } else {
            Self::WHITE
        }
    }

    /// Get complementary color
    pub fn complementary(&self) -> Self {
        let mut result = *self;
        result.hue_shift(0.5);
        result
    }

    /// Mix with another color
    pub fn mix(&self, other: &Self, amount: f32) -> Self {
        let amount = amount.clamp(0.0, 1.0);
        Self::from_rgba_f32(
            self.red_f() + (other.red_f() - self.red_f()) * amount,
            self.green_f() + (other.green_f() - self.green_f()) * amount,
            self.blue_f() + (other.blue_f() - self.blue_f()) * amount,
            self.alpha_f() + (other.alpha_f() - self.alpha_f()) * amount,
        )
    }

    /// Multiply color intensity
    pub fn multiply(&self, amount: f32) -> Self {
        Self::from_rgba_f32(
            (self.red_f() * amount).clamp(0.0, 1.0),
            (self.green_f() * amount).clamp(0.0, 1.0),
            (self.blue_f() * amount).clamp(0.0, 1.0),
            self.alpha_f(),
        )
    }

    // Helper functions for color space conversion
    fn srgb_to_linear(value: f32) -> f32 {
        if value <= 0.04045 {
            value / 12.92
        } else {
            ((value + 0.055) / 1.055).powf(2.4)
        }
    }

    fn linear_to_srgb(value: f32) -> f32 {
        if value <= 0.0031308 {
            value * 12.92
        } else {
            1.055 * value.powf(1.0 / 2.4) - 0.055
        }
    }
}

impl std::ops::Add for Color {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        Self::from_rgba_f32(
            (self.red_f() + rhs.red_f()).clamp(0.0, 1.0),
            (self.green_f() + rhs.green_f()).clamp(0.0, 1.0),
            (self.blue_f() + rhs.blue_f()).clamp(0.0, 1.0),
            (self.alpha_f() + rhs.alpha_f()).clamp(0.0, 1.0),
        )
    }
}

impl std::ops::AddAssign for Color {
    fn add_assign(&mut self, rhs: Self) {
        *self = *self + rhs;
    }
}

impl std::ops::Mul for Color {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        Self::from_rgba_f32(
            self.red_f() * rhs.red_f(),
            self.green_f() * rhs.green_f(),
            self.blue_f() * rhs.blue_f(),
            self.alpha_f() * rhs.alpha_f(),
        )
    }
}

impl std::ops::MulAssign for Color {
    fn mul_assign(&mut self, rhs: Self) {
        *self = *self * rhs;
    }
}

impl fmt::Display for Color {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "#{}", self.to_hex())
    }
}

impl fmt::Debug for Color {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("Color")
            .field("r", &self.red())
            .field("g", &self.green())
            .field("b", &self.blue())
            .field("a", &self.alpha())
            .finish()
    }
}

impl FromStr for Color {
    type Err = crate::error::Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        // Try hex first
        if s.starts_with('#') || s.chars().all(|c| c.is_ascii_hexdigit()) {
            return Self::from_hex(s);
        }

        // Try named colors
        match s.to_lowercase().as_str() {
            "red" => Ok(Self::RED),
            "orange" => Ok(Self::ORANGE),
            "yellow" => Ok(Self::YELLOW),
            "green" => Ok(Self::GREEN),
            "blue" => Ok(Self::BLUE),
            "indigo" => Ok(Self::INDIGO),
            "violet" => Ok(Self::VIOLET),
            "black" => Ok(Self::BLACK),
            "white" => Ok(Self::WHITE),
            "magenta" => Ok(Self::MAGENTA),
            "cyan" => Ok(Self::CYAN),
            "gray" | "grey" => Ok(Self::GRAY),
            "pink" => Ok(Self::PINK),
            "clear" | "transparent" => Ok(Self::CLEAR),
            _ => Err(crate::error::Error::Color(format!(
                "Unknown color: {}",
                s
            ))),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_color_from_rgba() {
        let c = Color::from_rgba_u8(255, 128, 64, 200);
        assert_eq!(c.red(), 255);
        assert_eq!(c.green(), 128);
        assert_eq!(c.blue(), 64);
        assert_eq!(c.alpha(), 200);
    }

    #[test]
    fn test_color_from_hex() {
        let c = Color::from_hex("#FF8040").unwrap();
        assert_eq!(c.red(), 255);
        assert_eq!(c.green(), 128);
        assert_eq!(c.blue(), 64);
        assert_eq!(c.alpha(), 255);

        let c = Color::from_hex("FF8040C8").unwrap();
        assert_eq!(c.red(), 255);
        assert_eq!(c.green(), 128);
        assert_eq!(c.blue(), 64);
        assert_eq!(c.alpha(), 200);
    }

    #[test]
    fn test_color_to_hex() {
        let c = Color::from_rgb_u8(255, 128, 64);
        assert_eq!(c.to_hex(), "FF8040");

        let c = Color::from_rgba_u8(255, 128, 64, 200);
        assert_eq!(c.to_hex(), "FF8040C8");
    }

    #[test]
    fn test_color_hsv() {
        let c = Color::from_hsv(0.0, 1.0, 1.0);
        assert!((c.red_f() - 1.0).abs() < 0.01);
        assert!(c.green_f() < 0.01);
        assert!(c.blue_f() < 0.01);
    }

    #[test]
    fn test_color_mix() {
        let c1 = Color::BLACK;
        let c2 = Color::WHITE;
        let mixed = c1.mix(&c2, 0.5);
        assert!((mixed.red_f() - 0.5).abs() < 0.01);
        assert!((mixed.green_f() - 0.5).abs() < 0.01);
        assert!((mixed.blue_f() - 0.5).abs() < 0.01);
    }

    #[test]
    fn test_named_colors() {
        assert_eq!(Color::RED.red(), 255);
        assert_eq!(Color::RED.green(), 0);
        assert_eq!(Color::RED.blue(), 0);

        assert_eq!(Color::WHITE.red(), 255);
        assert_eq!(Color::WHITE.green(), 255);
        assert_eq!(Color::WHITE.blue(), 255);

        assert_eq!(Color::BLACK.red(), 0);
        assert_eq!(Color::BLACK.green(), 0);
        assert_eq!(Color::BLACK.blue(), 0);
    }

    #[test]
    fn test_from_string() {
        let c: Color = "red".parse().unwrap();
        assert_eq!(c, Color::RED);

        let c: Color = "#FF0000".parse().unwrap();
        assert_eq!(c.red(), 255);
        assert_eq!(c.green(), 0);
        assert_eq!(c.blue(), 0);
    }

    #[test]
    fn test_contrasting() {
        assert_eq!(Color::WHITE.contrasting(), Color::BLACK);
        assert_eq!(Color::BLACK.contrasting(), Color::WHITE);
    }
}
