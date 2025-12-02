//! DataStream reader and writer compatible with C++ StarDataStream
//!
//! This provides binary serialization compatible with the existing
//! C++ save file and network protocol formats.

use crate::error::{Error, Result};
use crate::serialization::vlq;
use std::io::{Read, Write};

/// Data stream reader for binary deserialization
pub struct DataReader<R: Read> {
    reader: R,
    buffer: Vec<u8>,
}

impl<R: Read> DataReader<R> {
    /// Create a new DataReader
    pub fn new(reader: R) -> Self {
        Self {
            reader,
            buffer: Vec::new(),
        }
    }

    /// Read a single byte
    pub fn read_u8(&mut self) -> Result<u8> {
        let mut buf = [0u8; 1];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(buf[0])
    }

    /// Read a signed 8-bit integer
    pub fn read_i8(&mut self) -> Result<i8> {
        Ok(self.read_u8()? as i8)
    }

    /// Read a little-endian u16
    pub fn read_u16_le(&mut self) -> Result<u16> {
        let mut buf = [0u8; 2];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(u16::from_le_bytes(buf))
    }

    /// Read a big-endian u16
    pub fn read_u16_be(&mut self) -> Result<u16> {
        let mut buf = [0u8; 2];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(u16::from_be_bytes(buf))
    }

    /// Read a little-endian i16
    pub fn read_i16_le(&mut self) -> Result<i16> {
        let mut buf = [0u8; 2];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(i16::from_le_bytes(buf))
    }

    /// Read a little-endian u32
    pub fn read_u32_le(&mut self) -> Result<u32> {
        let mut buf = [0u8; 4];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(u32::from_le_bytes(buf))
    }

    /// Read a big-endian u32
    pub fn read_u32_be(&mut self) -> Result<u32> {
        let mut buf = [0u8; 4];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(u32::from_be_bytes(buf))
    }

    /// Read a little-endian i32
    pub fn read_i32_le(&mut self) -> Result<i32> {
        let mut buf = [0u8; 4];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(i32::from_le_bytes(buf))
    }

    /// Read a little-endian u64
    pub fn read_u64_le(&mut self) -> Result<u64> {
        let mut buf = [0u8; 8];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(u64::from_le_bytes(buf))
    }

    /// Read a little-endian i64
    pub fn read_i64_le(&mut self) -> Result<i64> {
        let mut buf = [0u8; 8];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(i64::from_le_bytes(buf))
    }

    /// Read a little-endian f32
    pub fn read_f32_le(&mut self) -> Result<f32> {
        let mut buf = [0u8; 4];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(f32::from_le_bytes(buf))
    }

    /// Read a little-endian f64
    pub fn read_f64_le(&mut self) -> Result<f64> {
        let mut buf = [0u8; 8];
        self.reader
            .read_exact(&mut buf)
            .map_err(|e| Error::Io(e.to_string()))?;
        Ok(f64::from_le_bytes(buf))
    }

    /// Read a boolean
    pub fn read_bool(&mut self) -> Result<bool> {
        Ok(self.read_u8()? != 0)
    }

    /// Read a VLQ-encoded unsigned integer
    pub fn read_vlq_u64(&mut self) -> Result<u64> {
        self.buffer.clear();
        loop {
            let byte = self.read_u8()?;
            self.buffer.push(byte);
            if byte & 0x80 == 0 {
                break;
            }
        }
        let (value, _) = vlq::decode_unsigned(&self.buffer)?;
        Ok(value)
    }

    /// Read a VLQ-encoded signed integer
    pub fn read_vlq_i64(&mut self) -> Result<i64> {
        self.buffer.clear();
        loop {
            let byte = self.read_u8()?;
            self.buffer.push(byte);
            if byte & 0x80 == 0 {
                break;
            }
        }
        let (value, _) = vlq::decode_signed(&self.buffer)?;
        Ok(value)
    }

    /// Read a length-prefixed string (VLQ length + UTF-8 bytes)
    pub fn read_string(&mut self) -> Result<String> {
        let len = self.read_vlq_u64()? as usize;
        let mut buf = vec![0u8; len];
        self.reader.read_exact(&mut buf).map_err(|e| Error::Io(e.to_string()))?;
        String::from_utf8(buf).map_err(|e| Error::Serialization(e.to_string()))
    }

    /// Read exact number of bytes
    pub fn read_bytes(&mut self, len: usize) -> Result<Vec<u8>> {
        let mut buf = vec![0u8; len];
        self.reader.read_exact(&mut buf).map_err(|e| Error::Io(e.to_string()))?;
        Ok(buf)
    }

    /// Read a length-prefixed byte array
    pub fn read_byte_array(&mut self) -> Result<Vec<u8>> {
        let len = self.read_vlq_u64()? as usize;
        self.read_bytes(len)
    }

    /// Read a Vec2F
    pub fn read_vec2f(&mut self) -> Result<crate::math::Vec2F> {
        let x = self.read_f32_le()?;
        let y = self.read_f32_le()?;
        Ok(crate::math::Vec2F::new(x, y))
    }

    /// Read a Vec2I
    pub fn read_vec2i(&mut self) -> Result<crate::math::Vec2I> {
        let x = self.read_i32_le()?;
        let y = self.read_i32_le()?;
        Ok(crate::math::Vec2I::new(x, y))
    }

    /// Read a Vec3F
    pub fn read_vec3f(&mut self) -> Result<crate::math::Vec3F> {
        let x = self.read_f32_le()?;
        let y = self.read_f32_le()?;
        let z = self.read_f32_le()?;
        Ok(crate::math::Vec3F::new(x, y, z))
    }

    /// Read a Vec4F
    pub fn read_vec4f(&mut self) -> Result<crate::math::Vec4F> {
        let x = self.read_f32_le()?;
        let y = self.read_f32_le()?;
        let z = self.read_f32_le()?;
        let w = self.read_f32_le()?;
        Ok(crate::math::Vec4F::new(x, y, z, w))
    }

    /// Read a Color
    pub fn read_color(&mut self) -> Result<crate::types::Color> {
        let r = self.read_u8()?;
        let g = self.read_u8()?;
        let b = self.read_u8()?;
        let a = self.read_u8()?;
        Ok(crate::types::Color::from_rgba_u8(r, g, b, a))
    }

    /// Read a value of type T using the Readable trait
    pub fn read<T: crate::serialization::Readable>(&mut self) -> Result<T> {
        T::read(self)
    }

    /// Read a VLQ-encoded unsigned 32-bit integer
    pub fn read_var_u32(&mut self) -> Result<u32> {
        Ok(self.read_vlq_u64()? as u32)
    }

    /// Read a VLQ-encoded signed 32-bit integer
    pub fn read_var_i32(&mut self) -> Result<i32> {
        Ok(self.read_vlq_i64()? as i32)
    }

    /// Read a VLQ-encoded unsigned 64-bit integer
    pub fn read_var_u64(&mut self) -> Result<u64> {
        self.read_vlq_u64()
    }

    /// Read a VLQ-encoded signed 64-bit integer
    pub fn read_var_i64(&mut self) -> Result<i64> {
        self.read_vlq_i64()
    }

    /// Read a f32 (little-endian)
    pub fn read_f32(&mut self) -> Result<f32> {
        self.read_f32_le()
    }

    /// Read a f64 (little-endian)
    pub fn read_f64(&mut self) -> Result<f64> {
        self.read_f64_le()
    }

    /// Read a u64 (little-endian)
    pub fn read_u64(&mut self) -> Result<u64> {
        self.read_u64_le()
    }

    /// Read a u32 (little-endian)
    pub fn read_u32(&mut self) -> Result<u32> {
        self.read_u32_le()
    }

    /// Read a i32 (little-endian)
    pub fn read_i32(&mut self) -> Result<i32> {
        self.read_i32_le()
    }

    /// Read a u16 (little-endian)
    pub fn read_u16(&mut self) -> Result<u16> {
        self.read_u16_le()
    }

    /// Read a i16 (little-endian)
    pub fn read_i16(&mut self) -> Result<i16> {
        self.read_i16_le()
    }
}

/// Data stream writer for binary serialization
pub struct DataWriter<W: Write> {
    writer: W,
}

impl<W: Write> DataWriter<W> {
    /// Create a new DataWriter
    pub fn new(writer: W) -> Self {
        Self { writer }
    }

    /// Write a single byte
    pub fn write_u8(&mut self, value: u8) -> Result<()> {
        self.writer.write_all(&[value]).map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a signed 8-bit integer
    pub fn write_i8(&mut self, value: i8) -> Result<()> {
        self.write_u8(value as u8)
    }

    /// Write a little-endian u16
    pub fn write_u16_le(&mut self, value: u16) -> Result<()> {
        self.writer
            .write_all(&value.to_le_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a big-endian u16
    pub fn write_u16_be(&mut self, value: u16) -> Result<()> {
        self.writer
            .write_all(&value.to_be_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a little-endian i16
    pub fn write_i16_le(&mut self, value: i16) -> Result<()> {
        self.writer
            .write_all(&value.to_le_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a little-endian u32
    pub fn write_u32_le(&mut self, value: u32) -> Result<()> {
        self.writer
            .write_all(&value.to_le_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a big-endian u32
    pub fn write_u32_be(&mut self, value: u32) -> Result<()> {
        self.writer
            .write_all(&value.to_be_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a little-endian i32
    pub fn write_i32_le(&mut self, value: i32) -> Result<()> {
        self.writer
            .write_all(&value.to_le_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a little-endian u64
    pub fn write_u64_le(&mut self, value: u64) -> Result<()> {
        self.writer
            .write_all(&value.to_le_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a little-endian i64
    pub fn write_i64_le(&mut self, value: i64) -> Result<()> {
        self.writer
            .write_all(&value.to_le_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a little-endian f32
    pub fn write_f32_le(&mut self, value: f32) -> Result<()> {
        self.writer
            .write_all(&value.to_le_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a little-endian f64
    pub fn write_f64_le(&mut self, value: f64) -> Result<()> {
        self.writer
            .write_all(&value.to_le_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a boolean
    pub fn write_bool(&mut self, value: bool) -> Result<()> {
        self.write_u8(if value { 1 } else { 0 })
    }

    /// Write a VLQ-encoded unsigned integer
    pub fn write_vlq_u64(&mut self, value: u64) -> Result<()> {
        let bytes = vlq::encode_unsigned(value);
        self.writer.write_all(&bytes).map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a VLQ-encoded signed integer
    pub fn write_vlq_i64(&mut self, value: i64) -> Result<()> {
        let bytes = vlq::encode_signed(value);
        self.writer.write_all(&bytes).map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a VLQ-encoded unsigned 32-bit integer
    pub fn write_var_u32(&mut self, value: u32) -> Result<()> {
        self.write_vlq_u64(value as u64)
    }

    /// Write a VLQ-encoded signed 32-bit integer
    pub fn write_var_i32(&mut self, value: i32) -> Result<()> {
        self.write_vlq_i64(value as i64)
    }

    /// Write a VLQ-encoded unsigned 64-bit integer
    pub fn write_var_u64(&mut self, value: u64) -> Result<()> {
        self.write_vlq_u64(value)
    }

    /// Write a VLQ-encoded signed 64-bit integer
    pub fn write_var_i64(&mut self, value: i64) -> Result<()> {
        self.write_vlq_i64(value)
    }

    /// Write a f32 (little-endian)
    pub fn write_f32(&mut self, value: f32) -> Result<()> {
        self.write_f32_le(value)
    }

    /// Write a f64 (little-endian)
    pub fn write_f64(&mut self, value: f64) -> Result<()> {
        self.write_f64_le(value)
    }

    /// Write a u64 (little-endian)
    pub fn write_u64(&mut self, value: u64) -> Result<()> {
        self.write_u64_le(value)
    }

    /// Write a u32 (little-endian)
    pub fn write_u32(&mut self, value: u32) -> Result<()> {
        self.write_u32_le(value)
    }

    /// Write a i32 (little-endian)
    pub fn write_i32(&mut self, value: i32) -> Result<()> {
        self.write_i32_le(value)
    }

    /// Write a u16 (little-endian)
    pub fn write_u16(&mut self, value: u16) -> Result<()> {
        self.write_u16_le(value)
    }

    /// Write a i16 (little-endian)
    pub fn write_i16(&mut self, value: i16) -> Result<()> {
        self.write_i16_le(value)
    }

    /// Write a length-prefixed string
    pub fn write_string(&mut self, value: &str) -> Result<()> {
        self.write_vlq_u64(value.len() as u64)?;
        self.writer
            .write_all(value.as_bytes())
            .map_err(|e| Error::Io(e.to_string()))
    }

    /// Write bytes
    pub fn write_bytes(&mut self, bytes: &[u8]) -> Result<()> {
        self.writer.write_all(bytes).map_err(|e| Error::Io(e.to_string()))
    }

    /// Write a length-prefixed byte array
    pub fn write_byte_array(&mut self, bytes: &[u8]) -> Result<()> {
        self.write_vlq_u64(bytes.len() as u64)?;
        self.write_bytes(bytes)
    }

    /// Write a Vec2F
    pub fn write_vec2f(&mut self, value: &crate::math::Vec2F) -> Result<()> {
        self.write_f32_le(value.x())?;
        self.write_f32_le(value.y())
    }

    /// Write a Vec2I
    pub fn write_vec2i(&mut self, value: &crate::math::Vec2I) -> Result<()> {
        self.write_i32_le(value.x())?;
        self.write_i32_le(value.y())
    }

    /// Write a Vec3F
    pub fn write_vec3f(&mut self, value: &crate::math::Vec3F) -> Result<()> {
        self.write_f32_le(value.x())?;
        self.write_f32_le(value.y())?;
        self.write_f32_le(value.z())
    }

    /// Write a Vec4F
    pub fn write_vec4f(&mut self, value: &crate::math::Vec4F) -> Result<()> {
        self.write_f32_le(value.x())?;
        self.write_f32_le(value.y())?;
        self.write_f32_le(value.z())?;
        self.write_f32_le(value.w())
    }

    /// Write a Color
    pub fn write_color(&mut self, value: &crate::types::Color) -> Result<()> {
        self.write_u8(value.red())?;
        self.write_u8(value.green())?;
        self.write_u8(value.blue())?;
        self.write_u8(value.alpha())
    }

    /// Flush the writer
    pub fn flush(&mut self) -> Result<()> {
        self.writer.flush().map_err(|e| Error::Io(e.to_string()))
    }

    /// Get the inner writer
    pub fn into_inner(self) -> W {
        self.writer
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    #[test]
    fn test_roundtrip_primitives() {
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            writer.write_u8(42).unwrap();
            writer.write_i8(-42).unwrap();
            writer.write_u16_le(1234).unwrap();
            writer.write_u32_le(123456).unwrap();
            writer.write_f32_le(3.14).unwrap();
            writer.write_bool(true).unwrap();
            writer.write_bool(false).unwrap();
        }

        let mut reader = DataReader::new(Cursor::new(buf));
        assert_eq!(reader.read_u8().unwrap(), 42);
        assert_eq!(reader.read_i8().unwrap(), -42);
        assert_eq!(reader.read_u16_le().unwrap(), 1234);
        assert_eq!(reader.read_u32_le().unwrap(), 123456);
        assert!((reader.read_f32_le().unwrap() - 3.14).abs() < 0.001);
        assert!(reader.read_bool().unwrap());
        assert!(!reader.read_bool().unwrap());
    }

    #[test]
    fn test_roundtrip_vlq() {
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            writer.write_vlq_u64(0).unwrap();
            writer.write_vlq_u64(127).unwrap();
            writer.write_vlq_u64(128).unwrap();
            writer.write_vlq_u64(16384).unwrap();
            writer.write_vlq_i64(-1).unwrap();
            writer.write_vlq_i64(1).unwrap();
        }

        let mut reader = DataReader::new(Cursor::new(buf));
        assert_eq!(reader.read_vlq_u64().unwrap(), 0);
        assert_eq!(reader.read_vlq_u64().unwrap(), 127);
        assert_eq!(reader.read_vlq_u64().unwrap(), 128);
        assert_eq!(reader.read_vlq_u64().unwrap(), 16384);
        assert_eq!(reader.read_vlq_i64().unwrap(), -1);
        assert_eq!(reader.read_vlq_i64().unwrap(), 1);
    }

    #[test]
    fn test_roundtrip_string() {
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            writer.write_string("Hello, World!").unwrap();
            writer.write_string("").unwrap();
            writer.write_string("Unicode: 日本語").unwrap();
        }

        let mut reader = DataReader::new(Cursor::new(buf));
        assert_eq!(reader.read_string().unwrap(), "Hello, World!");
        assert_eq!(reader.read_string().unwrap(), "");
        assert_eq!(reader.read_string().unwrap(), "Unicode: 日本語");
    }

    #[test]
    fn test_roundtrip_vectors() {
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            writer
                .write_vec2f(&crate::math::Vec2F::new(1.0, 2.0))
                .unwrap();
            writer
                .write_vec3f(&crate::math::Vec3F::new(1.0, 2.0, 3.0))
                .unwrap();
        }

        let mut reader = DataReader::new(Cursor::new(buf));
        let v2 = reader.read_vec2f().unwrap();
        assert!((v2.x() - 1.0).abs() < 0.001);
        assert!((v2.y() - 2.0).abs() < 0.001);

        let v3 = reader.read_vec3f().unwrap();
        assert!((v3.x() - 1.0).abs() < 0.001);
        assert!((v3.y() - 2.0).abs() < 0.001);
        assert!((v3.z() - 3.0).abs() < 0.001);
    }

    #[test]
    fn test_roundtrip_color() {
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            writer
                .write_color(&crate::types::Color::from_rgba_u8(255, 128, 64, 200))
                .unwrap();
        }

        let mut reader = DataReader::new(Cursor::new(buf));
        let color = reader.read_color().unwrap();
        assert_eq!(color.red(), 255);
        assert_eq!(color.green(), 128);
        assert_eq!(color.blue(), 64);
        assert_eq!(color.alpha(), 200);
    }
}
