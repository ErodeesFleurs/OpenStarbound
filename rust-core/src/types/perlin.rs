//! Perlin noise generation compatible with C++ Star::Perlin
//!
//! This module provides coherent noise generation using Perlin noise
//! with multiple noise types (standard, billow, ridged multi).

use super::random::RandomSource;
use serde::{Deserialize, Serialize};

/// Sample size for Perlin noise tables
const PERLIN_SAMPLE_SIZE: usize = 512;

/// Type of Perlin noise
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum PerlinType {
    Uninitialized,
    Perlin,
    Billow,
    RidgedMulti,
}

impl Default for PerlinType {
    fn default() -> Self {
        PerlinType::Uninitialized
    }
}

/// Perlin noise generator compatible with C++ Star::Perlin
#[derive(Clone, Serialize, Deserialize)]
pub struct Perlin {
    noise_type: PerlinType,
    seed: u64,
    octaves: u32,
    frequency: f64,
    amplitude: f64,
    bias: f64,
    alpha: f64,
    beta: f64,
    offset: f64,
    gain: f64,
    
    // Permutation and gradient tables
    p: Vec<i32>,
    g1: Vec<f64>,
    g2: Vec<[f64; 2]>,
    g3: Vec<[f64; 3]>,
}

impl Default for Perlin {
    fn default() -> Self {
        Self {
            noise_type: PerlinType::Uninitialized,
            seed: 0,
            octaves: 0,
            frequency: 0.0,
            amplitude: 0.0,
            bias: 0.0,
            alpha: 0.0,
            beta: 0.0,
            offset: 0.0,
            gain: 0.0,
            p: Vec::new(),
            g1: Vec::new(),
            g2: Vec::new(),
            g3: Vec::new(),
        }
    }
}

impl Perlin {
    /// Create a new Perlin noise generator
    pub fn new(octaves: u32, freq: f64, amp: f64, bias: f64, alpha: f64, beta: f64, seed: u64) -> Self {
        Self::with_type(PerlinType::Perlin, octaves, freq, amp, bias, alpha, beta, seed)
    }

    /// Create a new Perlin noise generator with specified type
    pub fn with_type(
        noise_type: PerlinType,
        octaves: u32,
        freq: f64,
        amp: f64,
        bias: f64,
        alpha: f64,
        beta: f64,
        seed: u64,
    ) -> Self {
        let mut perlin = Self {
            noise_type,
            seed,
            octaves,
            frequency: freq,
            amplitude: amp,
            bias,
            alpha,
            beta,
            offset: 1.0,
            gain: 2.0,
            p: Vec::new(),
            g1: Vec::new(),
            g2: Vec::new(),
            g3: Vec::new(),
        };
        perlin.init_tables(seed);
        perlin
    }

    /// Initialize the noise tables
    fn init_tables(&mut self, seed: u64) {
        let mut random = RandomSource::with_seed(seed);
        let size = PERLIN_SAMPLE_SIZE + PERLIN_SAMPLE_SIZE + 2;

        self.p = vec![0; size];
        self.g1 = vec![0.0; size];
        self.g2 = vec![[0.0; 2]; size];
        self.g3 = vec![[0.0; 3]; size];

        // Initialize gradients
        for i in 0..PERLIN_SAMPLE_SIZE {
            self.p[i] = i as i32;
            
            self.g1[i] = random.rand_int_range(-(PERLIN_SAMPLE_SIZE as i64), PERLIN_SAMPLE_SIZE as i64) as f64
                / PERLIN_SAMPLE_SIZE as f64;

            for j in 0..2 {
                self.g2[i][j] = random.rand_int_range(-(PERLIN_SAMPLE_SIZE as i64), PERLIN_SAMPLE_SIZE as i64) as f64
                    / PERLIN_SAMPLE_SIZE as f64;
            }
            Self::normalize2(&mut self.g2[i]);

            for j in 0..3 {
                self.g3[i][j] = random.rand_int_range(-(PERLIN_SAMPLE_SIZE as i64), PERLIN_SAMPLE_SIZE as i64) as f64
                    / PERLIN_SAMPLE_SIZE as f64;
            }
            Self::normalize3(&mut self.g3[i]);
        }

        // Shuffle permutation table
        for i in (1..PERLIN_SAMPLE_SIZE).rev() {
            let j = random.rand_uint(i as u64) as usize;
            self.p.swap(i, j);
        }

        // Duplicate tables for wrapping
        for i in 0..(PERLIN_SAMPLE_SIZE + 2) {
            self.p[PERLIN_SAMPLE_SIZE + i] = self.p[i];
            self.g1[PERLIN_SAMPLE_SIZE + i] = self.g1[i];
            self.g2[PERLIN_SAMPLE_SIZE + i] = self.g2[i];
            self.g3[PERLIN_SAMPLE_SIZE + i] = self.g3[i];
        }
    }

    fn normalize2(v: &mut [f64; 2]) {
        let s = (v[0] * v[0] + v[1] * v[1]).sqrt();
        if s.abs() < f64::EPSILON {
            v[0] = 1.0;
            v[1] = 0.0;
        } else {
            v[0] /= s;
            v[1] /= s;
        }
    }

    fn normalize3(v: &mut [f64; 3]) {
        let s = (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]).sqrt();
        if s.abs() < f64::EPSILON {
            v[0] = 1.0;
            v[1] = 0.0;
            v[2] = 0.0;
        } else {
            v[0] /= s;
            v[1] /= s;
            v[2] /= s;
        }
    }

    fn s_curve(t: f64) -> f64 {
        t * t * (3.0 - 2.0 * t)
    }

    fn lerp(t: f64, a: f64, b: f64) -> f64 {
        a + t * (b - a)
    }

    fn setup(v: f64) -> (i32, i32, f64, f64) {
        let iv = v.floor() as i32;
        let fv = v - iv as f64;
        let b0 = iv & (PERLIN_SAMPLE_SIZE as i32 - 1);
        let b1 = (iv + 1) & (PERLIN_SAMPLE_SIZE as i32 - 1);
        (b0, b1, fv, fv - 1.0)
    }

    fn at2(q: [f64; 2], rx: f64, ry: f64) -> f64 {
        rx * q[0] + ry * q[1]
    }

    fn at3(q: [f64; 3], rx: f64, ry: f64, rz: f64) -> f64 {
        rx * q[0] + ry * q[1] + rz * q[2]
    }

    fn noise1(&self, arg: f64) -> f64 {
        let (bx0, bx1, rx0, rx1) = Self::setup(arg);
        let sx = Self::s_curve(rx0);
        let u = rx0 * self.g1[self.p[bx0 as usize] as usize];
        let v = rx1 * self.g1[self.p[bx1 as usize] as usize];
        Self::lerp(sx, u, v)
    }

    fn noise2(&self, x: f64, y: f64) -> f64 {
        let (bx0, bx1, rx0, rx1) = Self::setup(x);
        let (by0, by1, ry0, ry1) = Self::setup(y);

        let i = self.p[bx0 as usize];
        let j = self.p[bx1 as usize];

        let b00 = self.p[(i + by0) as usize];
        let b10 = self.p[(j + by0) as usize];
        let b01 = self.p[(i + by1) as usize];
        let b11 = self.p[(j + by1) as usize];

        let sx = Self::s_curve(rx0);
        let sy = Self::s_curve(ry0);

        let u = Self::at2(self.g2[b00 as usize], rx0, ry0);
        let v = Self::at2(self.g2[b10 as usize], rx1, ry0);
        let a = Self::lerp(sx, u, v);

        let u = Self::at2(self.g2[b01 as usize], rx0, ry1);
        let v = Self::at2(self.g2[b11 as usize], rx1, ry1);
        let b = Self::lerp(sx, u, v);

        Self::lerp(sy, a, b)
    }

    fn noise3(&self, x: f64, y: f64, z: f64) -> f64 {
        let (bx0, bx1, rx0, rx1) = Self::setup(x);
        let (by0, by1, ry0, ry1) = Self::setup(y);
        let (bz0, bz1, rz0, rz1) = Self::setup(z);

        let i = self.p[bx0 as usize];
        let j = self.p[bx1 as usize];

        let b00 = self.p[(i + by0) as usize];
        let b10 = self.p[(j + by0) as usize];
        let b01 = self.p[(i + by1) as usize];
        let b11 = self.p[(j + by1) as usize];

        let sx = Self::s_curve(rx0);
        let sy = Self::s_curve(ry0);
        let sz = Self::s_curve(rz0);

        let u = Self::at3(self.g3[(b00 + bz0) as usize], rx0, ry0, rz0);
        let v = Self::at3(self.g3[(b10 + bz0) as usize], rx1, ry0, rz0);
        let a = Self::lerp(sx, u, v);

        let u = Self::at3(self.g3[(b01 + bz0) as usize], rx0, ry1, rz0);
        let v = Self::at3(self.g3[(b11 + bz0) as usize], rx1, ry1, rz0);
        let b = Self::lerp(sx, u, v);

        let c = Self::lerp(sy, a, b);

        let u = Self::at3(self.g3[(b00 + bz1) as usize], rx0, ry0, rz1);
        let v = Self::at3(self.g3[(b10 + bz1) as usize], rx1, ry0, rz1);
        let a = Self::lerp(sx, u, v);

        let u = Self::at3(self.g3[(b01 + bz1) as usize], rx0, ry1, rz1);
        let v = Self::at3(self.g3[(b11 + bz1) as usize], rx1, ry1, rz1);
        let b = Self::lerp(sx, u, v);

        let d = Self::lerp(sy, a, b);

        Self::lerp(sz, c, d)
    }

    fn perlin1(&self, x: f64) -> f64 {
        let mut sum = 0.0;
        let mut scale = 1.0;
        let mut p = x * self.frequency;

        for _ in 0..self.octaves {
            let val = self.noise1(p);
            sum += val / scale;
            scale *= self.alpha;
            p *= self.beta;
        }

        sum * self.amplitude + self.bias
    }

    fn perlin2(&self, x: f64, y: f64) -> f64 {
        let mut sum = 0.0;
        let mut scale = 1.0;
        let mut px = x * self.frequency;
        let mut py = y * self.frequency;

        for _ in 0..self.octaves {
            let val = self.noise2(px, py);
            sum += val / scale;
            scale *= self.alpha;
            px *= self.beta;
            py *= self.beta;
        }

        sum * self.amplitude + self.bias
    }

    fn perlin3(&self, x: f64, y: f64, z: f64) -> f64 {
        let mut sum = 0.0;
        let mut scale = 1.0;
        let mut px = x * self.frequency;
        let mut py = y * self.frequency;
        let mut pz = z * self.frequency;

        for _ in 0..self.octaves {
            let val = self.noise3(px, py, pz);
            sum += val / scale;
            scale *= self.alpha;
            px *= self.beta;
            py *= self.beta;
            pz *= self.beta;
        }

        sum * self.amplitude + self.bias
    }

    fn billow1(&self, x: f64) -> f64 {
        let mut sum = 0.0;
        let mut scale = 1.0;
        let mut p = x * self.frequency;

        for _ in 0..self.octaves {
            let val = self.noise1(p);
            let val = 2.0 * val.abs() - 1.0;
            sum += val / scale;
            scale *= self.alpha;
            p *= self.beta;
        }

        (sum + 0.5) * self.amplitude + self.bias
    }

    fn billow2(&self, x: f64, y: f64) -> f64 {
        let mut sum = 0.0;
        let mut scale = 1.0;
        let mut px = x * self.frequency;
        let mut py = y * self.frequency;

        for _ in 0..self.octaves {
            let val = self.noise2(px, py);
            let val = 2.0 * val.abs() - 1.0;
            sum += val / scale;
            scale *= self.alpha;
            px *= self.beta;
            py *= self.beta;
        }

        (sum + 0.5) * self.amplitude + self.bias
    }

    fn billow3(&self, x: f64, y: f64, z: f64) -> f64 {
        let mut sum = 0.0;
        let mut scale = 1.0;
        let mut px = x * self.frequency;
        let mut py = y * self.frequency;
        let mut pz = z * self.frequency;

        for _ in 0..self.octaves {
            let val = self.noise3(px, py, pz);
            let val = 2.0 * val.abs() - 1.0;
            sum += val / scale;
            scale *= self.alpha;
            px *= self.beta;
            py *= self.beta;
            pz *= self.beta;
        }

        (sum + 0.5) * self.amplitude + self.bias
    }

    fn ridged_multi1(&self, x: f64) -> f64 {
        let mut sum = 0.0;
        let mut scale = 1.0;
        let mut weight = 1.0;
        let mut p = x * self.frequency;

        for _ in 0..self.octaves {
            let val = self.noise1(p);
            let mut val = self.offset - val.abs();
            val *= val;
            val *= weight;
            weight = (val * self.gain).clamp(0.0, 1.0);
            sum += val / scale;
            scale *= self.alpha;
            p *= self.beta;
        }

        ((sum * 1.25) - 1.0) * self.amplitude + self.bias
    }

    fn ridged_multi2(&self, x: f64, y: f64) -> f64 {
        let mut sum = 0.0;
        let mut scale = 1.0;
        let mut weight = 1.0;
        let mut px = x * self.frequency;
        let mut py = y * self.frequency;

        for _ in 0..self.octaves {
            let val = self.noise2(px, py);
            let mut val = self.offset - val.abs();
            val *= val;
            val *= weight;
            weight = (val * self.gain).clamp(0.0, 1.0);
            sum += val / scale;
            scale *= self.alpha;
            px *= self.beta;
            py *= self.beta;
        }

        ((sum * 1.25) - 1.0) * self.amplitude + self.bias
    }

    fn ridged_multi3(&self, x: f64, y: f64, z: f64) -> f64 {
        let mut sum = 0.0;
        let mut scale = 1.0;
        let mut weight = 1.0;
        let mut px = x * self.frequency;
        let mut py = y * self.frequency;
        let mut pz = z * self.frequency;

        for _ in 0..self.octaves {
            let val = self.noise3(px, py, pz);
            let mut val = self.offset - val.abs();
            val *= val;
            val *= weight;
            weight = (val * self.gain).clamp(0.0, 1.0);
            sum += val / scale;
            scale *= self.alpha;
            px *= self.beta;
            py *= self.beta;
            pz *= self.beta;
        }

        ((sum * 1.25) - 1.0) * self.amplitude + self.bias
    }

    /// Get 1D noise value
    /// Returns None if the noise generator is uninitialized
    pub fn get1(&self, x: f64) -> Option<f64> {
        match self.noise_type {
            PerlinType::Perlin => Some(self.perlin1(x)),
            PerlinType::Billow => Some(self.billow1(x)),
            PerlinType::RidgedMulti => Some(self.ridged_multi1(x)),
            PerlinType::Uninitialized => None,
        }
    }

    /// Get 2D noise value
    /// Returns None if the noise generator is uninitialized
    pub fn get2(&self, x: f64, y: f64) -> Option<f64> {
        match self.noise_type {
            PerlinType::Perlin => Some(self.perlin2(x, y)),
            PerlinType::Billow => Some(self.billow2(x, y)),
            PerlinType::RidgedMulti => Some(self.ridged_multi2(x, y)),
            PerlinType::Uninitialized => None,
        }
    }

    /// Get 3D noise value
    /// Returns None if the noise generator is uninitialized
    pub fn get3(&self, x: f64, y: f64, z: f64) -> Option<f64> {
        match self.noise_type {
            PerlinType::Perlin => Some(self.perlin3(x, y, z)),
            PerlinType::Billow => Some(self.billow3(x, y, z)),
            PerlinType::RidgedMulti => Some(self.ridged_multi3(x, y, z)),
            PerlinType::Uninitialized => None,
        }
    }

    /// Check if the noise generator is initialized
    pub fn is_initialized(&self) -> bool {
        self.noise_type != PerlinType::Uninitialized
    }

    /// Get the noise type
    pub fn noise_type(&self) -> PerlinType {
        self.noise_type
    }

    /// Get the number of octaves
    pub fn octaves(&self) -> u32 {
        self.octaves
    }

    /// Get the frequency
    pub fn frequency(&self) -> f64 {
        self.frequency
    }

    /// Get the amplitude
    pub fn amplitude(&self) -> f64 {
        self.amplitude
    }

    /// Get the bias
    pub fn bias(&self) -> f64 {
        self.bias
    }

    /// Get the alpha (lacunarity)
    pub fn alpha(&self) -> f64 {
        self.alpha
    }

    /// Get the beta (persistence)
    pub fn beta(&self) -> f64 {
        self.beta
    }
}

/// Type alias for f32 Perlin (uses f64 internally but returns f32)
pub struct PerlinF(Perlin);

impl PerlinF {
    pub fn new(octaves: u32, freq: f32, amp: f32, bias: f32, alpha: f32, beta: f32, seed: u64) -> Self {
        Self(Perlin::new(octaves, freq as f64, amp as f64, bias as f64, alpha as f64, beta as f64, seed))
    }

    pub fn get1(&self, x: f32) -> Option<f32> {
        self.0.get1(x as f64).map(|v| v as f32)
    }

    pub fn get2(&self, x: f32, y: f32) -> Option<f32> {
        self.0.get2(x as f64, y as f64).map(|v| v as f32)
    }

    pub fn get3(&self, x: f32, y: f32, z: f32) -> Option<f32> {
        self.0.get3(x as f64, y as f64, z as f64).map(|v| v as f32)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_perlin_deterministic() {
        let p1 = Perlin::new(4, 1.0, 1.0, 0.0, 2.0, 2.0, 12345);
        let p2 = Perlin::new(4, 1.0, 1.0, 0.0, 2.0, 2.0, 12345);
        
        assert_eq!(p1.get2(1.5, 2.5), p2.get2(1.5, 2.5));
    }

    #[test]
    fn test_perlin_different_seeds() {
        let p1 = Perlin::new(4, 1.0, 1.0, 0.0, 2.0, 2.0, 12345);
        let p2 = Perlin::new(4, 1.0, 1.0, 0.0, 2.0, 2.0, 54321);
        
        // Different seeds should produce different values
        assert_ne!(p1.get2(1.5, 2.5), p2.get2(1.5, 2.5));
    }

    #[test]
    fn test_perlin_range() {
        let p = Perlin::new(4, 1.0, 1.0, 0.0, 2.0, 2.0, 12345);
        
        // Sample many points and check they're in reasonable range
        for i in 0..100 {
            for j in 0..100 {
                let x = i as f64 * 0.1;
                let y = j as f64 * 0.1;
                let val = p.get2(x, y).unwrap();
                assert!(val > -10.0 && val < 10.0, "Value {} out of expected range", val);
            }
        }
    }

    #[test]
    fn test_perlin_continuity() {
        let p = Perlin::new(4, 1.0, 1.0, 0.0, 2.0, 2.0, 12345);
        
        // Check that nearby points have similar values (continuity)
        let v1 = p.get2(1.0, 1.0).unwrap();
        let v2 = p.get2(1.001, 1.0).unwrap();
        assert!((v1 - v2).abs() < 0.01, "Perlin noise is not continuous");
    }

    #[test]
    fn test_billow() {
        let p = Perlin::with_type(PerlinType::Billow, 4, 1.0, 1.0, 0.0, 2.0, 2.0, 12345);
        let val = p.get2(1.5, 2.5).unwrap();
        assert!(val.is_finite());
    }

    #[test]
    fn test_ridged_multi() {
        let p = Perlin::with_type(PerlinType::RidgedMulti, 4, 1.0, 1.0, 0.0, 2.0, 2.0, 12345);
        let val = p.get2(1.5, 2.5).unwrap();
        assert!(val.is_finite());
    }

    #[test]
    fn test_3d_noise() {
        let p = Perlin::new(4, 1.0, 1.0, 0.0, 2.0, 2.0, 12345);
        let val = p.get3(1.5, 2.5, 3.5).unwrap();
        assert!(val.is_finite());
    }

    #[test]
    fn test_uninitialized() {
        let p = Perlin::default();
        assert!(!p.is_initialized());
        assert!(p.get2(1.0, 1.0).is_none());
    }
}
