#pragma once

#include <format>
#include <string>
#include <string_view>


namespace Star {

template <typename... T>
std::string strf(std::format_string<T...> fmt, T&&... args);

template <typename... T>
std::string vstrf(std::string_view fmt, T&&... args);

}
