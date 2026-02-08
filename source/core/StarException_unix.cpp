#include "StarException.hpp"

#include "StarCasting.hpp"
#include "StarLogging.hpp"

#include <execinfo.h>

#ifdef STAR_USE_CPPTRACE
#include "cpptrace/cpptrace.hpp"
#include "cpptrace/formatting.hpp"
#endif

import std;

namespace Star {
using std::bind;

#ifdef STAR_USE_CPPTRACE

static inline std::string captureBacktrace() {
  auto formatter = cpptrace::formatter{}
                     .paths(cpptrace::formatter::path_mode::basename);
  std::ostringstream out;
  formatter.print(out, cpptrace::generate_trace());
  return out.str();
}

#endif

static std::size_t const StackLimit = 256;

using StackCapture = std::pair<Array<void*, StackLimit>, std::size_t>;

inline auto captureStack() -> StackCapture {
  StackCapture stackCapture;
  stackCapture.second = backtrace(stackCapture.first.ptr(), StackLimit);
  return stackCapture;
}

auto outputStack(StackCapture stack) -> OutputProxy {
  return {[stack = std::move(stack)](std::ostream& os) -> void {
    char** symbols = backtrace_symbols(stack.first.ptr(), stack.second);
    for (std::size_t i = 0; i < stack.second; ++i) {
      os << symbols[i];
      if (i + 1 < stack.second)
        os << std::endl;
    }

    if (stack.second == StackLimit)
      os << std::endl
         << "[Stack Output Limit Reached]";

    std::free(symbols);
  }};
}

StarException::StarException() noexcept
    : StarException(std::string("StarException")) {}

StarException::~StarException() noexcept = default;

StarException::StarException(std::string message, bool genStackTrace) noexcept
    : StarException("StarException", std::move(message), genStackTrace) {}

StarException::StarException(std::exception const& cause) noexcept
    : StarException("StarException", std::string(), cause) {}

StarException::StarException(std::string message, std::exception const& cause) noexcept
    : StarException("StarException", std::move(message), cause) {}

auto StarException::what() const noexcept -> const char* {
  if (m_whatBuffer.empty()) {
    std::ostringstream os;
    m_printException(os, false);
    m_whatBuffer = os.str();
  }
  return m_whatBuffer.c_str();
}

StarException::StarException(char const* type, std::string message, bool genStackTrace) noexcept {
#ifdef STAR_USE_CPPTRACE
  auto printException = [](std::ostream& os, bool fullStacktrace, char const* type, std::string message, std::string stack) {
#else
  auto printException = [](std::ostream& os, bool fullStacktrace, char const* type, std::string message, std::optional<StackCapture> stack) -> void {
#endif
    os << "(" << type << ")";
    if (!message.empty())
      os << " " << message;

#ifdef STAR_USE_CPPTRACE
    if (fullStacktrace && !stack.empty()) {
      os << std::endl;
      os << stack;
    }
#else
    if (fullStacktrace && stack) {
      os << std::endl;
      os << outputStack(*stack);
    }
#endif
  };
#ifdef STAR_USE_CPPTRACE
  m_printException = bind(printException, _1, _2, type, std::move(message), genStackTrace ? captureBacktrace() : std::string());
#else
  m_printException = [printException, type, capture0 = std::move(message), capture1 = genStackTrace ? captureStack() : std::optional<StackCapture>()](auto&& PH1, auto&& PH2) -> auto { printException(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), type, capture0, capture1); };
#endif
}

StarException::StarException(char const* type, std::string message, std::exception const& cause) noexcept
    : StarException(type, std::move(message)) {
  auto printException = [](std::ostream& os, bool fullStacktrace, std::function<void(std::ostream&, bool)> self, std::function<void(std::ostream&, bool)> cause) -> void {
    self(os, fullStacktrace);
    os << std::endl
       << "Caused by: ";
    cause(os, fullStacktrace);
  };

  std::function<void(std::ostream&, bool)> printCause;
  if (auto starException = as<StarException>(&cause)) {
    printCause = bind(starException->m_printException, std::placeholders::_1, std::placeholders::_2);
  } else {
    printCause = bind([](std::ostream& os, bool, std::string causeWhat) -> void {
      os << "std::exception: " << causeWhat;
    },
                      std::placeholders::_1, std::placeholders::_2, std::string(cause.what()));
  }

  m_printException = [printException, this, capture0 = std::move(printCause)](auto&& PH1, auto&& PH2) -> auto { printException(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), m_printException, capture0); };
}

auto printException(std::exception const& e, bool fullStacktrace) -> std::string {
  std::ostringstream os;
  printException(os, e, fullStacktrace);
  return os.str();
}

void printException(std::ostream& os, std::exception const& e, bool fullStacktrace) {
  if (auto starException = as<StarException>(&e))
    starException->m_printException(os, fullStacktrace);
  else
    os << "std::exception: " << e.what();
}

auto outputException(std::exception const& e, bool fullStacktrace) -> OutputProxy {
  if (auto starException = as<StarException>(&e))
    return {bind(starException->m_printException, std::placeholders::_1, fullStacktrace)};
  else
    return {bind([](std::ostream& os, std::string what) -> void { os << "std::exception: " << what; }, std::placeholders::_1, std::string(e.what()))};
}

void printStack(char const* message) {
#ifdef STAR_USE_CPPTRACE
  Logger::info("Stack Trace ({})...\n{}", message, captureBacktrace());
#else
  Logger::info("Stack Trace ({})...\n{}", message, outputStack(captureStack()));
#endif
}

void fatalError(char const* message, bool showStackTrace) {
  if (showStackTrace)
#ifdef STAR_USE_CPPTRACE
    Logger::error("Fatal Error: {}\n{}", message, captureBacktrace());
#else
    Logger::error("Fatal Error: {}\n{}", message, outputStack(captureStack()));
#endif
  else
    Logger::error("Fatal Error: {}", message);

  std::abort();
}

void fatalException(std::exception const& e, bool showStackTrace) {
  if (showStackTrace)
#ifdef STAR_USE_CPPTRACE
    Logger::error("Fatal Exception caught: {}\nCaught at:\n{}", outputException(e, true), captureBacktrace());
#else
    Logger::error("Fatal Exception caught: {}\nCaught at:\n{}", outputException(e, true), outputStack(captureStack()));
#endif
  else
    Logger::error("Fatal Exception caught: {}", outputException(e, showStackTrace));

  std::abort();
}

}// namespace Star
