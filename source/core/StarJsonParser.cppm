export module star.json.parser;

import std;
import star.unicode;

namespace detail {
auto need_escape(char32_t c) noexcept -> bool { return c == U'"' || c == U'\\' || c < 0x20; }
}// namespace detail

export namespace star::json {

enum class json_parser_errc : std::uint8_t {
    success = 0,
    expected_object_or_array,
    unexpected_end,
    invalid_character,
    bad_object,
    bad_array,
    bad_number,
    bad_string,
    bad_escape,
    bad_word,
    invalid_comment,
};

}

template <> struct std::is_error_code_enum<star::json::json_parser_errc> : true_type {};

export namespace star::json {
inline auto make_error_code(json_parser_errc e) noexcept -> std::error_code {
    static struct : std::error_category {
        [[nodiscard]] auto name() const noexcept -> const char* override { return "json"; }
        [[nodiscard]] auto message(int ev) const -> std::string override {
            using enum json_parser_errc;
            switch (static_cast<json_parser_errc>(ev)) {
            case success: return "success";
            case expected_object_or_array: return "expected JSON object or array at top level";
            case unexpected_end: return "unexpected end of stream";
            case invalid_character: return "invalid character";
            case bad_object: return "malformed object";
            case bad_array: return "malformed array";
            case bad_number: return "malformed number";
            case bad_string: return "malformed string";
            case bad_escape: return "invalid escape sequence";
            case bad_word: return "invalid literal (true/false/null)";
            case invalid_comment: return "invalid comment syntax";
            default: return "unknown JSON error";
            }
        }
    } category;
    return {static_cast<int>(e), category};
}

struct parse_error {
    std::error_code ec;
    std::size_t line;
    std::size_t column;
    std::u32string_view fragment;
};

enum class parse_type : std::uint8_t { top, value, sequence };

class json_stream {
  public:
    json_stream(const json_stream&) = default;
    json_stream(json_stream&&) = delete;
    auto operator=(const json_stream&) -> json_stream& = default;
    auto operator=(json_stream&&) -> json_stream& = delete;
    virtual ~json_stream() = default;

    virtual void begin_object() = 0;
    virtual void object_key(std::u32string_view key) = 0;
    virtual void end_object() = 0;

    virtual void begin_array() = 0;
    virtual void end_array() = 0;

    virtual void put_string(std::u32string_view s) = 0;
    virtual void put_double(double value) = 0;
    virtual void put_integer(std::int64_t value) = 0;
    virtual void put_boolean(bool value) = 0;
    virtual void put_null() = 0;

    virtual void put_whitespace(std::u32string_view ws) = 0;
    virtual void put_colon() = 0;
    virtual void put_comma() = 0;
};

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
class parser {
  public:
    explicit parser(json_stream& stream) : m_stream(stream) {}

    [[nodiscard]]
    auto parse(Iter begin, Iter end, parse_type type = parse_type::top)
      -> std::expected<Iter, parse_error>;

  private:
    class impl {
      public:
        impl(json_stream& stream, Iter begin, Iter end)
            : m_stream(stream), m_current(begin), m_end(end) {
            advance();
        }

      private:
        [[nodiscard]] auto top() -> std::expected<void, parse_error>;
        [[nodiscard]] auto value() -> std::expected<void, parse_error>;
        [[nodiscard]] auto object() -> std::expected<void, parse_error>;
        [[nodiscard]] auto array() -> std::expected<void, parse_error>;
        [[nodiscard]] auto sequence() -> std::expected<void, parse_error>;
        [[nodiscard]] auto string() -> std::expected<std::u32string, parse_error>;
        [[nodiscard]] auto number() -> std::expected<void, parse_error>;
        [[nodiscard]] auto word() -> std::expected<void, parse_error>;
        [[nodiscard]] auto white() -> std::expected<void, parse_error>;

        void advance() noexcept;
        [[nodiscard]] auto at_end() const noexcept -> bool { return m_current == m_end; }
        [[nodiscard]] auto is_space(char32_t c) const noexcept -> bool;
        [[nodiscard]] auto fail(json_parser_errc ec, std::string_view context = {})
          -> std::unexpected<parse_error>;

        json_stream& m_stream;
        Iter m_current;
        Iter m_end;
        char32_t m_char = 0;
        std::size_t m_line = 0;
        std::size_t m_column = 0;
        std::optional<parse_error> m_err;
    };

    json_stream& m_stream;
};

template <std::output_iterator<char32_t> Out> class writer : public json_stream {
  public:
    explicit writer(Out out, unsigned indent = 0) noexcept
        : m_out(std::move(out)), m_indent(indent) {}

    void begin_object() override;
    void object_key(std::u32string_view key) override;
    void end_object() override;
    void begin_array() override;
    void end_array() override;
    void put_string(std::u32string_view s) override;
    void put_double(double value) override;
    void put_integer(std::int64_t value) override;
    void put_boolean(bool value) override;
    void put_null() override;
    void put_whitespace(std::u32string_view ws) override;
    void put_colon() override;
    void put_comma() override;

  private:
    enum class state : std::uint8_t { top, object, object_element, array, array_element };

    void push_state(state s);
    void pop_state(state expected);
    [[nodiscard]] auto current_state() const noexcept -> state;

    void start_value();
    void indent();
    void write(char32_t c);
    void write(std::u32string_view sv);
    void write_escaped_string(std::u32string_view s);

    Out m_out;
    std::uint32_t m_indent;
    std::vector<state> m_state;
};

template <std::output_iterator<char32_t> Out>
inline auto make_json_writer(Out out, unsigned indent = 0) -> writer<Out> {
    return writer<Out>(std::move(out), indent);
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
void parser<Iter>::impl::advance() noexcept {
    if (at_end()) {
        m_char = 0;
        return;
    }
    if (m_char == U'\n') {
        ++m_line;
        m_column = 0;
    } else {
        ++m_column;
    }
    ++m_current;
    m_char = at_end() ? 0 : *m_current;
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::fail(json_parser_errc ec, [[maybe_unused]] std::string_view context)
  -> std::unexpected<parse_error> {
    constexpr std::size_t CONTEXT_LEN = 10;
    std::u32string_view frag;
    if (!at_end()) {
        auto start = m_current;
        auto end = m_current + std::min(CONTEXT_LEN, std::distance(m_current, m_end));
        frag = std::u32string_view(&*start, end - start);
    }
    return std::unexpected(parse_error{.ec = make_error_code(ec),
                                       .line = m_line + 1,
                                       .column = m_column + 1,
                                       .fragment = frag});
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::white() -> std::expected<void, parse_error> {
    std::u32string ws_buf;
    while (!at_end()) {
        if (m_char == U'/') {
            ws_buf += m_char;
            advance();
            if (at_end()) {
                return fail(json_parser_errc::unexpected_end);
            }
            if (m_char == U'/') {
                ws_buf += m_char;
                advance();
                while (!at_end() && m_char != U'\n') {
                    ws_buf += m_char;
                    advance();
                }
            } else if (m_char == U'*') {
                ws_buf += m_char;
                advance();
                while (!at_end()) {
                    ws_buf += m_char;
                    advance();
                    if (!at_end() && m_char == U'/') {
                        ws_buf += m_char;
                        advance();
                        break;
                    }
                    if (at_end()) {
                        return fail(json_parser_errc::invalid_comment);
                    }
                }
            } else {
                return fail(json_parser_errc::invalid_comment);
            }
        } else if (is_space(m_char)) {
            ws_buf += m_char;
            advance();
        } else {
            break;
        }
    }
    if (!ws_buf.empty()) {
        m_stream.put_whitespace(ws_buf);
    }
    return {};
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::top() -> std::expected<void, parse_error> {
    if (auto res = white(); !res) {
        return res;
    }
    switch (m_char) {
    case U'{': return object();
    case U'[': return array();
    default: return fail(json_parser_errc::expected_object_or_array);
    }
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::value() -> std::expected<void, parse_error> {
    if (auto res = white(); !res) {
        return res;
    }
    switch (m_char) {
    case U'{': return object();
    case U'[': return array();
    case U'"':
        return string().and_then(
          [this](const std::u32string& s) -> std::expected<void, parse_error> {
              m_stream.put_string(s);
              return {};
          });
    case U'-':
    case U'0':
    case U'1':
    case U'2':
    case U'3':
    case U'4':
    case U'5':
    case U'6':
    case U'7':
    case U'8':
    case U'9': return number();
    case U't':
    case U'f':
    case U'n': return word();
    case 0: return fail(json_parser_errc::unexpected_end);
    default: return fail(json_parser_errc::invalid_character);
    }
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::object() -> std::expected<void, parse_error> {
    if (m_char != U'{') {
        return fail(json_parser_errc::bad_object);
    }
    advance();
    m_stream.begin_object();

    if (auto res = white(); !res) {
        return res;
    }
    if (m_char == U'}') {
        advance();
        m_stream.end_object();
        return {};
    }

    while (true) {
        auto key_res = string();
        if (!key_res) {
            return std::unexpected(key_res.error());
        }
        m_stream.object_key(*key_res);

        if (auto res = white(); !res) {
            return res;
        }
        if (m_char != U':') {
            return fail(json_parser_errc::bad_object);
        }
        advance();
        m_stream.put_colon();

        if (auto res = white(); !res) {
            return res;
        }
        if (auto val_res = value(); !val_res) {
            return val_res;
        }

        if (auto res = white(); !res) {
            return res;
        }
        if (m_char == U'}') {
            advance();
            m_stream.end_object();
            return {};
        } else if (m_char == U',') {
            advance();
            m_stream.put_comma();
            if (auto res = white(); !res) {
                return res;
            }
        } else if (m_char == 0) {
            return fail(json_parser_errc::unexpected_end);
        } else {
            return fail(json_parser_errc::bad_object);
        }
    }
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::array() -> std::expected<void, parse_error> {
    if (m_char != U'[') {
        return fail(json_parser_errc::bad_array);
    }
    advance();
    m_stream.begin_array();

    if (auto res = white(); !res) {
        return res;
    }
    if (m_char == U']') {
        advance();
        m_stream.end_array();
        return {};
    }

    while (true) {
        if (auto val_res = value(); !val_res) {
            return val_res;
        }

        if (auto res = white(); !res) {
            return res;
        }
        if (m_char == U']') {
            advance();
            m_stream.end_array();
            return {};
        } else if (m_char == U',') {
            advance();
            m_stream.put_comma();
            if (auto res = white(); !res) {
                return res;
            }
        } else if (m_char == 0) {
            return fail(json_parser_errc::unexpected_end);
        } else {
            return fail(json_parser_errc::bad_array);
        }
    }
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::sequence() -> std::expected<void, parse_error> {
    m_stream.begin_array();
    while (!at_end()) {
        if (auto res = white(); !res) {
            return res;
        }
        if (m_char == 0) {
            break;
        }
        auto try_parse = [&]() -> std::expected<void, parse_error> {
            if (m_char == U'{') {
                return object();
            }
            if (m_char == U'[') {
                return array();
            }
            if (m_char == U'"') {
                auto s = string();
                if (!s) {
                    return std::unexpected(s.error());
                }
                m_stream.put_string(*s);
                return {};
            }
            if (m_char == U'-' || (m_char >= U'0' && m_char <= U'9')) {
                return number();
            }
            if (m_char == U't' || m_char == U'f' || m_char == U'n') {
                return word();
            }
            return fail(json_parser_errc::invalid_character);
        };

        if (auto res = try_parse(); res) {
            if (auto ws = white(); !ws) {
                return ws;
            }
            continue;
        } else {
            std::u32string fallback;
            while (!at_end() && !is_space(m_char)) {
                fallback += m_char;
                advance();
            }
            m_stream.put_string(fallback);
        }
    }
    m_stream.end_array();
    return {};
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::string() -> std::expected<std::u32string, parse_error> {
    if (m_char != U'"') {
        return fail(json_parser_errc::bad_string);
    }
    advance();

    std::u32string result;
    while (true) {
        if (at_end()) {
            return fail(json_parser_errc::unexpected_end);
        }
        if (m_char == U'"') {
            advance();
            return result;
        }
        if (m_char == U'\\') {
            advance();
            if (at_end()) {
                return fail(json_parser_errc::unexpected_end);
            }
            if (m_char == U'u') {
                advance();
                std::string hex;
                for (int i = 0; i < 4; ++i) {
                    if (at_end()) {
                        return fail(json_parser_errc::unexpected_end);
                    }
                    hex.push_back(static_cast<char>(m_char));
                    advance();
                }
                auto cp = unicode::hex_to_utf32(hex);
                if (!cp) {
                    return fail(json_parser_errc::bad_escape);
                }
                if (unicode::is_utf16_lead_surrogate(*cp)) {
                    if (at_end() || m_char != U'\\') {
                        return fail(json_parser_errc::bad_escape);
                    }
                    advance();
                    if (at_end() || m_char != U'u') {
                        return fail(json_parser_errc::bad_escape);
                    }
                    advance();
                    std::string low_hex;
                    for (int i = 0; i < 4; ++i) {
                        if (at_end()) {
                            return fail(json_parser_errc::unexpected_end);
                        }
                        low_hex.push_back(static_cast<char>(m_char));
                        advance();
                    }
                    auto low_cp = unicode::hex_to_utf32(low_hex);
                    if (!low_cp) {
                        return fail(json_parser_errc::bad_escape);
                    }
                    auto pair_cp = unicode::utf16_surrogate_pair_to_utf32(*cp, *low_cp);
                    if (!pair_cp) {
                        return fail(json_parser_errc::bad_escape);
                    }
                    result += *pair_cp;
                } else {
                    result += *cp;
                }
            } else {
                char32_t escaped;
                switch (m_char) {
                case U'"': escaped = U'"'; break;
                case U'\\': escaped = U'\\'; break;
                case U'/': escaped = U'/'; break;
                case U'b': escaped = U'\b'; break;
                case U'f': escaped = U'\f'; break;
                case U'n': escaped = U'\n'; break;
                case U'r': escaped = U'\r'; break;
                case U't': escaped = U'\t'; break;
                default: return fail(json_parser_errc::bad_escape);
                }
                result += escaped;
                advance();
            }
        } else {
            result += m_char;
            advance();
        }
    }
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::number() -> std::expected<void, parse_error> {
    auto start = m_current;
    bool is_double = false;

    if (m_char == U'-') {
        advance();
    }
    if (m_char == U'0') {
        advance();
    } else if (m_char >= U'1' && m_char <= U'9') {
        while (m_char >= U'0' && m_char <= U'9') {
            advance();
        }
    } else {
        return fail(json_parser_errc::bad_number);
    }

    if (m_char == U'.') {
        is_double = true;
        advance();
        if (m_char < U'0' || m_char > U'9') {
            return fail(json_parser_errc::bad_number);
        }
        while (m_char >= U'0' && m_char <= U'9') {
            advance();
        }
    }

    if (m_char == U'e' || m_char == U'E') {
        is_double = true;
        advance();
        if (m_char == U'-' || m_char == U'+') {
            advance();
        }
        if (m_char < U'0' || m_char > U'9') {
            return fail(json_parser_errc::bad_number);
        }
        while (m_char >= U'0' && m_char <= U'9') {
            advance();
        }
    }

    std::size_t len = std::distance(start, m_current);
    std::u32string_view num_str(&*start, len);
    if (is_double) {
        double val;
        auto [ptr, ec] = std::from_chars(
          reinterpret_cast<const char*>(num_str.data()),
          reinterpret_cast<const char*>(num_str.data()) + len * sizeof(char32_t), val);
        if (ec != std::errc{}) {
            return fail(json_parser_errc::bad_number);
        }
        m_stream.put_double(val);
    } else {
        std::int64_t val;
        auto [ptr, ec] = std::from_chars(
          reinterpret_cast<const char*>(num_str.data()),
          reinterpret_cast<const char*>(num_str.data()) + len * sizeof(char32_t), val);
        if (ec != std::errc{}) {
            return fail(json_parser_errc::bad_number);
        }
        m_stream.put_integer(val);
    }
    return {};
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::word() -> std::expected<void, parse_error> {
    if (m_char == U't') {
        advance();
        if (m_char != U'r') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        if (m_char != U'u') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        if (m_char != U'e') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        m_stream.put_boolean(true);
    } else if (m_char == U'f') {
        advance();
        if (m_char != U'a') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        if (m_char != U'l') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        if (m_char != U's') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        if (m_char != U'e') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        m_stream.put_boolean(false);
    } else if (m_char == U'n') {
        advance();
        if (m_char != U'u') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        if (m_char != U'l') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        if (m_char != U'l') {
            return fail(json_parser_errc::bad_word);
        }
        advance();
        m_stream.put_null();
    } else {
        return fail(json_parser_errc::bad_word);
    }
    return {};
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::impl::is_space(char32_t c) const noexcept -> bool {
    return c == U' ' || c == U'\t' || c == U'\n' || c == U'\r' || c == U'\ufeff';
}

template <std::input_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, char32_t>
auto parser<Iter>::parse(Iter begin, Iter end, parse_type type)
  -> std::expected<Iter, parse_error> {
    impl p(m_stream, begin, end);
    std::expected<void, parse_error> res;
    switch (type) {
    case parse_type::top: res = p.top(); break;
    case parse_type::value: res = p.value(); break;
    case parse_type::sequence: res = p.sequence(); break;
    }
    if (res) {
        return p.m_current;
    } else {
        return std::unexpected(res.error());
    }
}

template <std::output_iterator<char32_t> Out> void writer<Out>::begin_object() {
    start_value();
    write(U'{');
    push_state(state::object);
}

template <std::output_iterator<char32_t> Out>
void writer<Out>::object_key(std::u32string_view key) {
    if (current_state() == state::object_element) {
        if (m_indent) {
            write(U'\n');
        }
        indent();
    } else {
        push_state(state::object_element);
        if (m_indent) {
            write(U'\n');
        }
        indent();
    }
    write_escaped_string(key);
    if (m_indent) {
        write(U' ');
    }
}

template <std::output_iterator<char32_t> Out> void writer<Out>::end_object() {
    bool is_non_empty = (current_state() == state::object_element);
    pop_state(state::object);
    if (is_non_empty) {
        if (m_indent) {
            write(U'\n');
        }
        indent();
    }
    write(U'}');
}

template <std::output_iterator<char32_t> Out> void writer<Out>::begin_array() {
    start_value();
    push_state(state::array);
    write(U'[');
}

template <std::output_iterator<char32_t> Out> void writer<Out>::end_array() {
    pop_state(state::array);
    write(U']');
}

template <std::output_iterator<char32_t> Out> void writer<Out>::put_string(std::u32string_view s) {
    start_value();
    write_escaped_string(s);
}

template <std::output_iterator<char32_t> Out> void writer<Out>::put_double(double value) {
    start_value();
    std::array<char, 32> buf{};
    auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
    for (char* p = buf.data(); p != ptr; ++p) {
        write(static_cast<char32_t>(*p));
    }
}

template <std::output_iterator<char32_t> Out> void writer<Out>::put_integer(std::int64_t value) {
    start_value();
    std::array<char, 21> buf{};
    auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
    for (char* p = buf.data(); p != ptr; ++p) {
        write(static_cast<char32_t>(*p));
    }
}

template <std::output_iterator<char32_t> Out> void writer<Out>::put_boolean(bool b) {
    start_value();
    if (b) {
        write(U't');
        write(U'r');
        write(U'u');
        write(U'e');
    } else {
        write(U'f');
        write(U'a');
        write(U'l');
        write(U's');
        write(U'e');
    }
}

template <std::output_iterator<char32_t> Out> void writer<Out>::put_null() {
    start_value();
    write(U'n');
    write(U'u');
    write(U'l');
    write(U'l');
}

template <std::output_iterator<char32_t> Out>
void writer<Out>::put_whitespace(std::u32string_view ws) {
    // 若缩进 > 0，忽略额外的空白（避免重复缩进）
    if (m_indent == 0) {
        for (char32_t c : ws) {
            write(c);
        }
    }
}

template <std::output_iterator<char32_t> Out> void writer<Out>::put_colon() {
    write(U':');
    if (m_indent) {
        write(U' ');
    }
}

template <std::output_iterator<char32_t> Out> void writer<Out>::put_comma() { write(U','); }

template <std::output_iterator<char32_t> Out> void writer<Out>::push_state(state s) {
    m_state.push_back(s);
}

template <std::output_iterator<char32_t> Out> void writer<Out>::pop_state(state expected) {
    while (!m_state.empty()) {
        state last = m_state.back();
        m_state.pop_back();
        if (last == expected) {
            return;
        }
    }
}

template <std::output_iterator<char32_t> Out>
auto writer<Out>::current_state() const noexcept -> state {
    if (m_state.empty()) {
        return state::top;
    }
    return m_state.back();
}

template <std::output_iterator<char32_t> Out> void writer<Out>::start_value() {
    if (current_state() == state::array_element) {
        if (m_indent) {
            write(U' ');
        }
    } else if (current_state() == state::array) {
        push_state(state::array_element);
    }
}

template <std::output_iterator<char32_t> Out> void writer<Out>::indent() {
    auto level = static_cast<std::uint32_t>(m_state.size() / 2);
    for (unsigned i = 0; i < level; ++i) {
        for (unsigned j = 0; j < m_indent; ++j) {
            write(U' ');
        }
    }
}

template <std::output_iterator<char32_t> Out> void writer<Out>::write(char32_t c) { *m_out++ = c; }

template <std::output_iterator<char32_t> Out> void writer<Out>::write(std::u32string_view sv) {
    for (char32_t c : sv) {
        write(c);
    }
}

template <std::output_iterator<char32_t> Out>
void writer<Out>::write_escaped_string(std::u32string_view s) {
    write(U'"');
    for (char32_t c : s) {
        if (detail::need_escape(c)) {
            switch (c) {
            case U'"':
                write(U'\\');
                write(U'"');
                break;
            case U'\\':
                write(U'\\');
                write(U'\\');
                break;
            case U'\b':
                write(U'\\');
                write(U'b');
                break;
            case U'\f':
                write(U'\\');
                write(U'f');
                break;
            case U'\n':
                write(U'\\');
                write(U'n');
                break;
            case U'\r':
                write(U'\\');
                write(U'r');
                break;
            case U'\t':
                write(U'\\');
                write(U't');
                break;
            default:
                auto hex = unicode::utf32_to_hex(c);
                if (hex.size() == 4) {
                    write(U'\\');
                    write(U'u');
                    for (char ch : hex) {
                        write(static_cast<char32_t>(ch));
                    }
                } else if (hex.size() == 8) {
                    write(U'\\');
                    write(U'u');
                    for (char ch : hex.substr(0, 4)) {
                        write(static_cast<char32_t>(ch));
                    }
                    write(U'\\');
                    write(U'u');
                    for (char ch : hex.substr(4, 4)) {
                        write(static_cast<char32_t>(ch));
                    }
                }
                break;
            }
        } else {
            write(c);
        }
    }
    write(U'"');
}

}// namespace star::json
