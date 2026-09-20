#pragma once

#include "StarMemory.hpp"
#include "StarOutputProxy.hpp"

#include <string>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <concepts>
#include <source_location>

namespace Star {

// A format string that fmt cannot check at compile time: string literals are
// excluded (they take the checked overload directly), as are fmt's own
// format_string types (they are already checked), and Star::String is included
// through its utf8Ptr() rather than a conversion to string_view.
template <typename S>
struct IsFormatStringType : std::false_type {};
template <typename C, typename... A>
struct IsFormatStringType<fmt::basic_format_string<C, A...>> : std::true_type {};

template <typename S, typename = void>
struct HasUtf8Ptr : std::false_type {};
template <typename S>
struct HasUtf8Ptr<S, std::void_t<decltype(std::declval<S const&>().utf8Ptr())>> : std::true_type {};

// A format string that is only known at run time: string literals are excluded
// (they take the checked overload directly), as are fmt's own format_string
// types (they are already checked), and Star::String is included through its
// utf8Ptr() rather than a conversion to string_view.
template <typename S>
concept RuntimeFormatString = !std::is_array_v<std::remove_reference_t<S>>
    && !IsFormatStringType<std::remove_cv_t<std::remove_reference_t<S>>>::value
    && (std::is_convertible_v<S const&, std::string_view>
        || HasUtf8Ptr<std::remove_reference_t<S>>::value);

template <typename... T>
std::string strf(fmt::format_string<T...> fmt, T&&... args);

template <typename Char, typename... T>
std::string strfChecked(fmt::basic_format_string<Char, T...> fmt, T const&... args);

class StarException : public std::exception {
public:
  template <typename... Args>
  static StarException format(fmt::format_string<Args...> fmt, Args const&... args);
  template <typename S, typename... Args>
  requires RuntimeFormatString<S>
  static StarException format(S const& fmt, Args const&... args);

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

// The file and line of an assertion are a language construct now
// (std::source_location) instead of __FILE__/__LINE__ stringification.
// starAssert stays a macro only because a release build must not evaluate its
// condition at all, which a function call cannot express; everything else lives
// in these functions.
// The condition is a forwarding reference, not a bool: it has to go through the
// same contextual conversion the old macro's `if (COND)` performed, otherwise
// types with an explicit operator bool (std::unique_ptr, for instance) stop
// compiling.
template <typename T>
inline void starAssertFailed(T&& condition, std::source_location location = std::source_location::current()) {
#ifdef STAR_DEBUG
  if (condition)
    return;

  std::string const message = strf("assert failure in file {} line {}", location.file_name(), location.line());
  Star::fatalError(message.c_str(), true);
#else
  (void)condition;
  (void)location;
#endif
}

inline void debugPrintStack(std::source_location location = std::source_location::current()) {
#ifdef STAR_DEBUG
  // The message outlives the call: printStack may hand it on before returning.
  std::string const message = strf("Debug: file {} line {}", location.file_name(), location.line());
  Star::printStack(message.c_str());
#else
  (void)location;
#endif
}

#ifdef STAR_DEBUG
#define starAssert(COND) Star::starAssertFailed((COND), std::source_location::current())
#else
#define starAssert(COND) ((void)0)
#endif

// An exception class used to be generated by the STAR_EXCEPTION macro.  It is a
// template now, so that a module could export it.  The tag names one exception
// (with the alias below every one stays a distinct type, exactly as a generated
// class was) and supplies the type string that #ClassName provided.
template <typename Tag, typename Base>
class StarError : public Base {
public:
  template <typename... Args>
  static StarError format(fmt::format_string<Args...> fmt, Args const&... args) {
    return StarError(strfChecked(fmt, args...));
  }

  template <typename S, typename... Args>
    requires RuntimeFormatString<S>
  static StarError format(S const& fmt, Args const&... args) {
    return StarError(strf(fmt, args...));
  }

  StarError() : Base(Tag::name(), std::string()) {}
  explicit StarError(std::string message, bool genStackTrace = true)
    : Base(Tag::name(), std::move(message), genStackTrace) {}
  explicit StarError(std::exception const& cause) : Base(Tag::name(), std::string(), cause) {}
  StarError(std::string message, std::exception const& cause) : Base(Tag::name(), std::move(message), cause) {}

protected:
  StarError(char const* type, std::string message, bool genStackTrace = true)
    : Base(type, std::move(message), genStackTrace) {}
  StarError(char const* type, std::string message, std::exception const& cause)
    : Base(type, std::move(message), cause) {}
};

struct OutOfRangeExceptionTag {
  static constexpr char const* name() { return "OutOfRangeException"; }
};
using OutOfRangeException = StarError<OutOfRangeExceptionTag, StarException>;
struct IOExceptionTag {
  static constexpr char const* name() { return "IOException"; }
};
using IOException = StarError<IOExceptionTag, StarException>;
struct MemoryExceptionTag {
  static constexpr char const* name() { return "MemoryException"; }
};
using MemoryException = StarError<MemoryExceptionTag, StarException>;

template <typename... Args>
StarException StarException::format(fmt::format_string<Args...> fmt, Args const&... args) {
  return StarException(strfChecked(fmt, args...));
}

template <typename S, typename... Args>
  requires RuntimeFormatString<S>
StarException StarException::format(S const& fmt, Args const&... args) {
  return StarException(strf(fmt, args...));
}

}
