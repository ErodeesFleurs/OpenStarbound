module;

export module star.json;

import std;
import star.data_stream;
import star.exception;
import star.hash;

export namespace star::json {

enum class json_errc : std::uint8_t {
    type_mismatch,
    index_out_of_range,
    key_not_found,
    not_a_container
};

class json_value;

using json_array = std::vector<json_value>;
using json_object = std::flat_map<std::u8string, json_value>;

template <class... Ts> struct overload : Ts... {
    using Ts::operator()...;
};

template <typename T, bool Mutable = false> struct json_traits {
    using storage_type = T;
};
template <bool Mutable> struct json_traits<json_array, Mutable> {
    using storage_type =
      std::conditional_t<Mutable, std::shared_ptr<json_array>, std::shared_ptr<const json_array>>;
};
template <bool Mutable> struct json_traits<json_object, Mutable> {
    using storage_type =
      std::conditional_t<Mutable, std::shared_ptr<json_object>, std::shared_ptr<const json_object>>;
};

template <typename T, bool Mutable = false>
using json_storage_t = typename json_traits<T, Mutable>::storage_type;

class json_value {
  public:
    using storage_type =
      std::variant<std::nullptr_t, bool, std::int64_t, double, std::shared_ptr<const std::u8string>,
                   std::shared_ptr<const json_array>, std::shared_ptr<const json_object>,
                   std::shared_ptr<json_array>, std::shared_ptr<json_object>>;

    json_value() noexcept : m_storage(nullptr) {}

    template <typename T>
        requires(!std::is_same_v<std::decay_t<T>, json_value>)
      && std::is_constructible_v<storage_type, T>
    explicit json_value(T&& val) noexcept(std::is_nothrow_constructible_v<storage_type, T>)
        : m_storage(std::forward<T>(val)) {}

    [[nodiscard]] static auto null() noexcept -> json_value;
    [[nodiscard]] static auto boolean(bool b) noexcept -> json_value;
    [[nodiscard]] static auto integer(std::int64_t i) noexcept -> json_value;
    [[nodiscard]] static auto floating(std::double_t d) noexcept -> json_value;
    [[nodiscard]] static auto string(const std::u8string& s) -> json_value;
    [[nodiscard]] static auto array() -> json_value;
    [[nodiscard]] static auto object() -> json_value;
    [[nodiscard]] static auto mutable_array() -> json_value;
    [[nodiscard]] static auto mutable_object() -> json_value;

    template <typename T, bool Mutable = false>
    [[nodiscard]] auto as() const noexcept -> std::expected<json_storage_t<T, Mutable>, json_errc>;
    template <typename T, bool Mutable = false> [[nodiscard]] auto is() const noexcept -> bool;
    template <typename T, bool Mutable = false>
    [[nodiscard]] auto opt() const noexcept -> std::optional<json_storage_t<T>>;

    [[nodiscard]] auto freeze() const& -> std::expected<json_value, json_errc>;
    [[nodiscard]] auto freeze() && -> std::expected<json_value, json_errc>;

    auto operator==(const json_value& other) const -> bool = default;
    auto operator!=(const json_value& other) const -> bool = default;

    [[nodiscard]] auto operator[](std::size_t index) const -> std::expected<json_value, json_errc>;
    [[nodiscard]] auto operator[](std::u8string_view key) const
      -> std::expected<json_value, json_errc>;

    template <typename Visitor> auto visit(this auto&& json, Visitor&& vis) -> decltype(auto) {
        return std::visit(std::forward<Visitor>(vis), json.m_storage);
    }

  private:
    storage_type m_storage = nullptr;
};

auto json_value::null() noexcept -> json_value { return {}; }

auto json_value::boolean(bool b) noexcept -> json_value { return json_value(b); }

auto json_value::integer(std::int64_t i) noexcept -> json_value { return json_value(i); }

auto json_value::floating(std::double_t d) noexcept -> json_value { return json_value(d); }

auto json_value::string(const std::u8string& s) -> json_value {
    json_value j;
    j.m_storage = std::make_shared<std::u8string>(s);
    return j;
}

auto json_value::array() -> json_value {
    json_value j;
    j.m_storage = std::make_shared<json_array>();
    return j;
}

auto json_value::object() -> json_value {
    json_value j;
    j.m_storage = std::make_shared<json_object>();
    return j;
}

auto json_value::mutable_array() -> json_value {
    json_value j;
    j.m_storage = std::make_shared<json_array>();
    return j;
}

auto json_value::mutable_object() -> json_value {
    json_value j;
    j.m_storage = std::make_shared<json_object>();
    return j;
}

}// namespace star::json
