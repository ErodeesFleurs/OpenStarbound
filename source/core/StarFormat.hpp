#pragma once

#include "StarException.hpp"
#include "StarStrf.hpp"

#include <format>
#include <iostream>
#include <ostream>
#include <print>
#include <string>
#include <utility>

namespace Star {

STAR_EXCEPTION(FormatException, StarException);

template <typename... T>
std::string strf(std::format_string<T...> fmt, T&&... args) {
  try {
    return std::format(fmt, std::forward<T>(args)...);
  } catch (std::exception const& e) {
    throw FormatException(std::format("Exception thrown during string format: {}", e.what()));
  }
}

template <typename... T>
std::string vstrf(std::string_view fmt, T&&... args) {
  try {
    return std::vformat(fmt, std::make_format_args(args...));
  } catch (std::exception const& e) {
    throw FormatException(std::format("Exception thrown during runtime string format: {}", e.what()));
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
    throw FormatException(std::format("Exception thrown during runtime string format: {}", e.what()));
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
    throw FormatException(std::format("Exception thrown during runtime string format: {}", e.what()));
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
    throw FormatException(std::format("Exception thrown during runtime string format: {}", e.what()));
  }
}

template <class Type>
inline std::string toString(Type const& t) {
  return strf("{}", t);
}

}
