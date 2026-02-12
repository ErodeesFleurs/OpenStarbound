export module star.io_device;

import std;
import star.byte_array;
import star.exception;

export namespace star {

using eof_exception = exception_derived<"eof_exception">;

enum class io_mode : std::uint8_t {
    closed = 0x0,
    read = 0x1,
    write = 0x2,
    read_write = 0x3,
    append = 0x4,
    truncate = 0x8,
};

[[nodiscard]] constexpr auto operator|(io_mode a, io_mode b) noexcept -> io_mode {
    return static_cast<io_mode>(std::to_underlying(a) | std::to_underlying(b));
}

[[nodiscard]] constexpr auto operator&(io_mode a, io_mode b) noexcept -> bool {
    return (std::to_underlying(a) & std::to_underlying(b)) != 0;
}

constexpr auto operator|=(io_mode& a, io_mode b) noexcept -> io_mode& {
    a = static_cast<io_mode>(std::to_underlying(a) | std::to_underlying(b));
    return a;
}

enum class io_seek : std::uint8_t {
    absolute = 0,// SEEK_SET
    relative = 1,// SEEK_CUR
    end = 2      // SEEK_END
};

// Abstract Interface to a random access I/O device.
class io_device {
  public:
    explicit io_device(io_mode mode = io_mode::closed) : m_mode(mode) {}
    virtual ~io_device() = default;

    io_device(io_device&&) = delete;
    auto operator=(io_device&&) -> io_device& = delete;

    // Do a read or write that may result in less data read or written than
    // requested.
    [[nodiscard]] virtual auto read(std::span<std::byte> data) -> std::size_t = 0;
    [[nodiscard]] virtual auto write(std::span<std::byte const> data) -> std::size_t = 0;

    [[nodiscard]] virtual auto pos() -> std::int64_t = 0;
    virtual void seek(std::int64_t pos, io_seek mode = io_seek::absolute) = 0;

    virtual void resize(std::int64_t size);

    virtual auto read_absolute(std::int64_t read_position, std::span<std::byte> data)
      -> std::size_t;
    virtual auto write_absolute(std::int64_t write_position, std::span<const std::byte> data)
      -> std::size_t;

    virtual void read_full(std::span<std::byte> data);
    virtual void write_full(std::span<const std::byte> data);

    virtual void read_full_absolute(std::int64_t read_position, std::span<std::byte> data);
    virtual void write_full_absolute(std::int64_t write_position, std::span<const std::byte> data);

    virtual void open(io_mode mode);

    virtual void close();

    virtual void sync();

    [[nodiscard]] virtual auto clone() -> std::shared_ptr<io_device> = 0;

    [[nodiscard]] virtual auto device_name() const -> std::string;

    [[nodiscard]] virtual auto at_end() -> bool;

    [[nodiscard]] virtual auto size() -> std::int64_t;

    [[nodiscard]] auto mode() const noexcept -> io_mode;
    [[nodiscard]] auto is_open() const noexcept -> bool;
    [[nodiscard]] auto is_readable() const noexcept -> bool;
    [[nodiscard]] auto is_writable() const noexcept -> bool;

    [[nodiscard]] auto read_bytes(std::size_t size) -> byte_array;
    void write_bytes(byte_array const& p);

    [[nodiscard]] auto read_bytes_absolute(std::int64_t read_position, std::size_t size)
      -> byte_array;
    void write_bytes_absolute(std::int64_t write_position, byte_array const& p);

  protected:
    void set_mode(io_mode mode) noexcept;

    io_device(io_device const&);
    auto operator=(io_device const&) -> io_device&;

  private:
    std::atomic<io_mode> m_mode;
};

inline auto io_device::mode() const noexcept -> io_mode {
    return m_mode.load(std::memory_order_relaxed);
}

inline auto io_device::is_open() const noexcept -> bool { return mode() != io_mode::closed; }

inline auto io_device::is_readable() const noexcept -> bool { return mode() & io_mode::read; }

inline auto io_device::is_writable() const noexcept -> bool { return mode() & io_mode::write; }

}// namespace star
