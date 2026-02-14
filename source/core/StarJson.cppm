module;

export module star.json;

import std;

export namespace star::json {

enum class json_errc : std::uint8_t {
    type_mismatch,
    index_out_of_range,
    key_not_found,
    not_a_container
};

class json_value;

using json_array = std::pmr::vector<json_value>;
using json_object = std::pmr::map<std::string, json_value, std::less<>>;
using json_string = std::pmr::string;

template <class... Ts> struct overload : Ts... {
    using Ts::operator()...;
};

template <typename T> struct pmr_deleter {
    std::pmr::memory_resource* res;
    void operator()(T* ptr) const {
        std::destroy_at(ptr);
        res->deallocate(ptr, sizeof(T), alignof(T));
    }
};

class json_value {
  public:
    using storage_type =
      std::variant<std::nullptr_t, bool, std::int64_t, std::double_t, json_string,
                   std::unique_ptr<json_array, pmr_deleter<json_array>>,
                   std::unique_ptr<json_object, pmr_deleter<json_object>>>;

    explicit json_value(std::pmr::memory_resource* res = std::pmr::get_default_resource()) noexcept
        : m_storage(nullptr), m_res(res) {}

    json_value(json_value&&) noexcept = default;
    auto operator=(json_value&&) noexcept -> json_value& = default;

    json_value(const json_value& other)
        : m_storage(other.clone_storage(other.m_res)), m_res(other.m_res) {}
    auto operator=(const json_value& other) -> json_value& {
        if (this != &other) {
            m_storage = other.clone_storage(m_res);
        }
        return *this;
    }

  private:
    template <typename T> static auto create_ptr(std::pmr::memory_resource* res) {
        auto* buf = res->allocate(sizeof(T), alignof(T));
        auto* ptr = std::construct_at(static_cast<T*>(buf), res);
        return std::unique_ptr<T, pmr_deleter<T>>{ptr, pmr_deleter<T>{res}};
    }

  public:
    [[nodiscard]] static auto
    null(std::pmr::memory_resource* res = std::pmr::get_default_resource()) -> json_value {
        return json_value(res);
    }
    [[nodiscard]] static auto
    boolean(bool b, std::pmr::memory_resource* res = std::pmr::get_default_resource())
      -> json_value {
        json_value v(res);
        v.m_storage = b;
        return v;
    }
    [[nodiscard]] static auto
    integer(std::int64_t i, std::pmr::memory_resource* res = std::pmr::get_default_resource())
      -> json_value {
        json_value v(res);
        v.m_storage = i;
        return v;
    }
    [[nodiscard]] static auto
    floating(double d, std::pmr::memory_resource* res = std::pmr::get_default_resource())
      -> json_value {
        json_value v(res);
        v.m_storage = d;
        return v;
    }
    [[nodiscard]] static auto
    string(std::string_view s, std::pmr::memory_resource* res = std::pmr::get_default_resource())
      -> json_value {
        json_value v(res);
        v.m_storage = json_string(s, res);
        return v;
    }
    [[nodiscard]] static auto
    array(std::pmr::memory_resource* res = std::pmr::get_default_resource()) -> json_value {
        return {create_ptr<json_array>(res), res};
    }
    [[nodiscard]] static auto
    object(std::pmr::memory_resource* res = std::pmr::get_default_resource()) -> json_value {
        return {create_ptr<json_object>(res), res};
    }

    template <typename T> [[nodiscard]] auto is() const noexcept -> bool {
        return std::holds_alternative<T>(m_storage);
    }
    template <typename T>
    [[nodiscard]] auto as() const noexcept
      -> std::expected<std::reference_wrapper<const T>, json_errc> {
        if constexpr (requires { std::get_if<T>(&m_storage); }) {
            if (const auto* p = std::get_if<T>(&m_storage)) {
                return std::cref(*p);
            }
        } else if constexpr (std::is_same_v<T, json_array>) {
            using target_ptr = std::unique_ptr<json_array, pmr_deleter<json_array>>;
            if (const auto* p = std::get_if<target_ptr>(&m_storage)) {
                return std::cref(**p);
            }
        } else if constexpr (std::is_same_v<T, json_object>) {
            using target_ptr = std::unique_ptr<json_object, pmr_deleter<json_object>>;
            if (const auto* p = std::get_if<target_ptr>(&m_storage)) {
                return std::cref(**p);
            }
        }
        return std::unexpected(json_errc::type_mismatch);
    }
    template <typename T>
    [[nodiscard]] auto opt() const noexcept -> std::optional<std::reference_wrapper<const T>> {
        if (const auto* p = std::get_if<T>(&m_storage)) {
            return *p;
        }
        return std::nullopt;
    }

    [[nodiscard]] auto operator[](std::size_t index) const
      -> std::expected<std::reference_wrapper<const json_value>, json_errc> {
        auto res = as<json_array>();
        if (!res) {
            return std::unexpected(res.error());
        }
        if (index >= res->get().size()) {
            return std::unexpected(json_errc::index_out_of_range);
        }
        return std::cref(res->get()[index]);
    }

    [[nodiscard]] auto operator[](std::string_view key) const
      -> std::expected<std::reference_wrapper<const json_value>, json_errc> {
        auto res = as<json_object>();
        if (!res) {
            return std::unexpected(res.error());
        }
        auto it = res->get().find(key);
        if (it == res->get().end()) {
            return std::unexpected(json_errc::key_not_found);
        }
        return std::cref(it->second);
    }

    template <typename Visitor> auto visit(this auto&& self, Visitor&& vis) -> decltype(auto) {
        return std::visit(std::forward<Visitor>(vis), self.m_storage);
    }

  private:
    storage_type m_storage = nullptr;
    std::pmr::memory_resource* m_res{};

    json_value(storage_type&& s, std::pmr::memory_resource* r)
        : m_storage(std::move(s)), m_res(r) {}

    auto clone_storage(std::pmr::memory_resource* target_res) const -> storage_type {
        return std::visit(
          overload{
            [](std::nullptr_t v) -> storage_type { return v; },
            [](bool v) -> storage_type { return v; },
            [](std::int64_t v) -> storage_type { return v; },
            [](double v) -> storage_type { return v; },
            [target_res](const json_string& s) -> storage_type {
                return json_string(s, target_res);
            },
            [target_res](
              const std::unique_ptr<json_array, pmr_deleter<json_array>>& ptr) -> storage_type {
                auto new_arr = create_ptr<json_array>(target_res);
                for (const auto& item : *ptr) {
                    new_arr->push_back(item);
                }
                return new_arr;
            },
            [target_res](
              const std::unique_ptr<json_object, pmr_deleter<json_object>>& ptr) -> storage_type {
                auto new_obj = create_ptr<json_object>(target_res);
                for (const auto& [k, v] : *ptr) {
                    new_obj->emplace(json_string(k, target_res), v);
                }
                return new_obj;
            }},
          m_storage);
    }
};

}// namespace star::json
