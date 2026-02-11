
import std;

namespace Star {


void IODevice::resize(std::int64_t /*unused*/) { throw IOException("resize not supported"); }

void IODevice::readFull(std::span<std::byte> data) {
	while (!data.empty()) {
		std::size_t r = read(data);
		if (r == 0) {
			if (atEnd()) {
				throw EofException("Failed to read full buffer, eof reached.");
			} else {
				throw IOException("Failed to read full buffer, device returned zero-length read.");
			}
		}
		data = data.subspan(r);
	}
}

void IODevice::writeFull(std::span<std::byte const> data) {
	while (!data.empty()) {
		std::size_t r = write(data);
		if (r == 0) {
			throw IOException("Failed to write full buffer, device returned zero-length write.");
		}
		data = data.subspan(r);
	}
}

void IODevice::open(IOMode mode) {
	if (mode != m_mode.load()) {
		throw IOException::format("Cannot reopen device '{}' with different mode", deviceName());
	}
}

void IODevice::close() { m_mode.store(IOMode::Closed); }

void IODevice::sync() {}

auto IODevice::deviceName() const -> std::string {
	return std::format("IODevice <{}>", static_cast<void const*>(this));
}

auto IODevice::atEnd() -> bool { return pos() >= size(); }

auto IODevice::size() -> std::int64_t {
	try {
		std::int64_t storedPos = pos();
		seek(0, IOSeek::End);
		std::int64_t size = pos();
		seek(storedPos);
		return size;
	} catch (IOException const& e) { throw IOException("Cannot call size() on IODevice", e); }
}

auto IODevice::readAbsolute(std::int64_t readPosition, std::span<std::byte> data) -> std::size_t {
	std::int64_t storedPos = pos();
	seek(readPosition);
	std::size_t ret = read(data);
	seek(storedPos);
	return ret;
}

auto IODevice::writeAbsolute(std::int64_t writePosition, std::span<const std::byte> data) -> std::size_t {
	std::int64_t storedPos = pos();
	seek(writePosition);
	std::size_t ret = write(data);
	seek(storedPos);
	return ret;
}

void IODevice::readFullAbsolute(std::int64_t readPosition, std::span<std::byte> data) {
	while (!data.empty()) {
		std::size_t r = readAbsolute(readPosition, data);
		if (r == 0) {
			throw IOException("Failed to read full buffer in readFullAbsolute");
		}
		readPosition += static_cast<std::uint32_t>(r);
		data = data.subspan(r);
	}
}

void IODevice::writeFullAbsolute(std::int64_t writePosition, std::span<const std::byte> data) {
	while (!data.empty()) {
		std::size_t r = writeAbsolute(writePosition, data);
		if (r == 0) {
			throw IOException("Failed to write full buffer in writeFullAbsolute");
		}
		writePosition += static_cast<std::uint32_t>(r);
		data = data.subspan(r);
	}
}

auto IODevice::readBytes(std::size_t size) -> ByteArray {
	if (!size) {
		return {};
	}
	ByteArray p;
	p.resize(size);
	readFull({reinterpret_cast<std::byte*>(p.data()), size});
	return p;
}

void IODevice::writeBytes(ByteArray const& p) { writeFull({reinterpret_cast<std::byte const*>(p.data()), p.size()}); }

auto IODevice::readBytesAbsolute(std::int64_t readPosition, std::size_t size) -> ByteArray {
	if (!size) {
		return {};
	}
	ByteArray p;
	p.resize(size);
	readFullAbsolute(readPosition, {reinterpret_cast<std::byte*>(p.data()), size});
	return p;
}

void IODevice::writeBytesAbsolute(std::int64_t writePosition, ByteArray const& p) {
	writeFullAbsolute(writePosition, {reinterpret_cast<std::byte const*>(p.data()), p.size()});
}

void IODevice::setMode(IOMode m) noexcept { m_mode.store(m); }

IODevice::IODevice(const IODevice& rhs) : m_mode(rhs.mode()) {}

auto IODevice::operator=(const IODevice& rhs) -> IODevice& {
	m_mode.store(rhs.mode());
	return *this;
}

}// namespace Star
