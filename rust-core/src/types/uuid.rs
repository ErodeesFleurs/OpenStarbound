//! UUID type compatible with C++ Star::Uuid
//!
//! This module provides a UUID type that matches the binary layout
//! of the C++ implementation.

use serde::{Deserialize, Serialize};
use std::fmt;

/// UUID Size in bytes
pub const UUID_SIZE: usize = 16;

/// A universally unique identifier (UUID)
#[derive(Clone, Copy, PartialEq, Eq, Hash, Default, Serialize, Deserialize)]
#[repr(C)]
pub struct Uuid {
    data: [u8; UUID_SIZE],
}

impl Uuid {
    /// Create a new UUID from random bytes
    /// Note: This uses a simple PRNG based on system time. For production use,
    /// consider using a cryptographically secure random generator.
    pub fn new() -> Self {
        use std::time::{SystemTime, UNIX_EPOCH};
        
        let mut data = [0u8; UUID_SIZE];
        
        // Generate pseudo-random bytes based on time and memory address for additional entropy
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default();
        
        let mut seed = now.as_nanos() as u64;
        // Add some additional entropy from the stack address
        seed = seed.wrapping_add(&data as *const _ as u64);
        
        let mut state = seed;
        
        for byte in &mut data {
            // Simple xorshift PRNG - adequate for game use, not cryptographic
            state ^= state << 13;
            state ^= state >> 7;
            state ^= state << 17;
            *byte = (state & 0xFF) as u8;
        }
        
        // Set version 4 (random) and variant bits per RFC 4122
        data[6] = (data[6] & 0x0F) | 0x40; // Version 4
        data[8] = (data[8] & 0x3F) | 0x80; // Variant 1
        
        Self { data }
    }

    /// Create a nil (all zeros) UUID
    pub fn nil() -> Self {
        Self {
            data: [0u8; UUID_SIZE],
        }
    }

    /// Create a UUID from raw bytes
    pub fn from_bytes(bytes: [u8; UUID_SIZE]) -> Self {
        Self { data: bytes }
    }

    /// Create a UUID from a byte slice
    pub fn from_slice(bytes: &[u8]) -> Result<Self, crate::error::Error> {
        if bytes.len() != UUID_SIZE {
            return Err(crate::error::Error::Star(format!(
                "Invalid UUID length: expected {}, got {}",
                UUID_SIZE,
                bytes.len()
            )));
        }
        let mut data = [0u8; UUID_SIZE];
        data.copy_from_slice(bytes);
        Ok(Self { data })
    }

    /// Create a UUID from a hex string
    pub fn from_hex(s: &str) -> Result<Self, crate::error::Error> {
        let s = s.trim_matches(|c| c == '{' || c == '}' || c == '-');
        
        if s.len() != UUID_SIZE * 2 {
            return Err(crate::error::Error::Star(format!(
                "Invalid UUID hex length: expected {}, got {}",
                UUID_SIZE * 2,
                s.len()
            )));
        }

        let mut data = [0u8; UUID_SIZE];
        for (i, chunk) in s.as_bytes().chunks(2).enumerate() {
            let high = hex_char_to_value(chunk[0])?;
            let low = hex_char_to_value(chunk[1])?;
            data[i] = (high << 4) | low;
        }

        Ok(Self { data })
    }

    /// Get the raw bytes of the UUID
    pub fn as_bytes(&self) -> &[u8; UUID_SIZE] {
        &self.data
    }

    /// Get the raw bytes as a slice
    pub fn as_slice(&self) -> &[u8] {
        &self.data
    }

    /// Convert to hex string
    pub fn to_hex(&self) -> String {
        let mut s = String::with_capacity(UUID_SIZE * 2);
        for byte in &self.data {
            s.push(HEX_CHARS[(byte >> 4) as usize]);
            s.push(HEX_CHARS[(byte & 0x0F) as usize]);
        }
        s
    }

    /// Convert to standard UUID format (8-4-4-4-12)
    pub fn to_string_formatted(&self) -> String {
        format!(
            "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
            self.data[0], self.data[1], self.data[2], self.data[3],
            self.data[4], self.data[5],
            self.data[6], self.data[7],
            self.data[8], self.data[9],
            self.data[10], self.data[11], self.data[12], self.data[13], self.data[14], self.data[15]
        )
    }

    /// Check if this is a nil UUID (all zeros)
    pub fn is_nil(&self) -> bool {
        self.data.iter().all(|&b| b == 0)
    }
}

const HEX_CHARS: [char; 16] = [
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
];

fn hex_char_to_value(c: u8) -> Result<u8, crate::error::Error> {
    match c {
        b'0'..=b'9' => Ok(c - b'0'),
        b'a'..=b'f' => Ok(c - b'a' + 10),
        b'A'..=b'F' => Ok(c - b'A' + 10),
        _ => Err(crate::error::Error::Star(format!(
            "Invalid hex character: {}",
            c as char
        ))),
    }
}

impl fmt::Display for Uuid {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.to_hex())
    }
}

impl fmt::Debug for Uuid {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Uuid({})", self.to_string_formatted())
    }
}

impl std::str::FromStr for Uuid {
    type Err = crate::error::Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        Self::from_hex(s)
    }
}

impl PartialOrd for Uuid {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Uuid {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.data.cmp(&other.data)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_uuid_from_bytes() {
        let bytes = [1u8, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16];
        let uuid = Uuid::from_bytes(bytes);
        assert_eq!(uuid.as_bytes(), &bytes);
    }

    #[test]
    fn test_uuid_from_hex() {
        let uuid = Uuid::from_hex("0102030405060708090a0b0c0d0e0f10").unwrap();
        assert_eq!(uuid.as_bytes(), &[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]);
    }

    #[test]
    fn test_uuid_to_hex() {
        let bytes = [1u8, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16];
        let uuid = Uuid::from_bytes(bytes);
        assert_eq!(uuid.to_hex(), "0102030405060708090a0b0c0d0e0f10");
    }

    #[test]
    fn test_uuid_formatted() {
        let bytes = [0x55u8, 0x44, 0x33, 0x22, 0x11, 0x00, 0xff, 0xee, 
                     0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66];
        let uuid = Uuid::from_bytes(bytes);
        assert_eq!(uuid.to_string_formatted(), "55443322-1100-ffee-ddcc-bbaa99887766");
    }

    #[test]
    fn test_uuid_nil() {
        let nil = Uuid::default();
        assert!(nil.is_nil());

        let non_nil = Uuid::from_bytes([1u8; UUID_SIZE]);
        assert!(!non_nil.is_nil());
    }

    #[test]
    fn test_uuid_ordering() {
        let uuid1 = Uuid::from_bytes([0u8; UUID_SIZE]);
        let uuid2 = Uuid::from_bytes([1u8; UUID_SIZE]);
        assert!(uuid1 < uuid2);
    }

    #[test]
    fn test_uuid_parse() {
        let uuid: Uuid = "0102030405060708090a0b0c0d0e0f10".parse().unwrap();
        assert_eq!(uuid.as_bytes(), &[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]);
    }
}
