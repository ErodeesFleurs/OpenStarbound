export module star.byte_array;

import std;
import star.hash;

export namespace star {

/**
 * Class to hold an array of bytes. Contains an internal buffer that may be
 * larger than what is reported by size(), to avoid repeated allocations when a
 * ByteArray grows.
 */
class byte_array {
  public:
    static constexpr std::size_t SBO_SIZE = 23;

    constexpr byte_array() noexcept : m_size(0) { m_store.local.fill(std::byte{0}); }

    constexpr explicit byte_array(std::string_view sv) : byte_array() { assign(sv); }

    constexpr explicit byte_array(std::size_t size, std::byte fill_byte = std::byte{0})
        : byte_array() {
        resize(size, fill_byte);
    }

    constexpr byte_array(const byte_array& other) : byte_array() { assign(other.view()); }

    constexpr byte_array(byte_array&& other) noexcept : byte_array() { swap(other); }

    constexpr ~byte_array() {
        if (is_heap()) {
            delete[] m_store.heap.ptr;
        }
    }

    constexpr auto operator=(const byte_array& other) -> byte_array& {
        if (this != &other) {
            assign(other.view());
        }
        return *this;
    }

    constexpr auto operator=(byte_array&& other) noexcept -> byte_array& {
        byte_array(std::move(other)).swap(*this);
        return *this;
    }

    [[nodiscard]] constexpr auto is_heap() const noexcept -> bool {
        return (static_cast<std::uint8_t>(m_store.local[SBO_SIZE]) & 0x80) != 0;// NOLINT
    }

    [[nodiscard]] constexpr auto data(this auto& self) noexcept -> auto {
        using RT = std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>,
                                      const char*, char*>;
        if (self.is_heap()) [[unlikely]] {
            return reinterpret_cast<RT>(self.m_store.heap.ptr);
        }
        return reinterpret_cast<RT>(self.m_store.local.data());
    }

    [[nodiscard]] constexpr auto view(this auto& self) noexcept -> std::string_view {
        return {self.data(), self.m_size};
    }

    [[nodiscard]] constexpr auto span(this auto& self) noexcept -> std::span<const char> {
        return {self.data(), self.m_size};
    }

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t { return m_size; }

    constexpr auto reserve(std::size_t new_capacity) -> void { ensure_capacity(new_capacity); }

    constexpr auto resize(std::size_t new_size, std::byte fill_byte = std::byte{0}) -> void {
        if (new_size > m_size) {
            ensure_capacity(new_size);
            auto* dest = reinterpret_cast<std::byte*>(data());
            std::ranges::fill_n(dest + m_size, static_cast<std::uint32_t>(new_size - m_size),
                                fill_byte);
        }
        m_size = new_size;
    }

    constexpr void clear() { m_size = 0; }

    [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_size == 0; }

    [[nodiscard]] constexpr auto at(this auto& self, std::size_t index) noexcept
      -> std::expected<char, std::errc> {
        if (index >= self.m_size) [[unlikely]] {
            return std::unexpected(std::errc::result_out_of_range);
        }
        return std::ref(self.data()[index]);
    }

    constexpr auto operator[](std::size_t index) noexcept -> char& { return data()[index]; }

    constexpr void assign(std::string_view sv) {
        std::size_t new_size = sv.size();
        ensure_capacity(new_size);
        std::ranges::copy(sv, data());
        m_size = new_size;
    }

    constexpr void swap(byte_array& other) noexcept {
        std::swap(m_store, other.m_store);
        std::swap(m_size, other.m_size);
    }

    auto operator<=>(const byte_array& rhs) const noexcept { return view() <=> rhs.view(); }

    auto operator==(const byte_array& rhs) const noexcept -> bool { return view() == rhs.view(); }

  private:
    constexpr void set_heap_flag(bool heap) {
        if (heap) {
            m_store.local[SBO_SIZE] |=
              std::byte{0x80};// NOLINT(hicpp-signed-bitwise, bugprone-narrowing-conversions)
        } else {
            m_store.local[SBO_SIZE] &= std::byte{0x7F};// NOLINT(hicpp-signed-bitwise)
        }
    }

    constexpr void ensure_capacity(std::size_t capacity) {
        bool current_heap = is_heap();
        std::size_t current_capacity = current_heap ? m_store.heap.cap : SBO_SIZE;
        if (capacity <= current_capacity) {
            return;
        }
        std::size_t next_cap = std::max(capacity, current_capacity * 2);
        auto* new_ptr = new std::byte[next_cap];
        if (m_size > 0) {
            std::ranges::copy_n(reinterpret_cast<const std::byte*>(data()),
                                static_cast<std::uint32_t>(m_size), new_ptr);
        }
        if (current_heap) {
            delete[] m_store.heap.ptr;
        }

        m_store.heap.ptr = new_ptr;
        m_store.heap.cap = next_cap;
        set_heap_flag(true);
    }

    struct HeapData {
        std::byte* ptr;
        std::size_t cap;
    };

    union Store {
        std::array<std::byte, SBO_SIZE + 1> local;
        HeapData heap;
        constexpr Store() : local{} {}
    } m_store;

    std::size_t m_size;
};

template <> struct hash<star::byte_array> {
    auto operator()(const star::byte_array& b) const noexcept -> std::size_t {
        return std::hash<std::string_view>{}(b.view());
    }
};

}// namespace star

template <> struct std::formatter<star::byte_array> : std::formatter<std::string_view> {
    bool hex_mode = false;

    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == 'x') {
            hex_mode = true;
            ++it;
        }
        return it;
    }

    auto format(const star::byte_array& b, format_context& ctx) const {
        if (hex_mode) {
            auto out = ctx.out();
            auto v = b.view();
            for (size_t i = 0; i < v.size(); ++i) {
                out = std::format_to(out, "{:02X}{}", static_cast<uint8_t>(v[i]),
                                     (i == v.size() - 1 ? "" : " "));
            }
            return out;
        }
        return std::formatter<std::string_view>::format(b.view(), ctx);
    }
};
