export module star.data_stream;

import std;
import star.bytes;
import star.byte_array;
import star.exception;
import star.net_compatibility;

export namespace star {

using data_stream_exception = star::exception_derived<"data_stream_exception", io_exception>;
constexpr std::uint32_t current_stream_version = 14;

// Writes complex types to bytes in a portable big-endian fashion.
class data_stream {
  public:
    data_stream();
    virtual ~data_stream() = default;
    data_stream(data_stream const&) = default;
    data_stream(data_stream&&) = default;
    auto operator=(data_stream const&) -> data_stream& = default;
    auto operator=(data_stream&&) -> data_stream& = default;

    // DataStream defaults to big-endian order for all primitive types
    [[nodiscard]] auto get_byte_order() const -> star::byte_order;
    void set_byte_order(byte_order byte_order_value);

    // DataStream can optionally write strings as null terminated rather than
    // length prefixed
    [[nodiscard]] auto get_null_terminated_strings() const -> bool;
    void set_null_terminated_strings(bool null_terminated_strings);

    // stream_compatibility_version defaults to current_stream_version, but can be
    // changed for compatibility with older versions of DataStream serialization.
    [[nodiscard]] auto get_stream_compatibility_version() const -> unsigned;
    void set_stream_compatibility_version(unsigned stream_compatibility_version);
    void set_stream_compatibility_version(net_compatibility_rules const& rules);
    // Do direct reads and writes
    virtual void read_data(std::span<std::byte> data) = 0;
    virtual void write_data(std::span<std::byte const> data) = 0;

    // Compatibility overloads
    void read_data(char* data, std::size_t len) { read_data({reinterpret_cast<std::byte*>(data), len}); }
    void write_data(char const* data, std::size_t len) { write_data({reinterpret_cast<std::byte const*>(data), len}); }

    virtual auto at_end() -> bool { return false; };

    // These do not read / write sizes, they simply read / write directly.
    auto read_bytes(std::size_t len) -> byte_array;
    void write_bytes(byte_array const& ba);

    auto operator<<(bool d) -> data_stream&;
    auto operator<<(char c) -> data_stream&;
    auto operator<<(std::int8_t d) -> data_stream&;
    auto operator<<(std::uint8_t d) -> data_stream&;
    auto operator<<(std::int16_t d) -> data_stream&;
    auto operator<<(std::uint16_t d) -> data_stream&;
    auto operator<<(std::int32_t d) -> data_stream&;
    auto operator<<(std::uint32_t d) -> data_stream&;
    auto operator<<(std::int64_t d) -> data_stream&;
    auto operator<<(std::uint64_t d) -> data_stream&;
    auto operator<<(std::float_t d) -> data_stream&;
    auto operator<<(std::double_t d) -> data_stream&;

    auto operator>>(bool& d) -> data_stream&;
    auto operator>>(char& c) -> data_stream&;
    auto operator>>(std::int8_t& d) -> data_stream&;
    auto operator>>(std::uint8_t& d) -> data_stream&;
    auto operator>>(std::int16_t& d) -> data_stream&;
    auto operator>>(std::uint16_t& d) -> data_stream&;
    auto operator>>(std::int32_t& d) -> data_stream&;
    auto operator>>(std::uint32_t& d) -> data_stream&;
    auto operator>>(std::int64_t& d) -> data_stream&;
    auto operator>>(std::uint64_t& d) -> data_stream&;
    auto operator>>(std::float_t& d) -> data_stream&;
    auto operator>>(std::double_t& d) -> data_stream&;

    // Writes and reads a VLQ encoded integer.  Can write / read anywhere from 1
    // to 10 bytes of data, with integers of smaller (absolute) value taking up
    // fewer bytes.  std::size_t version can be used to portably write a std::size_t type,
    // and portably and efficiently handles the case of std::numeric_limits<std::size_t>::max().

    auto write_vlq_u(std::uint64_t i) -> std::size_t;
    auto write_vlq_i(std::int64_t i) -> std::size_t;
    auto write_vlq_s(std::size_t i) -> std::size_t;

    auto read_vlq_u(std::uint64_t& i) -> std::size_t;
    auto read_vlq_i(std::int64_t& i) -> std::size_t;
    auto read_vlq_s(std::size_t& i) -> std::size_t;

    auto read_vlq_u() -> std::uint64_t;
    auto read_vlq_i() -> std::int64_t;
    auto read_vlq_s() -> std::size_t;

    // The following functions write / read data with length and then content
    // following, but note that the length is encoded as an unsigned VLQ integer.
    // String objects are encoded in utf8, and can optionally be written as null
    // terminated rather than length then content.

    auto operator<<(const char* s) -> data_stream&;
    auto operator<<(const std::string& d) -> data_stream&;
    auto operator<<(const byte_array& d) -> data_stream&;

    auto operator>>(std::string& d) -> data_stream&;
    auto operator>>(byte_array& d) -> data_stream&;

    // All enum types are automatically serializable

    template <typename EnumType> requires std::is_enum_v<EnumType>
    auto operator<<(EnumType const& e) -> data_stream&;

    template <typename EnumType> requires std::is_enum_v<EnumType>
    auto operator>>(EnumType& e) -> data_stream&;

    // Convenience method to avoid temporary.
    template <typename T> [[nodiscard]] auto read() -> T;

    // Convenient argument style reading / writing

    template <typename Data> void read(Data& data);

    template <typename Data> void write(Data const& data);

    // Argument style reading / writing with casting.

    template <typename ReadType, typename Data> void cread(Data& data);

    template <typename WriteType, typename Data> void cwrite(Data const& data);

    // Argument style reading / writing of variable length integers.  Arguments
    // are explicitly casted, so things like enums are allowed.

    template <std::integral IntegralType> void vu_read(IntegralType& data);

    template <std::integral IntegralType> void vi_read(IntegralType& data);

    template <std::integral IntegralType> void vs_read(IntegralType& data);

    template <std::integral IntegralType> void vu_write(IntegralType const& data);

    template <std::integral IntegralType> void vi_write(IntegralType const& data);

    template <std::integral IntegralType> void vs_write(IntegralType const& data);

    // Store a fixed point number as a variable length integer

    template <std::floating_point FloatType> void vf_read(FloatType& data, FloatType base);

    template <std::floating_point FloatType> void vf_write(FloatType const& data, FloatType base);

    // Read a shared / unique ptr, and store whether the pointer is initialized.

    template <typename PointerType, typename ReadFunction>
    void p_read(PointerType& pointer, ReadFunction readFunction);

    template <typename PointerType, typename WriteFunction>
    void p_write(PointerType const& pointer, WriteFunction writeFunction);

    template <typename PointerType> void p_read(PointerType& pointer);

    template <typename PointerType> void p_write(PointerType const& pointer);

    // WriteFunction should be void (DataStream& ds, Element const& e)
    template <typename Container, typename WriteFunction>
    void write_container(Container const& container, WriteFunction function);

    // ReadFunction should be void (DataStream& ds, Element& e)
    template <typename Container, typename ReadFunction>
    void read_container(Container& container, ReadFunction function);

    template <typename Container, typename WriteFunction>
    void write_map_container(Container& map, WriteFunction function);

    // Specialization of read_container for map types (whose elements are a pair
    // with the key type marked const)
    template <typename Container, typename ReadFunction>
    void read_map_container(Container& map, ReadFunction function);

    template <typename Container> void write_container(Container const& container);

    template <typename Container> void read_container(Container& container);

    template <typename Container> void write_map_container(Container const& container);

    template <typename Container> void read_map_container(Container& container);

  private:
    void write_string_data(char const* data, std::size_t len);

    star::byte_order m_byte_order;
    bool m_null_terminated_strings;
    unsigned m_stream_compatibility_version;
};

template <typename EnumType> requires std::is_enum_v<EnumType>
auto data_stream::operator<<(EnumType const& e) -> data_stream& {
    *this << std::to_underlying(e);
    return *this;
}

template <typename EnumType> requires std::is_enum_v<EnumType>
auto data_stream::operator>>(EnumType& e) -> data_stream& {
    std::underlying_type_t<EnumType> i;
    *this >> i;
    e = static_cast<EnumType>(i);
    return *this;
}

template <typename T> auto data_stream::read() -> T {
    T t;
    *this >> t;
    return t;
}

template <typename Data> void data_stream::read(Data& data) { *this >> data; }

template <typename Data> void data_stream::write(Data const& data) { *this << data; }

template <typename ReadType, typename Data> void data_stream::cread(Data& data) {
    ReadType v;
    *this >> v;
    data = static_cast<Data>(v);
}

template <typename WriteType, typename Data> void data_stream::cwrite(Data const& data) {
    auto v = static_cast<WriteType>(data);
    *this << v;
}

template <std::integral IntegralType> void data_stream::vu_read(IntegralType& data) {
    std::uint64_t i = read_vlq_u();
    data = static_cast<IntegralType>(i);
}

template <std::integral IntegralType> void data_stream::vi_read(IntegralType& data) {
    std::int64_t i = read_vlq_i();
    data = static_cast<IntegralType>(i);
}

template <std::integral IntegralType> void data_stream::vs_read(IntegralType& data) {
    std::size_t s = read_vlq_s();
    data = static_cast<IntegralType>(s);
}

template <std::integral IntegralType> void data_stream::vu_write(const IntegralType& data) {
    write_vlq_u(static_cast<std::uint64_t>(data));
}

template <std::integral IntegralType> void data_stream::vi_write(const IntegralType& data) {
    write_vlq_i(static_cast<std::int64_t>(data));
}

template <std::integral IntegralType> void data_stream::vs_write(const IntegralType& data) {
    write_vlq_s(static_cast<std::size_t>(data));
}

template <std::floating_point FloatType> void data_stream::vf_read(FloatType& data, FloatType base) {
    std::int64_t i = read_vlq_i();
    data = static_cast<FloatType>(i) * base;
}

template <std::floating_point FloatType> void data_stream::vf_write(FloatType const& data, FloatType base) {
    write_vlq_i(static_cast<std::int64_t>(std::round(data / base)));
}

template <typename PointerType, typename ReadFunction>
void data_stream::p_read(PointerType& pointer, ReadFunction readFunction) {
    bool initialized = read<bool>();
    if (initialized) {
        auto element = std::make_unique<std::remove_cvref_t<typename PointerType::element_type>>();
        readFunction(*this, *element);
        pointer.reset(element.release());
    } else {
        pointer.reset();
    }
}

template <typename PointerType, typename WriteFunction>
void data_stream::p_write(PointerType const& pointer, WriteFunction writeFunction) {
    if (pointer) {
        write(true);
        writeFunction(*this, *pointer);
    } else {
        write(false);
    }
}

template <typename PointerType> void data_stream::p_read(PointerType& pointer) {
    return p_read(
      pointer,
      [](data_stream& ds, std::decay_t<typename PointerType::element_type>& value) -> auto {
          ds.read(value);
      });
}

template <typename PointerType> void data_stream::p_write(PointerType const& pointer) {
    return p_write(
      pointer,
            [](data_stream& ds, std::remove_cvref_t<typename PointerType::element_type> const& value)
                -> auto {
          ds.write(value);
      });
}

template <typename Container, typename WriteFunction>
void data_stream::write_container(Container const& container, WriteFunction function) {
    write_vlq_u(container.size());
    for (auto const& elem : container) {
        function(*this, elem);
    }
}

template <typename Container, typename ReadFunction>
void data_stream::read_container(Container& container, ReadFunction function) {
    container.clear();
    std::size_t size = read_vlq_u();
    for (std::size_t i = 0; i < size; ++i) {
        typename Container::value_type elem;
        function(*this, elem);
        container.insert(container.end(), elem);
    }
}

template <typename Container, typename WriteFunction>
void data_stream::write_map_container(Container& map, WriteFunction function) {
    write_vlq_u(map.size());
    for (auto const& elem : map) {
        function(*this, elem.first, elem.second);
    }
}

template <typename Container, typename ReadFunction>
void data_stream::read_map_container(Container& map, ReadFunction function) {
    map.clear();
    std::size_t size = read_vlq_u();
    for (std::size_t i = 0; i < size; ++i) {
        typename Container::key_type key;
        typename Container::mapped_type mapped;
        function(*this, key, mapped);
        map.insert(std::make_pair(std::move(key), std::move(mapped)));
    }
}

template <typename Container> void data_stream::write_container(Container const& container) {
    write_container(container,
                   [](data_stream& ds, typename Container::value_type const& element) -> auto {
                       ds << element;
                   });
}

template <typename Container> void data_stream::read_container(Container& container) {
    read_container(container,
                   [](data_stream& ds, typename Container::value_type& element) -> auto {
        ds >> element;
    });
}

template <typename Container> void data_stream::write_map_container(Container const& container) {
    write_map_container(container,
                      [](data_stream& ds, typename Container::key_type const& key,
                         typename Container::mapped_type const& mapped) -> auto {
                          ds << key;
                          ds << mapped;
                      });
}

template <typename Container> void data_stream::read_map_container(Container& container) {
    read_map_container(container,
                       [](data_stream& ds, typename Container::key_type& key,
                          typename Container::mapped_type& mapped) -> auto {
                           ds >> key;
                           ds >> mapped;
                       });
}

}// namespace star
