use anyhow::{Context, Result};
use image::{Rgb, RgbImage};
use std::path::Path;

use crate::worldfile::{WorldFile, SECTOR_SIZE};

/// Convert a material ID to a color for visualization
fn material_to_color(material: u16, mod_value: u16) -> Rgb<u8> {
    if material == 0 {
        // Empty material - black
        return Rgb([0, 0, 0]);
    }
    
    // Generate a pseudo-random color based on material ID
    // This mimics the C++ implementation's color generation
    let hue = ((material as f32 * 137.508) % 360.0) / 360.0;
    let saturation = if mod_value != 0 {
        ((mod_value as f32 * 73.219) % 100.0) / 100.0
    } else {
        0.8
    };
    let value = 1.0;
    
    hsv_to_rgb(hue, saturation, value)
}

/// Convert HSV color to RGB
fn hsv_to_rgb(h: f32, s: f32, v: f32) -> Rgb<u8> {
    let c = v * s;
    let x = c * (1.0 - ((h * 6.0) % 2.0 - 1.0).abs());
    let m = v - c;
    
    let (r, g, b) = if h < 1.0/6.0 {
        (c, x, 0.0)
    } else if h < 2.0/6.0 {
        (x, c, 0.0)
    } else if h < 3.0/6.0 {
        (0.0, c, x)
    } else if h < 4.0/6.0 {
        (0.0, x, c)
    } else if h < 5.0/6.0 {
        (x, 0.0, c)
    } else {
        (c, 0.0, x)
    };
    
    let r = ((r + m) * 255.0) as u8;
    let g = ((g + m) * 255.0) as u8;
    let b = ((b + m) * 255.0) as u8;
    
    Rgb([r, g, b])
}

/// Render the world file to an image file
pub fn render_to_image<P: AsRef<Path>>(world: &WorldFile, output_path: P) -> Result<()> {
    let width = world.metadata.width;
    let height = world.metadata.height;
    
    // Create image
    let mut img = RgbImage::new(width, height);
    
    // Fill with black (empty)
    for y in 0..height {
        for x in 0..width {
            img.put_pixel(x, y, Rgb([0, 0, 0]));
        }
    }
    
    // Render each loaded sector
    for ((sector_x, sector_y), sector) in &world.sectors {
        let base_x = *sector_x as u32 * SECTOR_SIZE as u32;
        let base_y = *sector_y as u32 * SECTOR_SIZE as u32;
        
        for sy in 0..SECTOR_SIZE {
            for sx in 0..SECTOR_SIZE {
                let x = base_x + sx as u32;
                let y = base_y + sy as u32;
                
                // Check bounds
                if x >= width || y >= height {
                    continue;
                }
                
                let tile = sector.tiles[sy][sx];
                
                let color = if tile.foreground != 0 {
                    // Foreground material - red-based color
                    let mut c = material_to_color(tile.foreground, tile.foreground_mod);
                    // Tint towards red
                    c[0] = c[0].saturating_add(50);
                    c
                } else if tile.background != 0 {
                    // Background material - darker color (cave)
                    let mut c = material_to_color(tile.background, tile.background_mod);
                    // Make it darker
                    c[0] = c[0] / 2;
                    c[1] = c[1] / 2;
                    c[2] = c[2] / 2;
                    c
                } else {
                    // Empty space - black
                    Rgb([0, 0, 0])
                };
                
                img.put_pixel(x, y, color);
            }
        }
    }
    
    img.save(output_path.as_ref())
        .context("Failed to save rendered image")?;
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_material_to_color() {
        let color = material_to_color(0, 0);
        assert_eq!(color, Rgb([0, 0, 0]));
        
        let color = material_to_color(1, 0);
        assert_ne!(color, Rgb([0, 0, 0]));
    }

    #[test]
    fn test_hsv_to_rgb() {
        // Red
        let color = hsv_to_rgb(0.0, 1.0, 1.0);
        assert_eq!(color, Rgb([255, 0, 0]));
        
        // Green
        let color = hsv_to_rgb(1.0/3.0, 1.0, 1.0);
        assert_eq!(color, Rgb([0, 255, 0]));
        
        // Blue
        let color = hsv_to_rgb(2.0/3.0, 1.0, 1.0);
        assert_eq!(color, Rgb([0, 0, 255]));
    }
}
