#pragma once

#include "StarMemory.hpp"
#include "StarException.hpp"

#include "fmt/core.h"
#include "fmt/ostream.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include <string_view>

namespace Star {

STAR_EXCEPTION(FormatException, StarException);

namespace Detail {
  // Runtime format strings: fmt cannot check those at compile time, so they are
  // handed to fmt::runtime explicitly.
  template <typename S>
  std::string_view formatStringView(S const& fmt) {
    if constexpr (HasUtf8Ptr<std::remove_reference_t<S>>::value)
      return fmt.utf8();
    else
      return fmt;
  }

  template <typename S, typename... T>
  std::string formatRuntime(S const& fmt, T&&... args) {
    try {
      return fmt::format(fmt::runtime(formatStringView(fmt)), std::forward<T>(args)...);
    } catch (std::exception const& e) {
      throw FormatException(fmt::format("Exception thrown during string format: {}", e.what()));
    }
  }
}

// Compile time checked: this takes fmt::format_string directly, so the
// conversion (and with it the check) happens at the call site where the literal
// still is a constant expression.  A literal cannot be forwarded through a
// template parameter and checked afterwards.
template <typename... T>
std::string strf(fmt::format_string<T...> fmt, T&&... args) {
  try {
    return fmt::format(fmt, std::forward<T>(args)...);
  } catch (std::exception const& e) {
    throw FormatException(Detail::formatRuntime("Exception thrown during string format: {}", e.what()));
  }
}

// An already checked format string: the argument types have to match it exactly.
// Used where a wrapper forwards the format_string it received; passing it through
// the overload above would redo the conversion, which cannot be a constant
// expression at that point.
template <typename Char, typename... T>
std::string strfChecked(fmt::basic_format_string<Char, T...> fmt, T const&... args) {
  try {
    return fmt::format(fmt, args...);
  } catch (std::exception const& e) {
    throw FormatException(Detail::formatRuntime("Exception thrown during string format: {}", e.what()));
  }
}

// Runtime format strings: char pointers, std::string, std::string_view and
// friends.  String literals are excluded here, they take the overload above.
template <typename S,
    typename... T,
    std::enable_if_t<isRuntimeFormatStringOrString<S>, int> = 0>
std::string strf(S const& fmt, T&&... args) {
  return Detail::formatRuntime(fmt, std::forward<T>(args)...);
}

template <typename... T>
void format(std::ostream& out, fmt::format_string<T...> fmt, T&&... args) {
  out << strf(fmt, args...);
}

template <typename S,
    typename... T,
    std::enable_if_t<isRuntimeFormatStringOrString<S>, int> = 0>
void format(std::ostream& out, S const& fmt, T&&... args) {
  out << Detail::formatRuntime(fmt, args...);
}

// Automatically flushes, use format to avoid flushing.
template <typename... T>
void coutf(fmt::format_string<T...> fmt, T&&... args) {
  format(std::cout, fmt, args...);
  std::cout.flush();
}

template <typename S,
    typename... T,
    std::enable_if_t<isRuntimeFormatStringOrString<S>, int> = 0>
void coutf(S const& fmt, T&&... args) {
  format(std::cout, fmt, args...);
  std::cout.flush();
}

// Automatically flushes, use format to avoid flushing.
template <typename... T>
void cerrf(fmt::format_string<T...> fmt, T&&... args) {
  format(std::cerr, fmt, args...);
  std::cerr.flush();
}

template <typename S,
    typename... T,
    std::enable_if_t<isRuntimeFormatStringOrString<S>, int> = 0>
void cerrf(S const& fmt, T&&... args) {
  format(std::cerr, fmt, args...);
  std::cerr.flush();
}

template <class Type>
inline std::string toString(Type const& t) {
  return fmt::to_string(t);
}

}
