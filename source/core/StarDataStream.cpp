module star.data_stream;

import star.vlq_encoding;
import star.bytes;
import std;

namespace star {

data_stream::data_stream()
    : m_byte_order(byte_order::BigEndian), m_null_terminated_strings(false),
      m_stream_compatibility_version(current_stream_version) {}

auto data_stream::get_byte_order() const -> byte_order { return m_byte_order; }

void data_stream::set_byte_order(byte_order byte_order_value) { m_byte_order = byte_order_value; }

auto data_stream::get_null_terminated_strings() const -> bool { return m_null_terminated_strings; }

void data_stream::set_null_terminated_strings(bool null_terminated_strings) {
    m_null_terminated_strings = null_terminated_strings;
}

auto data_stream::get_stream_compatibility_version() const -> unsigned {
    return m_stream_compatibility_version;
}

void data_stream::set_stream_compatibility_version(unsigned stream_compatibility_version) {
    m_stream_compatibility_version = stream_compatibility_version;
}

void data_stream::set_stream_compatibility_version(net_compatibility_rules const& rules) {
    m_stream_compatibility_version = rules.version();
}

auto data_stream::read_bytes(std::size_t len) -> byte_array {
    byte_array ba;
    ba.resize(len);
    read_data({reinterpret_cast<std::byte*>(ba.data()), len});
    return ba;
}

void data_stream::write_bytes(byte_array const& ba) {
    write_data({reinterpret_cast<std::byte const*>(ba.data()), ba.size()});
}

auto data_stream::operator<<(bool d) -> data_stream& {
    operator<<(static_cast<std::uint8_t>(d));
    return *this;
}

auto data_stream::operator<<(char c) -> data_stream& {
    write_data(std::as_bytes(std::span{&c, 1}));
    return *this;
}

auto data_stream::operator<<(std::int8_t d) -> data_stream& {
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator<<(std::uint8_t d) -> data_stream& {
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator<<(std::int16_t d) -> data_stream& {
    d = to_byte_order(m_byte_order, d);
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator<<(std::uint16_t d) -> data_stream& {
    d = to_byte_order(m_byte_order, d);
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator<<(std::int32_t d) -> data_stream& {
    d = to_byte_order(m_byte_order, d);
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator<<(std::uint32_t d) -> data_stream& {
    d = to_byte_order(m_byte_order, d);
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator<<(std::int64_t d) -> data_stream& {
    d = to_byte_order(m_byte_order, d);
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator<<(std::uint64_t d) -> data_stream& {
    d = to_byte_order(m_byte_order, d);
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator<<(float d) -> data_stream& {
    d = to_byte_order(m_byte_order, d);
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator<<(double d) -> data_stream& {
    d = to_byte_order(m_byte_order, d);
    write_data(std::as_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator>>(bool& d) -> data_stream& {
    std::uint8_t bu;
    read_data(std::as_writable_bytes(std::span{&bu, 1}));
    d = static_cast<bool>(bu);
    return *this;
}

auto data_stream::operator>>(char& c) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&c, 1}));
    return *this;
}

auto data_stream::operator>>(std::int8_t& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator>>(std::uint8_t& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    return *this;
}

auto data_stream::operator>>(std::int16_t& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    d = from_byte_order(m_byte_order, d);
    return *this;
}

auto data_stream::operator>>(std::uint16_t& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    d = from_byte_order(m_byte_order, d);
    return *this;
}

auto data_stream::operator>>(std::int32_t& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    d = from_byte_order(m_byte_order, d);
    return *this;
}

auto data_stream::operator>>(std::uint32_t& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    d = from_byte_order(m_byte_order, d);
    return *this;
}

auto data_stream::operator>>(std::int64_t& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    d = from_byte_order(m_byte_order, d);
    return *this;
}

auto data_stream::operator>>(std::uint64_t& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    d = from_byte_order(m_byte_order, d);
    return *this;
}

auto data_stream::operator>>(float& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    d = from_byte_order(m_byte_order, d);
    return *this;
}

auto data_stream::operator>>(double& d) -> data_stream& {
    read_data(std::as_writable_bytes(std::span{&d, 1}));
    d = from_byte_order(m_byte_order, d);
    return *this;
}

auto data_stream::write_vlq_u(std::uint64_t i) -> std::size_t {
    struct out_it {
        data_stream* ds;
        auto operator=(std::uint8_t b) -> auto& {
            *ds << b;
            return *this;
        }
        auto operator*() -> auto& { return *this; }
        auto operator++() -> auto& { return *this; }
        auto operator++(int) { return *this; }
    };
    return star::write_vlq_u(i, out_it{this});
}

auto data_stream::write_vlq_i(std::int64_t i) -> std::size_t {
    struct out_it {
        data_stream* ds;
        auto operator=(std::uint8_t b) -> auto& {
            *ds << b;
            return *this;
        }
        auto operator*() -> auto& { return *this; }
        auto operator++() -> auto& { return *this; }
        auto operator++(int) { return *this; }
    };
    return star::write_vlq_i(i, out_it{this});
}

auto data_stream::write_vlq_s(std::size_t i) -> std::size_t {
    std::uint64_t i64 =
      (i == std::numeric_limits<std::size_t>::max()) ? 0 : static_cast<std::uint64_t>(i + 1);
    return write_vlq_u(i64);
}

auto data_stream::read_vlq_u(std::uint64_t& i) -> std::size_t {
    struct in_it {
        data_stream* ds;
        auto operator*() const { return ds->read<std::uint8_t>(); }
        auto operator++() -> auto& { return *this; }
        auto operator++(int) { return *this; }
    };

    std::size_t bytes_read = star::read_vlq_u(i, in_it{this});

    if (bytes_read == std::numeric_limits<std::size_t>::max()) {
        throw data_stream_exception("Error reading VLQ encoded integer!");
    }

    return bytes_read;
}

auto data_stream::read_vlq_i(std::int64_t& i) -> std::size_t {
    struct in_it {
        data_stream* ds;
        auto operator*() const { return ds->read<std::uint8_t>(); }
        auto operator++() -> auto& { return *this; }
        auto operator++(int) { return *this; }
    };

    std::size_t bytes_read = star::read_vlq_i(i, in_it{this});

    if (bytes_read == std::numeric_limits<std::size_t>::max()) {
        throw data_stream_exception("Error reading VLQ encoded integer!");
    }

    return bytes_read;
}

auto data_stream::read_vlq_s(std::size_t& i) -> std::size_t {
    std::uint64_t i64;
    std::size_t res = read_vlq_u(i64);
    if (i64 == 0) {
        i = std::numeric_limits<std::size_t>::max();
    } else {
        i = static_cast<std::size_t>(i64 - 1);
    }
    return res;
}

auto data_stream::read_vlq_u() -> std::uint64_t {
    std::uint64_t i;
    read_vlq_u(i);
    return i;
}

auto data_stream::read_vlq_i() -> std::int64_t {
    std::int64_t i;
    read_vlq_i(i);
    return i;
}

auto data_stream::read_vlq_s() -> std::size_t {
    std::size_t i;
    read_vlq_s(i);
    return i;
}

auto data_stream::operator<<(char const* s) -> data_stream& {
    write_string_data(s, std::strlen(s));
    return *this;
}

auto data_stream::operator<<(std::string const& d) -> data_stream& {
    write_string_data(d.c_str(), d.size());
    return *this;
}

auto data_stream::operator<<(byte_array const& d) -> data_stream& {
    write_vlq_u(d.size());
    write_data(std::as_bytes(d.span()));
    return *this;
}

auto data_stream::operator>>(std::string& d) -> data_stream& {
    if (m_null_terminated_strings) {
        d.clear();
        char c;
        while (true) {
            read_data(std::as_writable_bytes(std::span{&c, 1}));
            if (c == '\0') {
                break;
            }
            d.push_back(c);
        }
    } else {
        d.resize(static_cast<std::size_t>(read_vlq_u()));
        if (!d.empty()) {
            read_data(std::as_writable_bytes(std::span{d.data(), d.size()}));
        }
    }
    return *this;
}

auto data_stream::operator>>(byte_array& d) -> data_stream& {
    d.resize(static_cast<std::size_t>(read_vlq_u()));
    if (!d.empty()) {
        read_data(std::as_writable_bytes(std::span{d.data(), d.size()}));
    }
    return *this;
}

void data_stream::write_string_data(char const* data, std::size_t len) {
    if (m_null_terminated_strings) {
        write_data(std::as_bytes(std::span{data, len}));
        operator<<(static_cast<std::uint8_t>(0x00));
    } else {
        write_vlq_u(len);
        write_data(std::as_bytes(std::span{data, len}));
    }
}

}// namespace star
