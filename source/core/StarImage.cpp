module;

#include <png.h>

module star.image;

import std;
import star.logging;

namespace star {

struct png_read_struct {
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    png_infop end_info = nullptr;

    png_read_struct() = default;

    png_read_struct(const png_read_struct&) = default;
    png_read_struct(png_read_struct&&) = delete;

    auto operator=(const png_read_struct&) -> png_read_struct& = default;
    auto operator=(png_read_struct&&) -> png_read_struct& = delete;

    ~png_read_struct() {
        if (png_ptr) {
            png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : nullptr,
                                    end_info ? &end_info : nullptr);
        }
    }
};

struct png_write_struct {
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;

    png_write_struct() = default;
    png_write_struct(const png_write_struct&) = default;
    png_write_struct(png_write_struct&&) = delete;

    auto operator=(const png_write_struct&) -> png_write_struct& = default;
    auto operator=(png_write_struct&&) -> png_write_struct& = delete;

    ~png_write_struct() {
        if (png_ptr) {
            png_destroy_write_struct(&png_ptr, info_ptr ? &info_ptr : nullptr);
        }
    }
};

void log_png_error(png_structp png_ptr, png_const_charp c) {
    auto* device = (io_device*)png_get_error_ptr(png_ptr);
    logger::debug("PNG error in file: '{}', {}", device->device_name(), c);
};

void log_png_warning(png_structp png_ptr, png_const_charp c) {
    auto* device = (io_device*)png_get_error_ptr(png_ptr);
    logger::debug("PNG warning in file: '{}', {}", device->device_name(), c);
};

void read_png_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    auto* device = (io_device*)png_get_io_ptr(png_ptr);
    device->read_full(std::as_writable_bytes(std::span{(char*)data, length}));
};

auto image::is_png(const std::shared_ptr<io_device>& device) -> bool {
    std::array<png_byte, 8> header{};
    return !png_sig_cmp(header.data(), 0,
                        device->read_absolute(0, std::as_writable_bytes(std::span{header})));
}

auto image::read_png(const std::shared_ptr<io_device>& device) -> image {
    std::array<png_byte, 8> header{};
    device->read_full(std::as_writable_bytes(std::span{header}));

    if (png_sig_cmp(header.data(), 0, header.size())) {
        throw image_exception(std::format("File {} is not a png image!", device->device_name()));
    }

    png_read_struct png;
    png.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png.png_ptr) {
        throw image_exception("Internal libPNG error");
    }

    // Use custom warning function to suppress cerr warnings
    png_set_error_fn(png.png_ptr, (png_voidp)device.get(), log_png_error, log_png_warning);

    png.info_ptr = png_create_info_struct(png.png_ptr);
    if (!png.info_ptr) {
        throw image_exception("Internal libPNG error");
    }

    png.end_info = png_create_info_struct(png.png_ptr);
    if (!png.end_info) {
        throw image_exception("Internal libPNG error");
    }

    if (setjmp(png_jmpbuf(png.png_ptr))) {
        throw image_exception("Internal error reading png.");
    }

    png_set_read_fn(png.png_ptr, device.get(), read_png_data);

    // Tell libPNG that we read some of the header.
    png_set_sig_bytes(png.png_ptr, sizeof(header));//NOLINT

    png_read_info(png.png_ptr, png.info_ptr);

    png_uint_32 img_width = png_get_image_width(png.png_ptr, png.info_ptr);
    png_uint_32 img_height = png_get_image_height(png.png_ptr, png.info_ptr);

    png_uint_32 bitdepth = png_get_bit_depth(png.png_ptr, png.info_ptr);
    png_uint_32 channels = png_get_channels(png.png_ptr, png.info_ptr);

    // Color type. (RGB, RGBA, Luminance, luminance alpha... palette... etc)
    png_uint_32 color_type = png_get_color_type(png.png_ptr, png.info_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png.png_ptr);
        channels = 3;
        bitdepth = 8;
    }

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        if (bitdepth < 8) {
            png_set_expand_gray_1_2_4_to_8(png.png_ptr);
            bitdepth = 8;
        }
        png_set_gray_to_rgb(png.png_ptr);
        if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
            channels = 4;
        } else {
            channels = 3;
        }
    }

    // If the image has a transperancy set, convert it to a full alpha channel
    if (png_get_valid(png.png_ptr, png.info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png.png_ptr);
        channels += 1;
    }

    // We don't support 16 bit precision.. so if the image Has 16 bits per channel
    // precision... round it down to 8.
    if (bitdepth == 16) {
        png_set_strip_16(png.png_ptr);
        bitdepth = 8;
    }

    if (bitdepth != 8 || (channels != 3 && channels != 4)) {
        throw image_exception(
          std::format("Unsupported PNG pixel format in file {}", device->device_name()));
    }

    image img(img_width, img_height, channels == 3 ? pixel_format::rgb24 : pixel_format::rgba32);

    std::vector<png_bytep> row_ptrs(img_height);
    auto stride = static_cast<std::size_t>(img_width * channels);//NOLINT
    for (std::size_t i = 0; i < img_height; ++i) {
        row_ptrs[i] = (png_bytep)img.data() + (img_height - i - 1) * stride;
    }

    png_read_image(png.png_ptr, row_ptrs.data());

    return img;
}

auto image::read_png_metadata(const std::shared_ptr<io_device>& device)
  -> std::tuple<vec_2u, pixel_format> {
    std::array<png_byte, 8> header{};
    device->read_full(std::as_writable_bytes(std::span{header}));

    if (png_sig_cmp(header.data(), 0, header.size())) {
        throw image_exception(std::format("File {} is not a png image!", device->device_name()));
    }

    png_read_struct png;
    png.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png.png_ptr) {
        throw image_exception("Internal libPNG error");
    }

    // Use custom warning function to suppress cerr warnings
    png_set_error_fn(png.png_ptr, (png_voidp)device.get(), log_png_error, log_png_warning);

    png.info_ptr = png_create_info_struct(png.png_ptr);
    if (!png.info_ptr) {
        throw image_exception("Internal libPNG error");
    }

    png.end_info = png_create_info_struct(png.png_ptr);
    if (!png.end_info) {
        throw image_exception("Internal libPNG error");
    }

    if (setjmp(png_jmpbuf(png.png_ptr))) {
        throw image_exception("Internal error reading png.");
    }

    png_set_read_fn(png.png_ptr, device.get(), read_png_data);

    // Tell libPNG that we read some of the header.
    png_set_sig_bytes(png.png_ptr, sizeof(header));//NOLINT

    png_read_info(png.png_ptr, png.info_ptr);

    png_uint_32 img_width = png_get_image_width(png.png_ptr, png.info_ptr);
    png_uint_32 img_height = png_get_image_height(png.png_ptr, png.info_ptr);

    png_uint_32 bitdepth = png_get_bit_depth(png.png_ptr, png.info_ptr);
    png_uint_32 channels = png_get_channels(png.png_ptr, png.info_ptr);

    // Color type. (RGB, RGBA, Luminance, luminance alpha... palette... etc)
    png_uint_32 color_type = png_get_color_type(png.png_ptr, png.info_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png.png_ptr);
        channels = 3;
        bitdepth = 8;
    }

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        if (bitdepth < 8) {
            png_set_expand_gray_1_2_4_to_8(png.png_ptr);
            bitdepth = 8;
        }
        png_set_gray_to_rgb(png.png_ptr);
        if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
            channels = 4;
        } else {
            channels = 3;
        }
    }

    // If the image has a transperancy set, convert it to a full alpha channel
    if (png_get_valid(png.png_ptr, png.info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png.png_ptr);
        channels += 1;
    }

    vec_2u image_size{img_width, img_height};
    pixel_format pixel_format = channels == 3 ? pixel_format::rgb24 : pixel_format::rgba32;

    return std::make_tuple(image_size, pixel_format);
}

auto image::filled(vec_2u size, vec_4b color, pixel_format pf) -> image {
    image img(size, pf);
    img.fill(color);
    return img;
}

image::image(pixel_format pf) : m_pixel_format(pf) {}

image::image(vec_2u size, pixel_format pf) : image(size[0], size[1], pf) {}

image::image(unsigned width, unsigned height, pixel_format pf) : image(pf) {
    reset(width, height, pf);
}

void image::reset(vec_2u size, std::optional<pixel_format> pf) { reset(size[0], size[1], pf); }

void image::reset(unsigned width, unsigned height, std::optional<pixel_format> pf) {
    if (!pf) {
        pf = m_pixel_format;
    }

    if (m_width == width && m_height == height && m_pixel_format == *pf) {
        return;
    }

    auto image_size = static_cast<std::size_t>(width * height * star::bytes_per_pixel(*pf));//NOLINT
    m_data.assign(image_size, 0);

    m_pixel_format = *pf;
    m_width = width;
    m_height = height;
}

void image::fill(vec_3b const& c) {
    if (bits_per_pixel() == 24) {
        for (unsigned y = 0; y < m_height; ++y) {
            for (unsigned x = 0; x < m_width; ++x) {
                set24(x, y, c);
            }
        }
    } else {
        for (unsigned y = 0; y < m_height; ++y) {
            for (unsigned x = 0; x < m_width; ++x) {
                set32(x, y, vec_4b(c, 255));
            }
        }
    }
}

void image::fill(vec_4b const& c) {
    if (bits_per_pixel() == 24) {
        for (unsigned y = 0; y < m_height; ++y) {
            for (unsigned x = 0; x < m_width; ++x) {
                set24(x, y, c.to_size<3>());
            }
        }
    } else {
        for (unsigned y = 0; y < m_height; ++y) {
            for (unsigned x = 0; x < m_width; ++x) {
                set32(x, y, c);
            }
        }
    }
}

void image::fill_rect(vec_2u const& pos, vec_2u const& size, vec_3b const& c) {
    for (unsigned y = pos[1]; y < pos[1] + size[1] && y < m_height; ++y) {
        for (unsigned x = pos[0]; x < pos[0] + size[0] && x < m_width; ++x) {
            set(vec_2u(x, y), c);
        }
    }
}

void image::fill_rect(vec_2u const& pos, vec_2u const& size, vec_4b const& c) {
    for (unsigned y = pos[1]; y < pos[1] + size[1] && y < m_height; ++y) {
        for (unsigned x = pos[0]; x < pos[0] + size[0] && x < m_width; ++x) {
            set(vec_2u(x, y), c);
        }
    }
}

void image::set(vec_2u const& pos, vec_4b const& c) {
    if (pos[0] >= m_width || pos[1] >= m_height) {
        throw image_exception(std::format("{} out of range in image::set", pos));
    } else if (bytes_per_pixel() == 4) {
        std::size_t offset = pos[1] * m_width * 4 + pos[0] * 4;
        m_data[offset] = c[0];
        m_data[offset + 1] = c[1];
        m_data[offset + 2] = c[2];
        m_data[offset + 3] = c[3];
    } else if (bytes_per_pixel() == 3) {
        std::size_t offset = pos[1] * m_width * 3 + pos[0] * 3;
        m_data[offset] = c[0];
        m_data[offset + 1] = c[1];
        m_data[offset + 2] = c[2];
    }
}

void image::set(vec_2u const& pos, vec_3b const& c) {
    if (pos[0] >= m_width || pos[1] >= m_height) {
        throw image_exception(std::format("{} out of range in image::set", pos));
    } else if (bytes_per_pixel() == 4) {
        std::size_t offset = pos[1] * m_width * 4 + pos[0] * 4;
        m_data[offset] = c[0];
        m_data[offset + 1] = c[1];
        m_data[offset + 2] = c[2];
        m_data[offset + 3] = 255;
    } else if (bytes_per_pixel() == 3) {
        std::size_t offset = pos[1] * m_width * 3 + pos[0] * 3;
        m_data[offset] = c[0];
        m_data[offset + 1] = c[1];
        m_data[offset + 2] = c[2];
    }
}

auto image::get(vec_2u const& pos) const -> vec_4b {
    vec_4b c;
    if (pos[0] >= m_width || pos[1] >= m_height) {
        throw image_exception(std::format("{} out of range in image::get", pos));
    } else if (bytes_per_pixel() == 4) {
        std::size_t offset = pos[1] * m_width * 4 + pos[0] * 4;
        c[0] = m_data[offset];
        c[1] = m_data[offset + 1];
        c[2] = m_data[offset + 2];
        c[3] = m_data[offset + 3];
    } else if (bytes_per_pixel() == 3) {
        std::size_t offset = pos[1] * m_width * 3 + pos[0] * 3;
        c[0] = m_data[offset];
        c[1] = m_data[offset + 1];
        c[2] = m_data[offset + 2];
        c[3] = 255;
    }
    return c;
}

void image::set_rgb(vec_2u const& pos, vec_4b const& c) {
    if (m_pixel_format == pixel_format::bgr24 || m_pixel_format == pixel_format::bgra32) {
        set(pos, vec_4b{c[2], c[1], c[0], c[3]});
    } else {
        set(pos, c);
    }
}

void image::set_rgb(vec_2u const& pos, vec_3b const& c) {
    if (m_pixel_format == pixel_format::bgr24 || m_pixel_format == pixel_format::bgra32) {
        set(pos, vec_3b{c[2], c[1], c[0]});
    } else {
        set(pos, c);
    }
}

auto image::get_rgb(vec_2u const& pos) const -> vec_4b {
    auto c = get(pos);
    if (m_pixel_format == pixel_format::bgr24 || m_pixel_format == pixel_format::bgra32) {
        return vec_4b{c[2], c[1], c[0], c[3]};
    } else {
        return c;
    }
}

auto image::clamp(vec_2i const& pos) const -> vec_4b {
    vec_4b c;
    auto x = (unsigned)std::clamp<int>(pos[0], 0, m_width - 1); //NOLINT
    auto y = (unsigned)std::clamp<int>(pos[1], 0, m_height - 1);//NOLINT
    if (m_width == 0 || m_height == 0) {
        return vec_4b{0, 0, 0, 0};
    } else if (bytes_per_pixel() == 4) {
        std::size_t offset = y * m_width * 4 + x * 4;
        c[0] = m_data[offset];
        c[1] = m_data[offset + 1];
        c[2] = m_data[offset + 2];
        c[3] = m_data[offset + 3];
    } else if (bytes_per_pixel() == 3) {
        std::size_t offset = y * m_width * 3 + x * 3;
        c[0] = m_data[offset];
        c[1] = m_data[offset + 1];
        c[2] = m_data[offset + 2];
        c[3] = 255;
    }
    return c;
}

auto image::clamp_rgb(vec_2i const& pos) const -> vec_4b {
    auto c = clamp(pos);
    if (m_pixel_format == pixel_format::bgr24 || m_pixel_format == pixel_format::bgra32) {
        return vec_4b{c[2], c[1], c[0], c[3]};
    } else {
        return c;
    }
}

auto image::sub_image(vec_2u const& pos, vec_2u const& size) const -> image {
    if (pos[0] + size[0] > m_width || pos[1] + size[1] > m_height) {
        throw image_exception(
          std::format("call to sub_image with pos {} size {} out of image bounds ({}, {})", pos,
                      size, m_width, m_height));
    }

    image sub(size[0], size[1], m_pixel_format);

    for (unsigned y = 0; y < size[1]; ++y) {
        for (unsigned x = 0; x < size[0]; ++x) {
            sub.set(vec_2u{x, y}, get(pos + vec_2u(x, y)));
        }
    }

    return sub;
}

void image::copy_into(vec_2u const& min, image const& image) {
    vec_2u max = (min + image.size());
    max[0] = std::min(max[0], size()[0]);
    max[1] = std::min(max[1], size()[1]);

    for (unsigned y = min[1]; y < max[1]; ++y) {
        for (unsigned x = min[0]; x < max[0]; ++x) {
            set(x, y, image.get(vec_2u(x, y) - min));
        }
    }
}

void image::draw_into(vec_2u const& min, image const& image) {
    vec_2u max = (min + image.size());
    max[0] = std::min(max[0], size()[0]);
    max[1] = std::min(max[1], size()[1]);

    for (unsigned y = min[1]; y < max[1]; ++y) {
        for (unsigned x = min[0]; x < max[0]; ++x) {
            vec_4b dest = get(vec_2u(x, y));
            vec_4b src = image.get(vec_2u(x, y) - min);

            vec_3u dest_multiplied = vec_3u(dest[0], dest[1], dest[2]) * dest[3] / 255;
            vec_3u src_multiplied = vec_3u(src[0], src[1], src[2]) * src[3] / 255;

            // Src over dest alpha composition
            vec_3u over = src_multiplied + dest_multiplied * (255 - src[3]) / 255;
            unsigned alpha = src[3] + dest[3] * (255 - src[3]) / 255;

            set(x, y, vec_4b(over[0], over[1], over[2], alpha));
        }
    }
}

auto image::convert(pixel_format pixel_format) const -> image {
    image converted(m_width, m_height, pixel_format);
    converted.copy_into(vec_2u(), *this);
    return converted;
}

void image::write_png(const std::shared_ptr<io_device>& device) const {
    auto write_png_data = [](png_structp png_ptr, png_bytep data, png_size_t length) -> void {
        auto* device = (io_device*)png_get_io_ptr(png_ptr);
        device->write_full(std::as_bytes(std::span{(char*)data, length}));
    };

    auto flush_png_data = [](png_structp) -> void {};

    png_write_struct png;

    png.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png.png_ptr) {
        throw image_exception("Internal libPNG error");
    }

    png.info_ptr = png_create_info_struct(png.png_ptr);
    if (!png.info_ptr) {
        throw image_exception("Internal libPNG error");
    }

    if (setjmp(png_jmpbuf(png.png_ptr))) {
        throw image_exception("Internal error reading png.");
    }

    unsigned channels = m_pixel_format == pixel_format::rgb24 ? 3 : 4;

    png_set_IHDR(png.png_ptr, png.info_ptr, m_width, m_height, 8,
                 channels == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    std::vector<png_bytep> row_ptrs(m_height);
    std::size_t stride = m_width * 8 * channels / 8;
    for (std::size_t i = 0; i < m_height; ++i) {
        std::size_t q = (m_height - i - 1) * stride;
        row_ptrs[i] = (png_bytep)m_data.data() + q;
    }

    png_set_write_fn(png.png_ptr, device.get(), write_png_data, flush_png_data);
    png_set_rows(png.png_ptr, png.info_ptr, row_ptrs.data());
    png_write_png(png.png_ptr, png.info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);
}

image_view::image_view(image const& image) {
    size = image.size();
    data = image.data();
    format = image.format();
}

}// namespace star
