module star.json;

import std;
import star.data_stream;
import star.exception;
import star.hash;

namespace star::json {

template <typename T, bool Mutable = false>
auto json_value::as() const noexcept -> std::expected<json_storage_t<T, Mutable>, json_errc> {
    using target_storage = json_storage_t<T, Mutable>;
    return std::visit(overload{[](const target_storage& v) -> std::expected<T, json_errc> {
                                   if constexpr (Mutable) {
                                       return v;
                                   }
                                   if constexpr (std::is_same_v<T, json_array>
                                                 || std::is_same_v<T, json_object>) {
                                       return *v;
                                   } else {
                                       return v;
                                   }
                               },
                               [](std::int64_t i) -> std::expected<T, json_errc> {
                                   if constexpr (std::is_same_v<T, std::double_t>) {
                                       return static_cast<std::double_t>(i);
                                   }
                                   return std::unexpected(json_errc::type_mismatch);
                               },
                               [](const auto&) -> std::expected<T, json_errc> {
                                   return std::unexpected(json_errc::type_mismatch);
                               }},
                      m_storage);
}

template <typename T, bool Mutable = false> auto json_value::is() const noexcept -> bool {
    using target_storage = json_storage_t<T, Mutable>;
    if constexpr (Mutable && !std::is_same_v<T, json_array> && !std::is_same_v<T, json_object>) {
        return false;
    }
    return std::holds_alternative<target_storage>(m_storage);
}

template <typename T, bool Mutable = false>
auto json_value::opt() const noexcept -> std::optional<json_storage_t<T>> {
    if (auto res = as<T, Mutable>()) {
        return *res;
    }
    return std::nullopt;
}

auto json_value::freeze() const& -> std::expected<json_value, json_errc> {
    if (auto* arr = std::get_if<std::shared_ptr<json_array>>(&m_storage)) {
        return json_value(std::shared_ptr<const json_array>(*arr));
    }
    if (auto* obj = std::get_if<std::shared_ptr<json_object>>(&m_storage)) {
        return json_value(std::shared_ptr<const json_object>(*obj));
    }
    return std::unexpected(json_errc::type_mismatch);
}

auto json_value::freeze() && -> std::expected<json_value, json_errc> {
    return std::visit(
      overload{[](std::shared_ptr<json_array>& arr) -> std::expected<json_value, json_errc> {
                   return json_value(std::const_pointer_cast<const json_array>(std::move(arr)));
               },
               [](std::shared_ptr<json_object>& obj) -> std::expected<json_value, json_errc> {
                   return json_value(std::const_pointer_cast<const json_object>(std::move(obj)));
               },
               [](auto&) -> std::expected<json_value, json_errc> {
                   return std::unexpected(json_errc::type_mismatch);
               }},
      m_storage);
}

auto json_value::operator[](std::size_t index) const -> std::expected<json_value, json_errc> {
    if (auto arr = std::get_if<std::shared_ptr<const json_array>>(&m_storage)) {
        if (index < (*arr)->size()) {
            return (*arr)->at(index);
        } else [[unlikely]] {
            return std::unexpected(json_errc::index_out_of_range);
        }
    } else {
        return std::unexpected(json_errc::not_a_container);
    }
}

auto json_value::operator[](std::u8string_view key) const -> std::expected<json_value, json_errc> {
    if (auto obj = std::get_if<std::shared_ptr<const json_object>>(&m_storage)) {
        auto it = (*obj)->find(std::u8string(key));
        if (it != (*obj)->end()) {
            return it->second;
        } else [[unlikely]] {
            return std::unexpected(json_errc::key_not_found);
        }
    } else {
        return std::unexpected(json_errc::not_a_container);
    }
}

}// namespace star::json
