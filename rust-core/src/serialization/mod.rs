//! Serialization utilities compatible with C++ binary format
//!
//! This module provides serialization that is binary-compatible with
//! the C++ StarDataStream format for save file compatibility.

mod data_stream;

pub use data_stream::{DataReader, DataWriter};

/// VLQ (Variable Length Quantity) encoding used by Starbound
pub mod vlq {
    /// Encode a u64 value as VLQ bytes
    pub fn encode_unsigned(mut value: u64) -> Vec<u8> {
        let mut result = Vec::new();
        loop {
            let mut byte = (value & 0x7F) as u8;
            value >>= 7;
            if value != 0 {
                byte |= 0x80;
            }
            result.push(byte);
            if value == 0 {
                break;
            }
        }
        result
    }

    /// Decode a VLQ-encoded u64 value from bytes
    pub fn decode_unsigned(bytes: &[u8]) -> Result<(u64, usize), crate::error::Error> {
        let mut result: u64 = 0;
        let mut shift = 0;
        let mut bytes_read = 0;

        for &byte in bytes {
            bytes_read += 1;
            result |= ((byte & 0x7F) as u64) << shift;
            if byte & 0x80 == 0 {
                return Ok((result, bytes_read));
            }
            shift += 7;
            if shift >= 64 {
                return Err(crate::error::Error::Serialization(
                    "VLQ overflow".to_string(),
                ));
            }
        }

        Err(crate::error::Error::Serialization(
            "Unexpected end of VLQ data".to_string(),
        ))
    }

    /// Encode a signed i64 value using zigzag encoding + VLQ
    pub fn encode_signed(value: i64) -> Vec<u8> {
        let zigzag = ((value << 1) ^ (value >> 63)) as u64;
        encode_unsigned(zigzag)
    }

    /// Decode a signed i64 value from zigzag-encoded VLQ bytes
    pub fn decode_signed(bytes: &[u8]) -> Result<(i64, usize), crate::error::Error> {
        let (zigzag, bytes_read) = decode_unsigned(bytes)?;
        let value = ((zigzag >> 1) as i64) ^ (-((zigzag & 1) as i64));
        Ok((value, bytes_read))
    }
}

#[cfg(test)]
mod tests {
    use super::vlq;

    #[test]
    fn test_vlq_unsigned() {
        // Test encoding
        assert_eq!(vlq::encode_unsigned(0), vec![0]);
        assert_eq!(vlq::encode_unsigned(127), vec![127]);
        assert_eq!(vlq::encode_unsigned(128), vec![0x80, 0x01]);
        assert_eq!(vlq::encode_unsigned(16383), vec![0xFF, 0x7F]);

        // Test decoding
        assert_eq!(vlq::decode_unsigned(&[0]).unwrap(), (0, 1));
        assert_eq!(vlq::decode_unsigned(&[127]).unwrap(), (127, 1));
        assert_eq!(vlq::decode_unsigned(&[0x80, 0x01]).unwrap(), (128, 2));
    }

    #[test]
    fn test_vlq_signed() {
        // Test encoding
        assert_eq!(vlq::encode_signed(0), vec![0]);
        assert_eq!(vlq::encode_signed(-1), vec![1]);
        assert_eq!(vlq::encode_signed(1), vec![2]);
        assert_eq!(vlq::encode_signed(-2), vec![3]);

        // Test decoding
        assert_eq!(vlq::decode_signed(&[0]).unwrap(), (0, 1));
        assert_eq!(vlq::decode_signed(&[1]).unwrap(), (-1, 1));
        assert_eq!(vlq::decode_signed(&[2]).unwrap(), (1, 1));
    }

    #[test]
    fn test_vlq_roundtrip() {
        for value in [0u64, 1, 127, 128, 255, 256, 16383, 16384, u64::MAX / 2] {
            let encoded = vlq::encode_unsigned(value);
            let (decoded, _) = vlq::decode_unsigned(&encoded).unwrap();
            assert_eq!(value, decoded);
        }

        for value in [0i64, 1, -1, 127, -128, i64::MAX / 2, i64::MIN / 2] {
            let encoded = vlq::encode_signed(value);
            let (decoded, _) = vlq::decode_signed(&encoded).unwrap();
            assert_eq!(value, decoded);
        }
    }
}
