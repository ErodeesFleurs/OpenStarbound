export module star.color;

import std;
import star.exception;
import star.math_common;
import star.vector;

export namespace star {

using color_exception = exception_derived<"color_exception">;

class color {
  public:
    static const color Red;
    static const color Orange;
    static const color Yellow;
    static const color Green;
    static const color Blue;
    static const color Indigo;
    static const color Violet;
    static const color Black;
    static const color White;
    static const color Magenta;
    static const color DarkMagenta;
    static const color Cyan;
    static const color DarkCyan;
    static const color CornFlowerBlue;
    static const color Gray;
    static const color LightGray;
    static const color DarkGray;
    static const color DarkGreen;
    static const color Pink;
    static const color Clear;

    static const std::flat_map<std::string, color> named_colors;

    [[nodiscard]] static constexpr auto rgbf(float r, float g, float b) -> color {
        return rgbaf(r, g, b, 1.0F);
    }

    [[nodiscard]] static constexpr auto rgbaf(float r, float g, float b, float a) -> color {
        color c;
        c.m_data = vec_4f{std::clamp(r, 0.0F, 1.0F), std::clamp(g, 0.0F, 1.0F),
                         std::clamp(b, 0.0F, 1.0F), std::clamp(a, 0.0F, 1.0F)};
        return c;
    }

    [[nodiscard]] static constexpr auto rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b,
                                             std::uint8_t a) -> color {
        return rgbaf(static_cast<std::float_t>(r) / 255.0F, static_cast<std::float_t>(g) / 255.0F,
                     static_cast<std::float_t>(b) / 255.0F, static_cast<std::float_t>(a) / 255.0F);
    }

    [[nodiscard]] static constexpr auto rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
      -> color {
        return rgba(r, g, b, 255);
    }

    [[nodiscard]] static constexpr auto from_uint32(std::uint32_t v) -> color {
        auto [b, g, r, a] = std::bit_cast<std::array<std::uint8_t, 4>>(v);
        return rgba(r, g, b, a);
    }

    constexpr color() : m_data{0, 0, 0, 1} {}
    explicit color(std::u8string_view name);

    template <typename Self> [[nodiscard]] constexpr auto data(this Self&& self) -> auto&& {
        return std::forward<Self>(self).m_data;
    }

    [[nodiscard]] constexpr auto red() const -> std::uint8_t {
        return static_cast<std::uint8_t>(std::round(m_data[0] * 255));
    }
    [[nodiscard]] constexpr auto green() const -> std::uint8_t {
        return static_cast<std::uint8_t>(std::round(m_data[1] * 255));
    }
    [[nodiscard]] constexpr auto blue() const -> std::uint8_t {
        return static_cast<std::uint8_t>(std::round(m_data[2] * 255));
    }
    [[nodiscard]] constexpr auto alpha() const -> std::uint8_t {
        return static_cast<std::uint8_t>(std::round(m_data[3] * 255));
    }

    constexpr void set_red(std::uint8_t r) { m_data[0] = static_cast<std::float_t>(r) / 255.0F; }
    constexpr void set_green(std::uint8_t g) { m_data[1] = static_cast<std::float_t>(g) / 255.0F; }
    constexpr void set_blue(std::uint8_t b) { m_data[2] = static_cast<std::float_t>(b) / 255.0F; }
    constexpr void set_alpha(std::uint8_t a) { m_data[3] = static_cast<std::float_t>(a) / 255.0F; }

    [[nodiscard]] constexpr auto isClear() const -> bool { return m_data[3] == 0.0F; }

    [[nodiscard]] constexpr auto to_uint32() const -> std::uint32_t {
        return std::bit_cast<std::uint32_t>(
          std::array<std::uint8_t, 4>{blue(), green(), red(), alpha()});
    }

    [[nodiscard]] constexpr auto to_rgba() const -> vec_4b {
        return vec_4b{red(), green(), blue(), alpha()};
    }
    [[nodiscard]] constexpr auto to_rgbaf() const -> vec_4f { return m_data; }

    [[nodiscard]] constexpr auto operator==(color const&) const -> bool = default;

    [[nodiscard]] constexpr auto mix(color const& c, float amount = 0.5F) const -> color {
        return rgbaf(
          std::lerp(m_data[0], c.m_data[0], amount), std::lerp(m_data[1], c.m_data[1], amount),
          std::lerp(m_data[2], c.m_data[2], amount), std::lerp(m_data[3], c.m_data[3], amount));
    }

    [[nodiscard]] static constexpr auto hsva(float h, float s, float v, float a) -> color;
    [[nodiscard]] static constexpr auto from_hex(std::u8string_view s) -> color;
    [[nodiscard]] static constexpr auto temperature(float temp) -> color;

    // Some useful conversion methods for dealing with Vec3 / Vec4 as colors
    [[nodiscard]] static constexpr auto v3b_to_float(const vec_3b& b) -> vec_3f;
    [[nodiscard]] static constexpr auto v3f_to_byte(const vec_3f& f, bool doClamp = true) -> vec_3b;
    [[nodiscard]] static constexpr auto v4b_to_float(const vec_4b& b) -> vec_4f;
    [[nodiscard]] static constexpr auto v4f_to_byte(const vec_4f& f, bool doClamp = true) -> vec_4b;

    [[nodiscard]] static constexpr auto rgbf(const vec_3f& c) -> color;
    [[nodiscard]] static constexpr auto rgbaf(const vec_4f& c) -> color;

    [[nodiscard]] static constexpr auto rgb(vec_3b const& c) -> color;
    [[nodiscard]] static constexpr auto rgba(vec_4b const& c) -> color;

    [[nodiscard]] static constexpr auto hsv(float h, float s, float b) -> color;
    [[nodiscard]] static constexpr auto hsv(vec_3f const& c) -> color;
    [[nodiscard]] static constexpr auto hsva(vec_4f const& c) -> color;

    [[nodiscard]] static constexpr auto grayf(float g) -> color;
    [[nodiscard]] static constexpr auto gray(std::uint8_t g) -> color;

    static auto hue_shift_vec_4b(vec_4b color, float hue) -> vec_4b;
    static auto hex_to_vec_4b(std::u8string_view s) -> vec_4b;
    // Black;

    [[nodiscard]] constexpr auto redf() const -> float { return m_data[0]; };
    [[nodiscard]] constexpr auto greenf() const -> float { return m_data[1]; };
    [[nodiscard]] constexpr auto bluef() const -> float { return m_data[2]; };
    [[nodiscard]] constexpr auto alphaf() const -> float { return m_data[3]; };

    constexpr void set_redf(float r) { m_data[0] = std::clamp(r, 0.0F, 1.0F); };
    constexpr void set_greenf(float g) { m_data[1] = std::clamp(g, 0.0F, 1.0F); };
    constexpr void set_bluef(float b) { m_data[2] = std::clamp(b, 0.0F, 1.0F); };
    constexpr void set_alphaf(float a) { m_data[3] = std::clamp(a, 0.0F, 1.0F); };

    [[nodiscard]] constexpr auto to_rgb() const -> vec_3b;
    [[nodiscard]] constexpr auto to_rgbf() const -> vec_3f;

    [[nodiscard]] constexpr auto toHsva() const -> vec_4f;

    [[nodiscard]] constexpr auto hue() const -> float;
    [[nodiscard]] constexpr auto saturation() const -> float;
    [[nodiscard]] constexpr auto value() const -> float;

    constexpr void set_hue(float hue);
    constexpr void set_saturation(float saturation);
    constexpr void set_value(float value);

    // Shift the current hue by the given value, with hue wrapping.
    constexpr void hue_shift(float hue);

    // Reduce the color toward black by the given amount, from 0.0 to 1.0.
    constexpr void fade(float value);

    [[nodiscard]] constexpr auto to_hex() const -> std::u8string;

    constexpr void convert_to_linear();
    constexpr void convert_to_srgb();

    [[nodiscard]] constexpr auto to_linear() -> color;
    [[nodiscard]] constexpr auto to_srgb() -> color;

    [[nodiscard]] constexpr auto contrasting() -> color;
    [[nodiscard]] constexpr auto complementary() -> color;

    // Mix two colors, giving the second color the given amount
    [[nodiscard]] constexpr auto multiply(float amount) const -> color;

    auto operator!=(color const& c) const -> bool;
    auto operator+(color const& c) const -> color;
    auto operator*(color const& c) const -> color;
    auto operator+=(color const& c) -> color&;
    auto operator*=(color const& c) -> color&;

    [[nodiscard]] constexpr static auto to_linear(float in) -> float;
    [[nodiscard]] constexpr static auto from_linear(float in) -> float;

  private:
    vec_4f m_data{};
};

auto operator<<(std::ostream& os, color const& c) -> std::ostream&;

inline constexpr auto color::v3b_to_float(vec_3b const& b) -> vec_3f {
    return vec_3f{byte_to_float(b[0]), byte_to_float(b[1]), byte_to_float(b[2])};
}

inline constexpr auto color::v3f_to_byte(vec_3f const& f, bool doClamp) -> vec_3b {
    return vec_3b{float_to_byte(f[0], doClamp), float_to_byte(f[1], doClamp),
                 float_to_byte(f[2], doClamp)};
}

inline constexpr auto color::v4b_to_float(vec_4b const& b) -> vec_4f {
    return vec_4f{byte_to_float(b[0]), byte_to_float(b[1]), byte_to_float(b[2]),
                 byte_to_float(b[3])};
}

inline constexpr auto color::v4f_to_byte(vec_4f const& f, bool doClamp) -> vec_4b {
    return vec_4b{float_to_byte(f[0], doClamp), float_to_byte(f[1], doClamp),
                 float_to_byte(f[2], doClamp), float_to_byte(f[3], doClamp)};
}

inline constexpr color color::Red = color::rgba(255, 73, 66, 255);
inline constexpr color color::Orange = color::rgba(255, 180, 47, 255);
inline constexpr color color::Yellow = color::rgba(255, 239, 30, 255);
inline constexpr color color::Green = color::rgba(79, 230, 70, 255);
inline constexpr color color::Blue = color::rgba(38, 96, 255, 255);
inline constexpr color color::Indigo = color::rgba(75, 0, 130, 255);
inline constexpr color color::Violet = color::rgba(160, 119, 255, 255);
inline constexpr color color::Black = color::rgba(0, 0, 0, 255);
inline constexpr color color::White = color::rgba(255, 255, 255, 255);
inline constexpr color color::Magenta = color::rgba(221, 92, 249, 255);
inline constexpr color color::DarkMagenta = color::rgba(142, 33, 144, 255);
inline constexpr color color::Cyan = color::rgba(0, 220, 233, 255);
inline constexpr color color::DarkCyan = color::rgba(0, 137, 165, 255);
inline constexpr color color::CornFlowerBlue = color::rgba(100, 149, 237, 255);
inline constexpr color color::Gray = color::rgba(160, 160, 160, 255);
inline constexpr color color::LightGray = color::rgba(192, 192, 192, 255);
inline constexpr color color::DarkGray = color::rgba(128, 128, 128, 255);
inline constexpr color color::DarkGreen = color::rgba(0, 128, 0, 255);
inline constexpr color color::Pink = color::rgba(255, 162, 187, 255);
inline constexpr color color::Clear = color::rgba(0, 0, 0, 0);

}// namespace star
