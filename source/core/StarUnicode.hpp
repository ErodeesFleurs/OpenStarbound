#pragma once

#include "StarException.hpp"

import std;

namespace Star {

using UnicodeException = ExceptionDerived<"UnicodeException">;

using Utf8Type = char;
using Utf32Type = char32_t;

#define STAR_UTF32_REPLACEMENT_CHAR 0x000000b7L

void throwInvalidUtf8Sequence();
void throwMissingUtf8End();
void throwInvalidUtf32CodePoint(Utf32Type val);

// If passed std::numeric_limits<std::size_t>::max() as a size, assumes modified UTF-8 and stops on NULL byte.
// Otherwise, ignores NULL.
auto utf8Length(Utf8Type const* utf8, std::size_t size = std::numeric_limits<std::size_t>::max()) -> std::size_t;
// Encode up to six utf8 bytes into a utf32 character.  If passed std::numeric_limits<std::size_t>::max() as len,
// assumes modified UTF-8 and stops on NULL, otherwise ignores.
auto utf8DecodeChar(Utf8Type const* utf8, Utf32Type* utf32, std::size_t len = std::numeric_limits<std::size_t>::max()) -> std::size_t;
// Encode single utf32 char into up to 6 utf8 characters.
auto utf8EncodeChar(Utf8Type* utf8, Utf32Type utf32, std::size_t len = 6) -> std::size_t;

auto hexStringToUtf32(std::string const& codepoint, std::optional<Utf32Type> previousCodepoint = {}) -> Utf32Type;
auto hexStringFromUtf32(Utf32Type character) -> std::string;

auto isUtf16LeadSurrogate(Utf32Type codepoint) -> bool;
auto isUtf16TrailSurrogate(Utf32Type codepoint) -> bool;

auto utf32FromUtf16SurrogatePair(Utf32Type lead, Utf32Type trail) -> Utf32Type;
auto utf32ToUtf16SurrogatePair(Utf32Type codepoint) -> std::pair<Utf32Type, std::optional<Utf32Type>>;

// Bidirectional iterator that can make utf8 appear as utf32
template <class BaseIterator, class U32Type = Utf32Type>
class U8ToU32Iterator {
public:
  using difference_type = std::ptrdiff_t;
  using value_type = U32Type;
  using pointer = U32Type*;
  using reference = U32Type&;
  using iterator_category = std::bidirectional_iterator_tag;

  U8ToU32Iterator() : m_position(), m_value(pending_read) {}

  U8ToU32Iterator(BaseIterator b) : m_position(b), m_value(pending_read) {}

  auto base() const -> BaseIterator const& {
    return m_position;
  }

  auto operator*() const -> U32Type const& {
    if (m_value == pending_read)
      extract_current();
    return m_value;
  }

  auto operator++() -> U8ToU32Iterator const& {
    increment();
    return *this;
  }

  auto operator++(int) -> U8ToU32Iterator {
    U8ToU32Iterator clone(*this);
    increment();
    return clone;
  }

  auto operator--() -> U8ToU32Iterator const& {
    decrement();
    return *this;
  }

  auto operator--(int) -> U8ToU32Iterator {
    U8ToU32Iterator clone(*this);
    decrement();
    return clone;
  }

  auto operator==(U8ToU32Iterator const& that) const -> bool {
    return equal(that);
  }

  auto operator!=(U8ToU32Iterator const& that) const -> bool {
    return !equal(that);
  }

private:
  // special values for pending iterator reads:
  static U32Type const pending_read = 0xffffffffu;

  static void invalid_sequence() {
    throwInvalidUtf8Sequence();
  }

  static auto utf8_byte_count(Utf8Type c) -> unsigned {
    // if the most significant bit with a zero in it is in position
    // 8-N then there are N bytes in this UTF-8 sequence:
    std::uint8_t mask = 0x80u;
    unsigned result = 0;
    while (c & mask) {
      ++result;
      mask >>= 1;
    }
    return (result == 0) ? 1 : ((result > 4) ? 4 : result);
  }

  static auto utf8_trailing_byte_count(Utf8Type c) -> unsigned {
    return utf8_byte_count(c) - 1;
  }

  void increment() {
    // skip high surrogate first if there is one:
    unsigned c = utf8_byte_count(*m_position);
    std::advance(m_position, c);
    m_value = pending_read;
  }

  void decrement() {
    // Keep backtracking until we don't have a trailing character:
    unsigned count = 0;
    while (((std::uint8_t) * --m_position & 0xC0u) == 0x80u)
      ++count;
    // now check that the sequence was valid:
    if (count != utf8_trailing_byte_count(*m_position))
      invalid_sequence();
    m_value = pending_read;
  }

  auto equal(const U8ToU32Iterator& that) const -> bool {
    return m_position == that.m_position;
  }

  void extract_current() const {
    m_value = static_cast<Utf8Type>(*m_position);
    // we must not have a continuation character:
    if (((std::uint8_t)m_value & 0xC0u) == 0x80u)
      invalid_sequence();
    // see how many extra byts we have:
    unsigned extra = utf8_trailing_byte_count(*m_position);
    // extract the extra bits, 6 from each extra byte:
    BaseIterator next(m_position);
    for (unsigned c = 0; c < extra; ++c) {
      ++next;
      m_value <<= 6;
      auto entry = static_cast<std::uint8_t>(*next);
      if ((c > 0) && ((entry & 0xC0u) != 0x80u))
        invalid_sequence();
      m_value += entry & 0x3Fu;
    }
    // we now need to remove a few of the leftmost bits, but how many depends
    // upon how many extra bytes we've extracted:
    static const std::array<Utf32Type,4> masks = {
        0x7Fu, 0x7FFu, 0xFFFFu, 0x1FFFFFu,
    };
    m_value &= masks[extra];
    // check the result:
    if ((std::uint32_t)m_value > (std::uint32_t)0x10FFFFu)
      invalid_sequence();
  }

  BaseIterator m_position;
  mutable U32Type m_value;
};

// Output iterator
template <class BaseIterator, class U32Type = Utf32Type>
class Utf8OutputIterator {
public:
  using difference_type = void;
  using value_type = void;
  using pointer = U32Type*;
  using reference = U32Type&;

  Utf8OutputIterator(const BaseIterator& b) : m_position(b) {}
  Utf8OutputIterator(const Utf8OutputIterator& that) : m_position(that.m_position) {}
  auto operator=(const Utf8OutputIterator& that) -> Utf8OutputIterator& = default;

  auto operator*() const -> const Utf8OutputIterator& {
    return *this;
  }

  void operator=(U32Type val) const {
    push(val);
  }

  auto operator++() -> Utf8OutputIterator& {
    return *this;
  }

  auto operator++(int) -> Utf8OutputIterator& {
    return *this;
  }

private:
  static void invalid_utf32_code_point(U32Type val) {
    throwInvalidUtf32CodePoint(val);
  }

  void push(U32Type c) const {
    if (c > 0x10FFFFu)
      invalid_utf32_code_point(c);

    if ((std::uint32_t)c < 0x80u) {
      *m_position++ = static_cast<Utf8Type>((std::uint32_t)c);
    } else if ((std::uint32_t)c < 0x800u) {
      *m_position++ = static_cast<Utf8Type>(0xC0u + ((std::uint32_t)c >> 6));
      *m_position++ = static_cast<Utf8Type>(0x80u + ((std::uint32_t)c & 0x3Fu));
    } else if ((std::uint32_t)c < 0x10000u) {
      *m_position++ = static_cast<Utf8Type>(0xE0u + ((std::uint32_t)c >> 12));
      *m_position++ = static_cast<Utf8Type>(0x80u + (((std::uint32_t)c >> 6) & 0x3Fu));
      *m_position++ = static_cast<Utf8Type>(0x80u + ((std::uint32_t)c & 0x3Fu));
    } else {
      *m_position++ = static_cast<Utf8Type>(0xF0u + ((std::uint32_t)c >> 18));
      *m_position++ = static_cast<Utf8Type>(0x80u + (((std::uint32_t)c >> 12) & 0x3Fu));
      *m_position++ = static_cast<Utf8Type>(0x80u + (((std::uint32_t)c >> 6) & 0x3Fu));
      *m_position++ = static_cast<Utf8Type>(0x80u + ((std::uint32_t)c & 0x3Fu));
    }
  }

  mutable BaseIterator m_position;
};

}
