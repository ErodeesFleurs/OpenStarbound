use anyhow::{Context, Result};
use serde::{Deserialize, Serialize};
use std::fs;
use std::path::Path;

/// Represents a tile in the planet map
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub struct Tile {
    /// Foreground material ID
    pub foreground: u16,
    /// Background material ID
    pub background: u16,
    /// Foreground modification
    pub foreground_mod: u16,
    /// Background modification
    pub background_mod: u16,
}

impl Default for Tile {
    fn default() -> Self {
        Self {
            foreground: 0, // Empty material
            background: 0, // Empty material
            foreground_mod: 0,
            background_mod: 0,
        }
    }
}

/// Represents a planet map with tiles and metadata
#[derive(Debug, Serialize, Deserialize)]
pub struct PlanetMap {
    /// Width of the map
    pub width: u32,
    /// Height of the map
    pub height: u32,
    /// World seed
    pub seed: u64,
    /// World name
    pub name: String,
    /// Tile data (row-major order)
    pub tiles: Vec<Tile>,
}

impl PlanetMap {
    /// Create a new planet map with the given dimensions
    pub fn new(width: u32, height: u32) -> Self {
        let tile_count = (width * height) as usize;
        Self {
            width,
            height,
            seed: 0,
            name: String::from("Unnamed Planet"),
            tiles: vec![Tile::default(); tile_count],
        }
    }

    /// Load a planet map from a JSON file
    pub fn load<P: AsRef<Path>>(path: P) -> Result<Self> {
        let content = fs::read_to_string(path.as_ref())
            .context("Failed to read planet map file")?;
        let map: PlanetMap = serde_json::from_str(&content)
            .context("Failed to parse planet map JSON")?;
        
        // Validate dimensions
        if map.tiles.len() != (map.width * map.height) as usize {
            anyhow::bail!(
                "Invalid planet map: expected {} tiles, but found {}",
                map.width * map.height,
                map.tiles.len()
            );
        }
        
        Ok(map)
    }

    /// Save the planet map to a JSON file
    pub fn save<P: AsRef<Path>>(&self, path: P) -> Result<()> {
        let json = serde_json::to_string_pretty(self)
            .context("Failed to serialize planet map")?;
        fs::write(path.as_ref(), json)
            .context("Failed to write planet map file")?;
        Ok(())
    }

    /// Get a tile at the given coordinates
    pub fn get_tile(&self, x: u32, y: u32) -> Option<&Tile> {
        if x >= self.width || y >= self.height {
            return None;
        }
        let index = (y * self.width + x) as usize;
        self.tiles.get(index)
    }

    /// Get a mutable reference to a tile at the given coordinates
    pub fn get_tile_mut(&mut self, x: u32, y: u32) -> Option<&mut Tile> {
        if x >= self.width || y >= self.height {
            return None;
        }
        let index = (y * self.width + x) as usize;
        self.tiles.get_mut(index)
    }

    /// Set a tile at the given coordinates
    pub fn set_tile(&mut self, x: u32, y: u32, tile: Tile) -> Result<()> {
        if x >= self.width || y >= self.height {
            anyhow::bail!("Coordinates ({}, {}) out of bounds ({}x{})", x, y, self.width, self.height);
        }
        let index = (y * self.width + x) as usize;
        self.tiles[index] = tile;
        Ok(())
    }

    /// Print information about the planet map
    pub fn print_info(&self) {
        println!("Planet Map Information:");
        println!("  Name: {}", self.name);
        println!("  Dimensions: {}x{}", self.width, self.height);
        println!("  Seed: {}", self.seed);
        println!("  Total tiles: {}", self.tiles.len());
        
        // Count different material types
        let mut foreground_count = 0;
        let mut background_count = 0;
        for tile in &self.tiles {
            if tile.foreground != 0 {
                foreground_count += 1;
            }
            if tile.background != 0 {
                background_count += 1;
            }
        }
        println!("  Foreground tiles: {}", foreground_count);
        println!("  Background tiles: {}", background_count);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new_planet_map() {
        let map = PlanetMap::new(100, 100);
        assert_eq!(map.width, 100);
        assert_eq!(map.height, 100);
        assert_eq!(map.tiles.len(), 10000);
    }

    #[test]
    fn test_get_set_tile() {
        let mut map = PlanetMap::new(10, 10);
        let tile = Tile {
            foreground: 1,
            background: 2,
            foreground_mod: 0,
            background_mod: 0,
        };
        
        assert!(map.set_tile(5, 5, tile).is_ok());
        let retrieved = map.get_tile(5, 5).unwrap();
        assert_eq!(retrieved.foreground, 1);
        assert_eq!(retrieved.background, 2);
    }

    #[test]
    fn test_out_of_bounds() {
        let mut map = PlanetMap::new(10, 10);
        let tile = Tile::default();
        
        assert!(map.set_tile(10, 10, tile).is_err());
        assert!(map.get_tile(10, 10).is_none());
    }
}
