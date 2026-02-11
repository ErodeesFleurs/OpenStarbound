export module star.encode;

import std;

namespace detail {
inline constexpr std::string_view base_64_chars =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

constexpr auto create_base64_table() {
    std::array<std::uint8_t, 256> table{};
    table.fill(0xFF);
    for (std::size_t i = 0; i < base_64_chars.size(); ++i) {
        table[static_cast<std::uint8_t>(base_64_chars[i])] = static_cast<std::uint8_t>(i);
    }
    return table;
}

inline constexpr auto Base64Table = create_base64_table();

constexpr auto hex_to_nibble(char c) -> std::uint8_t {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return 0xFF;
}
}// namespace detail

export namespace star {

constexpr auto hex_encode(std::span<std::byte const> data, std::span<char> output) -> std::size_t {
    static constexpr char hexChars[] = "0123456789abcdef";//NOLINT
    std::size_t count = std::min(data.size(), output.size() / 2);

    for (std::size_t i = 0; i < count; ++i) {
        auto b = static_cast<std::uint8_t>(data[i]);
        output[i * 2] = hexChars[b >> 4];      // NOLINT(hicpp-signed-bitwise)
        output[i * 2 + 1] = hexChars[b & 0x0F];// NOLINT(hicpp-signed-bitwise)
    }
    return count * 2;
}
constexpr auto hex_decode(std::string_view src, std::span<std::byte> output) -> std::size_t {
    std::size_t count = std::min(src.size() / 2, output.size());
    for (std::size_t i = 0; i < count; ++i) {
        auto n1 = detail::hex_to_nibble(src[i * 2]);
        auto n2 = detail::hex_to_nibble(src[i * 2 + 1]);
        output[i] = static_cast<std::byte>((n1 << 4) | n2);// NOLINT(hicpp-signed-bitwise)
    }
    return count;
}

constexpr auto nibble_decode(std::string_view src, std::span<std::byte> output) -> std::size_t {
    std::size_t count = std::min(src.size(), output.size());
    for (std::size_t i = 0; i < count; ++i) {
        output[i] = static_cast<std::byte>(detail::hex_to_nibble(src[i]));
    }
    return count;
}

// Base64

constexpr auto base64_encode(std::span<std::byte const> data, std::span<char> output)
  -> std::size_t {
    std::size_t outIdx = 0;
    std::uint32_t buffer = 0;
    int bits = 0;

    for (auto b : data) {
        buffer = (buffer << 8) | static_cast<std::uint8_t>(b);// NOLINT(hicpp-signed-bitwise)
        bits += 8;
        while (bits >= 6) {
            if (outIdx >= output.size()) {
                return outIdx;
            }
            bits -= 6;
            output[outIdx++] =
              detail::base_64_chars[(buffer >> bits) & 0x3F];// NOLINT(hicpp-signed-bitwise)
        }
    }

    if (bits > 0) {
        if (outIdx >= output.size()) {
            return outIdx;
        }
        output[outIdx++] =
          detail::base_64_chars[(buffer << (6 - bits)) & 0x3F];// NOLINT(hicpp-signed-bitwise)
    }

    while (outIdx % 4 != 0) {
        if (outIdx >= output.size()) {
            return outIdx;
        }
        output[outIdx++] = '=';
    }

    return outIdx;
}

constexpr auto base64_decode(std::string_view src, std::span<std::byte> output) -> std::size_t {
    std::size_t outIdx = 0;
    std::uint32_t buffer = 0;
    int bits = 0;

    for (char c : src) {
        if (c == '=') {
            break;
        }
        auto val = detail::Base64Table[static_cast<std::uint8_t>(c)];
        if (val == 0xFF) {
            continue;
        }

        buffer = (buffer << 6) | val;// NOLINT(hicpp-signed-bitwise)
        bits += 6;
        if (bits >= 8) {
            if (outIdx >= output.size()) {
                return outIdx;
            }
            bits -= 8;
            output[outIdx++] =
              static_cast<std::byte>((buffer >> bits) & 0xFF);// NOLINT(hicpp-signed-bitwise)
        }
    }
    return outIdx;
}

// api

constexpr auto hex_encode(std::span<const std::byte> data) -> std::string {
    std::string res;
    res.resize_and_overwrite(data.size() * 2, [&](char* ptr, std::size_t n) -> std::size_t {
        return hex_encode(data, {ptr, n});
    });
    return res;
}

constexpr auto base64_encode(std::span<std::byte const> data) -> std::string {
    std::string res;
    std::size_t expected = (data.size() + 2) / 3 * 4;
    res.resize_and_overwrite(expected, [&](char* ptr, std::size_t n) -> std::size_t {
        return base64_encode(data, {ptr, n});
    });
    return res;
}

}// namespace star
