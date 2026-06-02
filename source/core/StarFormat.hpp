#pragma once

#include "StarMemory.hpp"
#include "StarException.hpp"

#include <string_view>

namespace Star {

STAR_EXCEPTION(FormatException, StarException);

template <typename... T>
std::string strf(std::string_view fmt, T&&... args) {
  try { return std::vformat(fmt, std::make_format_args(args...)); }
  catch (std::exception const& e) {
    throw FormatException(std::string("Exception thrown during string format: ") + e.what());
  }
}

template <typename... T>
void format(std::ostream& out, std::string_view fmt, T&&... args) {
  out << strf(fmt, args...);
}

// Automatically flushes, use format to avoid flushing.
template <typename... Args>
void coutf(char const* fmt, Args const&... args) {
  format(std::cout, fmt, args...);
  std::cout.flush();
}

// Automatically flushes, use format to avoid flushing.
template <typename... Args>
void cerrf(char const* fmt, Args const&... args) {
  format(std::cerr, fmt, args...);
  std::cerr.flush();
}

template <class Type>
inline std::string toString(Type const& t) {
  std::ostringstream os;
  os << t;
  return os.str();
}

}

template <typename T>
struct std::formatter<Star::OutputAnyDetail::Wrapper<T>> : Star::OstreamFormatter {};
template <> struct std::formatter<Star::OutputProxy> : Star::OstreamFormatter {};
