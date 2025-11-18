use anyhow::{Context, Result};
use byteorder::{LittleEndian, ReadBytesExt};
use std::collections::HashMap;
use std::fs::File;
use std::io::{Read, Seek, SeekFrom};

const HEADER_SIZE: u64 = 512;
const VERSION_MAGIC: &[u8; 8] = b"BTreeDB5";

/// BTreeDatabase header information
#[derive(Debug)]
pub struct BTreeHeader {
    pub content_identifier: String,
    pub key_size: u32,
    pub block_size: u32,
    pub root_node: u32,
    pub is_leaf: bool,
    pub free_block_count: u32,
}

/// Reads BTree database header
pub fn read_header(file: &mut File) -> Result<BTreeHeader> {
    file.seek(SeekFrom::Start(0))?;
    
    // Read magic
    let mut magic = [0u8; 8];
    file.read_exact(&mut magic)?;
    if &magic != VERSION_MAGIC {
        anyhow::bail!("Invalid BTree magic: expected BTreeDB5");
    }
    
    // Read content identifier (16 bytes)
    let mut content_id_bytes = [0u8; 16];
    file.read_exact(&mut content_id_bytes)?;
    let content_identifier = String::from_utf8_lossy(&content_id_bytes)
        .trim_end_matches('\0')
        .to_string();
    
    // Read key size
    let key_size = file.read_u32::<LittleEndian>()?;
    
    // Skip unused bytes (4 bytes)
    file.read_u32::<LittleEndian>()?;
    
    // Read block size
    let block_size = file.read_u32::<LittleEndian>()?;
    
    // Read root node
    let root_node = file.read_u32::<LittleEndian>()?;
    
    // Read is_leaf flag
    let is_leaf = file.read_u8()? != 0;
    
    // Skip to free block count (at offset 45)
    file.seek(SeekFrom::Start(45))?;
    let free_block_count = file.read_u32::<LittleEndian>()?;
    
    Ok(BTreeHeader {
        content_identifier,
        key_size,
        block_size,
        root_node,
        is_leaf,
        free_block_count,
    })
}

/// Reads all key-value pairs from the BTree database
pub fn read_all_entries(file: &mut File, header: &BTreeHeader) -> Result<HashMap<Vec<u8>, Vec<u8>>> {
    let mut entries = HashMap::new();
    
    if header.root_node == u32::MAX {
        // Empty database
        return Ok(entries);
    }
    
    // Read the root node
    read_node(file, header, header.root_node, header.is_leaf, &mut entries)?;
    
    Ok(entries)
}

fn read_node(
    file: &mut File,
    header: &BTreeHeader,
    block_index: u32,
    is_leaf: bool,
    entries: &mut HashMap<Vec<u8>, Vec<u8>>,
) -> Result<()> {
    // Calculate block position
    let block_pos = HEADER_SIZE + (block_index as u64 * header.block_size as u64);
    file.seek(SeekFrom::Start(block_pos))?;
    
    // Read block type (0 = index, 1 = leaf)
    let block_type = file.read_u8()?;
    
    // Read entry count
    let count = file.read_u32::<LittleEndian>()? as usize;
    
    if block_type == 1 || is_leaf {
        // Leaf node - read key-value pairs
        for _ in 0..count {
            // Read key
            let mut key = vec![0u8; header.key_size as usize];
            file.read_exact(&mut key)?;
            
            // Read value size
            let value_size = file.read_u32::<LittleEndian>()? as usize;
            
            // Read value
            let mut value = vec![0u8; value_size];
            file.read_exact(&mut value)?;
            
            entries.insert(key, value);
        }
    } else {
        // Index node - recursively read child nodes
        // Read keys
        let mut keys = Vec::new();
        for _ in 0..count {
            let mut key = vec![0u8; header.key_size as usize];
            file.read_exact(&mut key)?;
            keys.push(key);
        }
        
        // Read child pointers (count + 1)
        let mut children = Vec::new();
        for _ in 0..=count {
            let child_block = file.read_u32::<LittleEndian>()?;
            let child_is_leaf = file.read_u8()? != 0;
            children.push((child_block, child_is_leaf));
        }
        
        // Recursively read children
        for (child_block, child_is_leaf) in children {
            if child_block != u32::MAX {
                read_node(file, header, child_block, child_is_leaf, entries)?;
            }
        }
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
