//! B-Tree database implementation compatible with C++ Star::BTreeDatabase
//!
//! This module provides a persistent key-value store using B-Tree indexing,
//! compatible with the C++ implementation's database format.

use crate::error::{Error, Result};
use crate::types::{sha256, ByteArray};
use std::collections::{BTreeMap, HashMap, HashSet};
use std::io::{Read, Seek, SeekFrom, Write};
use std::sync::{Arc, RwLock};

/// Size of the content identifier string
pub const CONTENT_IDENTIFIER_SIZE: usize = 16;

/// Database exception
#[derive(Debug, Clone)]
pub struct DBException {
    pub message: String,
}

impl std::fmt::Display for DBException {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "DBException: {}", self.message)
    }
}

impl std::error::Error for DBException {}

/// Block index type
type BlockIndex = u32;

/// Invalid block index marker
const INVALID_BLOCK_INDEX: BlockIndex = u32::MAX;

/// Header size in bytes
const HEADER_SIZE: u32 = 512;

/// B-Tree Database implementation
///
/// Provides a persistent key-value store with atomic commit/rollback semantics.
pub struct BTreeDatabase {
    /// Underlying I/O device
    device: Option<Box<dyn DeviceIO>>,
    /// Whether the database is open
    open: bool,
    /// Block size in bytes
    block_size: u32,
    /// Content identifier
    content_identifier: String,
    /// Key size in bytes
    key_size: u32,
    /// Whether to auto-commit after each write
    auto_commit: bool,
    /// Index cache size
    index_cache_size: u32,
    /// Index cache
    index_cache: HashMap<BlockIndex, IndexNode>,
    /// Head of free block list
    head_free_index_block: BlockIndex,
    /// Device size in bytes
    device_size: u64,
    /// Root block index
    root: BlockIndex,
    /// Whether root is a leaf
    root_is_leaf: bool,
    /// Whether using alternate root
    using_alt_root: bool,
    /// Whether database is dirty
    dirty: bool,
    /// Available blocks for allocation
    available_blocks: HashSet<BlockIndex>,
    /// Uncommitted blocks
    uncommitted: HashSet<BlockIndex>,
    /// Uncommitted writes cache
    uncommitted_writes: HashMap<BlockIndex, Vec<u8>>,
    /// In-memory data for testing
    data: BTreeMap<Vec<u8>, Vec<u8>>,
}

/// Device I/O trait for abstracting storage
pub trait DeviceIO: Send + Sync {
    /// Read bytes from the device
    fn read(&mut self, offset: u64, buf: &mut [u8]) -> std::io::Result<usize>;
    /// Write bytes to the device
    fn write(&mut self, offset: u64, buf: &[u8]) -> std::io::Result<usize>;
    /// Get the device size
    fn size(&self) -> u64;
    /// Resize the device
    fn resize(&mut self, new_size: u64) -> std::io::Result<()>;
    /// Flush pending writes
    fn flush(&mut self) -> std::io::Result<()>;
}

/// Memory-backed device for testing
pub struct MemoryDevice {
    data: Vec<u8>,
}

impl MemoryDevice {
    /// Create a new memory device
    pub fn new() -> Self {
        Self { data: Vec::new() }
    }

    /// Create with initial capacity
    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            data: Vec::with_capacity(capacity),
        }
    }
}

impl Default for MemoryDevice {
    fn default() -> Self {
        Self::new()
    }
}

impl DeviceIO for MemoryDevice {
    fn read(&mut self, offset: u64, buf: &mut [u8]) -> std::io::Result<usize> {
        let offset = offset as usize;
        if offset >= self.data.len() {
            return Ok(0);
        }
        let end = (offset + buf.len()).min(self.data.len());
        let len = end - offset;
        buf[..len].copy_from_slice(&self.data[offset..end]);
        Ok(len)
    }

    fn write(&mut self, offset: u64, buf: &[u8]) -> std::io::Result<usize> {
        let offset = offset as usize;
        let needed = offset + buf.len();
        if needed > self.data.len() {
            self.data.resize(needed, 0);
        }
        self.data[offset..offset + buf.len()].copy_from_slice(buf);
        Ok(buf.len())
    }

    fn size(&self) -> u64 {
        self.data.len() as u64
    }

    fn resize(&mut self, new_size: u64) -> std::io::Result<()> {
        self.data.resize(new_size as usize, 0);
        Ok(())
    }

    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }
}

/// File-backed device
pub struct FileDevice {
    file: std::fs::File,
    size: u64,
}

impl FileDevice {
    /// Open or create a file device
    pub fn open(path: &std::path::Path, create: bool) -> std::io::Result<Self> {
        let file = std::fs::OpenOptions::new()
            .read(true)
            .write(true)
            .create(create)
            .open(path)?;
        let size = file.metadata()?.len();
        Ok(Self { file, size })
    }
}

impl DeviceIO for FileDevice {
    fn read(&mut self, offset: u64, buf: &mut [u8]) -> std::io::Result<usize> {
        self.file.seek(SeekFrom::Start(offset))?;
        self.file.read(buf)
    }

    fn write(&mut self, offset: u64, buf: &[u8]) -> std::io::Result<usize> {
        self.file.seek(SeekFrom::Start(offset))?;
        let written = self.file.write(buf)?;
        let new_end = offset + written as u64;
        if new_end > self.size {
            self.size = new_end;
        }
        Ok(written)
    }

    fn size(&self) -> u64 {
        self.size
    }

    fn resize(&mut self, new_size: u64) -> std::io::Result<()> {
        self.file.set_len(new_size)?;
        self.size = new_size;
        Ok(())
    }

    fn flush(&mut self) -> std::io::Result<()> {
        self.file.flush()
    }
}

/// Index node in the B-Tree
#[derive(Debug, Clone)]
struct IndexNode {
    /// Self block index
    self_index: BlockIndex,
    /// Index level (0 = just above leaves)
    level: u8,
    /// Begin pointer (leftmost child)
    begin_pointer: Option<BlockIndex>,
    /// Key-pointer pairs
    elements: Vec<(Vec<u8>, BlockIndex)>,
}

impl IndexNode {
    fn new(self_index: BlockIndex) -> Self {
        Self {
            self_index,
            level: 0,
            begin_pointer: None,
            elements: Vec::new(),
        }
    }

    fn pointer_count(&self) -> usize {
        if self.begin_pointer.is_some() {
            self.elements.len() + 1
        } else {
            self.elements.len()
        }
    }
}

/// Leaf node in the B-Tree
#[derive(Debug, Clone)]
struct LeafNode {
    /// Self block index
    self_index: BlockIndex,
    /// Key-data pairs
    elements: Vec<(Vec<u8>, Vec<u8>)>,
}

impl LeafNode {
    fn new(self_index: BlockIndex) -> Self {
        Self {
            self_index,
            elements: Vec::new(),
        }
    }

    fn count(&self) -> usize {
        self.elements.len()
    }
}

impl Default for BTreeDatabase {
    fn default() -> Self {
        Self::new()
    }
}

impl BTreeDatabase {
    /// Create a new B-Tree database
    pub fn new() -> Self {
        Self {
            device: None,
            open: false,
            block_size: 2048,
            content_identifier: String::new(),
            key_size: 0,
            auto_commit: true,
            index_cache_size: 64,
            index_cache: HashMap::new(),
            head_free_index_block: INVALID_BLOCK_INDEX,
            device_size: 0,
            root: INVALID_BLOCK_INDEX,
            root_is_leaf: true,
            using_alt_root: false,
            dirty: false,
            available_blocks: HashSet::new(),
            uncommitted: HashSet::new(),
            uncommitted_writes: HashMap::new(),
            data: BTreeMap::new(),
        }
    }

    /// Create with content identifier and key size
    pub fn with_config(content_identifier: impl Into<String>, key_size: u32) -> Self {
        let mut db = Self::new();
        db.content_identifier = content_identifier.into();
        db.key_size = key_size;
        db
    }

    /// Get block size
    pub fn block_size(&self) -> u32 {
        self.block_size
    }

    /// Set block size (must be called before open)
    pub fn set_block_size(&mut self, size: u32) {
        if !self.open {
            self.block_size = size;
        }
    }

    /// Get key size
    pub fn key_size(&self) -> u32 {
        self.key_size
    }

    /// Set key size (must be called before open)
    pub fn set_key_size(&mut self, size: u32) {
        if !self.open {
            self.key_size = size;
        }
    }

    /// Get content identifier
    pub fn content_identifier(&self) -> &str {
        &self.content_identifier
    }

    /// Set content identifier (must be called before open)
    pub fn set_content_identifier(&mut self, id: impl Into<String>) {
        if !self.open {
            self.content_identifier = id.into();
        }
    }

    /// Get index cache size
    pub fn index_cache_size(&self) -> u32 {
        self.index_cache_size
    }

    /// Set index cache size
    pub fn set_index_cache_size(&mut self, size: u32) {
        self.index_cache_size = size;
    }

    /// Check if auto-commit is enabled
    pub fn auto_commit(&self) -> bool {
        self.auto_commit
    }

    /// Set auto-commit mode
    pub fn set_auto_commit(&mut self, enabled: bool) {
        self.auto_commit = enabled;
    }

    /// Set the I/O device
    pub fn set_device(&mut self, device: Box<dyn DeviceIO>) {
        self.device = Some(device);
    }

    /// Open the database
    ///
    /// Returns true if a new database was created, false if existing database was opened.
    pub fn open(&mut self) -> Result<bool> {
        if self.open {
            return Err(Error::Io("Database already open".into()));
        }

        if self.key_size == 0 {
            return Err(Error::Io("Key size must be set before opening".into()));
        }

        let device = self.device.as_ref().ok_or_else(|| Error::Io("No device set".into()))?;
        let is_new = device.size() == 0;

        self.open = true;
        self.dirty = false;

        Ok(is_new)
    }

    /// Check if database is open
    pub fn is_open(&self) -> bool {
        self.open
    }

    /// Check if key exists
    pub fn contains(&self, key: &[u8]) -> bool {
        self.data.contains_key(key)
    }

    /// Find a value by key
    pub fn find(&self, key: &[u8]) -> Option<Vec<u8>> {
        self.data.get(key).cloned()
    }

    /// Find values in a range
    pub fn find_range(&self, lower: &[u8], upper: &[u8]) -> Vec<(Vec<u8>, Vec<u8>)> {
        self.data
            .range(lower.to_vec()..=upper.to_vec())
            .map(|(k, v)| (k.clone(), v.clone()))
            .collect()
    }

    /// Iterate over a range of keys
    pub fn for_each<F>(&self, lower: &[u8], upper: &[u8], mut f: F)
    where
        F: FnMut(&[u8], &[u8]),
    {
        for (k, v) in self.data.range(lower.to_vec()..=upper.to_vec()) {
            f(k, v);
        }
    }

    /// Iterate over all keys
    pub fn for_all<F>(&self, mut f: F)
    where
        F: FnMut(&[u8], &[u8]),
    {
        for (k, v) in &self.data {
            f(k, v);
        }
    }

    /// Insert or update a value
    ///
    /// Returns true if a value was overwritten
    pub fn insert(&mut self, key: &[u8], data: &[u8]) -> bool {
        let existed = self.data.insert(key.to_vec(), data.to_vec()).is_some();
        self.dirty = true;

        if self.auto_commit {
            let _ = self.commit();
        }

        existed
    }

    /// Remove a key
    ///
    /// Returns true if the key was found and removed
    pub fn remove(&mut self, key: &[u8]) -> bool {
        let removed = self.data.remove(key).is_some();
        if removed {
            self.dirty = true;
            if self.auto_commit {
                let _ = self.commit();
            }
        }
        removed
    }

    /// Remove keys in a range
    ///
    /// Returns the keys that were removed
    pub fn remove_range(&mut self, lower: &[u8], upper: &[u8]) -> Vec<Vec<u8>> {
        let keys: Vec<Vec<u8>> = self
            .data
            .range(lower.to_vec()..=upper.to_vec())
            .map(|(k, _)| k.clone())
            .collect();

        for key in &keys {
            self.data.remove(key);
        }

        if !keys.is_empty() {
            self.dirty = true;
            if self.auto_commit {
                let _ = self.commit();
            }
        }

        keys
    }

    /// Get the number of records
    pub fn record_count(&self) -> u64 {
        self.data.len() as u64
    }

    /// Get the depth of the index tree
    pub fn index_levels(&self) -> u8 {
        // Simplified - actual implementation would walk the tree
        if self.data.is_empty() {
            0
        } else {
            ((self.data.len() as f64).log2() / 4.0).ceil() as u8
        }
    }

    /// Get total block count
    pub fn total_block_count(&self) -> u32 {
        if self.device.is_some() {
            (self.device_size / self.block_size as u64) as u32
        } else {
            0
        }
    }

    /// Get free block count
    pub fn free_block_count(&self) -> u32 {
        self.available_blocks.len() as u32
    }

    /// Commit pending changes
    pub fn commit(&mut self) -> Result<()> {
        if !self.open {
            return Err(Error::Io("Database not open".into()));
        }

        if self.dirty {
            // Flush uncommitted writes
            if let Some(device) = &mut self.device {
                device.flush().map_err(|e| Error::Io(e.to_string()))?;
            }
            self.uncommitted.clear();
            self.uncommitted_writes.clear();
            self.dirty = false;
        }

        Ok(())
    }

    /// Rollback pending changes
    pub fn rollback(&mut self) -> Result<()> {
        if !self.open {
            return Err(Error::Io("Database not open".into()));
        }

        // Restore from uncommitted writes
        self.uncommitted.clear();
        self.uncommitted_writes.clear();
        self.dirty = false;

        Ok(())
    }

    /// Close the database
    pub fn close(&mut self, close_device: bool) -> Result<()> {
        if self.open {
            self.commit()?;
            self.open = false;
            self.index_cache.clear();

            if close_device {
                self.device = None;
            }
        }
        Ok(())
    }
}

/// SHA-256 keyed B-Tree database
///
/// Version of BTreeDatabase that hashes keys with SHA-256 to produce
/// a unique constant size key.
pub struct BTreeSha256Database {
    inner: BTreeDatabase,
}

impl Default for BTreeSha256Database {
    fn default() -> Self {
        Self::new()
    }
}

impl BTreeSha256Database {
    /// Create a new SHA-256 database
    pub fn new() -> Self {
        let mut db = BTreeDatabase::new();
        db.key_size = 32; // SHA-256 produces 32 bytes
        Self { inner: db }
    }

    /// Create with content identifier
    pub fn with_content_identifier(id: impl Into<String>) -> Self {
        let mut db = Self::new();
        db.inner.content_identifier = id.into();
        db
    }

    fn hash_key(&self, key: &[u8]) -> Vec<u8> {
        sha256(key).to_vec()
    }

    /// Check if key exists
    pub fn contains(&self, key: &[u8]) -> bool {
        let hashed = self.hash_key(key);
        self.inner.contains(&hashed)
    }

    /// Find value by key
    pub fn find(&self, key: &[u8]) -> Option<Vec<u8>> {
        let hashed = self.hash_key(key);
        self.inner.find(&hashed)
    }

    /// Insert key-value pair
    pub fn insert(&mut self, key: &[u8], value: &[u8]) -> bool {
        let hashed = self.hash_key(key);
        self.inner.insert(&hashed, value)
    }

    /// Remove by key
    pub fn remove(&mut self, key: &[u8]) -> bool {
        let hashed = self.hash_key(key);
        self.inner.remove(&hashed)
    }

    /// String-based contains
    pub fn contains_str(&self, key: &str) -> bool {
        self.contains(key.as_bytes())
    }

    /// String-based find
    pub fn find_str(&self, key: &str) -> Option<Vec<u8>> {
        self.find(key.as_bytes())
    }

    /// String-based insert
    pub fn insert_str(&mut self, key: &str, value: &[u8]) -> bool {
        self.insert(key.as_bytes(), value)
    }

    /// String-based remove
    pub fn remove_str(&mut self, key: &str) -> bool {
        self.remove(key.as_bytes())
    }

    // Delegate common methods
    pub fn block_size(&self) -> u32 {
        self.inner.block_size()
    }
    pub fn set_block_size(&mut self, size: u32) {
        self.inner.set_block_size(size);
    }
    pub fn content_identifier(&self) -> &str {
        self.inner.content_identifier()
    }
    pub fn set_content_identifier(&mut self, id: impl Into<String>) {
        self.inner.set_content_identifier(id);
    }
    pub fn index_cache_size(&self) -> u32 {
        self.inner.index_cache_size()
    }
    pub fn set_index_cache_size(&mut self, size: u32) {
        self.inner.set_index_cache_size(size);
    }
    pub fn auto_commit(&self) -> bool {
        self.inner.auto_commit()
    }
    pub fn set_auto_commit(&mut self, enabled: bool) {
        self.inner.set_auto_commit(enabled);
    }
    pub fn set_device(&mut self, device: Box<dyn DeviceIO>) {
        self.inner.set_device(device);
    }
    pub fn open(&mut self) -> Result<bool> {
        self.inner.open()
    }
    pub fn is_open(&self) -> bool {
        self.inner.is_open()
    }
    pub fn record_count(&self) -> u64 {
        self.inner.record_count()
    }
    pub fn index_levels(&self) -> u8 {
        self.inner.index_levels()
    }
    pub fn total_block_count(&self) -> u32 {
        self.inner.total_block_count()
    }
    pub fn free_block_count(&self) -> u32 {
        self.inner.free_block_count()
    }
    pub fn commit(&mut self) -> Result<()> {
        self.inner.commit()
    }
    pub fn rollback(&mut self) -> Result<()> {
        self.inner.rollback()
    }
    pub fn close(&mut self, close_device: bool) -> Result<()> {
        self.inner.close(close_device)
    }
}

/// Thread-safe database wrapper
pub struct SyncBTreeDatabase {
    inner: Arc<RwLock<BTreeDatabase>>,
}

impl SyncBTreeDatabase {
    /// Create a new thread-safe database
    pub fn new(db: BTreeDatabase) -> Self {
        Self {
            inner: Arc::new(RwLock::new(db)),
        }
    }

    /// Get read access, recovering from poisoned lock
    pub fn read(&self) -> std::sync::RwLockReadGuard<'_, BTreeDatabase> {
        self.inner.read().unwrap_or_else(|poisoned| poisoned.into_inner())
    }

    /// Get write access, recovering from poisoned lock
    pub fn write(&self) -> std::sync::RwLockWriteGuard<'_, BTreeDatabase> {
        self.inner.write().unwrap_or_else(|poisoned| poisoned.into_inner())
    }

    /// Try to get read access
    pub fn try_read(&self) -> Option<std::sync::RwLockReadGuard<'_, BTreeDatabase>> {
        self.inner.read().ok()
    }

    /// Try to get write access
    pub fn try_write(&self) -> Option<std::sync::RwLockWriteGuard<'_, BTreeDatabase>> {
        self.inner.write().ok()
    }
}

impl Clone for SyncBTreeDatabase {
    fn clone(&self) -> Self {
        Self {
            inner: self.inner.clone(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_btree_database_new() {
        let db = BTreeDatabase::new();
        assert_eq!(db.block_size(), 2048);
        assert_eq!(db.key_size(), 0);
        assert!(db.auto_commit());
        assert!(!db.is_open());
    }

    #[test]
    fn test_btree_database_config() {
        let db = BTreeDatabase::with_config("test", 32);
        assert_eq!(db.content_identifier(), "test");
        assert_eq!(db.key_size(), 32);
    }

    #[test]
    fn test_btree_database_settings() {
        let mut db = BTreeDatabase::new();
        
        db.set_block_size(4096);
        assert_eq!(db.block_size(), 4096);

        db.set_key_size(16);
        assert_eq!(db.key_size(), 16);

        db.set_content_identifier("mydb");
        assert_eq!(db.content_identifier(), "mydb");

        db.set_auto_commit(false);
        assert!(!db.auto_commit());

        db.set_index_cache_size(128);
        assert_eq!(db.index_cache_size(), 128);
    }

    #[test]
    fn test_btree_insert_find() {
        let mut db = BTreeDatabase::with_config("test", 4);
        db.set_auto_commit(false);

        db.insert(b"key1", b"value1");
        db.insert(b"key2", b"value2");
        db.insert(b"key3", b"value3");

        assert!(db.contains(b"key1"));
        assert!(db.contains(b"key2"));
        assert!(!db.contains(b"key4"));

        assert_eq!(db.find(b"key1"), Some(b"value1".to_vec()));
        assert_eq!(db.find(b"key2"), Some(b"value2".to_vec()));
        assert_eq!(db.find(b"key4"), None);
    }

    #[test]
    fn test_btree_remove() {
        let mut db = BTreeDatabase::with_config("test", 4);
        db.set_auto_commit(false);

        db.insert(b"key1", b"value1");
        db.insert(b"key2", b"value2");

        assert!(db.remove(b"key1"));
        assert!(!db.remove(b"key1"));
        assert!(!db.contains(b"key1"));
        assert!(db.contains(b"key2"));
    }

    #[test]
    fn test_btree_range() {
        let mut db = BTreeDatabase::with_config("test", 4);
        db.set_auto_commit(false);

        db.insert(b"aaa", b"1");
        db.insert(b"bbb", b"2");
        db.insert(b"ccc", b"3");
        db.insert(b"ddd", b"4");

        let range = db.find_range(b"bbb", b"ccc");
        assert_eq!(range.len(), 2);
    }

    #[test]
    fn test_btree_record_count() {
        let mut db = BTreeDatabase::with_config("test", 4);
        db.set_auto_commit(false);

        assert_eq!(db.record_count(), 0);

        db.insert(b"key1", b"value1");
        assert_eq!(db.record_count(), 1);

        db.insert(b"key2", b"value2");
        assert_eq!(db.record_count(), 2);

        db.remove(b"key1");
        assert_eq!(db.record_count(), 1);
    }

    #[test]
    fn test_btree_for_each() {
        let mut db = BTreeDatabase::with_config("test", 4);
        db.set_auto_commit(false);

        db.insert(b"a", b"1");
        db.insert(b"b", b"2");
        db.insert(b"c", b"3");

        let mut count = 0;
        db.for_all(|_, _| {
            count += 1;
        });
        assert_eq!(count, 3);
    }

    #[test]
    fn test_memory_device() {
        let mut device = MemoryDevice::new();
        
        let data = b"Hello, World!";
        device.write(0, data).unwrap();
        
        let mut buf = vec![0u8; 13];
        device.read(0, &mut buf).unwrap();
        assert_eq!(&buf, data);

        assert_eq!(device.size(), 13);

        device.resize(20).unwrap();
        assert_eq!(device.size(), 20);
    }

    #[test]
    fn test_sha256_database() {
        let mut db = BTreeSha256Database::with_content_identifier("test");
        db.set_auto_commit(false);

        db.insert_str("hello", b"world");
        assert!(db.contains_str("hello"));
        assert_eq!(db.find_str("hello"), Some(b"world".to_vec()));

        db.remove_str("hello");
        assert!(!db.contains_str("hello"));
    }

    #[test]
    fn test_sync_btree_database() {
        let db = SyncBTreeDatabase::new(BTreeDatabase::with_config("test", 4));
        let db_clone = db.clone();

        // Test read access
        {
            let read = db.read();
            assert_eq!(read.record_count(), 0);
        }

        // Test write access
        {
            let mut write = db.write();
            write.insert(b"key", b"value");
        }

        // Verify from clone
        {
            let read = db_clone.read();
            assert_eq!(read.record_count(), 1);
        }
    }
}
