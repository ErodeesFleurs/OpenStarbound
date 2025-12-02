//! Random number generation compatible with C++ Star::RandomSource
//!
//! This module provides deterministic random number generation using
//! the multiply-with-carry algorithm matching the C++ implementation.

/// Size of the random state buffer
const RANDOM_BUFFER_SIZE: usize = 256;

/// Deterministic random number source using multiply-with-carry algorithm.
/// Compatible with C++ Star::RandomSource.
#[derive(Clone)]
pub struct RandomSource {
    data: [u32; RANDOM_BUFFER_SIZE],
    carry: u32,
    index: u8,
}

impl Default for RandomSource {
    fn default() -> Self {
        Self::new()
    }
}

impl RandomSource {
    /// Create a new RandomSource with a time-based seed
    pub fn new() -> Self {
        use std::time::{SystemTime, UNIX_EPOCH};
        let seed = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map(|d| d.as_nanos() as u64)
            .unwrap_or(12345);
        Self::with_seed(seed)
    }

    /// Create a new RandomSource with a specific seed
    pub fn with_seed(seed: u64) -> Self {
        let mut source = Self {
            data: [0; RANDOM_BUFFER_SIZE],
            carry: 0,
            index: 0,
        };
        source.init(seed);
        source
    }

    /// Re-initialize with a new seed
    pub fn init(&mut self, seed: u64) {
        // Initialize state using a simple LCG based on the seed
        let mut s = seed;
        for i in 0..RANDOM_BUFFER_SIZE {
            s = s.wrapping_mul(6364136223846793005).wrapping_add(1442695040888963407);
            self.data[i] = (s >> 32) as u32;
        }
        self.carry = (seed >> 32) as u32 | 1; // Ensure carry is odd
        self.index = 0;
    }

    /// Add entropy from system time
    pub fn add_entropy(&mut self) {
        use std::time::{SystemTime, UNIX_EPOCH};
        let entropy = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map(|d| d.as_nanos() as u64)
            .unwrap_or(0);
        self.add_entropy_seed(entropy);
    }

    /// Add entropy from a seed value
    pub fn add_entropy_seed(&mut self, seed: u64) {
        let idx = self.index as usize;
        self.data[idx] ^= seed as u32;
        self.data[(idx + 128) % RANDOM_BUFFER_SIZE] ^= (seed >> 32) as u32;
    }

    /// Generate a random u32
    fn gen32(&mut self) -> u32 {
        const A: u64 = 809430660;
        
        self.index = self.index.wrapping_add(1);
        let idx = self.index as usize;
        
        let t = A * self.data[idx] as u64 + self.carry as u64;
        self.carry = (t >> 32) as u32;
        let result = t as u32;
        self.data[idx] = result;
        
        result
    }

    /// Generate a random u32
    pub fn randu32(&mut self) -> u32 {
        self.gen32()
    }

    /// Generate a random u64
    pub fn randu64(&mut self) -> u64 {
        let low = self.gen32() as u64;
        let high = self.gen32() as u64;
        (high << 32) | low
    }

    /// Generate a random i32
    pub fn randi32(&mut self) -> i32 {
        self.gen32() as i32
    }

    /// Generate a random i64
    pub fn randi64(&mut self) -> i64 {
        self.randu64() as i64
    }

    /// Generate a random f32 in [0.0, 1.0]
    pub fn randf(&mut self) -> f32 {
        (self.gen32() as f64 / u32::MAX as f64) as f32
    }

    /// Generate a random f64 in [0.0, 1.0]
    pub fn randd(&mut self) -> f64 {
        self.randu64() as f64 / u64::MAX as f64
    }

    /// Generate a random integer in [0, max]
    pub fn rand_int(&mut self, max: i64) -> i64 {
        if max <= 0 {
            return 0;
        }
        (self.randu64() % (max as u64 + 1)) as i64
    }

    /// Generate a random integer in [min, max]
    pub fn rand_int_range(&mut self, min: i64, max: i64) -> i64 {
        if min >= max {
            return min;
        }
        min + self.rand_int(max - min)
    }

    /// Generate a random unsigned integer in [0, max]
    pub fn rand_uint(&mut self, max: u64) -> u64 {
        if max == 0 {
            return 0;
        }
        self.randu64() % (max + 1)
    }

    /// Generate a random unsigned integer in [min, max]
    pub fn rand_uint_range(&mut self, min: u64, max: u64) -> u64 {
        if min >= max {
            return min;
        }
        min + self.rand_uint(max - min)
    }

    /// Generate a random f32 in [min, max]
    pub fn randf_range(&mut self, min: f32, max: f32) -> f32 {
        min + self.randf() * (max - min)
    }

    /// Generate a random f64 in [min, max]
    pub fn randd_range(&mut self, min: f64, max: f64) -> f64 {
        min + self.randd() * (max - min)
    }

    /// Generate a random boolean
    pub fn randb(&mut self) -> bool {
        (self.gen32() & 1) != 0
    }

    /// Generate a random f32 from normal distribution using Box-Muller
    pub fn nrandf(&mut self, stddev: f32, mean: f32) -> f32 {
        let u1 = self.randf().max(f32::MIN_POSITIVE);
        let u2 = self.randf();
        let z = (-2.0 * u1.ln()).sqrt() * (2.0 * std::f32::consts::PI * u2).cos();
        z * stddev + mean
    }

    /// Generate a random f64 from normal distribution using Box-Muller
    pub fn nrandd(&mut self, stddev: f64, mean: f64) -> f64 {
        let u1 = self.randd().max(f64::MIN_POSITIVE);
        let u2 = self.randd();
        let z = (-2.0 * u1.ln()).sqrt() * (2.0 * std::f64::consts::PI * u2).cos();
        z * stddev + mean
    }

    /// Stochastic rounding - probabilistically round to floor or ceiling
    pub fn stochastic_round(&mut self, val: f64) -> i64 {
        let floor = val.floor() as i64;
        let frac = val - val.floor();
        if self.randd() < frac {
            floor + 1
        } else {
            floor
        }
    }

    /// Generate random bytes
    pub fn rand_bytes(&mut self, len: usize) -> Vec<u8> {
        let mut bytes = vec![0u8; len];
        self.fill_bytes(&mut bytes);
        bytes
    }

    /// Fill a buffer with random bytes
    pub fn fill_bytes(&mut self, buf: &mut [u8]) {
        for chunk in buf.chunks_mut(4) {
            let val = self.gen32();
            let bytes = val.to_le_bytes();
            for (i, byte) in chunk.iter_mut().enumerate() {
                *byte = bytes[i];
            }
        }
    }

    /// Pick a random element from a slice
    pub fn rand_from<'a, T>(&mut self, slice: &'a [T]) -> Option<&'a T> {
        if slice.is_empty() {
            None
        } else {
            let idx = self.rand_uint(slice.len() as u64 - 1) as usize;
            Some(&slice[idx])
        }
    }

    /// Shuffle a slice in place
    pub fn shuffle<T>(&mut self, slice: &mut [T]) {
        let len = slice.len();
        for i in (1..len).rev() {
            let j = self.rand_uint(i as u64) as usize;
            slice.swap(i, j);
        }
    }
}

/// Global random functions (thread-local)
pub mod random {
    use super::RandomSource;
    use std::cell::RefCell;

    thread_local! {
        static GLOBAL_RANDOM: RefCell<RandomSource> = RefCell::new(RandomSource::new());
    }

    pub fn init() {
        GLOBAL_RANDOM.with(|r| {
            *r.borrow_mut() = RandomSource::new();
        });
    }

    pub fn init_with_seed(seed: u64) {
        GLOBAL_RANDOM.with(|r| {
            r.borrow_mut().init(seed);
        });
    }

    pub fn randu32() -> u32 {
        GLOBAL_RANDOM.with(|r| r.borrow_mut().randu32())
    }

    pub fn randu64() -> u64 {
        GLOBAL_RANDOM.with(|r| r.borrow_mut().randu64())
    }

    pub fn randf() -> f32 {
        GLOBAL_RANDOM.with(|r| r.borrow_mut().randf())
    }

    pub fn randd() -> f64 {
        GLOBAL_RANDOM.with(|r| r.borrow_mut().randd())
    }

    pub fn randb() -> bool {
        GLOBAL_RANDOM.with(|r| r.borrow_mut().randb())
    }

    pub fn rand_int(max: i64) -> i64 {
        GLOBAL_RANDOM.with(|r| r.borrow_mut().rand_int(max))
    }

    pub fn rand_int_range(min: i64, max: i64) -> i64 {
        GLOBAL_RANDOM.with(|r| r.borrow_mut().rand_int_range(min, max))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_deterministic() {
        let mut r1 = RandomSource::with_seed(12345);
        let mut r2 = RandomSource::with_seed(12345);
        
        for _ in 0..100 {
            assert_eq!(r1.randu32(), r2.randu32());
        }
    }

    #[test]
    fn test_different_seeds() {
        let mut r1 = RandomSource::with_seed(12345);
        let mut r2 = RandomSource::with_seed(54321);
        
        // Very unlikely to be equal with different seeds
        let mut same_count = 0;
        for _ in 0..100 {
            if r1.randu32() == r2.randu32() {
                same_count += 1;
            }
        }
        assert!(same_count < 10);
    }

    #[test]
    fn test_randf_range() {
        let mut r = RandomSource::with_seed(12345);
        for _ in 0..1000 {
            let f = r.randf();
            assert!(f >= 0.0 && f <= 1.0);
        }
    }

    #[test]
    fn test_rand_int_range() {
        let mut r = RandomSource::with_seed(12345);
        for _ in 0..1000 {
            let i = r.rand_int_range(10, 20);
            assert!(i >= 10 && i <= 20);
        }
    }

    #[test]
    fn test_shuffle() {
        let mut r = RandomSource::with_seed(12345);
        let mut arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        r.shuffle(&mut arr);
        
        // Check all elements still present
        let mut sorted = arr;
        sorted.sort();
        assert_eq!(sorted, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]);
        
        // Very unlikely to be in original order
        assert_ne!(arr, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]);
    }

    #[test]
    fn test_rand_bytes() {
        let mut r = RandomSource::with_seed(12345);
        let bytes = r.rand_bytes(32);
        assert_eq!(bytes.len(), 32);
        
        // Check not all zeros
        assert!(bytes.iter().any(|&b| b != 0));
    }

    #[test]
    fn test_normal_distribution() {
        let mut r = RandomSource::with_seed(12345);
        let mut sum = 0.0;
        let count = 10000;
        
        for _ in 0..count {
            sum += r.nrandf(1.0, 0.0);
        }
        
        let mean = sum / count as f32;
        // Mean should be close to 0 for normal distribution with mean=0
        assert!(mean.abs() < 0.1);
    }

    #[test]
    fn test_stochastic_round() {
        let mut r = RandomSource::with_seed(12345);
        let mut sum = 0i64;
        let count = 10000;
        
        for _ in 0..count {
            sum += r.stochastic_round(5.3);
        }
        
        let avg = sum as f64 / count as f64;
        // Average should be close to 5.3
        assert!((avg - 5.3).abs() < 0.1);
    }
}
