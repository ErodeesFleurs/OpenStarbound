//! Image handling compatible with C++ Star::Image.
//!
//! This module provides image loading, manipulation, and saving capabilities.

use crate::math::{Vec2, Vec3, Vec4};

/// Pixel format for images.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum PixelFormat {
    /// 24-bit RGB (3 bytes per pixel)
    Rgb24 = 0,
    /// 32-bit RGBA (4 bytes per pixel)
    Rgba32 = 1,
    /// 24-bit BGR (3 bytes per pixel)
    Bgr24 = 2,
    /// 32-bit BGRA (4 bytes per pixel)
    Bgra32 = 3,
    /// 96-bit RGB float (12 bytes per pixel)
    RgbF = 4,
    /// 128-bit RGBA float (16 bytes per pixel)
    RgbaF = 5,
}

impl PixelFormat {
    /// Get the number of bits per pixel.
    pub fn bits_per_pixel(self) -> u8 {
        match self {
            PixelFormat::Rgb24 => 24,
            PixelFormat::Rgba32 => 32,
            PixelFormat::Bgr24 => 24,
            PixelFormat::Bgra32 => 32,
            PixelFormat::RgbF => 96,
            PixelFormat::RgbaF => 128,
        }
    }

    /// Get the number of bytes per pixel.
    pub fn bytes_per_pixel(self) -> u8 {
        match self {
            PixelFormat::Rgb24 => 3,
            PixelFormat::Rgba32 => 4,
            PixelFormat::Bgr24 => 3,
            PixelFormat::Bgra32 => 4,
            PixelFormat::RgbF => 12,
            PixelFormat::RgbaF => 16,
        }
    }

    /// Check if the format has an alpha channel.
    pub fn has_alpha(self) -> bool {
        matches!(self, PixelFormat::Rgba32 | PixelFormat::Bgra32 | PixelFormat::RgbaF)
    }
}

/// Type aliases for color vectors
pub type Vec3B = Vec3<u8>;
pub type Vec4B = Vec4<u8>;

/// An image stored in row-major order.
///
/// The coordinate system has (0, 0) at the lower-left corner.
#[derive(Clone)]
pub struct Image {
    data: Vec<u8>,
    width: u32,
    height: u32,
    pixel_format: PixelFormat,
}

impl Image {
    /// Create a new empty image with zero dimensions.
    pub fn new(pixel_format: PixelFormat) -> Self {
        Image {
            data: Vec::new(),
            width: 0,
            height: 0,
            pixel_format,
        }
    }

    /// Create a new image with the given dimensions.
    ///
    /// # Arguments
    /// * `width` - Image width in pixels
    /// * `height` - Image height in pixels
    /// * `pixel_format` - The pixel format to use
    pub fn with_size(width: u32, height: u32, pixel_format: PixelFormat) -> Self {
        let bytes_per_pixel = pixel_format.bytes_per_pixel() as usize;
        let size = (width as usize) * (height as usize) * bytes_per_pixel;

        Image {
            data: vec![0; size],
            width,
            height,
            pixel_format,
        }
    }

    /// Create an image filled with a specific color.
    ///
    /// # Arguments
    /// * `width` - Image width in pixels
    /// * `height` - Image height in pixels
    /// * `color` - The fill color (RGBA)
    /// * `pixel_format` - The pixel format to use
    pub fn filled(width: u32, height: u32, color: Vec4B, pixel_format: PixelFormat) -> Self {
        let mut image = Image::with_size(width, height, pixel_format);
        image.fill_rgba(color);
        image
    }

    /// Get the bits per pixel for this image.
    pub fn bits_per_pixel(&self) -> u8 {
        self.pixel_format.bits_per_pixel()
    }

    /// Get the bytes per pixel for this image.
    pub fn bytes_per_pixel(&self) -> u8 {
        self.pixel_format.bytes_per_pixel()
    }

    /// Get the image width.
    pub fn width(&self) -> u32 {
        self.width
    }

    /// Get the image height.
    pub fn height(&self) -> u32 {
        self.height
    }

    /// Get the image size as a 2D vector.
    pub fn size(&self) -> Vec2<u32> {
        Vec2::new(self.width, self.height)
    }

    /// Check if the image is empty (width or height is 0).
    pub fn is_empty(&self) -> bool {
        self.width == 0 || self.height == 0
    }

    /// Get the pixel format.
    pub fn pixel_format(&self) -> PixelFormat {
        self.pixel_format
    }

    /// Get a reference to the raw pixel data.
    pub fn data(&self) -> &[u8] {
        &self.data
    }

    /// Get a mutable reference to the raw pixel data.
    pub fn data_mut(&mut self) -> &mut [u8] {
        &mut self.data
    }

    /// Reset the image with new dimensions.
    ///
    /// The contents are zeroed after this call.
    ///
    /// # Arguments
    /// * `width` - New width
    /// * `height` - New height
    /// * `pixel_format` - Optional new pixel format
    pub fn reset(&mut self, width: u32, height: u32, pixel_format: Option<PixelFormat>) {
        if let Some(pf) = pixel_format {
            self.pixel_format = pf;
        }

        self.width = width;
        self.height = height;

        let bytes_per_pixel = self.pixel_format.bytes_per_pixel() as usize;
        let size = (width as usize) * (height as usize) * bytes_per_pixel;

        self.data.clear();
        self.data.resize(size, 0);
    }

    /// Fill the image with an RGB color.
    pub fn fill_rgb(&mut self, color: Vec3B) {
        self.fill_rgba(Vec4::new(color.x(), color.y(), color.z(), 255));
    }

    /// Fill the image with an RGBA color.
    pub fn fill_rgba(&mut self, color: Vec4B) {
        match self.pixel_format {
            PixelFormat::Rgb24 => {
                for chunk in self.data.chunks_exact_mut(3) {
                    chunk[0] = color.x();
                    chunk[1] = color.y();
                    chunk[2] = color.z();
                }
            }
            PixelFormat::Rgba32 => {
                for chunk in self.data.chunks_exact_mut(4) {
                    chunk[0] = color.x();
                    chunk[1] = color.y();
                    chunk[2] = color.z();
                    chunk[3] = color.w();
                }
            }
            PixelFormat::Bgr24 => {
                for chunk in self.data.chunks_exact_mut(3) {
                    chunk[0] = color.z();
                    chunk[1] = color.y();
                    chunk[2] = color.x();
                }
            }
            PixelFormat::Bgra32 => {
                for chunk in self.data.chunks_exact_mut(4) {
                    chunk[0] = color.z();
                    chunk[1] = color.y();
                    chunk[2] = color.x();
                    chunk[3] = color.w();
                }
            }
            _ => {
                // For float formats, just zero fill
                self.data.fill(0);
            }
        }
    }

    /// Calculate the byte offset for a pixel position.
    fn pixel_offset(&self, x: u32, y: u32) -> usize {
        let bpp = self.bytes_per_pixel() as usize;
        (y as usize * self.width as usize + x as usize) * bpp
    }

    /// Set a pixel value (RGBA).
    ///
    /// # Arguments
    /// * `x` - X coordinate
    /// * `y` - Y coordinate
    /// * `color` - The color to set
    ///
    /// # Panics
    /// Panics if coordinates are out of bounds.
    pub fn set(&mut self, x: u32, y: u32, color: Vec4B) {
        assert!(x < self.width && y < self.height, "Pixel coordinates out of bounds");

        let offset = self.pixel_offset(x, y);

        match self.pixel_format {
            PixelFormat::Rgb24 => {
                self.data[offset] = color.x();
                self.data[offset + 1] = color.y();
                self.data[offset + 2] = color.z();
            }
            PixelFormat::Rgba32 => {
                self.data[offset] = color.x();
                self.data[offset + 1] = color.y();
                self.data[offset + 2] = color.z();
                self.data[offset + 3] = color.w();
            }
            PixelFormat::Bgr24 => {
                self.data[offset] = color.z();
                self.data[offset + 1] = color.y();
                self.data[offset + 2] = color.x();
            }
            PixelFormat::Bgra32 => {
                self.data[offset] = color.z();
                self.data[offset + 1] = color.y();
                self.data[offset + 2] = color.x();
                self.data[offset + 3] = color.w();
            }
            _ => {
                // Float formats not supported for byte-based set
            }
        }
    }

    /// Set a pixel value (RGB with alpha = 255).
    pub fn set_rgb(&mut self, x: u32, y: u32, color: Vec3B) {
        self.set(x, y, Vec4::new(color.x(), color.y(), color.z(), 255));
    }

    /// Get a pixel value (RGBA).
    ///
    /// # Arguments
    /// * `x` - X coordinate
    /// * `y` - Y coordinate
    ///
    /// # Returns
    /// The pixel color as RGBA
    ///
    /// # Panics
    /// Panics if coordinates are out of bounds.
    pub fn get(&self, x: u32, y: u32) -> Vec4B {
        assert!(x < self.width && y < self.height, "Pixel coordinates out of bounds");

        let offset = self.pixel_offset(x, y);

        match self.pixel_format {
            PixelFormat::Rgb24 => Vec4::new(
                self.data[offset],
                self.data[offset + 1],
                self.data[offset + 2],
                255,
            ),
            PixelFormat::Rgba32 => Vec4::new(
                self.data[offset],
                self.data[offset + 1],
                self.data[offset + 2],
                self.data[offset + 3],
            ),
            PixelFormat::Bgr24 => Vec4::new(
                self.data[offset + 2],
                self.data[offset + 1],
                self.data[offset],
                255,
            ),
            PixelFormat::Bgra32 => Vec4::new(
                self.data[offset + 2],
                self.data[offset + 1],
                self.data[offset],
                self.data[offset + 3],
            ),
            _ => Vec4::new(0, 0, 0, 255),
        }
    }

    /// Get a pixel value, clamping coordinates to the image bounds.
    ///
    /// Returns (0, 0, 0, 0) if the image is empty.
    pub fn clamp(&self, x: i32, y: i32) -> Vec4B {
        if self.is_empty() {
            return Vec4::new(0, 0, 0, 0);
        }

        let x = x.clamp(0, self.width as i32 - 1) as u32;
        let y = y.clamp(0, self.height as i32 - 1) as u32;
        self.get(x, y)
    }

    /// Fast pixel set for 32-bit formats.
    ///
    /// # Panics
    /// Panics in debug mode if coordinates are out of bounds or format is not 32-bit.
    #[inline]
    pub fn set32(&mut self, x: u32, y: u32, color: Vec4B) {
        debug_assert!(self.bytes_per_pixel() == 4);
        debug_assert!(x < self.width && y < self.height, "Pixel coordinates out of bounds");
        let offset = (y as usize * self.width as usize + x as usize) * 4;
        self.data[offset] = color.x();
        self.data[offset + 1] = color.y();
        self.data[offset + 2] = color.z();
        self.data[offset + 3] = color.w();
    }

    /// Fast pixel get for 32-bit formats.
    ///
    /// # Panics
    /// Panics in debug mode if coordinates are out of bounds or format is not 32-bit.
    #[inline]
    pub fn get32(&self, x: u32, y: u32) -> Vec4B {
        debug_assert!(self.bytes_per_pixel() == 4);
        debug_assert!(x < self.width && y < self.height, "Pixel coordinates out of bounds");
        let offset = (y as usize * self.width as usize + x as usize) * 4;
        Vec4::new(
            self.data[offset],
            self.data[offset + 1],
            self.data[offset + 2],
            self.data[offset + 3],
        )
    }

    /// Fast pixel set for 24-bit formats.
    #[inline]
    pub fn set24(&mut self, x: u32, y: u32, color: Vec3B) {
        debug_assert!(self.bytes_per_pixel() == 3);
        let offset = (y as usize * self.width as usize + x as usize) * 3;
        self.data[offset] = color.x();
        self.data[offset + 1] = color.y();
        self.data[offset + 2] = color.z();
    }

    /// Fast pixel get for 24-bit formats.
    #[inline]
    pub fn get24(&self, x: u32, y: u32) -> Vec3B {
        debug_assert!(self.bytes_per_pixel() == 3);
        let offset = (y as usize * self.width as usize + x as usize) * 3;
        Vec3::new(
            self.data[offset],
            self.data[offset + 1],
            self.data[offset + 2],
        )
    }

    /// Iterate over all pixels.
    ///
    /// The callback is called with (x, y, color) for each pixel.
    pub fn for_each_pixel<F>(&self, mut callback: F)
    where
        F: FnMut(u32, u32, Vec4B),
    {
        for y in 0..self.height {
            for x in 0..self.width {
                callback(x, y, self.get(x, y));
            }
        }
    }

    /// Iterate over all pixels mutably.
    ///
    /// The callback is called with (x, y, color) and should return the new color.
    pub fn for_each_pixel_mut<F>(&mut self, mut callback: F)
    where
        F: FnMut(u32, u32, Vec4B) -> Vec4B,
    {
        for y in 0..self.height {
            for x in 0..self.width {
                let color = self.get(x, y);
                let new_color = callback(x, y, color);
                self.set(x, y, new_color);
            }
        }
    }

    /// Extract a sub-image.
    ///
    /// # Arguments
    /// * `x` - X offset
    /// * `y` - Y offset
    /// * `width` - Width of sub-image
    /// * `height` - Height of sub-image
    ///
    /// # Returns
    /// A new image containing the sub-region
    pub fn sub_image(&self, x: u32, y: u32, width: u32, height: u32) -> Image {
        assert!(x + width <= self.width && y + height <= self.height);

        let mut sub = Image::with_size(width, height, self.pixel_format);

        for sy in 0..height {
            for sx in 0..width {
                sub.set(sx, sy, self.get(x + sx, y + sy));
            }
        }

        sub
    }

    /// Copy another image into this one at the given position.
    ///
    /// # Arguments
    /// * `x` - X position to copy to
    /// * `y` - Y position to copy to
    /// * `other` - The image to copy
    pub fn copy_into(&mut self, x: u32, y: u32, other: &Image) {
        for sy in 0..other.height {
            for sx in 0..other.width {
                let dx = x + sx;
                let dy = y + sy;
                if dx < self.width && dy < self.height {
                    self.set(dx, dy, other.get(sx, sy));
                }
            }
        }
    }

    /// Draw another image over this one with alpha compositing.
    ///
    /// # Arguments
    /// * `x` - X position to draw at
    /// * `y` - Y position to draw at
    /// * `other` - The image to draw
    pub fn draw_into(&mut self, x: u32, y: u32, other: &Image) {
        for sy in 0..other.height {
            for sx in 0..other.width {
                let dx = x + sx;
                let dy = y + sy;
                if dx < self.width && dy < self.height {
                    let src = other.get(sx, sy);
                    let dst = self.get(dx, dy);
                    let blended = alpha_blend(src, dst);
                    self.set(dx, dy, blended);
                }
            }
        }
    }

    /// Convert the image to a different pixel format.
    pub fn convert(&self, pixel_format: PixelFormat) -> Image {
        if self.pixel_format == pixel_format {
            return self.clone();
        }

        let mut converted = Image::with_size(self.width, self.height, pixel_format);

        for y in 0..self.height {
            for x in 0..self.width {
                converted.set(x, y, self.get(x, y));
            }
        }

        converted
    }

    /// Fill a rectangle with a color.
    ///
    /// # Arguments
    /// * `x` - X position
    /// * `y` - Y position
    /// * `width` - Rectangle width
    /// * `height` - Rectangle height
    /// * `color` - Fill color
    pub fn fill_rect(&mut self, x: u32, y: u32, width: u32, height: u32, color: Vec4B) {
        for py in y..(y + height).min(self.height) {
            for px in x..(x + width).min(self.width) {
                self.set(px, py, color);
            }
        }
    }
}

impl Default for Image {
    fn default() -> Self {
        Image::new(PixelFormat::Rgba32)
    }
}

/// Alpha blend two colors (src over dst).
fn alpha_blend(src: Vec4B, dst: Vec4B) -> Vec4B {
    let sa = src.w() as u32;
    let da = dst.w() as u32;

    if sa == 0 {
        return dst;
    }
    if sa == 255 {
        return src;
    }

    let inv_sa = 255 - sa;
    let out_a = sa + (da * inv_sa) / 255;

    if out_a == 0 {
        return Vec4::new(0, 0, 0, 0);
    }

    let r = ((src.x() as u32 * sa + dst.x() as u32 * da * inv_sa / 255) / out_a) as u8;
    let g = ((src.y() as u32 * sa + dst.y() as u32 * da * inv_sa / 255) / out_a) as u8;
    let b = ((src.z() as u32 * sa + dst.z() as u32 * da * inv_sa / 255) / out_a) as u8;

    Vec4::new(r, g, b, out_a as u8)
}

/// A non-owning view into an image.
#[derive(Clone, Copy)]
pub struct ImageView<'a> {
    /// Image size
    pub size: Vec2<u32>,
    /// Raw pixel data
    pub data: &'a [u8],
    /// Pixel format
    pub format: PixelFormat,
}

impl<'a> ImageView<'a> {
    /// Check if the view is empty.
    pub fn is_empty(&self) -> bool {
        self.size.x() == 0 || self.size.y() == 0
    }

    /// Create a view from an image.
    pub fn from_image(image: &'a Image) -> Self {
        ImageView {
            size: image.size(),
            data: image.data(),
            format: image.pixel_format(),
        }
    }
}

impl<'a> From<&'a Image> for ImageView<'a> {
    fn from(image: &'a Image) -> Self {
        ImageView::from_image(image)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pixel_format() {
        assert_eq!(PixelFormat::Rgb24.bits_per_pixel(), 24);
        assert_eq!(PixelFormat::Rgba32.bits_per_pixel(), 32);
        assert_eq!(PixelFormat::Rgb24.bytes_per_pixel(), 3);
        assert_eq!(PixelFormat::Rgba32.bytes_per_pixel(), 4);

        assert!(!PixelFormat::Rgb24.has_alpha());
        assert!(PixelFormat::Rgba32.has_alpha());
    }

    #[test]
    fn test_image_new() {
        let img = Image::new(PixelFormat::Rgba32);
        assert_eq!(img.width(), 0);
        assert_eq!(img.height(), 0);
        assert!(img.is_empty());
    }

    #[test]
    fn test_image_with_size() {
        let img = Image::with_size(100, 50, PixelFormat::Rgba32);
        assert_eq!(img.width(), 100);
        assert_eq!(img.height(), 50);
        assert_eq!(img.size(), Vec2::new(100, 50));
        assert!(!img.is_empty());
        assert_eq!(img.data().len(), 100 * 50 * 4);
    }

    #[test]
    fn test_image_filled() {
        let color = Vec4::new(255, 128, 64, 255);
        let img = Image::filled(10, 10, color, PixelFormat::Rgba32);

        for y in 0..10 {
            for x in 0..10 {
                assert_eq!(img.get(x, y), color);
            }
        }
    }

    #[test]
    fn test_image_set_get() {
        let mut img = Image::with_size(10, 10, PixelFormat::Rgba32);
        let color = Vec4::new(255, 128, 64, 200);

        img.set(5, 5, color);
        assert_eq!(img.get(5, 5), color);
    }

    #[test]
    fn test_image_clamp() {
        let img = Image::filled(10, 10, Vec4::new(255, 0, 0, 255), PixelFormat::Rgba32);

        // In bounds
        assert_eq!(img.clamp(5, 5).x(), 255);

        // Out of bounds (clamped)
        assert_eq!(img.clamp(-10, -10).x(), 255);
        assert_eq!(img.clamp(100, 100).x(), 255);
    }

    #[test]
    fn test_image_sub_image() {
        let mut img = Image::filled(10, 10, Vec4::new(255, 0, 0, 255), PixelFormat::Rgba32);
        img.fill_rect(2, 2, 4, 4, Vec4::new(0, 255, 0, 255));

        let sub = img.sub_image(2, 2, 4, 4);
        assert_eq!(sub.width(), 4);
        assert_eq!(sub.height(), 4);

        for y in 0..4 {
            for x in 0..4 {
                assert_eq!(sub.get(x, y).y(), 255); // Green
            }
        }
    }

    #[test]
    fn test_image_convert() {
        let img = Image::filled(5, 5, Vec4::new(100, 150, 200, 255), PixelFormat::Rgba32);
        let converted = img.convert(PixelFormat::Rgb24);

        assert_eq!(converted.pixel_format(), PixelFormat::Rgb24);
        assert_eq!(converted.bytes_per_pixel(), 3);

        // RGB values should be preserved
        let color = converted.get(0, 0);
        assert_eq!(color.x(), 100);
        assert_eq!(color.y(), 150);
        assert_eq!(color.z(), 200);
    }

    #[test]
    fn test_alpha_blend() {
        // Fully opaque source
        let src = Vec4::new(255, 0, 0, 255);
        let dst = Vec4::new(0, 255, 0, 255);
        let result = alpha_blend(src, dst);
        assert_eq!(result, src);

        // Fully transparent source
        let src = Vec4::new(255, 0, 0, 0);
        let result = alpha_blend(src, dst);
        assert_eq!(result, dst);
    }

    #[test]
    fn test_image_view() {
        let img = Image::filled(10, 10, Vec4::new(255, 0, 0, 255), PixelFormat::Rgba32);
        let view = ImageView::from_image(&img);

        assert_eq!(view.size, Vec2::new(10, 10));
        assert!(!view.is_empty());
        assert_eq!(view.format, PixelFormat::Rgba32);
    }

    #[test]
    fn test_fast_accessors() {
        let mut img = Image::with_size(10, 10, PixelFormat::Rgba32);
        let color = Vec4::new(100, 150, 200, 250);

        img.set32(3, 3, color);
        assert_eq!(img.get32(3, 3), color);
    }

    #[test]
    fn test_for_each_pixel() {
        let img = Image::filled(3, 3, Vec4::new(100, 100, 100, 255), PixelFormat::Rgba32);
        let mut count = 0;

        img.for_each_pixel(|_, _, color| {
            assert_eq!(color.x(), 100);
            count += 1;
        });

        assert_eq!(count, 9);
    }
}
