#pragma once

import std;

namespace Star {

template <typename... T>
auto strf(std::format_string<T...> fmt, T&&... args) -> std::string;

template <typename... T>
auto vstrf(std::string_view fmt, T&&... args) -> std::string;

}
