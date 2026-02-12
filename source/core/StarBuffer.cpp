module star.buffer;

import std;
import star.exception;

namespace star {

// ===== buffer implementation =====

buffer::buffer() : m_pos(0) { set_mode(io_mode::read_write); }

buffer::buffer(std::size_t initial_size) : buffer() { reset(initial_size); }

buffer::buffer(byte_array b) : buffer() { reset(std::move(b)); }

buffer::buffer(buffer const& other) : buffer() { operator=(other); }

buffer::buffer(buffer&& other) noexcept : buffer() { operator=(std::move(other)); }

auto buffer::pos() -> std::int64_t { return static_cast<std::int64_t>(m_pos); }

void buffer::seek(std::int64_t pos, io_seek mode) {
    auto new_pos = static_cast<std::int64_t>(m_pos);

    switch (mode) {
    case io_seek::absolute: new_pos = pos; break;
    case io_seek::relative: new_pos += pos; break;
    case io_seek::end: new_pos = static_cast<std::int64_t>(m_bytes.size()) - pos; break;
    }

    m_pos = static_cast<std::size_t>(std::max(std::int64_t{0}, new_pos));
}

void buffer::resize(std::int64_t size) { data().resize(static_cast<std::size_t>(size)); }

auto buffer::at_end() -> bool { return m_pos >= m_bytes.size(); }

auto buffer::read(std::span<std::byte> data) -> std::size_t {
    std::size_t bytes_read = do_read(m_pos, data);
    m_pos += bytes_read;
    return bytes_read;
}

auto buffer::write(std::span<std::byte const> data) -> std::size_t {
    std::size_t bytes_written = do_write(m_pos, data);
    m_pos += bytes_written;
    return bytes_written;
}

auto buffer::read_absolute(std::int64_t read_position, std::span<std::byte> data) -> std::size_t {
    auto rpos = static_cast<std::size_t>(read_position);
    if (static_cast<std::int64_t>(rpos) != read_position) {
        throw io_exception("Error, read_position out of range");
    }

    return do_read(rpos, data);
}

auto buffer::write_absolute(std::int64_t write_position, std::span<std::byte const> data)
  -> std::size_t {
    auto wpos = static_cast<std::size_t>(write_position);
    if (static_cast<std::int64_t>(wpos) != write_position) {
        throw io_exception("Error, write_position out of range");
    }

    return do_write(wpos, data);
}

void buffer::open(io_mode mode) {
    set_mode(mode);

    if ((mode & io_mode::write) && (mode & io_mode::truncate)) {
        resize(0);
    }

    if (mode & io_mode::append) {
        seek(0, io_seek::end);
    }
}

auto buffer::device_name() const -> std::string {
    return std::format("buffer <{}>", static_cast<void const*>(this));
}

auto buffer::size() -> std::int64_t { return static_cast<std::int64_t>(m_bytes.size()); }

auto buffer::data() -> byte_array& { return m_bytes; }

auto buffer::data() const -> byte_array const& { return m_bytes; }

auto buffer::take_data() -> byte_array {
    byte_array ret = std::move(m_bytes);
    reset(0);
    return ret;
}

auto buffer::ptr() -> char* { return data().data(); }

auto buffer::ptr() const -> char const* { return m_bytes.data(); }

auto buffer::data_size() const noexcept -> std::size_t { return m_bytes.size(); }

void buffer::reserve(std::size_t size) { data().reserve(size); }

void buffer::clear() noexcept {
    m_pos = 0;
    m_bytes.clear();
}

auto buffer::empty() const noexcept -> bool { return m_bytes.empty(); }

void buffer::reset(std::size_t new_size) {
    m_pos = 0;
    m_bytes.resize(new_size, std::byte{0});
}

void buffer::reset(byte_array b) {
    m_pos = 0;
    m_bytes = std::move(b);
}

auto buffer::operator=(buffer const& other) -> buffer& = default;

auto buffer::operator=(buffer&& other) noexcept -> buffer& {
    io_device::operator=(other);
    m_pos = other.m_pos;
    m_bytes = std::move(other.m_bytes);

    other.m_pos = 0;
    other.m_bytes = byte_array();

    return *this;
}

auto buffer::clone() -> std::shared_ptr<io_device> {
    auto cloned = std::make_shared<buffer>(*this);
    // Reset position to 0 while preserving mode and data
    cloned->seek(0);
    return cloned;
}

auto buffer::do_read(std::size_t pos, std::span<std::byte> data) -> std::size_t {
    if (data.empty()) {
        return 0;
    }

    if (!is_readable()) {
        throw io_exception("Error, read called on non-readable buffer");
    }

    if (pos >= m_bytes.size()) {
        return 0;
    }

    std::size_t bytes_to_read = std::min(m_bytes.size() - pos, data.size());
    std::memcpy(data.data(), m_bytes.data() + pos, bytes_to_read);
    return bytes_to_read;
}

auto buffer::do_write(std::size_t pos, std::span<std::byte const> data) -> std::size_t {
    if (data.empty()) {
        return 0;
    }

    if (!is_writable()) {
        throw eof_exception("Error, write called on non-writable buffer");
    }

    if (pos + data.size() > m_bytes.size()) {
        m_bytes.resize(pos + data.size());
    }

    std::memcpy(m_bytes.data() + pos, data.data(), data.size());
    return data.size();
}

// ===== external_buffer implementation =====

external_buffer::external_buffer() : m_pos(0), m_bytes(nullptr), m_size(0) {
    set_mode(io_mode::read);
}

external_buffer::external_buffer(char const* external_data, std::size_t len) : external_buffer() {
    reset(external_data, len);
}

external_buffer::external_buffer(external_buffer&& other) noexcept
    : io_device(io_mode::read), m_pos(other.m_pos), m_bytes(other.m_bytes), m_size(other.m_size) {
    other.m_pos = 0;
    other.m_bytes = nullptr;
    other.m_size = 0;
}

auto external_buffer::operator=(external_buffer&& other) noexcept -> external_buffer& {
    if (this != &other) {
        io_device::operator=(other);
        m_pos = other.m_pos;
        m_bytes = other.m_bytes;
        m_size = other.m_size;

        other.m_pos = 0;
        other.m_bytes = nullptr;
        other.m_size = 0;
    }
    return *this;
}

auto external_buffer::pos() -> std::int64_t { return static_cast<std::int64_t>(m_pos); }

void external_buffer::seek(std::int64_t pos, io_seek mode) {
    auto new_pos = static_cast<std::int64_t>(m_pos);

    switch (mode) {
    case io_seek::absolute: new_pos = pos; break;
    case io_seek::relative: new_pos += pos; break;
    case io_seek::end: new_pos = static_cast<std::int64_t>(m_size) - pos; break;
    }

    m_pos = static_cast<std::size_t>(std::max(std::int64_t{0}, new_pos));
}

auto external_buffer::at_end() -> bool { return m_pos >= m_size; }

auto external_buffer::read(std::span<std::byte> data) -> std::size_t {
    std::size_t bytes_read = do_read(m_pos, data);
    m_pos += bytes_read;
    return bytes_read;
}

auto external_buffer::write(std::span<std::byte const> /*data*/) -> std::size_t {
    throw io_exception("Error, external_buffer is not writable");
}

auto external_buffer::read_absolute(std::int64_t read_position, std::span<std::byte> data)
  -> std::size_t {
    auto rpos = static_cast<std::size_t>(read_position);
    if (static_cast<std::int64_t>(rpos) != read_position) {
        throw io_exception("Error, read_position out of range");
    }

    return do_read(rpos, data);
}

auto external_buffer::write_absolute(std::int64_t /*write_position*/, std::span<std::byte const> /*data*/) -> std::size_t {
    throw io_exception("Error, external_buffer is not writable");
}

auto external_buffer::device_name() const -> std::string {
    return std::format("external_buffer <{}>", static_cast<void const*>(this));
}

auto external_buffer::size() -> std::int64_t { return static_cast<std::int64_t>(m_size); }

auto external_buffer::ptr() const noexcept -> char const* { return m_bytes; }

auto external_buffer::data_size() const noexcept -> std::size_t { return m_size; }

auto external_buffer::empty() const noexcept -> bool { return m_size == 0; }

external_buffer::operator bool() const noexcept { return m_size != 0; }

void external_buffer::reset(char const* external_data, std::size_t len) noexcept {
    m_pos = 0;
    m_bytes = external_data;
    m_size = len;
}

auto external_buffer::clone() -> std::shared_ptr<io_device> {
    return std::make_shared<external_buffer>(*this);
}

auto external_buffer::do_read(std::size_t pos, std::span<std::byte> data) -> std::size_t {
    if (data.empty()) {
        return 0;
    }

    if (!is_readable()) {
        throw io_exception("Error, read called on non-readable external_buffer");
    }

    if (pos >= m_size) {
        return 0;
    }

    std::size_t bytes_to_read = std::min(m_size - pos, data.size());
    std::memcpy(data.data(), m_bytes + pos, bytes_to_read);
    return bytes_to_read;
}

}// namespace star
