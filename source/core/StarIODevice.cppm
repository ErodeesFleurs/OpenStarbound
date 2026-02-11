module;

export module star.iodevice;

import std;
import star.bytearray;
import star.exception;

namespace star {

using eof_exception = exception_derived<"eof_exception">;

enum class io_mode : std::uint8_t {
    Closed = 0x0,
    Read = 0x1,
    Write = 0x2,
    ReadWrite = 0x3,
    Append = 0x4,
    Truncate = 0x8,
};

[[nodiscard]] constexpr auto operator|(io_mode a, io_mode b) noexcept -> io_mode {
    return static_cast<io_mode>(std::to_underlying(a) | std::to_underlying(b));
}

[[nodiscard]] constexpr auto operator&(io_mode a, io_mode b) noexcept -> bool {
    return (std::to_underlying(a) & std::to_underlying(b)) != 0;
}

enum class io_seek : std::uint8_t {
    Absolute = 0,// SEEK_SET
    Relative = 1,// SEEK_CUR
    End = 2      // SEEK_END
};

// Abstract Interface to a random access I/O device.
class io_device {
  public:
    explicit io_device(io_mode mode = io_mode::Closed) : m_mode(mode) {}
    virtual ~io_device() = default;

    io_device(io_device&&) = delete;
    auto operator=(io_device&&) -> io_device& = delete;

    // Do a read or write that may result in less data read or written than
    // requested.
    [[nodiscard]] virtual auto read(std::span<std::byte> data) -> std::size_t = 0;
    [[nodiscard]] virtual auto write(std::span<std::byte const> data) -> std::size_t = 0;

    [[nodiscard]] virtual auto pos() -> std::int64_t = 0;
    virtual void seek(std::int64_t pos, io_seek mode = io_seek::Absolute) = 0;

    virtual void resize([[maybe_unused]] std::int64_t size) {
        throw io_exception("resize not supported");
    }

    virtual auto read_absolute(std::int64_t readPosition, std::span<std::byte> data) -> std::size_t;
    virtual auto write_absolute(std::int64_t writePosition, std::span<const std::byte> data)
      -> std::size_t;

    virtual void readFull(std::span<std::byte> data);
    virtual void writeFull(std::span<const std::byte> data);

    virtual void readFullAbsolute(std::int64_t readPosition, std::span<std::byte> data);
    virtual void writeFullAbsolute(std::int64_t writePosition, std::span<const std::byte> data);

    virtual void open(io_mode mode);

    virtual void close();

    virtual void sync();

    [[nodiscard]] virtual auto clone() -> std::shared_ptr<io_device> = 0;

    [[nodiscard]] virtual auto deviceName() const -> std::string;

    [[nodiscard]] virtual auto atEnd() -> bool;

    [[nodiscard]] virtual auto size() -> std::int64_t;

    [[nodiscard]] auto mode() const noexcept -> io_mode;
    [[nodiscard]] auto isOpen() const noexcept -> bool;
    [[nodiscard]] auto isReadable() const noexcept -> bool;
    [[nodiscard]] auto isWritable() const noexcept -> bool;

    [[nodiscard]] auto readBytes(std::size_t size) -> bytearray;
    void writeBytes(bytearray const& p);

    [[nodiscard]] auto readBytesAbsolute(std::int64_t readPosition, std::size_t size) -> bytearray;
    void writeBytesAbsolute(std::int64_t writePosition, bytearray const& p);

  protected:
    void setMode(io_mode mode) noexcept;

    io_device(io_device const&);
    auto operator=(io_device const&) -> io_device&;

  private:
    std::atomic<io_mode> m_mode;
};

inline auto io_device::mode() const noexcept -> io_mode {
    return m_mode.load(std::memory_order_relaxed);
}

inline auto io_device::isOpen() const noexcept -> bool { return mode() != io_mode::Closed; }

inline auto io_device::isReadable() const noexcept -> bool { return mode() & io_mode::Read; }

inline auto io_device::isWritable() const noexcept -> bool { return mode() & io_mode::Write; }

}// namespace star
