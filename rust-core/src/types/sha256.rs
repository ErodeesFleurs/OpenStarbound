//! SHA-256 hashing compatible with C++ Star::Sha256
//!
//! This module provides SHA-256 cryptographic hashing functionality.

/// SHA-256 hash output size in bytes
pub const SHA256_SIZE: usize = 32;

/// SHA-256 hasher compatible with C++ Star::Sha256Hasher
pub struct Sha256Hasher {
    state: [u32; 8],
    buffer: [u8; 64],
    buffer_len: usize,
    total_len: u64,
}

// SHA-256 constants (first 32 bits of the fractional parts of the cube roots of the first 64 primes)
const K: [u32; 64] = [
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
];

// Initial hash values (first 32 bits of the fractional parts of the square roots of the first 8 primes)
const H0: [u32; 8] = [
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
];

impl Default for Sha256Hasher {
    fn default() -> Self {
        Self::new()
    }
}

impl Sha256Hasher {
    /// Create a new SHA-256 hasher
    pub fn new() -> Self {
        Self {
            state: H0,
            buffer: [0u8; 64],
            buffer_len: 0,
            total_len: 0,
        }
    }

    /// Push data to the hasher
    pub fn push(&mut self, data: &[u8]) {
        self.total_len += data.len() as u64;
        
        let mut data = data;
        
        // If we have buffered data, try to complete the block
        if self.buffer_len > 0 {
            let needed = 64 - self.buffer_len;
            if data.len() >= needed {
                self.buffer[self.buffer_len..64].copy_from_slice(&data[..needed]);
                self.process_block();
                self.buffer_len = 0;
                data = &data[needed..];
            } else {
                self.buffer[self.buffer_len..self.buffer_len + data.len()].copy_from_slice(data);
                self.buffer_len += data.len();
                return;
            }
        }
        
        // Process complete blocks
        while data.len() >= 64 {
            self.buffer.copy_from_slice(&data[..64]);
            self.process_block();
            data = &data[64..];
        }
        
        // Buffer remaining data
        if !data.is_empty() {
            self.buffer[..data.len()].copy_from_slice(data);
            self.buffer_len = data.len();
        }
    }

    /// Push a string to the hasher
    pub fn push_str(&mut self, data: &str) {
        self.push(data.as_bytes());
    }

    fn process_block(&mut self) {
        let mut w = [0u32; 64];
        
        // Prepare message schedule
        for i in 0..16 {
            w[i] = u32::from_be_bytes([
                self.buffer[i * 4],
                self.buffer[i * 4 + 1],
                self.buffer[i * 4 + 2],
                self.buffer[i * 4 + 3],
            ]);
        }
        
        for i in 16..64 {
            let s0 = w[i-15].rotate_right(7) ^ w[i-15].rotate_right(18) ^ (w[i-15] >> 3);
            let s1 = w[i-2].rotate_right(17) ^ w[i-2].rotate_right(19) ^ (w[i-2] >> 10);
            w[i] = w[i-16].wrapping_add(s0).wrapping_add(w[i-7]).wrapping_add(s1);
        }
        
        // Initialize working variables
        let [mut a, mut b, mut c, mut d, mut e, mut f, mut g, mut h] = self.state;
        
        // Main loop
        for i in 0..64 {
            let s1 = e.rotate_right(6) ^ e.rotate_right(11) ^ e.rotate_right(25);
            let ch = (e & f) ^ ((!e) & g);
            let temp1 = h.wrapping_add(s1).wrapping_add(ch).wrapping_add(K[i]).wrapping_add(w[i]);
            let s0 = a.rotate_right(2) ^ a.rotate_right(13) ^ a.rotate_right(22);
            let maj = (a & b) ^ (a & c) ^ (b & c);
            let temp2 = s0.wrapping_add(maj);
            
            h = g;
            g = f;
            f = e;
            e = d.wrapping_add(temp1);
            d = c;
            c = b;
            b = a;
            a = temp1.wrapping_add(temp2);
        }
        
        // Update state
        self.state[0] = self.state[0].wrapping_add(a);
        self.state[1] = self.state[1].wrapping_add(b);
        self.state[2] = self.state[2].wrapping_add(c);
        self.state[3] = self.state[3].wrapping_add(d);
        self.state[4] = self.state[4].wrapping_add(e);
        self.state[5] = self.state[5].wrapping_add(f);
        self.state[6] = self.state[6].wrapping_add(g);
        self.state[7] = self.state[7].wrapping_add(h);
    }

    /// Compute the final hash value
    pub fn compute(mut self) -> [u8; SHA256_SIZE] {
        // Padding
        let bit_len = self.total_len * 8;
        
        // Append 1 bit
        self.buffer[self.buffer_len] = 0x80;
        self.buffer_len += 1;
        
        // If not enough room for length, pad and process
        if self.buffer_len > 56 {
            self.buffer[self.buffer_len..64].fill(0);
            self.process_block();
            self.buffer_len = 0;
        }
        
        // Pad with zeros
        self.buffer[self.buffer_len..56].fill(0);
        
        // Append length in bits as big-endian
        self.buffer[56..64].copy_from_slice(&bit_len.to_be_bytes());
        self.process_block();
        
        // Convert state to bytes
        let mut result = [0u8; SHA256_SIZE];
        for (i, &val) in self.state.iter().enumerate() {
            result[i * 4..i * 4 + 4].copy_from_slice(&val.to_be_bytes());
        }
        result
    }

    /// Compute the hash as a Vec
    pub fn compute_vec(self) -> Vec<u8> {
        self.compute().to_vec()
    }
}

/// Compute SHA-256 hash of data
pub fn sha256(data: &[u8]) -> [u8; SHA256_SIZE] {
    let mut hasher = Sha256Hasher::new();
    hasher.push(data);
    hasher.compute()
}

/// Compute SHA-256 hash of a string
pub fn sha256_str(data: &str) -> [u8; SHA256_SIZE] {
    sha256(data.as_bytes())
}

/// Compute SHA-256 hash and return as hex string
pub fn sha256_hex(data: &[u8]) -> String {
    let hash = sha256(data);
    super::encode::hex_encode(&hash)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::types::encode::hex_encode;

    #[test]
    fn test_sha256_empty() {
        let hash = sha256(b"");
        let hex = hex_encode(&hash);
        assert_eq!(hex, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    }

    #[test]
    fn test_sha256_hello() {
        let hash = sha256(b"hello");
        let hex = hex_encode(&hash);
        assert_eq!(hex, "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
    }

    #[test]
    fn test_sha256_quick_brown_fox() {
        let hash = sha256(b"The quick brown fox jumps over the lazy dog");
        let hex = hex_encode(&hash);
        assert_eq!(hex, "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
    }

    #[test]
    fn test_sha256_incremental() {
        let mut hasher = Sha256Hasher::new();
        hasher.push(b"The quick brown fox ");
        hasher.push(b"jumps over the lazy dog");
        let hex = hex_encode(&hasher.compute());
        assert_eq!(hex, "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
    }

    #[test]
    fn test_sha256_large_data() {
        // Test with data larger than one block (64 bytes)
        let data: Vec<u8> = (0..200).collect();
        let hash = sha256(&data);
        // Just verify it doesn't crash and produces 32 bytes
        assert_eq!(hash.len(), 32);
    }

    #[test]
    fn test_sha256_exactly_one_block() {
        // 64 bytes exactly
        let data = vec![0u8; 64];
        let hash = sha256(&data);
        assert_eq!(hash.len(), 32);
    }

    #[test]
    fn test_sha256_hex() {
        let hex = sha256_hex(b"test");
        assert_eq!(hex, "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08");
    }
}
