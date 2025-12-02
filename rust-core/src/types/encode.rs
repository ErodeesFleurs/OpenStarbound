//! Base64 and Hex encoding/decoding compatible with C++ Star::Encode
//!
//! This module provides encoding utilities for binary data.

use crate::error::{Error, Result};

const BASE64_CHARS: &[u8] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const BASE64_PAD: u8 = b'=';

const HEX_CHARS: &[u8] = b"0123456789abcdef";

/// Encode data as hexadecimal string
pub fn hex_encode(data: &[u8]) -> String {
    let mut result = String::with_capacity(data.len() * 2);
    for byte in data {
        result.push(HEX_CHARS[(byte >> 4) as usize] as char);
        result.push(HEX_CHARS[(byte & 0x0F) as usize] as char);
    }
    result
}

/// Decode hexadecimal string to bytes
pub fn hex_decode(encoded: &str) -> Result<Vec<u8>> {
    let encoded = encoded.as_bytes();
    if encoded.len() % 2 != 0 {
        return Err(Error::parse("Hex string must have even length"));
    }

    let mut result = Vec::with_capacity(encoded.len() / 2);
    for chunk in encoded.chunks(2) {
        let high = hex_char_to_nibble(chunk[0])?;
        let low = hex_char_to_nibble(chunk[1])?;
        result.push((high << 4) | low);
    }
    Ok(result)
}

fn hex_char_to_nibble(c: u8) -> Result<u8> {
    match c {
        b'0'..=b'9' => Ok(c - b'0'),
        b'a'..=b'f' => Ok(c - b'a' + 10),
        b'A'..=b'F' => Ok(c - b'A' + 10),
        _ => Err(Error::parse("Invalid hex character")),
    }
}

/// Encode data as base64 string
pub fn base64_encode(data: &[u8]) -> String {
    let mut result = String::with_capacity((data.len() + 2) / 3 * 4);
    
    for chunk in data.chunks(3) {
        let b0 = chunk[0] as usize;
        let b1 = chunk.get(1).copied().unwrap_or(0) as usize;
        let b2 = chunk.get(2).copied().unwrap_or(0) as usize;

        result.push(BASE64_CHARS[b0 >> 2] as char);
        result.push(BASE64_CHARS[((b0 & 0x03) << 4) | (b1 >> 4)] as char);

        if chunk.len() > 1 {
            result.push(BASE64_CHARS[((b1 & 0x0F) << 2) | (b2 >> 6)] as char);
        } else {
            result.push(BASE64_PAD as char);
        }

        if chunk.len() > 2 {
            result.push(BASE64_CHARS[b2 & 0x3F] as char);
        } else {
            result.push(BASE64_PAD as char);
        }
    }

    result
}

/// Decode base64 string to bytes
pub fn base64_decode(encoded: &str) -> Result<Vec<u8>> {
    let encoded = encoded.as_bytes();
    let mut result = Vec::with_capacity(encoded.len() * 3 / 4);
    
    let mut buffer = [0u8; 4];
    let mut buffer_len = 0;

    for &c in encoded {
        if c == BASE64_PAD {
            break;
        }
        if c.is_ascii_whitespace() {
            continue;
        }

        let value = base64_char_to_value(c)?;
        buffer[buffer_len] = value;
        buffer_len += 1;

        if buffer_len == 4 {
            result.push((buffer[0] << 2) | (buffer[1] >> 4));
            result.push((buffer[1] << 4) | (buffer[2] >> 2));
            result.push((buffer[2] << 6) | buffer[3]);
            buffer_len = 0;
        }
    }

    // Handle remaining bytes
    if buffer_len >= 2 {
        result.push((buffer[0] << 2) | (buffer[1] >> 4));
    }
    if buffer_len >= 3 {
        result.push((buffer[1] << 4) | (buffer[2] >> 2));
    }

    Ok(result)
}

fn base64_char_to_value(c: u8) -> Result<u8> {
    match c {
        b'A'..=b'Z' => Ok(c - b'A'),
        b'a'..=b'z' => Ok(c - b'a' + 26),
        b'0'..=b'9' => Ok(c - b'0' + 52),
        b'+' => Ok(62),
        b'/' => Ok(63),
        _ => Err(Error::parse("Invalid base64 character")),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_hex_encode() {
        assert_eq!(hex_encode(&[0xDE, 0xAD, 0xBE, 0xEF]), "deadbeef");
        assert_eq!(hex_encode(&[0x00, 0xFF]), "00ff");
        assert_eq!(hex_encode(&[]), "");
    }

    #[test]
    fn test_hex_decode() {
        assert_eq!(hex_decode("deadbeef").unwrap(), vec![0xDE, 0xAD, 0xBE, 0xEF]);
        assert_eq!(hex_decode("00FF").unwrap(), vec![0x00, 0xFF]);
        assert_eq!(hex_decode("").unwrap(), Vec::<u8>::new());
    }

    #[test]
    fn test_hex_roundtrip() {
        let data = vec![0, 1, 127, 128, 255, 42, 100];
        assert_eq!(hex_decode(&hex_encode(&data)).unwrap(), data);
    }

    #[test]
    fn test_base64_encode() {
        assert_eq!(base64_encode(b"Hello"), "SGVsbG8=");
        assert_eq!(base64_encode(b"Hello!"), "SGVsbG8h");
        assert_eq!(base64_encode(b"Hi"), "SGk=");
        assert_eq!(base64_encode(b""), "");
    }

    #[test]
    fn test_base64_decode() {
        assert_eq!(base64_decode("SGVsbG8=").unwrap(), b"Hello");
        assert_eq!(base64_decode("SGVsbG8h").unwrap(), b"Hello!");
        assert_eq!(base64_decode("SGk=").unwrap(), b"Hi");
        assert_eq!(base64_decode("").unwrap(), Vec::<u8>::new());
    }

    #[test]
    fn test_base64_roundtrip() {
        let data = b"The quick brown fox jumps over the lazy dog";
        assert_eq!(base64_decode(&base64_encode(data)).unwrap(), data);
    }

    #[test]
    fn test_base64_binary_data() {
        let data: Vec<u8> = (0..=255).collect();
        assert_eq!(base64_decode(&base64_encode(&data)).unwrap(), data);
    }
}
