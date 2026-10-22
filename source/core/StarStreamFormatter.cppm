module;
import std;

namespace Star {
namespace Detail {

// Stream-backed formatters need contiguous text for string presentation specs.
// Keep ordinary diagnostics on the stack; spill only when the text outgrows it.
class FormattingStreamBuffer : public std::streambuf {
public:
  FormattingStreamBuffer() {
    setp(m_inline.data(), m_inline.data() + m_inline.size());
  }

  std::string_view view() const {
    if (!m_dynamic.empty())
      return m_dynamic;
    return {pbase(), static_cast<std::size_t>(pptr() - pbase())};
  }

protected:
  int_type overflow(int_type ch) override {
    if (traits_type::eq_int_type(ch, traits_type::eof()))
      return traits_type::not_eof(ch);
    if (m_dynamic.empty())
      spill(1);
    m_dynamic.push_back(traits_type::to_char_type(ch));
    return ch;
  }

  std::streamsize xsputn(char const* text, std::streamsize count) override {
    if (count <= 0)
      return 0;
    auto size = static_cast<std::size_t>(count);
    if (m_dynamic.empty()) {
      if (size <= static_cast<std::size_t>(epptr() - pptr())) {
        traits_type::copy(pptr(), text, size);
        pbump(static_cast<int>(size));
        return count;
      }
      spill(size);
    }
    m_dynamic.append(text, size);
    return count;
  }

private:
  void spill(std::size_t extra) {
    auto size = static_cast<std::size_t>(pptr() - pbase());
    m_dynamic.reserve(std::max(m_inline.size() * 2, size + extra));
    m_dynamic.assign(pbase(), size);
    setp(nullptr, nullptr);
  }

  std::array<char, 512> m_inline;
  std::string m_dynamic;
};

}

// Adapt the project's existing ostream representations to std::format without
// replacing them with the standard range/debug presentation.
struct StreamFormatter : std::formatter<std::string_view> {
  void set_debug_format() = delete;

  template <typename T, typename Context>
    requires requires(std::ostream& output, T const& value) {
      { output << value } -> std::same_as<std::ostream&>;
    }
  auto format(T const& value, Context& context) const -> typename Context::iterator {
    Detail::FormattingStreamBuffer buffer;
    std::ostream output(&buffer);
    output.imbue(std::locale::classic());
    output << value;
    return std::formatter<std::string_view>::format(buffer.view(), context);
  }
};

}

export module star.stream_formatter;

export namespace Star {
using ::Star::StreamFormatter;
}
