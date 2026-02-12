export module star.json.builder;

import star.json;
import star.json.parser;

import std;

namespace star::json {

class json_builder_stream : public json_stream {
  public:
    void begin_object() override;
    void object_key(std::u32string_view key) override;
    void end_object() override;

    void begin_array() override;
    void end_array() override;

    void put_string(std::u32string_view s) override;
    void put_double(std::double_t value) override;
    void put_integer(std::int64_t value) override;
    void put_boolean(bool value) override;
    void put_null() override;

    void put_whitespace(std::u32string_view s) override;
    void put_colon() override;
    void put_comma() override;

    [[nodiscard]] auto stack_size() const noexcept -> std::size_t;
    [[nodiscard]] auto take_top() -> std::optional<json_value>;

  private:
    void place_value(json_value&& value);

    std::vector<std::optional<json_value>> m_stack;
    std::optional<std::u8string> m_current_key;
};

template <typename Jsonlike> class json_streamer {
  public:
    static void to_stream(const Jsonlike& val, json_stream& stream, bool sort_keys);
};

template <> class json_streamer<json_value> {
  public:
    static void to_stream(const json_value& val, json_stream& stream, bool sort_keys);
};

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char8_t>
[[nodiscard]] auto parse_utf8_json(Iter begin, Iter end, parse_type type = parse_type::top)
  -> std::expected<json_value, parse_error>;

[[nodiscard]] inline auto parse_utf8_json(std::span<const char8_t> s,
                                          parse_type type = parse_type::top)
  -> std::expected<json_value, parse_error> {
    return parse_utf8_json(s.begin(), s.end(), type);
}

template <std::output_iterator<char8_t> Out>
[[nodiscard]] auto serialize_utf8_json(const json_value& val, Out out, unsigned indent = 0,
                                       bool sort_keys = false) -> Out;

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
[[nodiscard]] auto parse_utf32_json(Iter begin, Iter end, parse_type type = parse_type::top)
  -> std::expected<json_value, parse_error>;

[[nodiscard]] inline auto parse_utf32_json(std::span<const char32_t> s,
                                           parse_type type = parse_type::top)
  -> std::expected<json_value, parse_error> {
    return parse_utf32_json(s.begin(), s.end(), type);
}

template <std::output_iterator<char32_t> Out>
[[nodiscard]] auto serialize_utf32_json(const json_value& val, Out out, unsigned indent = 0,
                                        bool sort_keys = false) -> Out;

}// namespace star::json
