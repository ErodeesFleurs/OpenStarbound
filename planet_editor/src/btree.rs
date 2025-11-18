use anyhow::{Context, Result};
use byteorder::{BigEndian, ReadBytesExt};
use std::collections::HashMap;
use std::fs::File;
use std::io::{Cursor, Read, Seek, SeekFrom};

const HEADER_SIZE: u64 = 512;
const VERSION_MAGIC: &[u8; 8] = b"BTreeDB5";
const INDEX_MAGIC: &[u8; 2] = b"II";
const LEAF_MAGIC: &[u8; 2] = b"LL";
const INVALID_BLOCK: u32 = u32::MAX;

/// BTreeDatabase header information
#[derive(Debug)]
pub struct BTreeHeader {
    pub content_identifier: String,
    pub key_size: u32,
    pub block_size: u32,
    pub root_node: u32,
    pub is_leaf: bool,
}

/// Reads BTree database header
pub fn read_header(file: &mut File) -> Result<BTreeHeader> {
    file.seek(SeekFrom::Start(0))?;
    
    // Read magic (8 bytes)
    let mut magic = [0u8; 8];
    file.read_exact(&mut magic)?;
    if &magic != VERSION_MAGIC {
        anyhow::bail!("Invalid BTree magic: expected BTreeDB5, got {:?}", 
            String::from_utf8_lossy(&magic));
    }
    
    // Read block size (4 bytes) - BigEndian as per Starbound's DataStream default
    let block_size = file.read_u32::<BigEndian>()?;
    
    // Read content identifier (16 bytes)
    let mut content_id_bytes = [0u8; 16];
    file.read_exact(&mut content_id_bytes)?;
    let content_identifier = String::from_utf8_lossy(&content_id_bytes)
        .trim_end_matches('\0')
        .to_string();
    
    // Read key size (4 bytes) - BigEndian
    let key_size = file.read_u32::<BigEndian>()?;
    
    // Validate key_size is reasonable (between 1 and 256 bytes)
    if key_size == 0 || key_size > 256 {
        anyhow::bail!(
            "Invalid key size: {} (expected 1-256). File may be corrupted or not a valid BTreeDatabase.",
            key_size
        );
    }
    
    // Seek to root selector bit (offset 32)
    file.seek(SeekFrom::Start(32))?;
    let using_alt_root = file.read_u8()? != 0;
    
    // Seek to root info (offset 33 + (using_alt_root ? 17 : 0))
    let root_info_offset = if using_alt_root { 33 + 17 } else { 33 };
    file.seek(SeekFrom::Start(root_info_offset))?;
    
    // Skip head free index block (4 bytes)
    file.read_u32::<BigEndian>()?;
    
    // Skip device size (8 bytes)
    file.read_u64::<BigEndian>()?;
    
    // Read root block index (4 bytes)
    let root_node = file.read_u32::<BigEndian>()?;
    
    // Read root is leaf flag (1 byte)
    let is_leaf = file.read_u8()? != 0;
    
    Ok(BTreeHeader {
        content_identifier,
        key_size,
        block_size,
        root_node,
        is_leaf,
    })
}

/// Reads all key-value pairs from the BTree database
pub fn read_all_entries(file: &mut File, header: &BTreeHeader) -> Result<HashMap<Vec<u8>, Vec<u8>>> {
    let mut entries = HashMap::new();
    
    if header.root_node == INVALID_BLOCK {
        // Empty database
        return Ok(entries);
    }
    
    // Read the root node
    read_node(file, header, header.root_node, header.is_leaf, &mut entries)?;
    
    Ok(entries)
}

fn read_block(file: &mut File, header: &BTreeHeader, block_index: u32) -> Result<Vec<u8>> {
    let block_pos = HEADER_SIZE + (block_index as u64 * header.block_size as u64);
    file.seek(SeekFrom::Start(block_pos))?;
    
    let mut block = vec![0u8; header.block_size as usize];
    file.read_exact(&mut block)?;
    
    Ok(block)
}

fn read_node(
    file: &mut File,
    header: &BTreeHeader,
    block_index: u32,
    is_leaf: bool,
    entries: &mut HashMap<Vec<u8>, Vec<u8>>,
) -> Result<()> {
    let block = read_block(file, header, block_index)?;
    let mut cursor = Cursor::new(&block);
    
    // Read magic (2 bytes)
    let mut magic = [0u8; 2];
    cursor.read_exact(&mut magic)?;
    
    if is_leaf {
        // Verify leaf magic
        if &magic != LEAF_MAGIC {
            anyhow::bail!("Invalid leaf magic at block {}", block_index);
        }
        
        // Read leaf entries - they can span multiple blocks
        read_leaf_entries(file, header, block_index, entries)?;
    } else {
        // Verify index magic
        if &magic != INDEX_MAGIC {
            anyhow::bail!("Invalid index magic at block {}", block_index);
        }
        
        // Read index node
        // Level (u8) - if 0, children are leaves
        let level = cursor.read_u8()?;
        let children_are_leaves = level == 0;
        
        // Count (u32)
        let count = cursor.read_u32::<BigEndian>()? as usize;
        
        // Begin pointer (u32)
        let begin_pointer = cursor.read_u32::<BigEndian>()?;
        
        // Read keys and pointers
        let mut pointers = Vec::new();
        for _ in 0..count {
            let mut key = vec![0u8; header.key_size as usize];
            cursor.read_exact(&mut key)?;
            let pointer = cursor.read_u32::<BigEndian>()?;
            pointers.push(pointer);
        }
        
        // Recursively read children
        // First child is begin_pointer
        if begin_pointer != INVALID_BLOCK {
            read_node(file, header, begin_pointer, children_are_leaves, entries)?;
        }
        
        // Then read each keyed child
        for pointer in pointers {
            if pointer != INVALID_BLOCK {
                read_node(file, header, pointer, children_are_leaves, entries)?;
            }
        }
    }
    
    Ok(())
}

/// Read a VLQ (Variable Length Quantity) unsigned integer
fn read_vlq_u64<R: Read>(reader: &mut R) -> Result<u64> {
    let mut result = 0u64;
    for i in 0..10 {
        let byte = reader.read_u8()?;
        // Take lower 7 bits and shift into result
        result = (result << 7) | ((byte & 0x7F) as u64);
        // If MSB is 0, we're done
        if (byte & 0x80) == 0 {
            return Ok(result);
        }
        if i == 9 {
            anyhow::bail!("VLQ integer too long (>10 bytes)");
        }
    }
    unreachable!()
}

fn read_leaf_entries(
    file: &mut File,
    header: &BTreeHeader,
    start_block: u32,
    entries: &mut HashMap<Vec<u8>, Vec<u8>>,
) -> Result<()> {
    let mut current_block = start_block;
    let mut data = Vec::new();
    
    // Read all blocks in the leaf chain
    loop {
        let block = read_block(file, header, current_block)?;
        let mut cursor = Cursor::new(&block);
        
        // Skip magic (2 bytes)
        cursor.set_position(2);
        
        // Read data up to the pointer position (last 4 bytes)
        let data_end = (header.block_size as usize) - 4;
        let chunk_size = data_end - 2;
        let mut chunk = vec![0u8; chunk_size];
        cursor.read_exact(&mut chunk)?;
        data.extend_from_slice(&chunk);
        
        // Read next block pointer
        cursor.set_position(data_end as u64);
        let next_block = cursor.read_u32::<BigEndian>()?;
        
        if next_block == INVALID_BLOCK {
            break;
        }
        
        current_block = next_block;
    }
    
    // Now parse the concatenated data
    let mut cursor = Cursor::new(&data);
    
    // Read count (VLQ encoded)
    let count = read_vlq_u64(&mut cursor).context("Failed to read entry count from leaf")? as usize;
    
    // Validate count is reasonable
    if count > 100000 {
        anyhow::bail!("Unreasonable entry count in leaf: {} (may indicate corruption)", count);
    }
    
    // Read each entry
    for i in 0..count {
        // Read key
        let mut key = vec![0u8; header.key_size as usize];
        cursor.read_exact(&mut key)
            .with_context(|| format!("Failed to read key for entry {} of {}", i, count))?;
        
        // Read value (ByteArray format: VLQ length + data)
        let value_len = read_vlq_u64(&mut cursor)
            .with_context(|| format!("Failed to read value length for entry {} of {}", i, count))? as usize;
        
        // Validate value_len is reasonable
        if value_len > 10 * 1024 * 1024 {
            anyhow::bail!("Unreasonable value length for entry {}: {} bytes (may indicate corruption)", i, value_len);
        }
        
        let mut value = vec![0u8; value_len];
        cursor.read_exact(&mut value)
            .with_context(|| format!("Failed to read value data for entry {} of {} (expected {} bytes, {} bytes remaining)", 
                i, count, value_len, data.len() - cursor.position() as usize))?;
        
        entries.insert(key, value);
    }
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_version_magic() {
        assert_eq!(VERSION_MAGIC, b"BTreeDB5");
    }
}
