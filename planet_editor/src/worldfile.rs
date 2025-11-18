use anyhow::{Context, Result};
use byteorder::{BigEndian, ReadBytesExt};
use flate2::read::ZlibDecoder;
use std::collections::HashMap;
use std::fs::File;
use std::io::{Cursor, Read};
use std::path::Path;

use crate::btree;

/// Store types in the world database
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
enum StoreType {
    Metadata = 0,
    TileSector = 1,
    EntitySector = 2,
    UniqueIndex = 3,
    SectorUniques = 4,
}

/// World metadata
#[derive(Debug)]
pub struct WorldMetadata {
    pub width: u32,
    pub height: u32,
    pub metadata_json: Option<serde_json::Value>,
}

/// A tile in the world
#[derive(Debug, Clone, Copy)]
pub struct Tile {
    pub foreground: u16,
    pub foreground_mod: u16,
    pub background: u16,
    pub background_mod: u16,
    pub liquid: u8,
    pub liquid_level: f32,
    pub liquid_pressure: f32,
}

impl Default for Tile {
    fn default() -> Self {
        Self {
            foreground: 0,
            foreground_mod: 0,
            background: 0,
            background_mod: 0,
            liquid: 0,
            liquid_level: 0.0,
            liquid_pressure: 0.0,
        }
    }
}

/// A sector of tiles (32x32)
pub const SECTOR_SIZE: usize = 32;

#[derive(Debug)]
pub struct TileSector {
    pub tiles: [[Tile; SECTOR_SIZE]; SECTOR_SIZE],
}

/// World file representation
#[derive(Debug)]
pub struct WorldFile {
    pub metadata: WorldMetadata,
    pub sectors: HashMap<(u16, u16), TileSector>,
}

impl WorldFile {
    /// Load a world file from disk
    pub fn load<P: AsRef<Path>>(path: P) -> Result<Self> {
        let mut file = File::open(path.as_ref())
            .context("Failed to open world file")?;
        
        // Read BTree header
        let header = btree::read_header(&mut file)
            .context("Failed to read BTree header")?;
        
        // Validate it's a world file
        if header.content_identifier != "World4" {
            anyhow::bail!(
                "Not a Starbound world file: content identifier is '{}', expected 'World4'. \
                This may be a different type of BTree database.",
                header.content_identifier
            );
        }
        
        if header.key_size != 5 {
            anyhow::bail!(
                "Incompatible world file format: key size is {}, expected 5. \
                This world file may be from an older or newer version of Starbound.",
                header.key_size
            );
        }
        
        // Read all entries
        let entries = btree::read_all_entries(&mut file, &header)
            .context("Failed to read BTree entries")?;
        
        // Parse metadata
        let metadata = Self::parse_metadata(&entries)?;
        
        // Parse tile sectors
        let sectors = Self::parse_tile_sectors(&entries)?;
        
        Ok(WorldFile { metadata, sectors })
    }
    
    fn parse_metadata(entries: &HashMap<Vec<u8>, Vec<u8>>) -> Result<WorldMetadata> {
        // Metadata key is: [StoreType::Metadata, 0, 0, 0, 0]
        let metadata_key = vec![StoreType::Metadata as u8, 0, 0, 0, 0];
        
        let metadata_data = entries.get(&metadata_key)
            .context("World metadata not found")?;
        
        // Decompress
        let mut decoder = ZlibDecoder::new(Cursor::new(metadata_data));
        let mut decompressed = Vec::new();
        decoder.read_to_end(&mut decompressed)
            .context("Failed to decompress metadata")?;
        
        let mut cursor = Cursor::new(decompressed);
        
        // Read world size (BigEndian as per Starbound's DataStream)
        let width = cursor.read_u32::<BigEndian>()?;
        let height = cursor.read_u32::<BigEndian>()?;
        
        // Try to read JSON metadata (optional)
        let metadata_json = if cursor.position() < cursor.get_ref().len() as u64 {
            // There's more data - try to parse JSON
            let json_len = cursor.read_u32::<BigEndian>()? as usize;
            if json_len > 0 && json_len < 1024 * 1024 {
                let mut json_bytes = vec![0u8; json_len];
                cursor.read_exact(&mut json_bytes).ok();
                serde_json::from_slice(&json_bytes).ok()
            } else {
                None
            }
        } else {
            None
        };
        
        Ok(WorldMetadata {
            width,
            height,
            metadata_json,
        })
    }
    
    fn parse_tile_sectors(entries: &HashMap<Vec<u8>, Vec<u8>>) -> Result<HashMap<(u16, u16), TileSector>> {
        let mut sectors = HashMap::new();
        
        for (key, value) in entries {
            if key.len() == 5 && key[0] == StoreType::TileSector as u8 {
                // Parse sector coordinates from key (BigEndian)
                let mut cursor = Cursor::new(&key[1..]);
                let sector_x = cursor.read_u16::<BigEndian>()?;
                let sector_y = cursor.read_u16::<BigEndian>()?;
                
                // Decompress tile data
                let mut decoder = ZlibDecoder::new(Cursor::new(value));
                let mut decompressed = Vec::new();
                decoder.read_to_end(&mut decompressed)
                    .context("Failed to decompress tile sector")?;
                
                // Parse tile sector
                let sector = Self::parse_tile_sector(&decompressed)?;
                sectors.insert((sector_x, sector_y), sector);
            }
        }
        
        Ok(sectors)
    }
    
    fn parse_tile_sector(data: &[u8]) -> Result<TileSector> {
        let mut cursor = Cursor::new(data);
        
        // Read generation level (varint)
        let _generation_level = read_varint(&mut cursor)?;
        
        // Read serialization version (varint)
        let _serialization_version = read_varint(&mut cursor)?;
        
        // Read tiles (32x32)
        let mut tiles = [[Tile::default(); SECTOR_SIZE]; SECTOR_SIZE];
        
        for y in 0..SECTOR_SIZE {
            for x in 0..SECTOR_SIZE {
                tiles[y][x] = read_tile(&mut cursor)?;
            }
        }
        
        Ok(TileSector { tiles })
    }
}

/// Read a varint (variable-length integer)
fn read_varint<R: Read>(reader: &mut R) -> Result<u64> {
    let mut result = 0u64;
    let mut shift = 0;
    
    loop {
        let byte = reader.read_u8()?;
        result |= ((byte & 0x7F) as u64) << shift;
        
        if byte & 0x80 == 0 {
            break;
        }
        
        shift += 7;
        if shift >= 64 {
            anyhow::bail!("Varint too long");
        }
    }
    
    Ok(result)
}

/// Read a single tile from the data stream
fn read_tile<R: Read>(reader: &mut R) -> Result<Tile> {
    let mut tile = Tile::default();
    
    // Read foreground material (BigEndian as per Starbound's DataStream)
    tile.foreground = reader.read_u16::<BigEndian>()?;
    
    // Read foreground hue shift
    tile.foreground_mod = reader.read_u16::<BigEndian>()?;
    
    // Read foreground color variant
    let _foreground_variant = reader.read_u8()?;
    
    // Read background material
    tile.background = reader.read_u16::<BigEndian>()?;
    
    // Read background hue shift
    tile.background_mod = reader.read_u16::<BigEndian>()?;
    
    // Read background color variant
    let _background_variant = reader.read_u8()?;
    
    // Read liquid
    tile.liquid = reader.read_u8()?;
    
    // Read liquid level
    tile.liquid_level = reader.read_f32::<BigEndian>()?;
    
    // Read liquid pressure (if liquid exists)
    if tile.liquid != 0 {
        tile.liquid_pressure = reader.read_f32::<BigEndian>()?;
    }
    
    // Read collision (4 bytes)
    let _collision = reader.read_u32::<BigEndian>()?;
    
    // Read block biome index (2 bytes)
    let _block_biome = reader.read_u16::<BigEndian>()?;
    
    // Read environment biome index (2 bytes)
    let _env_biome = reader.read_u16::<BigEndian>()?;
    
    // Read indestructible flag
    let _indestructible = reader.read_u8()?;
    
    Ok(tile)
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_store_type() {
        assert_eq!(StoreType::Metadata as u8, 0);
        assert_eq!(StoreType::TileSector as u8, 1);
    }
}
