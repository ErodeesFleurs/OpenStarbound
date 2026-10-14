#pragma once
import std;

import star.memory;
#include "StarException.hpp"

namespace Star {

struct FormatExceptionTag {
  static constexpr char const* name() { return "FormatException"; }
};
using FormatException = StarError<FormatExceptionTag, StarException>;

namespace Detail {
  template <typename S>
  std::string_view formatStringView(S const& fmt) {
    if constexpr (HasUtf8Ptr<std::remove_reference_t<S>>::value)
      return fmt.utf8();
    else
      return fmt;
  }

  // Named arguments remain lvalues for make_format_args; the argument store
  // borrows them only for the synchronous vformat call.
  template <typename S, typename... T>
  std::string formatRuntime(S const& fmt, T&&... args) {
    try {
      return std::vformat(formatStringView(fmt), std::make_format_args(args...));
    } catch (std::exception const& e) {
      throw FormatException(std::format("Exception thrown during string format: {}", e.what()));
    }
  }

  template <typename... T>
  std::string vformatChecked(std::string_view fmt, T const&... args) {
    try {
      return std::vformat(fmt, std::make_format_args(args...));
    } catch (std::exception const& e) {
      throw FormatException(std::format("Exception thrown during string format: {}", e.what()));
    }
  }
}

// Consteval validation happens at the caller, before a literal is forwarded.
template <typename... T>
std::string strf(std::format_string<T...> fmt, T&&... args) {
  return Detail::vformatChecked(fmt.get(), args...);
}

// Keep the argument types used to check a wrapper's format string.
template <typename Char, typename... T>
std::string strfChecked(std::basic_format_string<Char, T...> fmt, T const&... args) {
  return Detail::vformatChecked(fmt.get(), args...);
}

// Runtime strings exclude literals and already checked format strings.
template <typename S, typename... T>
  requires RuntimeFormatString<S>
std::string strf(S const& fmt, T&&... args) {
  return Detail::formatRuntime(fmt, std::forward<T>(args)...);
}

template <typename... T>
void format(std::ostream& out, std::format_string<T...> fmt, T&&... args) {
  out << Detail::vformatChecked(fmt.get(), args...);
}

template <typename S, typename... T>
  requires RuntimeFormatString<S>
void format(std::ostream& out, S const& fmt, T&&... args) {
  out << Detail::formatRuntime(fmt, args...);
}

// Automatically flushes; use format to avoid flushing.
template <typename... T>
void coutf(std::format_string<T...> fmt, T&&... args) {
  std::cout << Detail::vformatChecked(fmt.get(), args...);
  std::cout.flush();
}

template <typename S, typename... T>
  requires RuntimeFormatString<S>
void coutf(S const& fmt, T&&... args) {
  format(std::cout, fmt, args...);
  std::cout.flush();
}

// Automatically flushes; use format to avoid flushing.
template <typename... T>
void cerrf(std::format_string<T...> fmt, T&&... args) {
  std::cerr << Detail::vformatChecked(fmt.get(), args...);
  std::cerr.flush();
}

template <typename S, typename... T>
  requires RuntimeFormatString<S>
void cerrf(S const& fmt, T&&... args) {
  format(std::cerr, fmt, args...);
  std::cerr.flush();
}

template <class Type>
inline std::string toString(Type const& t) {
  return std::format("{}", t);
}

}
