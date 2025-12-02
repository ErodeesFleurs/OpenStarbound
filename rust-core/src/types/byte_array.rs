//! ByteArray type compatible with C++ Star::ByteArray
//!
//! This module provides a byte array type that matches the functionality
//! of the C++ implementation.

use serde::{Deserialize, Serialize};
use std::fmt;
use std::ops::{Deref, DerefMut, Index, IndexMut};

/// A growable byte array compatible with C++ Star::ByteArray
#[derive(Clone, PartialEq, Eq, Hash, Default, Serialize, Deserialize)]
pub struct ByteArray {
    data: Vec<u8>,
}

impl ByteArray {
    /// Create a new empty ByteArray
    pub fn new() -> Self {
        Self { data: Vec::new() }
    }

    /// Create a ByteArray with the given capacity
    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            data: Vec::with_capacity(capacity),
        }
    }

    /// Create a ByteArray filled with a value
    pub fn filled(size: usize, value: u8) -> Self {
        Self {
            data: vec![value; size],
        }
    }

    /// Create from a C string without null terminator
    pub fn from_c_string(s: &str) -> Self {
        Self {
            data: s.as_bytes().to_vec(),
        }
    }

    /// Create from a C string with null terminator
    pub fn from_c_string_with_null(s: &str) -> Self {
        let mut data = s.as_bytes().to_vec();
        data.push(0);
        Self { data }
    }

    /// Create from a slice
    pub fn from_slice(data: &[u8]) -> Self {
        Self {
            data: data.to_vec(),
        }
    }

    /// Create from a Vec
    pub fn from_vec(data: Vec<u8>) -> Self {
        Self { data }
    }

    /// Get raw pointer to data
    pub fn as_ptr(&self) -> *const u8 {
        self.data.as_ptr()
    }

    /// Get mutable raw pointer to data
    pub fn as_mut_ptr(&mut self) -> *mut u8 {
        self.data.as_mut_ptr()
    }

    /// Get as slice
    pub fn as_slice(&self) -> &[u8] {
        &self.data
    }

    /// Get as mutable slice
    pub fn as_mut_slice(&mut self) -> &mut [u8] {
        &mut self.data
    }

    /// Get the size
    pub fn size(&self) -> usize {
        self.data.len()
    }

    /// Get the capacity
    pub fn capacity(&self) -> usize {
        self.data.capacity()
    }

    /// Check if empty
    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    /// Clear the array (sets size to 0)
    pub fn clear(&mut self) {
        self.data.clear();
    }

    /// Reset to empty (clears and resets capacity)
    pub fn reset(&mut self) {
        self.data = Vec::new();
    }

    /// Reserve capacity
    pub fn reserve(&mut self, additional: usize) {
        self.data.reserve(additional);
    }

    /// Resize to new size
    pub fn resize(&mut self, new_size: usize) {
        self.data.resize(new_size, 0);
    }

    /// Resize to new size with fill value
    pub fn resize_with(&mut self, new_size: usize, value: u8) {
        self.data.resize(new_size, value);
    }

    /// Fill with value
    pub fn fill(&mut self, value: u8) {
        self.data.fill(value);
    }

    /// Fill and resize
    pub fn fill_resize(&mut self, new_size: usize, value: u8) {
        self.data.resize(new_size, value);
        self.data.fill(value);
    }

    /// Append another ByteArray
    pub fn append(&mut self, other: &ByteArray) {
        self.data.extend_from_slice(&other.data);
    }

    /// Append bytes from slice
    pub fn append_slice(&mut self, data: &[u8]) {
        self.data.extend_from_slice(data);
    }

    /// Append a single byte
    pub fn append_byte(&mut self, byte: u8) {
        self.data.push(byte);
    }

    /// Push a byte (alias for append_byte)
    pub fn push(&mut self, byte: u8) {
        self.data.push(byte);
    }

    /// Pop the last byte
    pub fn pop(&mut self) -> Option<u8> {
        self.data.pop()
    }

    /// Copy data to a buffer
    pub fn copy_to(&self, dest: &mut [u8]) {
        let len = dest.len().min(self.data.len());
        dest[..len].copy_from_slice(&self.data[..len]);
    }

    /// Copy from position with length
    pub fn copy_range_to(&self, dest: &mut [u8], pos: usize, len: usize) {
        if pos >= self.data.len() {
            return;
        }
        let actual_len = len.min(self.data.len() - pos).min(dest.len());
        dest[..actual_len].copy_from_slice(&self.data[pos..pos + actual_len]);
    }

    /// Write from data to position
    pub fn write_from(&mut self, data: &[u8], pos: usize) {
        let end = pos + data.len();
        if end > self.data.len() {
            self.data.resize(end, 0);
        }
        self.data[pos..end].copy_from_slice(data);
    }

    /// Get a sub-array
    pub fn sub(&self, start: usize, len: usize) -> ByteArray {
        let end = (start + len).min(self.data.len());
        let start = start.min(self.data.len());
        ByteArray::from_slice(&self.data[start..end])
    }

    /// Get left portion
    pub fn left(&self, len: usize) -> ByteArray {
        self.sub(0, len)
    }

    /// Get right portion
    pub fn right(&self, len: usize) -> ByteArray {
        if len >= self.data.len() {
            self.clone()
        } else {
            self.sub(self.data.len() - len, len)
        }
    }

    /// Trim from left
    pub fn trim_left(&mut self, count: usize) {
        if count >= self.data.len() {
            self.clear();
        } else {
            self.data.drain(..count);
        }
    }

    /// Trim from right
    pub fn trim_right(&mut self, count: usize) {
        if count >= self.data.len() {
            self.clear();
        } else {
            self.data.truncate(self.data.len() - count);
        }
    }

    /// Insert at position
    pub fn insert(&mut self, pos: usize, byte: u8) {
        self.data.insert(pos, byte);
    }

    /// Get byte at index with bounds check
    pub fn at(&self, index: usize) -> Result<u8, crate::error::Error> {
        self.data.get(index).copied().ok_or_else(|| {
            crate::error::Error::Star(format!("Index {} out of range for ByteArray", index))
        })
    }

    /// Compare with another ByteArray
    pub fn compare(&self, other: &ByteArray) -> std::cmp::Ordering {
        self.data.cmp(&other.data)
    }

    /// Find first differing byte position
    pub fn diff_char(&self, other: &ByteArray) -> usize {
        for (i, (a, b)) in self.data.iter().zip(other.data.iter()).enumerate() {
            if a != b {
                return i;
            }
        }
        self.data.len().min(other.data.len())
    }

    /// XOR with another ByteArray
    pub fn xor_with(&self, other: &ByteArray, extend: bool) -> ByteArray {
        self.combine_with(other, extend, |a, b| a ^ b)
    }

    /// AND with another ByteArray
    pub fn and_with(&self, other: &ByteArray, extend: bool) -> ByteArray {
        self.combine_with(other, extend, |a, b| a & b)
    }

    /// OR with another ByteArray
    pub fn or_with(&self, other: &ByteArray, extend: bool) -> ByteArray {
        self.combine_with(other, extend, |a, b| a | b)
    }

    /// Combine with another ByteArray using a function
    /// 
    /// The function `f` is called with `(self[i], other[i])` for each overlapping index.
    /// If `extend` is true and the arrays have different lengths, the non-overlapping
    /// bytes from the longer array are appended to the result.
    pub fn combine_with<F>(&self, other: &ByteArray, extend: bool, f: F) -> ByteArray
    where
        F: Fn(u8, u8) -> u8,
    {
        let min_len = self.data.len().min(other.data.len());
        let max_len = self.data.len().max(other.data.len());
        
        let mut result = Vec::with_capacity(if extend { max_len } else { min_len });

        // Combine overlapping portion - always use self as first argument
        for i in 0..min_len {
            result.push(f(self.data[i], other.data[i]));
        }

        if extend {
            // Extend with remaining bytes from the longer array
            if self.data.len() > other.data.len() {
                result.extend_from_slice(&self.data[min_len..]);
            } else {
                result.extend_from_slice(&other.data[min_len..]);
            }
        }

        ByteArray { data: result }
    }

    /// Convert to Vec<u8>
    pub fn into_vec(self) -> Vec<u8> {
        self.data
    }

    /// Convert to hex string
    pub fn to_hex(&self) -> String {
        self.data.iter().map(|b| format!("{:02x}", b)).collect()
    }

    /// Create from hex string
    pub fn from_hex(s: &str) -> Result<Self, crate::error::Error> {
        if s.len() % 2 != 0 {
            return Err(crate::error::Error::Star(
                "Hex string must have even length".to_string(),
            ));
        }

        let mut data = Vec::with_capacity(s.len() / 2);
        for i in (0..s.len()).step_by(2) {
            let byte = u8::from_str_radix(&s[i..i + 2], 16)
                .map_err(|e| crate::error::Error::Star(e.to_string()))?;
            data.push(byte);
        }

        Ok(Self { data })
    }
}

impl Deref for ByteArray {
    type Target = [u8];

    fn deref(&self) -> &Self::Target {
        &self.data
    }
}

impl DerefMut for ByteArray {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.data
    }
}

impl Index<usize> for ByteArray {
    type Output = u8;

    fn index(&self, index: usize) -> &Self::Output {
        &self.data[index]
    }
}

impl IndexMut<usize> for ByteArray {
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        &mut self.data[index]
    }
}

impl<'a> IntoIterator for &'a ByteArray {
    type Item = &'a u8;
    type IntoIter = std::slice::Iter<'a, u8>;

    fn into_iter(self) -> Self::IntoIter {
        self.data.iter()
    }
}

impl<'a> IntoIterator for &'a mut ByteArray {
    type Item = &'a mut u8;
    type IntoIter = std::slice::IterMut<'a, u8>;

    fn into_iter(self) -> Self::IntoIter {
        self.data.iter_mut()
    }
}

impl From<Vec<u8>> for ByteArray {
    fn from(v: Vec<u8>) -> Self {
        Self::from_vec(v)
    }
}

impl From<&[u8]> for ByteArray {
    fn from(s: &[u8]) -> Self {
        Self::from_slice(s)
    }
}

impl From<ByteArray> for Vec<u8> {
    fn from(ba: ByteArray) -> Self {
        ba.data
    }
}

impl fmt::Display for ByteArray {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "ByteArray[{}]", self.data.len())
    }
}

impl fmt::Debug for ByteArray {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "ByteArray({:?})", self.to_hex())
    }
}

impl PartialOrd for ByteArray {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for ByteArray {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.data.cmp(&other.data)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new() {
        let ba = ByteArray::new();
        assert!(ba.is_empty());
        assert_eq!(ba.size(), 0);
    }

    #[test]
    fn test_filled() {
        let ba = ByteArray::filled(5, 0x42);
        assert_eq!(ba.size(), 5);
        assert_eq!(ba[0], 0x42);
        assert_eq!(ba[4], 0x42);
    }

    #[test]
    fn test_from_slice() {
        let ba = ByteArray::from_slice(&[1, 2, 3, 4, 5]);
        assert_eq!(ba.size(), 5);
        assert_eq!(ba.as_slice(), &[1, 2, 3, 4, 5]);
    }

    #[test]
    fn test_append() {
        let mut ba = ByteArray::from_slice(&[1, 2]);
        ba.append(&ByteArray::from_slice(&[3, 4]));
        assert_eq!(ba.as_slice(), &[1, 2, 3, 4]);
    }

    #[test]
    fn test_sub() {
        let ba = ByteArray::from_slice(&[1, 2, 3, 4, 5]);
        let sub = ba.sub(1, 3);
        assert_eq!(sub.as_slice(), &[2, 3, 4]);
    }

    #[test]
    fn test_left_right() {
        let ba = ByteArray::from_slice(&[1, 2, 3, 4, 5]);
        assert_eq!(ba.left(2).as_slice(), &[1, 2]);
        assert_eq!(ba.right(2).as_slice(), &[4, 5]);
    }

    #[test]
    fn test_trim() {
        let mut ba = ByteArray::from_slice(&[1, 2, 3, 4, 5]);
        ba.trim_left(2);
        assert_eq!(ba.as_slice(), &[3, 4, 5]);
        
        ba.trim_right(1);
        assert_eq!(ba.as_slice(), &[3, 4]);
    }

    #[test]
    fn test_xor() {
        let a = ByteArray::from_slice(&[0xFF, 0x00, 0xAA]);
        let b = ByteArray::from_slice(&[0x0F, 0xF0, 0x55]);
        let result = a.xor_with(&b, false);
        assert_eq!(result.as_slice(), &[0xF0, 0xF0, 0xFF]);
    }

    #[test]
    fn test_hex() {
        let ba = ByteArray::from_slice(&[0xDE, 0xAD, 0xBE, 0xEF]);
        assert_eq!(ba.to_hex(), "deadbeef");
        
        let ba2 = ByteArray::from_hex("deadbeef").unwrap();
        assert_eq!(ba, ba2);
    }

    #[test]
    fn test_diff_char() {
        let a = ByteArray::from_slice(&[1, 2, 3, 4]);
        let b = ByteArray::from_slice(&[1, 2, 5, 4]);
        assert_eq!(a.diff_char(&b), 2);
    }
}
