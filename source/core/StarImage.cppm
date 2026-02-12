export module star.image;

import std;
import star.exception;
import star.io_device;
import star.vector;

export namespace star {

enum class pixel_format : std::uint8_t { rgb24, rgba32, bgr24, bgra32, rgb_f, rgba_f };

auto bits_per_pixel(pixel_format pf) -> std::uint8_t;
auto bytes_per_pixel(pixel_format pf) -> std::uint8_t;

using image_exception = exception_derived<"image_exception">;

// Holds an image of PixelFormat in row major order, with no padding, with (0,
// 0) defined to be the *lower left* corner.
class image {
  public:
    static auto read_png(const std::shared_ptr<io_device>& device) -> image;
    static auto is_png(const std::shared_ptr<io_device>& device) -> bool;
    // Returns the size and pixel format that would be constructed from the given
    // png file, without actually loading it.
    static auto read_png_metadata(const std::shared_ptr<io_device>& device)
      -> std::tuple<vec_2u, pixel_format>;

    static auto filled(vec_2u size, vec_4b color, pixel_format pf = pixel_format::rgba32) -> image;

    // Creates a zero size image
    explicit image(pixel_format pf = pixel_format::rgba32);
    explicit image(vec_2u size, pixel_format pf = pixel_format::rgba32);
    image(unsigned width, unsigned height, pixel_format pf = pixel_format::rgba32);
    ~image() = default;

    image(image const& image) = default;
    image(image&& image) noexcept = default;

    auto operator=(image const& image) -> class image& = default;
    auto operator=(image&& image) noexcept -> class image& = default;

    [[nodiscard]] auto bits_per_pixel() const -> std::uint8_t;
    [[nodiscard]] auto bytes_per_pixel() const -> std::uint8_t;

    [[nodiscard]] auto width() const -> unsigned;
    [[nodiscard]] auto height() const -> unsigned;
    [[nodiscard]] auto size() const -> vec_2u;
    // width or height is 0
    [[nodiscard]] auto empty() const -> bool;

    [[nodiscard]] auto format() const -> pixel_format;

    // If the image is empty, the data ptr will be null
    [[nodiscard]] auto data() const -> std::uint8_t const*;
    auto data() -> std::uint8_t*;

    // Reallocate the image with the given width, height, and pixel format.  The
    // contents of the image are always zeroed after a call to reset.
    void reset(vec_2u size, std::optional<pixel_format> pf = {});
    void reset(unsigned width, unsigned height, std::optional<pixel_format> pf = {});

    // Fill the image with a given color
    void fill(vec_3b const& c);
    void fill(vec_4b const& c);

    // Fill a rectangle with a given color
    void fill_rect(vec_2u const& pos, vec_2u const& size, vec_3b const& c);
    void fill_rect(vec_2u const& pos, vec_2u const& size, vec_4b const& c);

    // Color parameters / return values here are in whatever the internal format
    // is.  Fourth byte, if missing or not provided, is assumed to be 255.  If
    // the position is out of range, then throws an exception.
    void set(vec_2u const& pos, vec_4b const& c);
    void set(vec_2u const& pos, vec_3b const& c);
    [[nodiscard]] auto get(vec_2u const& pos) const -> vec_4b;

    // Same as set / get, except color parameters / return values here are always
    // RGB[A], and converts if necessary.
    void set_rgb(vec_2u const& pos, vec_4b const& c);
    void set_rgb(vec_2u const& pos, vec_3b const& c);
    [[nodiscard]] auto get_rgb(vec_2u const& pos) const -> vec_4b;

    // Get pixel value, but if pos is out of the normal pixel range, it is
    // clamped back into the valid pixel range.  Returns (0, 0, 0, 0) if image is
    // empty.
    [[nodiscard]] auto clamp(vec_2i const& pos) const -> vec_4b;
    [[nodiscard]] auto clamp_rgb(vec_2i const& pos) const -> vec_4b;

    // x / y versions of set / get, for compatibility
    void set(unsigned x, unsigned y, vec_4b const& c);
    void set(unsigned x, unsigned y, vec_3b const& c);
    [[nodiscard]] auto get(unsigned x, unsigned y) const -> vec_4b;
    void set_rgb(unsigned x, unsigned y, vec_4b const& c);
    void set_rgb(unsigned x, unsigned y, vec_3b const& c);
    [[nodiscard]] auto get_rgb(unsigned x, unsigned y) const -> vec_4b;
    [[nodiscard]] auto clamp(int x, int y) const -> vec_4b;
    [[nodiscard]] auto clamp_rgb(int x, int y) const -> vec_4b;

    // Must be 32 bits_per_pixel, no format conversion or bounds checking takes
    // place.  Very fast inline versions.
    void set32(vec_2u const& pos, vec_4b const& c);
    void set32(unsigned x, unsigned y, vec_4b const& c);
    [[nodiscard]] auto get32(unsigned x, unsigned y) const -> vec_4b;

    // Must be 24 bits_per_pixel, no format conversion or bounds checking takes
    // place.  Very fast inline versions.
    void set24(vec_2u const& pos, vec_3b const& c);
    void set24(unsigned x, unsigned y, vec_3b const& c);
    [[nodiscard]] auto get24(unsigned x, unsigned y) const -> vec_3b;

    // Called as callback(unsigned x, unsigned y, vec_4b const& pixel)
    template <typename CallbackType> void for_each_pixel(CallbackType&& callback) const;

    // Called as callback(unsigned x, unsigned y, vec_4b& pixel)
    template <typename CallbackType> void for_each_pixel(CallbackType&& callback);

    // Pixel rectangle, lower left position and size of rectangle.
    [[nodiscard]] auto sub_image(vec_2u const& pos, vec_2u const& size) const -> image;

    // Copy given image into this one at pos
    void copy_into(vec_2u const& pos, image const& image);
    // Draw given image over this one at pos (with alpha composition)
    void draw_into(vec_2u const& pos, image const& image);

    // Convert this image into the given pixel format
    [[nodiscard]] auto convert(pixel_format pixel_format) const -> image;

    void write_png(const std::shared_ptr<io_device>& device) const;

  private:
    std::vector<std::uint8_t> m_data;
    unsigned m_width = 0;
    unsigned m_height = 0;
    pixel_format m_pixel_format = pixel_format::rgba32;
};

inline auto bits_per_pixel(pixel_format pf) -> std::uint8_t {
    switch (pf) {
    case pixel_format::rgb24: return 24;
    case pixel_format::rgba32: return 32;
    case pixel_format::bgr24: return 24;
    case pixel_format::bgra32: return 32;
    case pixel_format::rgb_f: return 96;
    default: return 128;
    }
}

inline auto bytes_per_pixel(pixel_format pf) -> std::uint8_t {
    switch (pf) {
    case pixel_format::rgb24: return 3;
    case pixel_format::rgba32: return 4;
    case pixel_format::bgr24: return 3;
    case pixel_format::bgra32: return 4;
    case pixel_format::rgb_f: return 12;
    default: return 16;
    }
}

inline auto image::bits_per_pixel() const -> std::uint8_t {
    return star::bits_per_pixel(m_pixel_format);
}

inline auto image::bytes_per_pixel() const -> std::uint8_t {
    return star::bytes_per_pixel(m_pixel_format);
}

inline auto image::width() const -> unsigned { return m_width; }

inline auto image::height() const -> unsigned { return m_height; }

inline auto image::empty() const -> bool { return m_width == 0 || m_height == 0; }

inline auto image::size() const -> vec_2u { return vec_2u{m_width, m_height}; }

inline auto image::format() const -> pixel_format { return m_pixel_format; }

inline auto image::data() const -> const std::uint8_t* {
    return m_data.empty() ? nullptr : m_data.data();
}

inline auto image::data() -> std::uint8_t* { return m_data.empty() ? nullptr : m_data.data(); }

inline void image::set(unsigned x, unsigned y, vec_4b const& c) { return set(vec_2u{x, y}, c); }

inline void image::set(unsigned x, unsigned y, vec_3b const& c) { return set(vec_2u{x, y}, c); }

inline auto image::get(unsigned x, unsigned y) const -> vec_4b { return get(vec_2u{x, y}); }

inline void image::set_rgb(unsigned x, unsigned y, vec_4b const& c) {
    return set_rgb(vec_2u{x, y}, c);
}

inline void image::set_rgb(unsigned x, unsigned y, vec_3b const& c) {
    return set_rgb(vec_2u{x, y}, c);
}

inline auto image::get_rgb(unsigned x, unsigned y) const -> vec_4b { return get_rgb(vec_2u{x, y}); }

inline auto image::clamp(int x, int y) const -> vec_4b { return clamp(vec_2i{x, y}); }

inline auto image::clamp_rgb(int x, int y) const -> vec_4b { return clamp_rgb(vec_2i{x, y}); }

inline void image::set32(vec_2u const& pos, vec_4b const& c) { set32(pos[0], pos[1], c); }

inline void image::set32(unsigned x, unsigned y, vec_4b const& c) {

    std::size_t offset = y * m_width * 4 + x * 4;
    m_data[offset] = c[0];
    m_data[offset + 1] = c[1];
    m_data[offset + 2] = c[2];
    m_data[offset + 3] = c[3];
}

inline auto image::get32(unsigned x, unsigned y) const -> vec_4b {

    vec_4b c;
    std::size_t offset = y * m_width * 4 + x * 4;
    c[0] = m_data[offset];
    c[1] = m_data[offset + 1];
    c[2] = m_data[offset + 2];
    c[3] = m_data[offset + 3];
    return c;
}

inline void image::set24(vec_2u const& pos, vec_3b const& c) { set24(pos[0], pos[1], c); }

inline void image::set24(unsigned x, unsigned y, vec_3b const& c) {

    std::size_t offset = y * m_width * 3 + x * 3;
    m_data[offset] = c[0];
    m_data[offset + 1] = c[1];
    m_data[offset + 2] = c[2];
}

inline auto image::get24(unsigned x, unsigned y) const -> vec_3b {

    vec_3b c;
    std::size_t offset = y * m_width * 3 + x * 3;
    c[0] = m_data[offset];
    c[1] = m_data[offset + 1];
    c[2] = m_data[offset + 2];
    return c;
}

template <typename CallbackType> void image::for_each_pixel(CallbackType&& callback) const {
    for (unsigned y = 0; y < m_height; y++) {
        for (unsigned x = 0; x < m_width; x++) {
            callback(x, y, get(x, y));
        }
    }
}

template <typename CallbackType> void image::for_each_pixel(CallbackType&& callback) {
    for (unsigned y = 0; y < m_height; y++) {
        for (unsigned x = 0; x < m_width; x++) {
            vec_4b pixel = get(x, y);
            std::forward<CallbackType>(callback)(x, y, pixel);
            set(x, y, pixel);
        }
    }
}

struct image_view {
    [[nodiscard]] inline auto empty() const -> bool { return size.x() == 0 || size.y() == 0; }
    image_view() = default;
    explicit image_view(image const& image);

    vec_2u size{0, 0};
    std::uint8_t const* data = nullptr;
    pixel_format format = pixel_format::rgb24;
};

}// namespace star
