export module star.bytes;

import std;

export namespace star {

enum class byte_order : std::uint8_t { BigEndian, LittleEndian, NoConversion };

auto platform_byte_order() -> byte_order;

void swap_byte_order(void* ptr, std::size_t len);
void swap_byte_order(void* dest, void const* src, std::size_t len);

void to_byte_order(byte_order order, void* ptr, std::size_t len);
void to_byte_order(byte_order order, void* dest, void const* src, std::size_t len);
void from_byte_order(byte_order order, void* ptr, std::size_t len);
void from_byte_order(byte_order order, void* dest, void const* src, std::size_t len);

template <typename T> auto to_byte_order(byte_order order, T const& t) -> T {
    T ret;
    to_byte_order(order, &ret, &t, sizeof(t));
    return ret;
}

template <typename T> auto from_byte_order(byte_order order, T const& t) -> T {
    T ret;
    from_byte_order(order, &ret, &t, sizeof(t));
    return ret;
}

template <typename T> auto to_big_endian(T const& t) -> T {
    return to_byte_order(byte_order::BigEndian, t);
}

template <typename T> auto from_big_endian(T const& t) -> T {
    return from_byte_order(byte_order::BigEndian, t);
}

template <typename T> auto to_little_endian(T const& t) -> T {
    return to_byte_order(byte_order::LittleEndian, t);
}

template <typename T> auto from_little_endian(T const& t) -> T {
    return from_byte_order(byte_order::LittleEndian, t);
}

inline auto platform_byte_order() -> byte_order {
#if STAR_LITTLE_ENDIAN
    return byte_order::LittleEndian;
#else
    return byte_order::BigEndian;
#endif
}

inline void swap_byte_order(void* ptr, std::size_t len) {
    auto* data = static_cast<std::uint8_t*>(ptr);
    std::uint8_t spare;
    for (std::size_t i = 0; i < len / 2; ++i) {
        spare = data[len - 1 - i];
        data[len - 1 - i] = data[i];
        data[i] = spare;
    }
}

inline void swap_byte_order(void* dest, const void* src, std::size_t len) {
    const auto* srcdata = reinterpret_cast<const std::uint8_t*>(src);
    auto* destdata = reinterpret_cast<std::uint8_t*>(dest);
    for (std::size_t i = 0; i < len; ++i) {
        destdata[len - 1 - i] = srcdata[i];
    }
}

inline void to_byte_order(byte_order order, void* ptr, std::size_t len) {
    if (order != byte_order::NoConversion && platform_byte_order() != order) {
        swap_byte_order(ptr, len);
    }
}

inline void to_byte_order(byte_order order, void* dest, void const* src, std::size_t len) {
    if (order != byte_order::NoConversion && platform_byte_order() != order) {
        swap_byte_order(dest, src, len);
    } else {
        std::memcpy(dest, src, len);
    }
}

inline void from_byte_order(byte_order order, void* ptr, std::size_t len) {
    if (order != byte_order::NoConversion && platform_byte_order() != order) {
        swap_byte_order(ptr, len);
    }
}

inline void from_byte_order(byte_order order, void* dest, void const* src, std::size_t len) {
    if (order != byte_order::NoConversion && platform_byte_order() != order) {
        swap_byte_order(dest, src, len);
    } else {
        std::memcpy(dest, src, len);
    }
}

}// namespace star
