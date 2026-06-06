#include "StarException.hpp"
#include "StarCasting.hpp"
#include "StarLogging.hpp"

#include <execinfo.h>
#include <cstdlib>
#ifdef STAR_USE_CPPTRACE
#include "cpptrace/cpptrace.hpp"
#include "cpptrace/formatting.hpp"
#endif

namespace Star {

#ifdef STAR_USE_CPPTRACE

static inline std::string captureBacktrace() {
  auto formatter = cpptrace::formatter{}
    .paths(cpptrace::formatter::path_mode::basename);
  std::ostringstream out;
  formatter.print(out, cpptrace::generate_trace());
  return out.str();
}
  
#endif

static size_t const StackLimit = 256;

using StackCapture = pair<Array<void*, StackLimit>, size_t>;

inline StackCapture captureStack() {
  StackCapture stackCapture;
  stackCapture.second = backtrace(stackCapture.first.ptr(), StackLimit);
  return stackCapture;
}

OutputProxy outputStack(StackCapture stack) {
  return OutputProxy([stack = std::move(stack)](std::ostream & os) {
      char** symbols = backtrace_symbols(stack.first.ptr(), stack.second);
      for (size_t i = 0; i < stack.second; ++i) {
        os << symbols[i];
        if (i + 1 < stack.second)
          os << std::endl;
      }

      if (stack.second == StackLimit)
        os << std::endl << "[Stack Output Limit Reached]";

      ::free(symbols);
    });
}

StarException::StarException() noexcept
  : StarException(std::string("StarException")) {}

StarException::~StarException() noexcept {}

StarException::StarException(std::string message, bool genStackTrace) noexcept
  : StarException("StarException", std::move(message), genStackTrace) {}

StarException::StarException(std::exception const& cause) noexcept
  : StarException("StarException", std::string(), cause) {}

StarException::StarException(std::string message, std::exception const& cause) noexcept
  : StarException("StarException", std::move(message), cause) {}

char const* StarException::what() const noexcept {
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
  auto printException = [](std::ostream& os, bool fullStacktrace, char const* type, std::string message, Maybe<StackCapture> stack) {
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
  m_printException = [printException, type, message = std::move(message), stack = genStackTrace ? captureBacktrace() : std::string()](std::ostream& os, bool fullStacktrace) mutable {
    printException(os, fullStacktrace, type, std::move(message), std::move(stack));
  };
#else
  m_printException = [printException, type, message = std::move(message), stack = genStackTrace ? captureStack() : Maybe<StackCapture>()](std::ostream& os, bool fullStacktrace) mutable {
    printException(os, fullStacktrace, type, std::move(message), std::move(stack));
  };
#endif
}

StarException::StarException(char const* type, std::string message, std::exception const& cause) noexcept
  : StarException(type, std::move(message)) {
  auto printException = [](std::ostream& os, bool fullStacktrace, function<void(std::ostream&, bool)> self, function<void(std::ostream&, bool)> cause) {
    self(os, fullStacktrace);
    os << std::endl << "Caused by: ";
    cause(os, fullStacktrace);
  };

  std::function<void(std::ostream&, bool)> printCause;
  if (auto starException = as<StarException>(&cause)) {
    printCause = [starException](std::ostream& os, bool fullStacktrace) { starException->m_printException(os, fullStacktrace); };
  } else {
    printCause = [causeWhat = std::string(cause.what())](std::ostream& os, bool) {
      os << "std::exception: " << causeWhat;
    };
  }

  m_printException = [printException, self = m_printException, cause = std::move(printCause)](std::ostream& os, bool fullStacktrace) {
    printException(os, fullStacktrace, self, cause);
  };
}

std::string printException(std::exception const& e, bool fullStacktrace) {
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

OutputProxy outputException(std::exception const& e, bool fullStacktrace) {
  if (auto starException = as<StarException>(&e))
    return OutputProxy([starException, fullStacktrace](std::ostream& os) { starException->m_printException(os, fullStacktrace); });
  else
    return OutputProxy([what = std::string(e.what())](std::ostream& os) { os << "std::exception: " << what; });
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

}
