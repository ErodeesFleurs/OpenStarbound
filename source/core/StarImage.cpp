#include <png.h>
import std;
import star.maybe;
import star.format;
import star.io_device;
import star.vector;
import star.image;
import star.logging;


import star.memory;
import star.math_common;
import star.algorithm;
namespace Star {

namespace {

struct PngState {
  Logger& logger;
  IODevice* device;
  bool writing;
  png_structp png = nullptr;
  png_infop info = nullptr;
  png_infop end = nullptr;
  std::exception_ptr exception{};

  ~PngState() {
    if (writing)
      png_destroy_write_struct(&png, &info);
    else
      png_destroy_read_struct(&png, &info, &end);
  }

  void check(bool success, char const* message) const {
    if (exception)
      std::rethrow_exception(exception);
    if (!success)
      throw ImageException(message);
  }
};

void logPngMessage(PngState& state, char const* kind, png_const_charp message) noexcept {
  try {
    state.logger.debug("PNG {} in file: '{}', {}", kind, state.device->deviceName(), message);
  } catch (...) {
    state.exception = std::current_exception();
  }
}

void pngError(png_structp png, png_const_charp message) noexcept {
  auto& state = *static_cast<PngState*>(png_get_error_ptr(png));
  logPngMessage(state, "error", message);
  png_longjmp(png, 1);
}

void pngWarning(png_structp png, png_const_charp message) noexcept {
  auto& state = *static_cast<PngState*>(png_get_error_ptr(png));
  logPngMessage(state, "warning", message);
  if (state.exception)
    png_longjmp(png, 1);
}

void readPngData(png_structp png, png_bytep data, png_size_t length) noexcept {
  auto& state = *static_cast<PngState*>(png_get_io_ptr(png));
  try {
    state.device->readFull(reinterpret_cast<char*>(data), length);
  } catch (...) {
    state.exception = std::current_exception();
  }
  // Leave the catch scope before jumping: no C++ exception unwinding crosses C.
  if (state.exception)
    png_longjmp(png, 1);
}

void writePngData(png_structp png, png_bytep data, png_size_t length) noexcept {
  auto& state = *static_cast<PngState*>(png_get_io_ptr(png));
  try {
    state.device->writeFull(reinterpret_cast<char*>(data), length);
  } catch (...) {
    state.exception = std::current_exception();
  }
  if (state.exception)
    png_longjmp(png, 1);
}

void flushPngData(png_structp) noexcept {}

// Each jump target lives in a separate frame with only trivial locals. Owners,
// Images, row arrays and exception_ptrs live in the caller, never across a jump.
bool createPngInfo(PngState& state) {
  if (setjmp(png_jmpbuf(state.png)))
    return false;
  state.info = png_create_info_struct(state.png);
  if (!state.info)
    return false;
  if (!state.writing) {
    state.end = png_create_info_struct(state.png);
    if (!state.end)
      return false;
  }
  return true;
}

void initializePng(PngState& state) {
  if (state.writing)
    state.png = png_create_write_struct(PNG_LIBPNG_VER_STRING, &state, pngError, pngWarning);
  else
    state.png = png_create_read_struct(PNG_LIBPNG_VER_STRING, &state, pngError, pngWarning);
  state.check(state.png != nullptr, "Internal libPNG error");
  state.check(createPngInfo(state), "Internal libPNG error");
}

struct PngMetadata {
  png_uint_32 width;
  png_uint_32 height;
  png_uint_32 bitdepth;
  png_uint_32 channels;
};

bool readPngInfo(PngState& state, PngMetadata& metadata, bool strip16) {
  if (setjmp(png_jmpbuf(state.png)))
    return false;
  png_set_read_fn(state.png, &state, readPngData);
  png_set_sig_bytes(state.png, 8);
  png_read_info(state.png, state.info);

  metadata.width = png_get_image_width(state.png, state.info);
  metadata.height = png_get_image_height(state.png, state.info);
  metadata.bitdepth = png_get_bit_depth(state.png, state.info);
  metadata.channels = png_get_channels(state.png, state.info);
  auto colorType = png_get_color_type(state.png, state.info);

  if (colorType == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(state.png);
    metadata.channels = 3;
    metadata.bitdepth = 8;
  }
  if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
    if (metadata.bitdepth < 8) {
      png_set_expand_gray_1_2_4_to_8(state.png);
      metadata.bitdepth = 8;
    }
    png_set_gray_to_rgb(state.png);
    metadata.channels = colorType == PNG_COLOR_TYPE_GRAY_ALPHA ? 4 : 3;
  }
  if (png_get_valid(state.png, state.info, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(state.png);
    metadata.channels += 1;
  }
  if (strip16 && metadata.bitdepth == 16) {
    png_set_strip_16(state.png);
    metadata.bitdepth = 8;
  }
  return true;
}

bool readPngRows(PngState& state, png_bytepp rows) {
  if (setjmp(png_jmpbuf(state.png)))
    return false;
  png_read_image(state.png, rows);
  return true;
}

bool writePngRows(PngState& state, unsigned width, unsigned height, unsigned channels, png_bytepp rows) {
  if (setjmp(png_jmpbuf(state.png)))
    return false;
  png_set_IHDR(state.png, state.info, width, height, 8,
      channels == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_write_fn(state.png, &state, writePngData, flushPngData);
  png_set_rows(state.png, state.info, rows);
  png_write_png(state.png, state.info, PNG_TRANSFORM_IDENTITY, nullptr);
  return true;
}

void readPngSignature(IODevice& device) {
  png_byte header[8]{};
  device.readFull(reinterpret_cast<char*>(header), sizeof(header));
  if (png_sig_cmp(header, 0, sizeof(header)))
    throw ImageException(strf("File {} is not a png image!", device.deviceName()));
}

}

bool Image::isPng(std::shared_ptr<IODevice> device) {
  png_byte header[8]{};
  return !png_sig_cmp(header, 0, device->readAbsolute(0, (char*)header, sizeof(header)));
}

Image Image::readPng(Logger& logger, std::shared_ptr<IODevice> device) {
  readPngSignature(*device);
  PngState state{logger, device.get(), false};
  initializePng(state);
  PngMetadata metadata{};
  state.check(readPngInfo(state, metadata, true), "Internal error reading png.");

  if (metadata.bitdepth != 8 || (metadata.channels != 3 && metadata.channels != 4))
    throw ImageException(strf("Unsupported PNG pixel format in file {}", device->deviceName()));

  Image image(metadata.width, metadata.height, metadata.channels == 3 ? PixelFormat::RGB24 : PixelFormat::RGBA32);
  auto rows = std::make_unique<png_bytep[]>(metadata.height);
  std::size_t stride = static_cast<std::size_t>(metadata.width) * metadata.channels;
  for (std::size_t i = 0; i < metadata.height; ++i)
    rows[i] = image.data() + (metadata.height - i - 1) * stride;
  state.check(readPngRows(state, rows.get()), "Internal error reading png.");
  return image;
}

std::tuple<Vec2U, PixelFormat> Image::readPngMetadata(Logger& logger, std::shared_ptr<IODevice> device) {
  readPngSignature(*device);
  PngState state{logger, device.get(), false};
  initializePng(state);
  PngMetadata metadata{};
  state.check(readPngInfo(state, metadata, false), "Internal error reading png.");
  return std::make_tuple(Vec2U{metadata.width, metadata.height},
      metadata.channels == 3 ? PixelFormat::RGB24 : PixelFormat::RGBA32);
}

Image Image::filled(Vec2U size, Vec4B color, PixelFormat pf) {
  Image image(size, pf);
  image.fill(color);
  return image;
}

Image::Image(PixelFormat pf)
  : m_data(nullptr), m_width(0), m_height(0), m_pixelFormat(pf) {}

Image::Image(Vec2U size, PixelFormat pf)
  : Image(size[0], size[1], pf) {}

Image::Image(unsigned width, unsigned height, PixelFormat pf)
  : Image(pf) {
  reset(width, height, pf);
}

Image::~Image() {
  if (m_data)
    Star::free(m_data);
}

Image::Image(Image const& image) : Image() {
  operator=(image);
}

Image::Image(Image&& image) : Image() {
  operator=(std::move(image));
}

Image& Image::operator=(Image const& image) {
  reset(image.m_width, image.m_height, image.m_pixelFormat);
  std::memcpy(data(), image.data(), m_width * m_height * bytesPerPixel());
  return *this;
}

Image& Image::operator=(Image&& image) {
  reset(0, 0, m_pixelFormat);

  m_data = std::exchange(image.m_data, nullptr);
  m_width = std::exchange(image.m_width, 0);
  m_height = std::exchange(image.m_height, 0);
  m_pixelFormat = std::exchange(image.m_pixelFormat, PixelFormat{});
  return *this;
}

void Image::reset(Vec2U size, Maybe<PixelFormat> pf) {
  reset(size[0], size[1], pf);
}

void Image::reset(unsigned width, unsigned height, Maybe<PixelFormat> pf) {
  if (!pf)
    pf = m_pixelFormat;

  if (m_width == width && m_height == height && m_pixelFormat == *pf)
    return;

  std::size_t imageSize = width * height * Star::bytesPerPixel(*pf);
  if (imageSize == 0) {
    if (m_data) {
      Star::free(m_data);
      m_data = nullptr;
    }
  } else {
    std::uint8_t* newData = nullptr;
    if (!m_data)
      newData = (std::uint8_t*)Star::malloc(imageSize);
    else
      newData = (std::uint8_t*)Star::realloc(m_data, imageSize);

    if (!newData)
      throw MemoryException::format("Could not allocate memory for new Image size {}\n", imageSize);

    m_data = newData;
    std::memset(m_data, 0, imageSize);
  }

  m_pixelFormat = *pf;
  m_width = width;
  m_height = height;
}

void Image::fill(Vec3B const& c) {
  if (bitsPerPixel() == 24) {
    for (unsigned y = 0; y < m_height; ++y)
      for (unsigned x = 0; x < m_width; ++x)
        set24(x, y, c);
  } else {
    for (unsigned y = 0; y < m_height; ++y)
      for (unsigned x = 0; x < m_width; ++x)
        set32(x, y, Vec4B(c, 255));
  }
}

void Image::fill(Vec4B const& c) {
  if (bitsPerPixel() == 24) {
    for (unsigned y = 0; y < m_height; ++y)
      for (unsigned x = 0; x < m_width; ++x)
        set24(x, y, c.vec3());
  } else {
    for (unsigned y = 0; y < m_height; ++y)
      for (unsigned x = 0; x < m_width; ++x)
        set32(x, y, c);
  }
}

void Image::fillRect(Vec2U const& pos, Vec2U const& size, Vec3B const& c) {
  for (unsigned y = pos[1]; y < pos[1] + size[1] && y < m_height; ++y)
    for (unsigned x = pos[0]; x < pos[0] + size[0] && x < m_width; ++x)
      set(Vec2U(x, y), c);
}

void Image::fillRect(Vec2U const& pos, Vec2U const& size, Vec4B const& c) {
  for (unsigned y = pos[1]; y < pos[1] + size[1] && y < m_height; ++y)
    for (unsigned x = pos[0]; x < pos[0] + size[0] && x < m_width; ++x)
      set(Vec2U(x, y), c);
}

void Image::set(Vec2U const& pos, Vec4B const& c) {
  if (pos[0] >= m_width || pos[1] >= m_height) {
    throw ImageException(strf("{} out of range in Image::set", pos));
  } else if (bytesPerPixel() == 4) {
    std::size_t offset = pos[1] * m_width * 4 + pos[0] * 4;
    m_data[offset] = c[0];
    m_data[offset + 1] = c[1];
    m_data[offset + 2] = c[2];
    m_data[offset + 3] = c[3];
  } else if (bytesPerPixel() == 3) {
    std::size_t offset = pos[1] * m_width * 3 + pos[0] * 3;
    m_data[offset] = c[0];
    m_data[offset + 1] = c[1];
    m_data[offset + 2] = c[2];
  }
}

void Image::set(Vec2U const& pos, Vec3B const& c) {
  if (pos[0] >= m_width || pos[1] >= m_height) {
    throw ImageException(strf("{} out of range in Image::set", pos));
  } else if (bytesPerPixel() == 4) {
    std::size_t offset = pos[1] * m_width * 4 + pos[0] * 4;
    m_data[offset] = c[0];
    m_data[offset + 1] = c[1];
    m_data[offset + 2] = c[2];
    m_data[offset + 3] = 255;
  } else if (bytesPerPixel() == 3) {
    std::size_t offset = pos[1] * m_width * 3 + pos[0] * 3;
    m_data[offset] = c[0];
    m_data[offset + 1] = c[1];
    m_data[offset + 2] = c[2];
  }
}

Vec4B Image::get(Vec2U const& pos) const {
  Vec4B c;
  if (pos[0] >= m_width || pos[1] >= m_height) {
    throw ImageException(strf("{} out of range in Image::get", pos));
  } else if (bytesPerPixel() == 4) {
    std::size_t offset = pos[1] * m_width * 4 + pos[0] * 4;
    c[0] = m_data[offset];
    c[1] = m_data[offset + 1];
    c[2] = m_data[offset + 2];
    c[3] = m_data[offset + 3];
  } else if (bytesPerPixel() == 3) {
    std::size_t offset = pos[1] * m_width * 3 + pos[0] * 3;
    c[0] = m_data[offset];
    c[1] = m_data[offset + 1];
    c[2] = m_data[offset + 2];
    c[3] = 255;
  }
  return c;
}

void Image::setrgb(Vec2U const& pos, Vec4B const& c) {
  if (m_pixelFormat == PixelFormat::BGR24 || m_pixelFormat == PixelFormat::BGRA32)
    set(pos, Vec4B{c[2], c[1], c[0], c[3]});
  else
    set(pos, c);
}

void Image::setrgb(Vec2U const& pos, Vec3B const& c) {
  if (m_pixelFormat == PixelFormat::BGR24 || m_pixelFormat == PixelFormat::BGRA32)
    set(pos, Vec3B{c[2], c[1], c[0]});
  else
    set(pos, c);
}

Vec4B Image::getrgb(Vec2U const& pos) const {
  auto c = get(pos);
  if (m_pixelFormat == PixelFormat::BGR24 || m_pixelFormat == PixelFormat::BGRA32)
    return Vec4B{c[2], c[1], c[0], c[3]};
  else
    return c;
}

Vec4B Image::clamp(Vec2I const& pos) const {
  Vec4B c;
  unsigned x = (unsigned)Star::clamp<int>(pos[0], 0, m_width - 1);
  unsigned y = (unsigned)Star::clamp<int>(pos[1], 0, m_height - 1);
  if (m_width == 0 || m_height == 0) {
    return {0, 0, 0, 0};
  } else if (bytesPerPixel() == 4) {
    std::size_t offset = y * m_width * 4 + x * 4;
    c[0] = m_data[offset];
    c[1] = m_data[offset + 1];
    c[2] = m_data[offset + 2];
    c[3] = m_data[offset + 3];
  } else if (bytesPerPixel() == 3) {
    std::size_t offset = y * m_width * 3 + x * 3;
    c[0] = m_data[offset];
    c[1] = m_data[offset + 1];
    c[2] = m_data[offset + 2];
    c[3] = 255;
  }
  return c;
}

Vec4B Image::clamprgb(Vec2I const& pos) const {
  auto c = clamp(pos);
  if (m_pixelFormat == PixelFormat::BGR24 || m_pixelFormat == PixelFormat::BGRA32)
    return Vec4B{c[2], c[1], c[0], c[3]};
  else
    return c;
}

Image Image::subImage(Vec2U const& pos, Vec2U const& size) const {
  if (pos[0] + size[0] > m_width || pos[1] + size[1] > m_height)
    throw ImageException(strf("call to subImage with pos {} size {} out of image bounds ({}, {})", pos, size, m_width, m_height));

  Image sub(size[0], size[1], m_pixelFormat);

  for (unsigned y = 0; y < size[1]; ++y) {
    for (unsigned x = 0; x < size[0]; ++x) {
      sub.set({x, y}, get(pos + Vec2U(x, y)));
    }
  }

  return sub;
}

void Image::copyInto(Vec2U const& min, Image const& image) {
  Vec2U max = (min + image.size()).piecewiseMin(size());

  for (unsigned y = min[1]; y < max[1]; ++y) {
    for (unsigned x = min[0]; x < max[0]; ++x)
      set(x, y, image.get(Vec2U(x, y) - min));
  }
}

void Image::drawInto(Vec2U const& min, Image const& image) {
  Vec2U max = (min + image.size()).piecewiseMin(size());

  for (unsigned y = min[1]; y < max[1]; ++y) {
    for (unsigned x = min[0]; x < max[0]; ++x) {
      Vec4B dest = get(Vec2U(x, y));
      Vec4B src = image.get(Vec2U(x, y) - min);

      Vec3U destMultiplied = Vec3U(dest[0], dest[1], dest[2]) * dest[3] / 255;
      Vec3U srcMultiplied = Vec3U(src[0], src[1], src[2]) * src[3] / 255;

      // Src over dest alpha composition
      Vec3U over = srcMultiplied + destMultiplied * (255 - src[3]) / 255;
      unsigned alpha = src[3] + dest[3] * (255 - src[3]) / 255;

      set(x, y, Vec4B(over[0], over[1], over[2], alpha));
    }
  }
}

Image Image::convert(PixelFormat pixelFormat) const {
  Image converted(m_width, m_height, pixelFormat);
  converted.copyInto(Vec2U(), *this);
  return converted;
}

void Image::writePng(Logger& logger, std::shared_ptr<IODevice> device) const {
  PngState state{logger, device.get(), true};
  initializePng(state);
  unsigned channels = m_pixelFormat == PixelFormat::RGB24 ? 3 : 4;
  auto rows = std::make_unique<png_bytep[]>(m_height);
  std::size_t stride = static_cast<std::size_t>(m_width) * channels;
  for (std::size_t i = 0; i < m_height; ++i)
    rows[i] = m_data + (m_height - i - 1) * stride;
  state.check(writePngRows(state, m_width, m_height, channels, rows.get()), "Internal error writing png.");
}

ImageView::ImageView(Image const& image) {
  size = image.size();
  data = image.data();
  format = image.pixelFormat();
}

}
