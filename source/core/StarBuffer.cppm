export module star.buffer;

import std;
import star.io_device;
import star.byte_array;

namespace star {

// Wraps a byte_array to an io_device
export class buffer : public io_device {
public:
  // Constructs buffer open read_write
  buffer();
  explicit buffer(std::size_t initial_size);
  explicit buffer(byte_array b);
  buffer(buffer const& other);
  buffer(buffer&& other) noexcept;
  ~buffer() override = default;

  [[nodiscard]] auto pos() -> std::int64_t override;
  void seek(std::int64_t pos, io_seek mode = io_seek::absolute) override;
  void resize(std::int64_t size) override;
  [[nodiscard]] auto at_end() -> bool override;

  // Override base class virtual functions with std::span
  [[nodiscard]] auto read(std::span<std::byte> data) -> std::size_t override;
  [[nodiscard]] auto write(std::span<std::byte const> data) -> std::size_t override;

  auto read_absolute(std::int64_t read_position, std::span<std::byte> data)
    -> std::size_t override;
  auto write_absolute(std::int64_t write_position, std::span<std::byte const> data)
    -> std::size_t override;

  void open(io_mode mode) override;

  [[nodiscard]] auto device_name() const -> std::string override;

  [[nodiscard]] auto size() -> std::int64_t override;

  [[nodiscard]] auto clone() -> std::shared_ptr<io_device> override;

  // Data access
  [[nodiscard]] auto data() -> byte_array&;
  [[nodiscard]] auto data() const -> byte_array const&;

  // Moves the data out of this buffer, leaving it empty
  [[nodiscard]] auto take_data() -> byte_array;

  // Returns a pointer to the beginning of the buffer
  [[nodiscard]] auto ptr() -> char*;
  [[nodiscard]] auto ptr() const -> char const*;

  // Same as size(), but as std::size_t (since this is in-memory)
  [[nodiscard]] auto data_size() const noexcept -> std::size_t;
  void reserve(std::size_t size);

  // Clears buffer and moves position to 0
  void clear() noexcept;
  [[nodiscard]] auto empty() const noexcept -> bool;

  // Reset buffer with new contents, moves position to 0
  void reset(std::size_t new_size);
  void reset(byte_array b);

  auto operator=(buffer const& other) -> buffer&;
  auto operator=(buffer&& other) noexcept -> buffer&;

private:
  auto do_read(std::size_t pos, std::span<std::byte> data) -> std::size_t;
  auto do_write(std::size_t pos, std::span<std::byte const> data) -> std::size_t;

  std::size_t m_pos;
  byte_array m_bytes;
};

// Wraps an externally held sequence of bytes to a read-only io_device
export class external_buffer : public io_device {
public:
  // Constructs an empty read-only external_buffer
  external_buffer();

  // Constructs a read-only external_buffer pointing to external data
  // The external data must remain valid for the lifetime of this object
  external_buffer(char const* external_data, std::size_t len);

  external_buffer(external_buffer const& other) = default;
  external_buffer(external_buffer&& other) noexcept;
  ~external_buffer() override = default;

  auto operator=(external_buffer const& other) -> external_buffer& = default;
  auto operator=(external_buffer&& other) noexcept -> external_buffer&;

  [[nodiscard]] auto pos() -> std::int64_t override;
  void seek(std::int64_t pos, io_seek mode = io_seek::absolute) override;
  [[nodiscard]] auto at_end() -> bool override;

  // Override base class virtual functions with std::span
  [[nodiscard]] auto read(std::span<std::byte> data) -> std::size_t override;
  [[nodiscard]] auto write(std::span<std::byte const> data) -> std::size_t override;

  auto read_absolute(std::int64_t read_position, std::span<std::byte> data)
    -> std::size_t override;
  auto write_absolute(std::int64_t write_position, std::span<std::byte const> data)
    -> std::size_t override;

  [[nodiscard]] auto device_name() const -> std::string override;

  [[nodiscard]] auto size() -> std::int64_t override;

  [[nodiscard]] auto clone() -> std::shared_ptr<io_device> override;

  // Returns a pointer to the beginning of the buffer
  [[nodiscard]] auto ptr() const noexcept -> char const*;

  // Same as size(), but as std::size_t (since this is in-memory)
  [[nodiscard]] auto data_size() const noexcept -> std::size_t;

  [[nodiscard]] auto empty() const noexcept -> bool;

  explicit operator bool() const noexcept;

  // Reset buffer with new contents, moves position to 0
  void reset(char const* external_data, std::size_t len) noexcept;

private:
  auto do_read(std::size_t pos, std::span<std::byte> data) -> std::size_t;

  std::size_t m_pos;
  char const* m_bytes;
  std::size_t m_size;
};

} // namespace star
