#pragma once

#include "StarBiMap.hpp"
#include "StarConfig.hpp"
#include "StarFile.hpp"
#include "StarPoly.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

enum class LogLevel {
  Debug,
  Info,
  Warn,
  Error
};
extern EnumMap<LogLevel> const LogLevelNames;

// A sink for Logger messages.
class LogSink {
public:
  LogSink();
  virtual ~LogSink();

  virtual void log(char const* msg, LogLevel level) = 0;

  void setLevel(LogLevel level);
  auto level() -> LogLevel;

private:
  std::atomic<LogLevel> m_level;
};

class StdoutLogSink : public LogSink {
public:
  void log(char const* msg, LogLevel level) override;

private:
  Mutex m_logMutex;
};

class FileLogSink : public LogSink {
public:
  FileLogSink(String const& filename, LogLevel level, bool truncate);

  void log(char const* msg, LogLevel level) override;

private:
  Ptr<File> m_output;
  Mutex m_logMutex;
};

// A basic loging system that logs to multiple streams.  Can log at Debug,
// Info, Warn, and Error logging levels.  By default logs to stdout.
class Logger {
public:
  static void addSink(Ptr<LogSink> s);
  static void removeSink(Ptr<LogSink> s);

  // Default LogSink that outputs to stdout.
  static auto stdoutSink() -> Ptr<LogSink>;
  // Don't use the stdout sink.
  static void removeStdoutSink();

  static void log(LogLevel level, char const* msg);

  template <typename... Args>
  static void logf(LogLevel level, std::format_string<Args...> msg, Args&&... args);
  template <typename... Args>
  static void vlogf(LogLevel level, std::string_view msg, Args&&... args);

  template <typename... Args>
  static void debug(std::format_string<Args...> msg, Args&&... args);
  template <typename... Args>
  static void vdebug(std::string_view msg, Args&&... args);
  template <typename... Args>
  static void info(std::format_string<Args...> msg, Args&&... args);
  template <typename... Args>
  static void vinfo(std::string_view msg, Args&&... args);
  template <typename... Args>
  static void warn(std::format_string<Args...> msg, Args&&... args);
  template <typename... Args>
  static void vwarn(std::string_view msg, Args&&... args);
  template <typename... Args>
  static void error(std::format_string<Args...> msg, Args&&... args);
  template <typename... Args>
  static void verror(std::string_view msg, Args&&... args);

  static auto loggable(LogLevel level) -> bool;
  static void refreshLoggable();

private:
  static std::shared_ptr<StdoutLogSink> s_stdoutSink;
  static HashSet<Ptr<LogSink>> s_sinks;
  static Array<bool, 4> s_loggable;
  static Mutex s_mutex;
};

// For logging data that is very high frequency. It is a map of debug values to
// be displayed every frame, or in a debug output window, etc.
class LogMap {
public:
  static auto getValue(String const& key) -> String;
  static void setValue(String const& key, String const& value);

  // Shorthand, converts given type to string using std::ostream.
  template <typename T>
  static void set(String const& key, T const& t);

  static auto getValues() -> Map<String, String>;
  static void clear();

private:
  static HashMap<String, String> s_logMap;
  static Mutex s_logMapMutex;
};

// Logging for spatial data.  Divided into multiple named coordinate spaces.
class SpatialLogger {
public:
  // Maximum count of objects stored per space
  static std::size_t const MaximumLines = 200000;
  static std::size_t const MaximumPoints = 200000;
  static std::size_t const MaximumText = 10000;

  struct Line {
    Vec2F begin;
    Vec2F end;
    Vec4B color;
  };

  struct Point {
    Vec2F position;
    Vec4B color;
  };

  struct LogText {
    String text;
    Vec2F position;
    Vec4B color;
  };

  static void logPoly(char const* space, PolyF const& poly, Vec4B const& color);
  static void logLine(char const* space, Line2F const& line, Vec4B const& color);
  static void logLine(char const* space, Vec2F const& begin, Vec2F const& end, Vec4B const& color);
  static void logPoint(char const* space, Vec2F const& position, Vec4B const& color);
  static void logText(char const* space, String text, Vec2F const& position, Vec4B const& color);

  static auto getLines(char const* space, bool andClear) -> Deque<Line>;
  static auto getPoints(char const* space, bool andClear) -> Deque<Point>;
  static auto getText(char const* space, bool andClear) -> Deque<LogText>;

  static void clear();

  static auto observed() -> bool;
  static void setObserved(bool observed);

private:
  static Mutex s_mutex;
  static StringMap<Deque<Line>> s_lines;
  static StringMap<Deque<Point>> s_points;
  static StringMap<Deque<LogText>> s_logText;
  static bool s_observed;
};

template <typename... Args>
void Logger::logf(LogLevel level, std::format_string<Args...> msg, Args&&... args) {
  if (loggable(level)) {
    std::string output = strf(msg, std::forward<Args>(args)...);
    MutexLocker locker(s_mutex);
    for (auto const& l : s_sinks) {
      if (l->level() <= level) {
        l->log(output.c_str(), level);
      }
    }
  }
}

template <typename... Args>
void Logger::vlogf(LogLevel level, std::string_view msg, Args&&... args) {
  if (loggable(level)) {
    std::string output = vstrf(msg, std::forward<Args>(args)...);
    MutexLocker locker(s_mutex);
    for (auto const& l : s_sinks) {
      if (l->level() <= level) {
        l->log(output.c_str(), level);
      }
    }
  }
}

template <typename... Args>
void Logger::debug(std::format_string<Args...> msg, Args&&... args) {
  logf(LogLevel::Debug, msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::vdebug(std::string_view msg, Args&&... args) {
  logf(LogLevel::Debug, msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::info(std::format_string<Args...> msg, Args&&... args) {
  logf(LogLevel::Info, msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::vinfo(std::string_view msg, Args&&... args) {
  logf(LogLevel::Info, msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::warn(std::format_string<Args...> msg, Args&&... args) {
  logf(LogLevel::Warn, msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::vwarn(std::string_view msg, Args&&... args) {
  logf(LogLevel::Warn, msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::error(std::format_string<Args...> msg, Args&&... args) {
  logf(LogLevel::Error, msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::verror(std::string_view msg, Args&&... args) {
  logf(LogLevel::Error, msg, std::forward<Args>(args)...);
}

template <typename T>
void LogMap::set(String const& key, T const& t) {
  setValue(key, toString(t));
}

}// namespace Star
