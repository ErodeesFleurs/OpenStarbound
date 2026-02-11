module star.color;

import std;
import star.encode;

namespace star {

std::flat_map<std::string, color> const color::named_colors{{"red", color::Red},
                                                           {"orange", color::Orange},
                                                           {"yellow", color::Yellow},
                                                           {"green", color::Green},
                                                           {"blue", color::Blue},
                                                           {"indigo", color::Indigo},
                                                           {"violet", color::Violet},
                                                           {"black", color::Black},
                                                           {"white", color::White},
                                                           {"magenta", color::Magenta},
                                                           {"darkmagenta", color::DarkMagenta},
                                                           {"cyan", color::Cyan},
                                                           {"darkcyan", color::DarkCyan},
                                                           {"cornflowerblue", color::CornFlowerBlue},
                                                           {"gray", color::Gray},
                                                           {"lightgray", color::LightGray},
                                                           {"darkgray", color::DarkGray},
                                                           {"darkgreen", color::DarkGreen},
                                                           {"pink", color::Pink},
                                                           {"clear", color::Clear}};

constexpr auto color::rgbf(const vec_3f& c) -> color { return rgbaf(c[0], c[1], c[2], 1.0F); }

constexpr auto color::rgbaf(const vec_4f& c) -> color { return rgbaf(c[0], c[1], c[2], c[3]); }

constexpr auto color::temperature(float temp) -> color {
	temp = std::clamp(temp, 1000.0F, 40000.0F) / 100.0F;
	double r, g, b;
	if (temp <= 66) {
		r = 255;
		g = std::clamp(99.4708025861 * std::log(temp) - 161.1195681661, 0.0, 255.0);
		b = (temp <= 19) ? 0 : std::clamp(138.5177312231 * std::log(temp - 10) - 305.0447927307, 0.0, 255.0);
	} else {
		r = std::clamp(329.698727446 * std::pow(temp - 60, -0.1332047592), 0.0, 255.0);
		g = std::clamp(288.1221695283 * std::pow(temp - 60, -0.0755148492), 0.0, 255.0);
		b = 255;
	}
	return rgbaf(static_cast<float>(r) / 255.0F, static_cast<float>(g) / 255.0F, static_cast<float>(b) / 255.0F, 1.0F);
}

constexpr auto color::rgb(vec_3b const& c) -> color { return rgb(c[0], c[1], c[2]); }

constexpr auto color::rgba(vec_4b const& c) -> color { return rgba(c[0], c[1], c[2], c[3]); }

constexpr auto color::hsv(float h, float s, float v) -> color { return hsva(h, s, v, 1.0F); }

constexpr auto color::hsva(float h, float s, float v, float a) -> color {
	h = std::clamp(h, 0.0F, 1.0F);
	s = std::clamp(s, 0.0F, 1.0F);
	v = std::clamp(v, 0.0F, 1.0F);

	if (s == 0.0F) {
		return rgbaf(v, v, v, a);
	}

	float var_h = (h == 1.0F ? 0.0F : h * 6.0F);
	int var_i = static_cast<int>(std::floor(var_h));
	float v1 = v * (1.0F - s);
	float v2 = v * (1.0F - s * (var_h - static_cast<float>(var_i)));
	float v3 = v * (1.0F - (1.0F - (var_h - static_cast<float>(var_i))) * s);

	switch (var_i) {
	case 0: return rgbaf(v, v3, v1, a);
	case 1: return rgbaf(v2, v, v1, a);
	case 2: return rgbaf(v1, v, v3, a);
	case 3: return rgbaf(v1, v2, v, a);
	case 4: return rgbaf(v3, v1, v, a);
	default: return rgbaf(v, v1, v2, a);
	}
}

constexpr auto color::hsv(vec_3f const& c) -> color { return color::hsv(c[0], c[1], c[2]); }

constexpr auto color::hsva(vec_4f const& c) -> color { return color::hsva(c[0], c[1], c[2], c[3]); }

constexpr auto color::grayf(float g) -> color { return color::rgbf(g, g, g); }

constexpr auto color::gray(std::uint8_t g) -> color { return color::rgb(g, g, g); }

color::color(std::string_view name) {
	if (name.starts_with('#')) {
		*this = from_hex(name.substr(1));
	} else {
		auto i = named_colors.find(std::string(name));
		if (i != named_colors.end()) {
			*this = i->second;
		} else {
			throw color_exception(std::format("Named color {} not found", name), false);
		}
	}
}

constexpr auto color::from_hex(std::string_view s) -> color { return color::rgba(hex_to_vec_4b(s)); }

constexpr auto color::to_rgb() const -> vec_3b { return vec_3b{red(), green(), blue()}; }

constexpr auto color::to_rgbf() const -> vec_3f { return vec_3f{redf(), greenf(), bluef()}; }

constexpr auto color::toHsva() const -> vec_4f {
	float h, s, v;

	float var_r = redf();
	float var_g = greenf();
	float var_b = bluef();

	// Min. value of RGB
	float var_min = std::min({var_r, var_g, var_b});

	// Max. value of RGB
	float var_max = std::max({var_r, var_g, var_b});

	// Delta RGB value
	float del_max = var_max - var_min;

	v = var_max;

	if (del_max == 0.0F) {// This is a gray, no chroma...
		h = 0.0F;
		s = 0.0F;
	} else {// Chromatic data
		s = del_max / var_max;

		float del_r = (((var_max - var_r) / 6.0F) + (del_max / 2.0F)) / del_max;
		float del_g = (((var_max - var_g) / 6.0F) + (del_max / 2.0F)) / del_max;
		float del_b = (((var_max - var_b) / 6.0F) + (del_max / 2.0F)) / del_max;

		if (var_r == var_max) {
			h = del_b - del_g;
		} else if (var_g == var_max) {
			h = (1.0F / 3.0F) + del_r - del_b;
		} else {
			h = (2.0F / 3.0F) + del_g - del_r;
		}

		if (h < 0.0F) {
			h += 1.0F;
		}
		if (h >= 1.0F) {
			h -= 1.0F;
		}
	}

	return vec_4f{h, s, v, alphaf()};
}

constexpr auto color::hue() const -> float { return toHsva()[0]; }

constexpr auto color::saturation() const -> float {
	// Min. value of RGB
	float var_min = std::min({m_data[0], m_data[1], m_data[2]});

	// Max. value of RGB
	float var_max = std::max({m_data[0], m_data[1], m_data[2]});

	// Delta RGB value
	float del_max = var_max - var_min;

	if (del_max == 0.0F) {// This is a gray, no chroma...
		return 0.0F;
	} else {
		return del_max / var_max;
	}
}

constexpr auto color::value() const -> float { return std::max({m_data[0], m_data[1], m_data[2]}); }

constexpr void color::set_hue(float h) {
	auto hsva = toHsva();
	*this = color::hsva(std::clamp(h, 0.0F, 1.0F), hsva[1], hsva[2], alphaf());
}

constexpr void color::set_saturation(float s) {
	auto hsva = toHsva();
	*this = color::hsva(hsva[0], std::clamp(s, 0.0F, 1.0F), hsva[2], alphaf());
}

constexpr void color::set_value(float v) {
	auto hsva = toHsva();
	*this = color::hsva(hsva[0], hsva[1], std::clamp(v, 0.0F, 1.0F), alphaf());
}

constexpr void color::hue_shift(float h) { set_hue(pfmod(hue() + h, 1.0F)); }

constexpr void color::fade(float value) {
	m_data *= (1.0F - value);
	m_data.clamp(0.0F, 1.0F);
}

auto operator<<(std::ostream& os, const color& c) -> std::ostream& {
	os << c.to_rgbaf();
	return os;
}

constexpr auto color::to_linear(float in) -> float {
	const float a = 0.055F;
	if (in <= 0.04045F) {
		return in / 12.92F;
	}
	return std::powf((in + a) / (1.0F + a), 2.4F);
}

constexpr auto color::from_linear(float in) -> float {
	const float a = 0.055F;
	if (in <= 0.0031308F) {
		return 12.92F * in;
	}
	return (1.0F + a) * std::powf(in, 1.0F / 2.4F) - a;
}

constexpr auto color::to_hex() const -> std::string {
	auto rgba = to_rgba();
	return hex_encode(std::as_bytes(std::span(rgba.ptr(), rgba[3] == 255 ? 3 : 4)));
}

constexpr void color::convert_to_linear() {
	set_redf(to_linear(redf()));
	set_greenf(to_linear(greenf()));
	set_bluef(to_linear(bluef()));
}

constexpr void color::convert_to_srgb() {
	set_redf(from_linear(redf()));
	set_greenf(from_linear(greenf()));
	set_bluef(from_linear(bluef()));
}

constexpr auto color::to_linear() -> color {
	color c = *this;
	c.convert_to_linear();
	return c;
}

constexpr auto color::to_srgb() -> color {
	color c = *this;
	c.convert_to_srgb();
	return c;
}

constexpr auto color::contrasting() -> color {
	color c = *this;
	c.set_hue(c.hue() + 120);
	return c;
}

constexpr auto color::complementary() -> color {
	color c = *this;
	c.set_hue(c.hue() + 180);
	return c;
}

constexpr auto color::multiply(float amount) const -> color { return color::rgbaf(m_data * amount); }

auto color::operator+(color const& c) const -> color { return color::rgbaf(m_data + c.to_rgbaf()); }

auto color::operator*(color const& c) const -> color { return color::rgbaf(m_data.piecewise_multiply(c.to_rgbaf())); }

auto color::operator+=(color const& c) -> color& { return *this = *this + c; }

auto color::operator*=(color const& c) -> color& { return *this = *this * c; }

auto color::hue_shift_vec_4b(vec_4b color, float hue) -> vec_4b {
	float h, s, v;

	float var_r = static_cast<std::float_t>(color[0]) / 255.0F;
	float var_g = static_cast<std::float_t>(color[1]) / 255.0F;
	float var_b = static_cast<std::float_t>(color[2]) / 255.0F;

	// Min. value of RGB
	float var_min = std::min({var_r, var_g, var_b});

	// Max. value of RGB
	float var_max = std::max({var_r, var_g, var_b});

	// Delta RGB value
	float del_max = var_max - var_min;

	v = var_max;

	if (del_max == 0.0F) {// This is a gray, no chroma...
		h = 0.0F;
		s = 0.0F;
	} else {// Chromatic data
		s = del_max / var_max;

		float vd = 1.0F / 6.0F;
		float dmh = del_max * 0.5F;
		float dmi = 1.0F / del_max;
		float del_r = (((var_max - var_r) * vd) + dmh) * dmi;
		float del_g = (((var_max - var_g) * vd) + dmh) * dmi;
		float del_b = (((var_max - var_b) * vd) + dmh) * dmi;

		if (var_r == var_max) {
			h = del_b - del_g;
		} else if (var_g == var_max) {
			h = (1.0F / 3.0F) + del_r - del_b;
		} else {
			h = (2.0F / 3.0F) + del_g - del_r;
		}

		if (h < 0.0F) {
			h += 1.0F;
		}
		if (h >= 1.0F) {
			h -= 1.0F;
		}
	}

	h += hue;

	if (h >= 1.0F) {
		h -= 1.0F;
	}

	if (s == 0.0F) {
		auto c = std::uint8_t(std::round(v * 255));
		return vec_4b{c, c, c, color[3]};
	} else {
		float var_h, var_i, var_1, var_2, var_3, var_r, var_g, var_b;

		var_h = h * 6.0F;
		if (var_h == 6.0F) {
			var_h = 0.0F;// H must be < 1
		}

		var_i = std::floor(var_h);

		var_1 = v * (1.0F - s);
		var_2 = v * (1.0F - s * (var_h - var_i));
		var_3 = v * (1.0F - s * (1.0F - (var_h - var_i)));

		if (var_i == 0) {
			var_r = v;
			var_g = var_3;
			var_b = var_1;
		} else if (var_i == 1) {
			var_r = var_2;
			var_g = v;
			var_b = var_1;
		} else if (var_i == 2) {
			var_r = var_1;
			var_g = v;
			var_b = var_3;
		} else if (var_i == 3) {
			var_r = var_1;
			var_g = var_2;
			var_b = v;
		} else if (var_i == 4) {
			var_r = var_3;
			var_g = var_1;
			var_b = v;
		} else {
			var_r = v;
			var_g = var_1;
			var_b = var_2;
		}

		return vec_4b{std::uint8_t(std::round(var_r * 255)), std::uint8_t(std::round(var_g * 255)),
		             std::uint8_t(std::round(var_b * 255)), color[3]};
	}
}

auto color::hex_to_vec_4b(std::string_view s) -> vec_4b {
	std::array<std::uint8_t, 4> cbytes{};
	auto out = std::as_writable_bytes(std::span(cbytes));

	if (s.size() == 3) {
		nibble_decode(s, out.subspan(0, 3));
		for (int i : {0, 1, 2}) {
			cbytes[i] = (cbytes[i] << 4) | cbytes[i];// NOLINT(hicpp-signed-bitwise)
		}
		cbytes[3] = 255;
	} else if (s.size() == 6) {
		hex_decode(s, out.subspan(0, 3));
		cbytes[3] = 255;
	} else if (s.size() == 8) {
		hex_decode(s, out);
	} else {
		throw color_exception(std::format("Invalid hex color size: {}", s.size()));
	}
	return vec_4b(cbytes[0], cbytes[1], cbytes[2], cbytes[3]);
}

}// namespace Star
