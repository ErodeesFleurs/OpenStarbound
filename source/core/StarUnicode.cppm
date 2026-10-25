module;
import star.memory;
import std;
import star.format;

namespace Star {
struct UnicodeExceptionTag { static constexpr std::string_view name() { return "UnicodeException"; } };
using UnicodeException = StarError<UnicodeExceptionTag, StarException>;
using Utf8Type = char;
using Utf32Type = char32_t;
inline constexpr long STAR_UTF32_REPLACEMENT_CHAR = 0x000000b7L;

// NPos denotes a null-terminated sequence; an explicit size preserves embedded NUL.
[[nodiscard]] std::expected<std::size_t, StarException> utf8Length(Utf8Type const*, std::size_t size = NPos);
[[nodiscard]] std::expected<std::size_t, StarException> utf8DecodeChar(Utf8Type const*, Utf32Type*, std::size_t len = NPos);
[[nodiscard]] std::expected<std::size_t, StarException> utf8EncodeChar(Utf8Type*, Utf32Type, std::size_t len = 6);
[[nodiscard]] std::expected<Utf32Type, StarException> hexStringToUtf32(std::string const&, std::optional<Utf32Type> previousCodepoint = {});
[[nodiscard]] std::expected<std::string, StarException> hexStringFromUtf32(Utf32Type);
bool isUtf16LeadSurrogate(Utf32Type);
bool isUtf16TrailSurrogate(Utf32Type);
[[nodiscard]] std::expected<Utf32Type, StarException> utf32FromUtf16SurrogatePair(Utf32Type, Utf32Type);
[[nodiscard]] std::expected<std::pair<Utf32Type, std::optional<Utf32Type>>, StarException> utf32ToUtf16SurrogatePair(Utf32Type);

// Lazy checked cursor for short-circuit algorithms over unvalidated bytes.
class Utf8Cursor {
public:
  explicit Utf8Cursor(std::string_view bytes) : m_bytes(bytes) {}
  static Utf8Cursor end(std::string_view bytes) {
    Utf8Cursor cursor(bytes);
    cursor.m_offset = bytes.size();
    return cursor;
  }
  bool atEnd() const { return m_offset == m_bytes.size(); }
  std::size_t offset() const { return m_offset; }
  [[nodiscard]] std::expected<Utf32Type, StarException> read() const;
  [[nodiscard]] std::expected<void, StarException> advance();
  [[nodiscard]] std::expected<void, StarException> retreat();
private:
  [[nodiscard]] std::expected<void, StarException> decode() const;
  std::string_view m_bytes;
  std::size_t m_offset = 0;
  mutable std::size_t m_width = 0;
  mutable Utf32Type m_point = 0;
};

// Validation happens at creation, not through an error channel hidden in a
// standard iterator operation. Positions must remain within the validated range.
template <class BaseIterator, class U32Type = Utf32Type>
class U8ToU32Iterator {
public:
  using difference_type = std::ptrdiff_t;
  using value_type = U32Type;
  using pointer = U32Type const*;
  using reference = U32Type const&;
  using iterator_category = std::bidirectional_iterator_tag;
  U8ToU32Iterator() = default;
  [[nodiscard]] static std::expected<U8ToU32Iterator, StarException> create(BaseIterator first, BaseIterator last) {
    if constexpr (std::contiguous_iterator<BaseIterator> && std::same_as<std::iter_value_t<BaseIterator>, char>) {
      auto validated = utf8Length(std::to_address(first), static_cast<std::size_t>(last - first));
      if (!validated) return std::unexpected(std::move(validated.error()));
    } else {
      auto position = first;
      while (position != last) {
        char bytes[4];
        std::size_t available = 0;
        auto next = position;
        while (available != 4 && next != last) bytes[available++] = *next++;
        Utf32Type point;
        auto decoded = utf8DecodeChar(bytes, &point, available);
        if (!decoded) return std::unexpected(std::move(decoded.error()));
        std::advance(position, *decoded);
      }
    }
    return U8ToU32Iterator(first);
  }
  static U8ToU32Iterator end(BaseIterator last) { return U8ToU32Iterator(last); }
  BaseIterator const& base() const { return m_position; }
  reference operator*() const {
    if (m_pending) {
      auto next = m_position;
      auto first = static_cast<unsigned char>(*next++);
      unsigned count = byteCount(first);
      std::uint32_t point = first & (count == 1 ? 0x7fu : (0x7fu >> count));
      for (unsigned i = 1; i != count; ++i) point = (point << 6) | (static_cast<unsigned char>(*next++) & 0x3fu);
      m_value = static_cast<U32Type>(point);
      m_pending = false;
    }
    return m_value;
  }
  U8ToU32Iterator& operator++() { std::advance(m_position, byteCount(static_cast<unsigned char>(*m_position))); m_pending = true; return *this; }
  U8ToU32Iterator operator++(int) { auto old = *this; ++*this; return old; }
  U8ToU32Iterator& operator--() { do { --m_position; } while ((static_cast<unsigned char>(*m_position) & 0xc0u) == 0x80u); m_pending = true; return *this; }
  U8ToU32Iterator operator--(int) { auto old = *this; --*this; return old; }
  bool operator==(U8ToU32Iterator const& other) const { return m_position == other.m_position; }
private:
  explicit U8ToU32Iterator(BaseIterator position) : m_position(position) {}
  static unsigned byteCount(unsigned char c) { return c < 0x80 ? 1 : c < 0xe0 ? 2 : c < 0xf0 ? 3 : 4; }
  BaseIterator m_position{};
  mutable U32Type m_value{};
  mutable bool m_pending = true;
};

// An explicit checked writer, rather than a standard output iterator whose
// assignment result would be discarded by algorithms.
template <class BaseIterator, class U32Type = Utf32Type>
class Utf8OutputIterator {
public:
  explicit Utf8OutputIterator(BaseIterator position) : m_position(position) {}
  [[nodiscard]] std::expected<void, StarException> push(U32Type point) {
    char bytes[4];
    auto encoded = utf8EncodeChar(bytes, static_cast<Utf32Type>(point), sizeof(bytes));
    if (!encoded) return std::unexpected(std::move(encoded.error()));
    for (std::size_t i = 0; i != *encoded; ++i) *m_position++ = bytes[i];
    return {};
  }
private:
  BaseIterator m_position;
};
}
export module star.unicode;
export namespace Star {
  using ::Star::UnicodeExceptionTag;
  using ::Star::UnicodeException;
  using ::Star::Utf8Type;
  using ::Star::Utf32Type;
  using ::Star::STAR_UTF32_REPLACEMENT_CHAR;
  using ::Star::utf8Length;
  using ::Star::utf8DecodeChar;
  using ::Star::utf8EncodeChar;
  using ::Star::hexStringToUtf32;
  using ::Star::hexStringFromUtf32;
  using ::Star::isUtf16LeadSurrogate;
  using ::Star::isUtf16TrailSurrogate;
  using ::Star::utf32FromUtf16SurrogatePair;
  using ::Star::utf32ToUtf16SurrogatePair;
  using ::Star::U8ToU32Iterator;
  using ::Star::Utf8OutputIterator;
  using ::Star::Utf8Cursor;
}
