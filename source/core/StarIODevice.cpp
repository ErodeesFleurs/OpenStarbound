module star.io_device;

import std;

namespace star {

void io_device::resize(std::int64_t /*unused*/) { throw io_exception("resize not supported"); }

void io_device::read_full(std::span<std::byte> data) {
	while (!data.empty()) {
		std::size_t r = read(data);
		if (r == 0) {
			if (at_end()) {
				throw eof_exception("Failed to read full buffer, eof reached.");
			} else {
				throw io_exception("Failed to read full buffer, device returned zero-length read.");
			}
		}
		data = data.subspan(r);
	}
}

void io_device::write_full(std::span<std::byte const> data) {
	while (!data.empty()) {
		std::size_t r = write(data);
		if (r == 0) {
			throw io_exception("Failed to write full buffer, device returned zero-length write.");
		}
		data = data.subspan(r);
	}
}

void io_device::open(io_mode mode) {
	if (mode != m_mode.load()) {
		throw io_exception::format("Cannot reopen device '{}' with different mode", device_name());
	}
}

void io_device::close() { m_mode.store(io_mode::closed); }

void io_device::sync() {}

auto io_device::device_name() const -> std::string {
	return std::format("io_device <{}>", static_cast<void const*>(this));
}

auto io_device::at_end() -> bool { return pos() >= size(); }

auto io_device::size() -> std::int64_t {
	try {
		std::int64_t stored_pos = pos();
		seek(0, io_seek::end);
		std::int64_t size = pos();
		seek(stored_pos);
		return size;
	} catch (const io_exception& e) { throw io_exception("Cannot call size() on io_device", e); }
}

auto io_device::read_absolute(std::int64_t read_position, std::span<std::byte> data) -> std::size_t {
	std::int64_t stored_pos = pos();
	seek(read_position);
	std::size_t ret = read(data);
	seek(stored_pos);
	return ret;
}

auto io_device::write_absolute(std::int64_t write_position, std::span<const std::byte> data) -> std::size_t {
	std::int64_t stored_pos = pos();
	seek(write_position);
	std::size_t ret = write(data);
	seek(stored_pos);
	return ret;
}

void io_device::read_full_absolute(std::int64_t read_position, std::span<std::byte> data) {
	while (!data.empty()) {
		std::size_t r = read_absolute(read_position, data);
		if (r == 0) {
			throw io_exception("Failed to read full buffer in read_full_absolute");
		}
		read_position += static_cast<std::uint32_t>(r);
		data = data.subspan(r);
	}
}

void io_device::write_full_absolute(std::int64_t write_position, std::span<const std::byte> data) {
	while (!data.empty()) {
		std::size_t r = write_absolute(write_position, data);
		if (r == 0) {
			throw io_exception("Failed to write full buffer in write_full_absolute");
		}
		write_position += static_cast<std::uint32_t>(r);
		data = data.subspan(r);
	}
}

auto io_device::read_bytes(std::size_t size) -> byte_array {
	if (!size) {
		return {};
	}
	byte_array p;
	p.resize(size);
	read_full({reinterpret_cast<std::byte*>(p.data()), size});
	return p;
}

void io_device::write_bytes(byte_array const& p) { write_full({reinterpret_cast<std::byte const*>(p.data()), p.size()}); }

auto io_device::read_bytes_absolute(std::int64_t read_position, std::size_t size) -> byte_array {
	if (!size) {
		return {};
	}
	byte_array p;
	p.resize(size);
	read_full_absolute(read_position, {reinterpret_cast<std::byte*>(p.data()), size});
	return p;
}

void io_device::write_bytes_absolute(std::int64_t write_position, byte_array const& p) {
	write_full_absolute(write_position, {reinterpret_cast<std::byte const*>(p.data()), p.size()});
}

void io_device::set_mode(io_mode m) noexcept { m_mode.store(m); }

io_device::io_device(const io_device& rhs) : m_mode(rhs.mode()) {}

auto io_device::operator=(const io_device& rhs) -> io_device& {
	m_mode.store(rhs.mode());
	return *this;
}

}// namespace star
