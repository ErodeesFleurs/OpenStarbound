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

  // Formats an already checked format string.  The string is taken as a view so
  // that the arguments are the only thing the template parameters are deduced
  // from; a fmt::format_string parameter cannot be deduced from, and the
  // consteval conversion cannot be redone once the literal is behind a
  // function parameter.
  template <typename Char, typename... T>
  std::string vformatChecked(fmt::basic_string_view<Char> fmt, T const&... args) {
    try {
      return fmt::vformat(fmt, fmt::make_format_args(args...));
    } catch (std::exception const& e) {
      throw FormatException(fmt::format("Exception thrown during string format: {}", e.what()));
    }
  }
}

// Compile time checked: these take fmt::format_string directly, so the
// conversion (and with it the check) happens at the call site where the literal
// still is a constant expression.  A literal cannot be forwarded through a
// template parameter and checked afterwards, so the bodies below only pass the
// resulting view on.
template <typename... T>
std::string strf(fmt::format_string<T...> fmt, T&&... args) {
  return Detail::vformatChecked(fmt::string_view(fmt), args...);
}

// An already checked format string, e.g. forwarded from a wrapper: the argument
// list is the one the string was checked with.
template <typename Char, typename... T>
std::string strfChecked(fmt::basic_format_string<Char, T...> fmt, T const&... args) {
  return Detail::vformatChecked(fmt::basic_string_view<Char>(fmt), args...);
}

// Runtime format strings: char pointers, std::string, std::string_view and
// friends.  String literals are excluded here, they take the overload above.
template <typename S,
    typename... T>
  requires RuntimeFormatString<S>
std::string strf(S const& fmt, T&&... args) {
  return Detail::formatRuntime(fmt, std::forward<T>(args)...);
}

template <typename... T>
void format(std::ostream& out, fmt::format_string<T...> fmt, T&&... args) {
  out << Detail::vformatChecked(fmt::string_view(fmt), args...);
}

template <typename S,
    typename... T>
  requires RuntimeFormatString<S>
void format(std::ostream& out, S const& fmt, T&&... args) {
  out << Detail::formatRuntime(fmt, args...);
}

// Automatically flushes, use format to avoid flushing.
template <typename... T>
void coutf(fmt::format_string<T...> fmt, T&&... args) {
  std::cout << Detail::vformatChecked(fmt::string_view(fmt), args...);
  std::cout.flush();
}

template <typename S,
    typename... T>
  requires RuntimeFormatString<S>
void coutf(S const& fmt, T&&... args) {
  format(std::cout, fmt, args...);
  std::cout.flush();
}

// Automatically flushes, use format to avoid flushing.
template <typename... T>
void cerrf(fmt::format_string<T...> fmt, T&&... args) {
  std::cerr << Detail::vformatChecked(fmt::string_view(fmt), args...);
  std::cerr.flush();
}

template <typename S,
    typename... T>
  requires RuntimeFormatString<S>
void cerrf(S const& fmt, T&&... args) {
  format(std::cerr, fmt, args...);
  std::cerr.flush();
}

template <class Type>
inline std::string toString(Type const& t) {
  return fmt::to_string(t);
}

}
