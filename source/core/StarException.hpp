#pragma once

#include "StarMemory.hpp"
#include "StarOutputProxy.hpp"

#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <sstream>

namespace Star {

class String;

template <typename... T>
std::string strf(std::string_view fmt, T&&... args);

struct OstreamFormatter {
  std::formatter<std::string> formatter;

  constexpr auto parse(std::format_parse_context& ctx) {
    return formatter.parse(ctx);
  }

  template <typename Type, typename FormatContext>
  auto format(Type const& value, FormatContext& ctx) const {
    std::ostringstream os;
    os << value;
    return formatter.format(os.str(), ctx);
  }
};

class StarException : public std::exception {
public:
  template <typename... Args>
  static StarException format(std::string_view fmt, Args const&... args);

  StarException() noexcept;
  virtual ~StarException() noexcept;

  explicit StarException(std::string message, bool genStackTrace = true) noexcept;
  explicit StarException(std::exception const& cause) noexcept;
  StarException(std::string message, std::exception const& cause) noexcept;

  virtual char const* what() const noexcept override;

  // If the given exception is really StarException, then this will call
  // StarException::printException, otherwise just prints std::exception::what.
  friend void printException(std::ostream& os, std::exception const& e, bool fullStacktrace);
  friend std::string printException(std::exception const& e, bool fullStacktrace);
  friend OutputProxy outputException(std::exception const& e, bool fullStacktrace);

protected:
  StarException(char const* type, std::string message, bool genStackTrace = true) noexcept;
  StarException(char const* type, std::string message, std::exception const& cause) noexcept;

private:
  // Takes the ostream to print to, whether to print the full stacktrace.  Must
  // not bind 'this', may outlive the exception in the case of chained
  // exception causes.
  function<void(std::ostream&, bool)> m_printException;

  // m_printException will be called without the stack-trace to print
  // m_whatBuffer, if the what() method is invoked.
  mutable std::string m_whatBuffer;
};

void printException(std::ostream& os, std::exception const& e, bool fullStacktrace);
std::string printException(std::exception const& e, bool fullStacktrace);
OutputProxy outputException(std::exception const& e, bool fullStacktrace);

void printStack(char const* message);

// Log error and stack-trace and possibly show a dialog box if available, then
// abort.
void fatalError(char const* message, bool showStackTrace);
void fatalException(std::exception const& e, bool showStackTrace);

#ifdef STAR_DEBUG
inline void debugPrintStack(std::source_location location = std::source_location::current()) {
  auto message = strf("Debug: file {} line {}", location.file_name(), location.line());
  Star::printStack(message.c_str());
}
#else
inline void debugPrintStack() {
}
#endif

inline void assertionFailure(std::source_location location = std::source_location::current()) {
  auto message = strf("assert failure in file {} line {}", location.file_name(), location.line());
  Star::fatalError(message.c_str(), true);
}

#ifdef STAR_DEBUG
#define starAssert(COND)                                                                                \
  {                                                                                                     \
    if (COND)                                                                                           \
      ;                                                                                                 \
    else                                                                                                \
      Star::assertionFailure(std::source_location::current());                                           \
  }
#else
#define starAssert(COND) \
  {}
#endif

template <typename BaseName, typename Tag>
class TypedException : public BaseName {
public:
  template <typename... Args>
  static TypedException format(std::string_view fmt, Args const&... args) {
    return TypedException(strf(fmt, args...));
  }

  TypedException()
    : BaseName(Tag::typeName, std::string()) {}

  explicit TypedException(std::string message, bool genStackTrace = true)
    : BaseName(Tag::typeName, std::move(message), genStackTrace) {}

  explicit TypedException(std::exception const& cause)
    : BaseName(Tag::typeName, std::string(), cause) {}

  TypedException(std::string message, std::exception const& cause)
    : BaseName(Tag::typeName, std::move(message), cause) {}

protected:
  TypedException(char const* type, std::string message, bool genStackTrace = true)
    : BaseName(type, std::move(message), genStackTrace) {}

  TypedException(char const* type, std::string message, std::exception const& cause)
    : BaseName(type, std::move(message), cause) {}
};

struct OutOfRangeExceptionTag { static constexpr char const* typeName = "OutOfRangeException"; };
using OutOfRangeException = TypedException<StarException, OutOfRangeExceptionTag>;

struct IOExceptionTag { static constexpr char const* typeName = "IOException"; };
using IOException = TypedException<StarException, IOExceptionTag>;

struct MemoryExceptionTag { static constexpr char const* typeName = "MemoryException"; };
using MemoryException = TypedException<StarException, MemoryExceptionTag>;

template <typename... Args>
StarException StarException::format(std::string_view fmt, Args const&... args) {
  return StarException(strf(fmt, args...));
}

}

template <> struct std::formatter<Star::String> : Star::OstreamFormatter {};
