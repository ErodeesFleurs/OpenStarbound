#pragma once

#include "StarMemory.hpp"
#include "StarOutputProxy.hpp"

#include <string>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <concepts>

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

#ifdef STAR_DEBUG
#define debugPrintStack() \
  { Star::printStack("Debug: file " STAR_STR(__FILE__) " line " STAR_STR(__LINE__)); }
#define starAssert(COND)                                                                                \
  {                                                                                                     \
    if (COND)                                                                                           \
      ;                                                                                                 \
    else                                                                                                \
      Star::fatalError("assert failure in file " STAR_STR(__FILE__) " line " STAR_STR(__LINE__), true); \
  }
#else
#define debugPrintStack() \
  {}
#define starAssert(COND) \
  {}
#endif

#define STAR_EXCEPTION(ClassName, BaseName)                                                                                       \
  class ClassName : public BaseName {                                                                                             \
  public:                                                                                                                         \
    template <typename... Args>                                                                                                   \
    static ClassName format(fmt::format_string<Args...> fmt, Args const&... args) {                                               \
      return ClassName(strfChecked(fmt, args...));                                                                                \
    }                                                                                                                             \
    template <typename S, typename... Args> requires RuntimeFormatString<S>                          \
    static ClassName format(S const& fmt, Args const&... args) {                                                                  \
      return ClassName(strf(fmt, args...));                                                                                       \
    }                                                                                                                             \
    ClassName() : BaseName(#ClassName, std::string()) {}                                                                          \
    explicit ClassName(std::string message, bool genStackTrace = true) : BaseName(#ClassName, std::move(message), genStackTrace) {} \
    explicit ClassName(std::exception const& cause) : BaseName(#ClassName, std::string(), cause) {}                               \
    ClassName(std::string message, std::exception const& cause) : BaseName(#ClassName, std::move(message), cause) {}              \
                                                                                                                                  \
  protected:                                                                                                                      \
    ClassName(char const* type, std::string message, bool genStackTrace = true) : BaseName(type, std::move(message), genStackTrace) {} \
    ClassName(char const* type, std::string message, std::exception const& cause)                                                 \
      : BaseName(type, std::move(message), cause) {}                                                                              \
  }

STAR_EXCEPTION(OutOfRangeException, StarException);
STAR_EXCEPTION(IOException, StarException);
STAR_EXCEPTION(MemoryException, StarException);

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
