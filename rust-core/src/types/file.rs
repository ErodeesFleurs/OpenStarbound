//! File I/O utilities compatible with C++ Star::File and Star::IODevice.
//!
//! This module provides file and I/O operations.

use crate::Error;
use std::fs::{self, File, OpenOptions};
use std::io::{BufReader, BufWriter, Read, Seek, SeekFrom, Write};
use std::path::{Path, PathBuf};

/// I/O device mode flags.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum IOMode {
    /// Read-only access
    Read,
    /// Write-only access
    Write,
    /// Read and write access
    ReadWrite,
    /// Append mode (write at end)
    Append,
}

/// File type enum.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FileType {
    /// Regular file
    Regular,
    /// Directory
    Directory,
    /// Symbolic link
    Symlink,
    /// Other/unknown type
    Other,
}

/// File metadata information.
#[derive(Debug, Clone)]
pub struct FileInfo {
    /// File path
    pub path: PathBuf,
    /// File type
    pub file_type: FileType,
    /// File size in bytes
    pub size: u64,
    /// Whether the file is read-only
    pub read_only: bool,
}

/// A buffered file I/O device.
pub struct FileDevice {
    file: Option<BufWriter<File>>,
    reader: Option<BufReader<File>>,
    path: PathBuf,
    mode: IOMode,
    position: u64,
}

impl FileDevice {
    /// Open a file with the given mode.
    ///
    /// # Arguments
    /// * `path` - Path to the file
    /// * `mode` - I/O mode
    pub fn open<P: AsRef<Path>>(path: P, mode: IOMode) -> Result<Self, Error> {
        let path = path.as_ref().to_path_buf();

        let file = match mode {
            IOMode::Read => {
                let f = File::open(&path)?;
                return Ok(FileDevice {
                    file: None,
                    reader: Some(BufReader::new(f)),
                    path,
                    mode,
                    position: 0,
                });
            }
            IOMode::Write => OpenOptions::new()
                .write(true)
                .create(true)
                .truncate(true)
                .open(&path),
            IOMode::ReadWrite => OpenOptions::new()
                .read(true)
                .write(true)
                .create(true)
                .open(&path),
            IOMode::Append => OpenOptions::new().append(true).create(true).open(&path),
        };

        let f = file?;

        Ok(FileDevice {
            file: Some(BufWriter::new(f)),
            reader: None,
            path,
            mode,
            position: 0,
        })
    }

    /// Create a new file for writing.
    pub fn create<P: AsRef<Path>>(path: P) -> Result<Self, Error> {
        Self::open(path, IOMode::Write)
    }

    /// Check if the device is open.
    pub fn is_open(&self) -> bool {
        self.file.is_some() || self.reader.is_some()
    }

    /// Get the file path.
    pub fn path(&self) -> &Path {
        &self.path
    }

    /// Get the I/O mode.
    pub fn mode(&self) -> IOMode {
        self.mode
    }

    /// Get the current position.
    pub fn position(&self) -> u64 {
        self.position
    }

    /// Get the file size.
    pub fn size(&self) -> Result<u64, Error> {
        Ok(fs::metadata(&self.path)?.len())
    }

    /// Read data from the file.
    pub fn read(&mut self, buffer: &mut [u8]) -> Result<usize, Error> {
        if let Some(ref mut reader) = self.reader {
            let n = reader.read(buffer)?;
            self.position += n as u64;
            Ok(n)
        } else {
            Err(Error::Star("File not opened for reading".into()))
        }
    }

    /// Read all remaining data from the file.
    pub fn read_to_end(&mut self) -> Result<Vec<u8>, Error> {
        let mut buffer = Vec::new();
        if let Some(ref mut reader) = self.reader {
            reader.read_to_end(&mut buffer)?;
            self.position += buffer.len() as u64;
        } else {
            return Err(Error::Star("File not opened for reading".into()));
        }
        Ok(buffer)
    }

    /// Write data to the file.
    pub fn write(&mut self, data: &[u8]) -> Result<usize, Error> {
        if let Some(ref mut file) = self.file {
            let n = file.write(data)?;
            self.position += n as u64;
            Ok(n)
        } else {
            Err(Error::Star("File not opened for writing".into()))
        }
    }

    /// Write all data to the file.
    pub fn write_all(&mut self, data: &[u8]) -> Result<(), Error> {
        if let Some(ref mut file) = self.file {
            file.write_all(data)?;
            self.position += data.len() as u64;
            Ok(())
        } else {
            Err(Error::Star("File not opened for writing".into()))
        }
    }

    /// Flush buffered data to disk.
    pub fn flush(&mut self) -> Result<(), Error> {
        if let Some(ref mut file) = self.file {
            file.flush()?;
        }
        Ok(())
    }

    /// Seek to a position in the file.
    pub fn seek(&mut self, pos: u64) -> Result<u64, Error> {
        let seek_from = SeekFrom::Start(pos);

        if let Some(ref mut file) = self.file {
            let new_pos = file.seek(seek_from)?;
            self.position = new_pos;
            Ok(new_pos)
        } else if let Some(ref mut reader) = self.reader {
            let new_pos = reader.seek(seek_from)?;
            self.position = new_pos;
            Ok(new_pos)
        } else {
            Err(Error::Star("File not open".into()))
        }
    }

    /// Close the file.
    /// 
    /// Note: Flush errors during close are logged but not propagated.
    /// Call flush() explicitly before close() if you need to handle flush errors.
    pub fn close(&mut self) {
        if let Some(ref mut file) = self.file.take() {
            // Flush errors during close are intentionally ignored as this is
            // common practice - users should call flush() explicitly if they
            // need to handle flush errors
            let _ = file.flush();
        }
        self.reader = None;
    }
}

impl Drop for FileDevice {
    fn drop(&mut self) {
        self.close();
    }
}

/// File system utilities.
pub struct FileSystem;

impl FileSystem {
    /// Check if a path exists.
    pub fn exists<P: AsRef<Path>>(path: P) -> bool {
        path.as_ref().exists()
    }

    /// Check if a path is a file.
    pub fn is_file<P: AsRef<Path>>(path: P) -> bool {
        path.as_ref().is_file()
    }

    /// Check if a path is a directory.
    pub fn is_directory<P: AsRef<Path>>(path: P) -> bool {
        path.as_ref().is_dir()
    }

    /// Get file metadata.
    pub fn file_info<P: AsRef<Path>>(path: P) -> Result<FileInfo, Error> {
        let path = path.as_ref();
        let metadata = fs::metadata(path)?;

        let file_type = if metadata.is_file() {
            FileType::Regular
        } else if metadata.is_dir() {
            FileType::Directory
        } else if metadata.file_type().is_symlink() {
            FileType::Symlink
        } else {
            FileType::Other
        };

        Ok(FileInfo {
            path: path.to_path_buf(),
            file_type,
            size: metadata.len(),
            read_only: metadata.permissions().readonly(),
        })
    }

    /// Get the file size.
    pub fn file_size<P: AsRef<Path>>(path: P) -> Result<u64, Error> {
        Ok(fs::metadata(path)?.len())
    }

    /// Read a file into a byte vector.
    pub fn read<P: AsRef<Path>>(path: P) -> Result<Vec<u8>, Error> {
        Ok(fs::read(path)?)
    }

    /// Read a file into a string.
    pub fn read_to_string<P: AsRef<Path>>(path: P) -> Result<String, Error> {
        Ok(fs::read_to_string(path)?)
    }

    /// Write data to a file.
    pub fn write<P: AsRef<Path>>(path: P, data: &[u8]) -> Result<(), Error> {
        Ok(fs::write(path, data)?)
    }

    /// Write a string to a file.
    pub fn write_string<P: AsRef<Path>>(path: P, content: &str) -> Result<(), Error> {
        Ok(fs::write(path, content)?)
    }

    /// Create a directory (and parent directories if needed).
    pub fn create_directory<P: AsRef<Path>>(path: P) -> Result<(), Error> {
        Ok(fs::create_dir_all(path)?)
    }

    /// Remove a file.
    pub fn remove_file<P: AsRef<Path>>(path: P) -> Result<(), Error> {
        Ok(fs::remove_file(path)?)
    }

    /// Remove a directory (must be empty).
    pub fn remove_directory<P: AsRef<Path>>(path: P) -> Result<(), Error> {
        Ok(fs::remove_dir(path)?)
    }

    /// Remove a directory and all its contents.
    pub fn remove_directory_all<P: AsRef<Path>>(path: P) -> Result<(), Error> {
        Ok(fs::remove_dir_all(path)?)
    }

    /// Copy a file.
    pub fn copy<P: AsRef<Path>, Q: AsRef<Path>>(from: P, to: Q) -> Result<u64, Error> {
        Ok(fs::copy(from, to)?)
    }

    /// Rename/move a file or directory.
    pub fn rename<P: AsRef<Path>, Q: AsRef<Path>>(from: P, to: Q) -> Result<(), Error> {
        Ok(fs::rename(from, to)?)
    }

    /// List directory contents.
    pub fn list_directory<P: AsRef<Path>>(path: P) -> Result<Vec<PathBuf>, Error> {
        let entries = fs::read_dir(path)?;

        let mut paths = Vec::new();
        for entry in entries {
            let entry = entry?;
            paths.push(entry.path());
        }

        Ok(paths)
    }

    /// List files in a directory (not directories).
    pub fn list_files<P: AsRef<Path>>(path: P) -> Result<Vec<PathBuf>, Error> {
        let entries = Self::list_directory(path)?;
        Ok(entries.into_iter().filter(|p| p.is_file()).collect())
    }

    /// List subdirectories in a directory.
    pub fn list_directories<P: AsRef<Path>>(path: P) -> Result<Vec<PathBuf>, Error> {
        let entries = Self::list_directory(path)?;
        Ok(entries.into_iter().filter(|p| p.is_dir()).collect())
    }

    /// Get the file name from a path.
    pub fn file_name<P: AsRef<Path>>(path: P) -> Option<String> {
        path.as_ref()
            .file_name()
            .and_then(|n| n.to_str())
            .map(|s| s.to_string())
    }

    /// Get the file extension from a path.
    pub fn extension<P: AsRef<Path>>(path: P) -> Option<String> {
        path.as_ref()
            .extension()
            .and_then(|e| e.to_str())
            .map(|s| s.to_string())
    }

    /// Get the parent directory from a path.
    pub fn parent<P: AsRef<Path>>(path: P) -> Option<PathBuf> {
        path.as_ref().parent().map(|p| p.to_path_buf())
    }

    /// Join paths.
    pub fn join<P: AsRef<Path>, Q: AsRef<Path>>(base: P, path: Q) -> PathBuf {
        base.as_ref().join(path)
    }

    /// Get the absolute path.
    pub fn absolute<P: AsRef<Path>>(path: P) -> Result<PathBuf, Error> {
        Ok(fs::canonicalize(path)?)
    }

    /// Get the current working directory.
    pub fn current_directory() -> Result<PathBuf, Error> {
        Ok(std::env::current_dir()?)
    }

    /// Set the current working directory.
    pub fn set_current_directory<P: AsRef<Path>>(path: P) -> Result<(), Error> {
        Ok(std::env::set_current_dir(path)?)
    }

    /// Get the temporary directory.
    pub fn temp_directory() -> PathBuf {
        std::env::temp_dir()
    }
}

/// Buffer for in-memory I/O operations.
pub struct Buffer {
    data: Vec<u8>,
    position: usize,
}

impl Buffer {
    /// Create a new empty buffer.
    pub fn new() -> Self {
        Buffer {
            data: Vec::new(),
            position: 0,
        }
    }

    /// Create a buffer with the given capacity.
    pub fn with_capacity(capacity: usize) -> Self {
        Buffer {
            data: Vec::with_capacity(capacity),
            position: 0,
        }
    }

    /// Create a buffer from existing data.
    pub fn from_data(data: Vec<u8>) -> Self {
        Buffer { data, position: 0 }
    }

    /// Get the buffer data.
    pub fn data(&self) -> &[u8] {
        &self.data
    }

    /// Take ownership of the buffer data.
    pub fn into_data(self) -> Vec<u8> {
        self.data
    }

    /// Get the current position.
    pub fn position(&self) -> usize {
        self.position
    }

    /// Get the buffer size.
    pub fn size(&self) -> usize {
        self.data.len()
    }

    /// Check if the buffer is empty.
    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    /// Clear the buffer.
    pub fn clear(&mut self) {
        self.data.clear();
        self.position = 0;
    }

    /// Seek to a position.
    pub fn seek(&mut self, pos: usize) {
        self.position = pos.min(self.data.len());
    }

    /// Read from the buffer.
    pub fn read(&mut self, buffer: &mut [u8]) -> usize {
        let available = self.data.len() - self.position;
        let to_read = buffer.len().min(available);

        buffer[..to_read].copy_from_slice(&self.data[self.position..self.position + to_read]);
        self.position += to_read;

        to_read
    }

    /// Write to the buffer.
    pub fn write(&mut self, data: &[u8]) {
        if self.position == self.data.len() {
            self.data.extend_from_slice(data);
        } else {
            let end = self.position + data.len();
            if end > self.data.len() {
                self.data.resize(end, 0);
            }
            self.data[self.position..end].copy_from_slice(data);
        }
        self.position += data.len();
    }

    /// Write a byte to the buffer.
    pub fn write_byte(&mut self, byte: u8) {
        self.write(&[byte]);
    }

    /// Read a byte from the buffer.
    pub fn read_byte(&mut self) -> Option<u8> {
        if self.position < self.data.len() {
            let byte = self.data[self.position];
            self.position += 1;
            Some(byte)
        } else {
            None
        }
    }
}

impl Default for Buffer {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::env::temp_dir;

    #[test]
    fn test_io_mode() {
        assert_eq!(IOMode::Read, IOMode::Read);
        assert_ne!(IOMode::Read, IOMode::Write);
    }

    #[test]
    fn test_file_type() {
        assert_eq!(FileType::Regular, FileType::Regular);
        assert_ne!(FileType::Regular, FileType::Directory);
    }

    #[test]
    fn test_file_system_temp_dir() {
        let temp = FileSystem::temp_directory();
        assert!(temp.exists());
    }

    #[test]
    fn test_file_system_current_dir() {
        let current = FileSystem::current_directory();
        assert!(current.is_ok());
    }

    #[test]
    fn test_file_device_create_write_read() {
        let temp_path = temp_dir().join("test_file_device.txt");

        // Write
        {
            let mut device = FileDevice::create(&temp_path).unwrap();
            device.write_all(b"Hello, World!").unwrap();
            device.flush().unwrap();
        }

        // Read
        {
            let mut device = FileDevice::open(&temp_path, IOMode::Read).unwrap();
            let data = device.read_to_end().unwrap();
            assert_eq!(data, b"Hello, World!");
        }

        // Cleanup
        let _ = fs::remove_file(&temp_path);
    }

    #[test]
    fn test_file_system_read_write() {
        let temp_path = temp_dir().join("test_fs_rw.txt");
        let content = b"Test content";

        FileSystem::write(&temp_path, content).unwrap();
        let read_content = FileSystem::read(&temp_path).unwrap();

        assert_eq!(read_content, content);

        // Cleanup
        let _ = FileSystem::remove_file(&temp_path);
    }

    #[test]
    fn test_file_system_path_operations() {
        let path = Path::new("/path/to/file.txt");

        assert_eq!(FileSystem::file_name(path), Some("file.txt".to_string()));
        assert_eq!(FileSystem::extension(path), Some("txt".to_string()));
        assert_eq!(
            FileSystem::parent(path),
            Some(PathBuf::from("/path/to"))
        );
    }

    #[test]
    fn test_buffer_read_write() {
        let mut buffer = Buffer::new();

        buffer.write(b"Hello");
        buffer.write(b" World");

        assert_eq!(buffer.size(), 11);
        assert_eq!(buffer.position(), 11);

        buffer.seek(0);
        let mut data = vec![0u8; 11];
        let read = buffer.read(&mut data);

        assert_eq!(read, 11);
        assert_eq!(&data, b"Hello World");
    }

    #[test]
    fn test_buffer_byte_operations() {
        let mut buffer = Buffer::new();

        buffer.write_byte(65);
        buffer.write_byte(66);
        buffer.write_byte(67);

        buffer.seek(0);

        assert_eq!(buffer.read_byte(), Some(65));
        assert_eq!(buffer.read_byte(), Some(66));
        assert_eq!(buffer.read_byte(), Some(67));
        assert_eq!(buffer.read_byte(), None);
    }

    #[test]
    fn test_buffer_from_data() {
        let data = vec![1, 2, 3, 4, 5];
        let mut buffer = Buffer::from_data(data);

        assert_eq!(buffer.size(), 5);
        assert_eq!(buffer.read_byte(), Some(1));
        assert_eq!(buffer.read_byte(), Some(2));
    }
}
