#pragma once

#include "StarOstreamFormatter.hpp"

import std;

namespace Star {

template <typename... T>
auto strf(std::format_string<T...> fmt, T&&... args) -> std::string {
  try {
    return std::format(fmt, std::forward<T>(args)...);
  } catch (std::exception const& e) {
    throw std::format_error(std::format("Exception thrown during compile-time string format: {}", e.what()));
  }
}

template <typename... T>
auto vstrf(std::string_view fmt, T&&... args) -> std::string {
  try {
    return std::vformat(fmt, std::make_format_args(args...));
  } catch (std::exception const& e) {
    throw std::format_error(std::format("Exception thrown during runtime string format: {}", e.what()));
  }
}

template <typename... T>
void format(std::ostream& out, std::format_string<T...> fmt, T&&... args) {
  std::print(out, fmt, std::forward<T>(args)...);
}

template <typename... T>
void vformat(std::ostream& out, std::string_view fmt, T&&... args) {
  try {
    std::vprint_unicode(out, fmt, std::make_format_args(args...));
  } catch (std::exception const& e) {
    throw std::format_error(std::format("Exception thrown during runtime string format: {}", e.what()));
  }
}

// Automatically flushes, use format to avoid flushing.
template <typename... Args>
void coutf(std::format_string<Args...> fmt, Args&&... args) {
  std::print(std::cout, fmt, std::forward<Args>(args)...);
  std::cout.flush();
}

// Automatically flushes, use format to avoid flushing.
template <typename... Args>
void vcoutf(std::string_view fmt, Args&&... args) {
  try {
    std::vprint_unicode(std::cout, fmt, std::make_format_args(args...));
    std::cout.flush();
  } catch (std::exception const& e) {
    throw std::format_error(std::format("Exception thrown during runtime string format: {}", e.what()));
  }
}

// Automatically flushes, use format to avoid flushing.
template <typename... Args>
void cerrf(std::format_string<Args...> fmt, Args&&... args) {
  std::print(std::cerr, fmt, std::forward<Args>(args)...);
  std::cerr.flush();
}

// Automatically flushes, use format to avoid flushing.
template <typename... Args>
void vcerrf(std::string_view fmt, Args&&... args) {
  try {
    std::vprint_unicode(std::cerr, fmt, std::make_format_args(args...));
    std::cerr.flush();
  } catch (std::exception const& e) {
    throw std::format_error(std::format("Exception thrown during runtime string format: {}", e.what()));
  }
}

template <class Type>
inline auto toString(Type const& t) -> std::string {
  return strf("{}", t);
}

}


template <typename T>
struct std::formatter<std::optional<T>> : Star::ostream_formatter {};
