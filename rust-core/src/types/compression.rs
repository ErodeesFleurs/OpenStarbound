//! Compression utilities compatible with C++ Star::Compression
//!
//! Provides zlib-based compression/decompression matching the C++ implementation.

use crate::error::{Error, Result};
use crate::types::ByteArray;
use flate2::read::{ZlibDecoder, ZlibEncoder};
use flate2::Compression;
use std::io::Read;

/// Compression level (0-9), matching C++ CompressionLevel
pub type CompressionLevel = u32;

/// Low compression (fast, larger output)
pub const LOW_COMPRESSION: CompressionLevel = 2;

/// Medium compression (balanced)
pub const MEDIUM_COMPRESSION: CompressionLevel = 5;

/// High compression (slow, smaller output)
pub const HIGH_COMPRESSION: CompressionLevel = 9;

/// Compress data using zlib.
///
/// # Arguments
/// * `data` - Data to compress
/// * `level` - Compression level (0-9, default is MEDIUM_COMPRESSION)
///
/// # Returns
/// Compressed data as ByteArray
///
/// # Example
/// ```
/// use starbound_core::types::compression::{compress, MEDIUM_COMPRESSION};
/// use starbound_core::ByteArray;
///
/// let data = ByteArray::from_slice(b"Hello, World! Hello, World!");
/// let compressed = compress(&data, MEDIUM_COMPRESSION).unwrap();
/// ```
pub fn compress(data: &ByteArray, level: CompressionLevel) -> Result<ByteArray> {
    compress_bytes(data.as_slice(), level)
}

/// Compress raw bytes using zlib.
pub fn compress_bytes(data: &[u8], level: CompressionLevel) -> Result<ByteArray> {
    let level = level.min(9);
    let compression = Compression::new(level);
    let mut encoder = ZlibEncoder::new(data, compression);
    let mut result = Vec::new();
    encoder
        .read_to_end(&mut result)
        .map_err(|e| Error::io(format!("Compression failed: {}", e)))?;
    Ok(ByteArray::from_vec(result))
}

/// Decompress zlib-compressed data.
///
/// # Arguments
/// * `data` - Compressed data
/// * `limit` - Maximum output size (0 for unlimited)
///
/// # Returns
/// Decompressed data as ByteArray
///
/// # Example
/// ```
/// use starbound_core::types::compression::{compress, uncompress, MEDIUM_COMPRESSION};
/// use starbound_core::ByteArray;
///
/// let data = ByteArray::from_slice(b"Hello, World! Hello, World!");
/// let compressed = compress(&data, MEDIUM_COMPRESSION).unwrap();
/// let decompressed = uncompress(&compressed, 0).unwrap();
/// assert_eq!(data.as_slice(), decompressed.as_slice());
/// ```
pub fn uncompress(data: &ByteArray, limit: usize) -> Result<ByteArray> {
    uncompress_bytes(data.as_slice(), limit)
}

/// Decompress raw zlib-compressed bytes.
pub fn uncompress_bytes(data: &[u8], limit: usize) -> Result<ByteArray> {
    let mut decoder = ZlibDecoder::new(data);
    let mut result = Vec::new();

    if limit > 0 {
        // Read with a limit
        let mut limited_result = vec![0u8; limit];
        let mut total_read = 0;
        loop {
            let remaining = limit - total_read;
            if remaining == 0 {
                break;
            }
            let read = decoder
                .read(&mut limited_result[total_read..])
                .map_err(|e| Error::io(format!("Decompression failed: {}", e)))?;
            if read == 0 {
                break;
            }
            total_read += read;
        }
        limited_result.truncate(total_read);
        result = limited_result;
    } else {
        decoder
            .read_to_end(&mut result)
            .map_err(|e| Error::io(format!("Decompression failed: {}", e)))?;
    }

    Ok(ByteArray::from_vec(result))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_compress_decompress() {
        let original = ByteArray::from_slice(b"Hello, World! Hello, World! Hello, World!");
        let compressed = compress(&original, MEDIUM_COMPRESSION).unwrap();
        let decompressed = uncompress(&compressed, 0).unwrap();
        assert_eq!(original.as_slice(), decompressed.as_slice());
    }

    #[test]
    fn test_compress_levels() {
        let data = ByteArray::from_slice(b"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

        let low = compress(&data, LOW_COMPRESSION).unwrap();
        let medium = compress(&data, MEDIUM_COMPRESSION).unwrap();
        let high = compress(&data, HIGH_COMPRESSION).unwrap();

        // All should decompress to original
        assert_eq!(uncompress(&low, 0).unwrap().as_slice(), data.as_slice());
        assert_eq!(uncompress(&medium, 0).unwrap().as_slice(), data.as_slice());
        assert_eq!(uncompress(&high, 0).unwrap().as_slice(), data.as_slice());
    }

    #[test]
    fn test_empty_data() {
        let empty = ByteArray::new();
        let compressed = compress(&empty, MEDIUM_COMPRESSION).unwrap();
        let decompressed = uncompress(&compressed, 0).unwrap();
        assert!(decompressed.is_empty());
    }

    #[test]
    fn test_large_data() {
        let data: Vec<u8> = (0..10000).map(|i| (i % 256) as u8).collect();
        let original = ByteArray::from_vec(data);
        let compressed = compress(&original, HIGH_COMPRESSION).unwrap();
        let decompressed = uncompress(&compressed, 0).unwrap();
        assert_eq!(original.as_slice(), decompressed.as_slice());
    }

    #[test]
    fn test_limit() {
        let original = ByteArray::from_slice(b"Hello, World! Hello, World! Hello, World!");
        let compressed = compress(&original, MEDIUM_COMPRESSION).unwrap();
        let decompressed = uncompress(&compressed, 10).unwrap();
        assert_eq!(decompressed.len(), 10);
        assert_eq!(&decompressed.as_slice()[..10], b"Hello, Wor");
    }
}
