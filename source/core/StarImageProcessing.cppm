export module star.image.processing;

import std;
import star.json;
import star.vector;
import star.array;
import star.rect;
import star.image;
import star.functional;
import star.color;

export namespace star {

enum class image_operation_errc : std::uint8_t {
    invalid_syntax,
    invalid_operation,
    invalid_parameter,
    missing_parameter,
    resource_exhaustion,
    resource_not_found,
    internal_error
};

auto color_directives_from_config(std::span<const json::json_value> directives)
  -> std::expected<std::vector<std::u8string>, image_operation_errc>;
auto palette_swap_directives_from_config(const json::json_value& swaps) -> std::u8string;

struct null_image_operation {
    bool unloaded = false;
};

struct error_image_operation {
    std::variant<std::string, std::exception_ptr> cause;
};

struct hue_shift_image_operation {
    static constexpr auto hue_shift_degrees(std::float_t degrees) noexcept
      -> hue_shift_image_operation;
    std::float_t hue_shift_amount;
};

struct saturation_shift_image_operation {
    static constexpr auto saturation_shift_100(std::float_t amount) noexcept
      -> saturation_shift_image_operation;
    std::float_t saturation_shift_amount;
};

struct brightness_multiply_image_operation {
    static constexpr auto brightness_multiply_100(std::float_t amount) noexcept
      -> brightness_multiply_image_operation;
    std::float_t brightness_multiply;
};

struct fade_to_color_image_operation {
    [[nodiscard]] auto apply(vec_4b input) const noexcept -> vec_4b {
        auto target_linear = color::rgb(color).to_linear();
        auto input_linear = color::rgb(vec_3b(input[0], input[1], input[2])).to_linear();

        auto mixed = input_linear.mix(target_linear, amount).to_srgb().to_rgb();

        return vec_4b(mixed[0], mixed[1], mixed[2], input[3]);
    }

    vec_3b color;
    std::float_t amount{};
};

// Applies two FadeToColor operations in alternating rows to produce a scanline effect
struct scan_lines_image_operation {
    fade_to_color_image_operation fade_1;
    fade_to_color_image_operation fade_2;
};

// Sets RGB values to the given color, and ignores the alpha channel
struct set_color_image_operation {
    vec_3b color;
};

using color_replace_map = std::flat_map<vec_4b, vec_4b>;

struct color_replace_image_operation {
    color_replace_map color_replace_map;
};

struct alpha_mask_image_operation {
    enum class mask_mode : std::uint8_t { additive, subtractive };

    mask_mode mode;
    std::vector<std::u8string> mask_images;
    vec_2i offset;
};

struct blend_image_operation {
    enum class blend_mode : std::uint8_t { multiply, screen };

    blend_mode mode;
    std::vector<std::u8string> blend_images;
    vec_2i offset;
};

struct multiply_image_operation {
    vec_4b color;
};

struct border_image_operation {
    std::uint32_t pixels{};
    vec_4b start_color;
    vec_4b end_color;
    bool outline_only{};
    bool include_transparent{};
};

struct scale_image_operation {
    enum class mode : std::uint8_t { nearest, bilinear, bicubic };

    mode mode{};
    vec_2f scale;
};

struct crop_image_operation {
    rect_i subset;
};

struct flip_image_operation {
    enum class mode : std::uint8_t { flip_x, flip_y, flip_xy };
    mode mode;
};

using image_operation =
  std::variant<null_image_operation, error_image_operation, hue_shift_image_operation,
               saturation_shift_image_operation, brightness_multiply_image_operation,
               fade_to_color_image_operation, scan_lines_image_operation, set_color_image_operation,
               color_replace_image_operation, alpha_mask_image_operation, blend_image_operation,
               multiply_image_operation, border_image_operation, scale_image_operation,
               crop_image_operation, flip_image_operation>;

[[nodiscard]] auto image_operation_from_string(std::u8string_view s) noexcept
  -> std::expected<image_operation, image_operation_errc>;
[[nodiscard]] auto image_operation_to_string(const image_operation& op) -> std::u8string;

using image_operation_output = star::move_only_function<void(image_operation&&)>;
void parse_image_operations(std::u8string_view params, image_operation_output outputter) noexcept;
[[nodiscard]] auto parse_image_operations(std::string_view params)
  -> std::expected<std::vector<image_operation>, image_operation_errc>;

// Each operation separated by '?', returns string with leading '?'
auto print_image_operations(std::span<const image_operation> operations) -> std::u8string;

void add_image_operation_references(const image_operation& operation,
                                    std::vector<std::u8string>& out);

[[nodiscard]] auto image_operation_references(std::span<const image_operation> operations)
  -> std::vector<std::u8string>;

using image_reference_callback =
  star::move_only_function<std::expected<const star::image*, image_operation_errc>(
    std::u8string_view) const>;

void processImageOperation(const image_operation& operation, star::image& input,
                           image_reference_callback ref_callback = {});

[[nodiscard]] auto processImageOperations(std::span<const image_operation> operations,
                                          star::image input,
                                          image_reference_callback ref_callback = {})
  -> star::image;

}// namespace star
