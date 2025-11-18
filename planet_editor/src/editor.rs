use anyhow::Result;
use crate::world::{PlanetMap, Tile};

/// Set a tile's foreground material
pub fn set_tile(map: &mut PlanetMap, x: u32, y: u32, material: u16) -> Result<()> {
    let tile = map.get_tile_mut(x, y)
        .ok_or_else(|| anyhow::anyhow!("Invalid coordinates: ({}, {})", x, y))?;
    
    tile.foreground = material;
    Ok(())
}

/// Set a tile's background material
pub fn set_background(map: &mut PlanetMap, x: u32, y: u32, material: u16) -> Result<()> {
    let tile = map.get_tile_mut(x, y)
        .ok_or_else(|| anyhow::anyhow!("Invalid coordinates: ({}, {})", x, y))?;
    
    tile.background = material;
    Ok(())
}

/// Fill a rectangular region with a specific material
pub fn fill_region(
    map: &mut PlanetMap,
    x: u32,
    y: u32,
    width: u32,
    height: u32,
    material: u16,
) -> Result<()> {
    for dy in 0..height {
        for dx in 0..width {
            let tx = x + dx;
            let ty = y + dy;
            if tx < map.width && ty < map.height {
                if let Some(tile) = map.get_tile_mut(tx, ty) {
                    tile.foreground = material;
                }
            }
        }
    }
    Ok(())
}

/// Clear a tile (set all materials to empty)
pub fn clear_tile(map: &mut PlanetMap, x: u32, y: u32) -> Result<()> {
    let tile = map.get_tile_mut(x, y)
        .ok_or_else(|| anyhow::anyhow!("Invalid coordinates: ({}, {})", x, y))?;
    
    *tile = Tile::default();
    Ok(())
}

/// Copy a tile from one location to another
pub fn copy_tile(map: &mut PlanetMap, src_x: u32, src_y: u32, dst_x: u32, dst_y: u32) -> Result<()> {
    let tile = *map.get_tile(src_x, src_y)
        .ok_or_else(|| anyhow::anyhow!("Invalid source coordinates: ({}, {})", src_x, src_y))?;
    
    map.set_tile(dst_x, dst_y, tile)?;
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_set_tile() {
        let mut map = PlanetMap::new(10, 10);
        assert!(set_tile(&mut map, 5, 5, 42).is_ok());
        assert_eq!(map.get_tile(5, 5).unwrap().foreground, 42);
    }

    #[test]
    fn test_fill_region() {
        let mut map = PlanetMap::new(10, 10);
        assert!(fill_region(&mut map, 2, 2, 3, 3, 99).is_ok());
        
        // Check that the region was filled
        for y in 2..5 {
            for x in 2..5 {
                assert_eq!(map.get_tile(x, y).unwrap().foreground, 99);
            }
        }
        
        // Check that outside the region is unchanged
        assert_eq!(map.get_tile(0, 0).unwrap().foreground, 0);
    }

    #[test]
    fn test_copy_tile() {
        let mut map = PlanetMap::new(10, 10);
        set_tile(&mut map, 1, 1, 42).unwrap();
        assert!(copy_tile(&mut map, 1, 1, 5, 5).is_ok());
        assert_eq!(map.get_tile(5, 5).unwrap().foreground, 42);
    }
}
