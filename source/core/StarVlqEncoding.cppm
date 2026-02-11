export module star.vlq_encoding;

import std;

export namespace star {

// Write an unsigned integer as a VLQ (Variable Length Quantity).  Writes the
// integer in 7 byte chunks, with the 8th bit of each octet indicates whether
// another chunk follows.  Endianness independent, as the chunks are always
// written most significant first. Returns number of octet written (writes a
// maximum of a 64 bit integer, so a maximum of 10)
/// @return The number of bytes written.
[[nodiscard]] constexpr auto vlq_u_size(std::uint64_t x) noexcept -> std::size_t {
    if (x == 0) [[unlikely]] {
        return 1;
    }
    return (static_cast<std::size_t>(std::bit_width(x)) + 6) / 7;
}

template <typename OutputIterator>
constexpr auto write_vlq_u(std::uint64_t x, OutputIterator out) -> std::size_t {
    const auto size = vlq_u_size(x);
    for (std::size_t i = size; i > 1; --i) {
        *out++ = static_cast<std::uint8_t>((x >> ((i - 1) * 7)) & 127) | 128;//NOLINT
    }

    *out++ = static_cast<std::uint8_t>(x & 127);//NOLINT
    return size;
}

// Read a VLQ (Variable Length Quantity) encoded unsigned integer.  Returns
// number of bytes read.  Reads a *maximum of 10 bytes*, cannot read a larger
// than 64 bit integer!  If no end marker is found within 'maxBytes' or 10
// bytes, whichever is smaller, then will return std::numeric_limits<std::size_t>::max() to signal error.
template <typename InputIterator>
constexpr auto read_vlq_u(std::uint64_t& x, InputIterator in, std::size_t max_bytes = 10)
  -> std::size_t {
    x = 0;
    const std::size_t limit = std::min<std::size_t>(10, max_bytes);
    for (std::size_t i = 0; i < limit; ++i) {
        const auto oct = static_cast<std::uint8_t>(*in++);
        x = (x << 7) | (std::uint64_t)(oct & 127);//NOLINT
        if (!(oct & 128)) {                       //NOLINT
            return i + 1;
        }
    }

    return std::numeric_limits<std::size_t>::max();
}

[[nodiscard]] constexpr auto vlq_i_size(std::int64_t v) noexcept -> std::size_t {
    return vlq_u_size((static_cast<std::uint64_t>(v) << 1)   //NOLINT
                      ^ static_cast<std::uint64_t>(v >> 63));//NOLINT
}

// Write a VLQ (Variable Length Quantity) encoded signed integer.  Encoded by
// making the sign bit the least significant bit in the integer.  Returns
// number of bytes written.
template <typename OutputIterator>
constexpr auto write_vlq_i(std::int64_t v, OutputIterator out) -> std::size_t {
    return write_vlq_u((static_cast<std::uint64_t>(v) << 1)    //NOLINT
                         ^ static_cast<std::uint64_t>(v >> 63),//NOLINT
                       out);
}

// Read a VLQ (Variable Length Quantity) encoded signed integer.  Returns
// number of bytes read.  Reads a *maximum of 10 bytes*, cannot read a larger
// than 64 bit integer!  If no end marker is found within 'maxBytes' or 10
// bytes, whichever is smaller, then will return std::numeric_limits<std::size_t>::max() to signal error.
template <typename InputIterator>
constexpr auto read_vlq_i(std::int64_t& v, InputIterator in, std::size_t max_bytes = 10)
  -> std::size_t {
    std::uint64_t source;
    const std::size_t bytes = read_vlq_u(source, in, max_bytes);
    if (bytes == std::numeric_limits<std::size_t>::max()) [[unlikely]] {
        return std::numeric_limits<std::size_t>::max();
    }

    v = static_cast<std::int64_t>((source >> 1) ^ -(source & 1));//NOLINT

    return bytes;
}

}// namespace star
