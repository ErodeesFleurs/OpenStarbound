module star.json.builder;

import std;
import star.json;
import star.unicode;

import std;

namespace star::json {

void json_builder_stream::place_value(json_value&& value) {
    if (m_stack.empty()) {
        m_stack.emplace_back(std::move(value));
        return;
    }
    std::optional<json_value>& top_opt = m_stack.back();

    if (!top_opt.has_value()) {
        top_opt = std::move(value);
        return;
    }

    json_value& top = *top_opt;

    if (top.is<json_array, true>()) {
        auto arr_res = top.as<json_array, true>();
        if (arr_res) {
            arr_res.value()->push_back(std::move(value));
            return;
        }
    }
    if (top.is<json_object, true>() && m_current_key.has_value()) {
        auto obj_res = top.as<json_object, true>();
        if (obj_res) {
            (**obj_res)[std::move(*m_current_key)] = std::move(value);
            m_current_key.reset();
            return;
        }
    }
}

void json_builder_stream::begin_object() {
    m_stack.emplace_back(std::nullopt);
    m_current_key.reset();
}

void json_builder_stream::object_key(std::u32string_view key) {
    if (m_stack.empty() || m_stack.back().has_value()) {
        m_stack.emplace_back(std::nullopt);
    }
    m_stack.back() = json_value::mutable_object();
    m_current_key = unicode::utf32_to_utf8(key).value_or(std::u8string{});
}

void json_builder_stream::end_object() {
    if (m_stack.empty()) {
        return;
    }
    auto top_opt = std::move(m_stack.back());
    m_stack.pop_back();

    if (!top_opt.has_value()) {
        place_value(json_value::object());
        return;
    }

    json_value& top = *top_opt;
    if (!top.is<json_object, true>()) {
        return;
    }

    auto frozen = std::move(top).freeze();
    if (!frozen) {
        return;
    }
    place_value(std::move(*frozen));
    m_current_key.reset();
}

void json_builder_stream::begin_array() {
    m_stack.emplace_back(std::nullopt);
    m_current_key.reset();
}

void json_builder_stream::end_array() {
    if (m_stack.empty()) {
        return;
    }
    auto top_opt = std::move(m_stack.back());
    m_stack.pop_back();

    if (!top_opt.has_value()) {
        place_value(json_value::array());
        return;
    }

    json_value& top = *top_opt;
    if (!top.is<json_array, true>()) {
        return;
    }

    auto frozen = std::move(top).freeze();
    if (!frozen) {
        return;
    }
    place_value(std::move(*frozen));
    m_current_key.reset();
}

void json_builder_stream::put_string(std::u32string_view s) {
    place_value(json_value::string(unicode::utf32_to_utf8(s).value_or(std::u8string{})));
}

void json_builder_stream::put_double(std::double_t value) {
    place_value(json_value::floating(value));
}

void json_builder_stream::put_integer(std::int64_t value) {
    place_value(json_value::integer(value));
}

void json_builder_stream::put_boolean(bool value) { place_value(json_value::boolean(value)); }

void json_builder_stream::put_null() { place_value(json_value::null()); }

auto json_builder_stream::stack_size() const noexcept -> std::size_t { return m_stack.size(); }

auto json_builder_stream::take_top() -> std::optional<json_value> {
    if (m_stack.size() != 1) {
        return std::nullopt;
    }
    auto& top_opt = m_stack.back();
    if (!top_opt.has_value()) {
        return std::nullopt;
    }
    auto top = std::move(*top_opt);
    m_stack.pop_back();

    if (top.is<json_array, true>() || top.is<json_object, true>()) {
        auto frozen = std::move(top).freeze();
        if (!frozen) {
            return std::nullopt;
        }
        return std::move(*frozen);
    }
    return std::move(top);
}

void json_streamer<json_value>::to_stream(const json_value& val, json_stream& stream,
                                          bool sort_keys) {
    val.visit(overload{[&](std::nullptr_t) -> void { stream.put_null(); },
                       [&](bool b) -> void { stream.put_boolean(b); },
                       [&](std::int64_t i) -> void { stream.put_integer(i); },
                       [&](std::double_t d) -> void { stream.put_double(d); },
                       [&](const std::shared_ptr<const std::u8string>& s) -> void {
                           stream.put_string(unicode::utf8_to_utf32(*s).value_or(std::u32string{}));
                       },
                       [&](const std::shared_ptr<const json_array>& arr_ptr) -> void {
                           stream.begin_array();
                           bool first = true;
                           for (const auto& elem : *arr_ptr) {
                               if (first) {
                                   first = false;
                               } else {
                                   stream.put_comma();
                               }
                               to_stream(elem, stream, sort_keys);
                           }
                           stream.end_array();
                       },
                       [&](const std::shared_ptr<const json_object>& obj_ptr) -> void {
                           stream.begin_object();
                           const auto& obj = *obj_ptr;
                           std::vector<std::u8string_view> keys;
                           keys.reserve(obj.size());
                           for (const auto& [k, v] : obj) {
                               keys.emplace_back(k);
                           }
                           if (sort_keys) {
                               std::ranges::sort(keys);
                           }
                           bool first = true;
                           for (auto k : keys) {
                               if (first) {
                                   first = false;
                               } else {
                                   stream.put_comma();
                               }
                               stream.object_key(
                                 unicode::utf8_to_utf32(k).value_or(std::u32string{}));
                               to_stream(obj.at(std::u8string{k}), stream, sort_keys);
                           }
                           stream.end_object();
                       },
                       [&](const std::shared_ptr<json_array>&) -> void {
                           auto frozen = val.freeze();
                           if (frozen) {
                               to_stream(*frozen, stream, sort_keys);
                           }
                       },
                       [&](const std::shared_ptr<json_object>&) -> void {
                           auto frozen = val.freeze();
                           if (frozen) {
                               to_stream(*frozen, stream, sort_keys);
                           }
                       }});
}

}// namespace star::json
