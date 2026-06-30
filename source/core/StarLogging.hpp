#pragma once

#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarBiMap.hpp"
#include "StarTime.hpp"
#include "StarFile.hpp"

namespace Star {

enum class LogLevel {
  Debug,
  Info,
  Warn,
  Error
};
extern EnumMap<LogLevel> const LogLevelNames;

class LogSink;
using LogSinkPtr = SharedPtr<LogSink>;

// A sink for Logger messages.
class LogSink {
public:
  LogSink() = default;
  virtual ~LogSink() = default;

  virtual void log(char const* msg, LogLevel level) = 0;

  void setLevel(LogLevel level);
  [[nodiscard]] LogLevel level();

private:
  atomic<LogLevel> m_level = LogLevel::Info;
};

class StdoutLogSink : public LogSink {
public:
  virtual void log(char const* msg, LogLevel level);

private:
  Mutex m_logMutex;
};

class FileLogSink : public LogSink {
public:
  FileLogSink(String const& filename, LogLevel level, bool truncate);

  virtual void log(char const* msg, LogLevel level);

private:
  FilePtr m_output;
  Mutex m_logMutex;
};

// A basic loging system that logs to multiple streams.  Can log at Debug,
// Info, Warn, and Error logging levels.  By default logs to stdout.
class Logger {
public:
  static void addSink(LogSinkPtr s);
  static void removeSink(LogSinkPtr s);

  // Default LogSink that outputs to stdout.
  [[nodiscard]] static LogSinkPtr stdoutSink();
  // Don't use the stdout sink.
  static void removeStdoutSink();

  static void log(LogLevel level, char const* msg);

  template <typename... Args>
  static void logf(LogLevel level, char const* msg, Args const&... args);

  template <typename... Args>
  static void debug(char const* msg, Args const&... args);
  template <typename... Args>
  static void info(char const* msg, Args const&... args);
  template <typename... Args>
  static void warn(char const* msg, Args const&... args);
  template <typename... Args>
  static void error(char const* msg, Args const&... args);

  [[nodiscard]] static bool loggable(LogLevel level);
  static void refreshLoggable();
private:

  static SharedPtr<StdoutLogSink> s_stdoutSink;
  static HashSet<LogSinkPtr> s_sinks;
  static Array<bool, 4> s_loggable;
  static Mutex s_mutex;
};

// For logging data that is very high frequency. It is a map of debug values to
// be displayed every frame, or in a debug output window, etc.
class LogMap {
public:
  [[nodiscard]] static String getValue(String const& key);
  static void setValue(String const& key, String const& value);

  // Shorthand, converts given type to string using std::ostream.
  template <typename T>
  static void set(String const& key, T const& t);

  [[nodiscard]] static Map<String, String> getValues();
  static void clear();

private:
  static HashMap<String, String> s_logMap;
  static Mutex s_logMapMutex;
};

// Logging for spatial data.  Divided into multiple named coordinate spaces.
class SpatialLogger {
public:
  // Maximum count of objects stored per space
  static constexpr size_t MaximumLines = 200000;
  static constexpr size_t MaximumPoints = 200000;
  static constexpr size_t MaximumText = 10000;

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

  [[nodiscard]] static Deque<Line> getLines(char const* space, bool andClear);
  [[nodiscard]] static Deque<Point> getPoints(char const* space, bool andClear);
  [[nodiscard]] static Deque<LogText> getText(char const* space, bool andClear);

  static void clear();

  [[nodiscard]] static bool observed();
  static void setObserved(bool observed);

private:
  static Mutex s_mutex;
  static StringMap<Deque<Line>> s_lines;
  static StringMap<Deque<Point>> s_points;
  static StringMap<Deque<LogText>> s_logText;
  static bool s_observed;
};

template <typename... Args>
void Logger::logf(LogLevel level, char const* msg, Args const&... args) {
  if (loggable(level)) {
    std::string output = strf(msg, args...);
    MutexLocker locker(s_mutex);
    for (auto const& l : s_sinks) {
      if (l->level() <= level) {
        l->log(output.c_str(), level);
      }
    }
  }
}

template <typename... Args>
void Logger::debug(char const* msg, Args const&... args) {
  logf(LogLevel::Debug, msg, args...);
}

template <typename... Args>
void Logger::info(char const* msg, Args const&... args) {
  logf(LogLevel::Info, msg, args...);
}

template <typename... Args>
void Logger::warn(char const* msg, Args const&... args) {
  logf(LogLevel::Warn, msg, args...);
}

template <typename... Args>
void Logger::error(char const* msg, Args const&... args) {
  logf(LogLevel::Error, msg, args...);
}

template <typename T>
void LogMap::set(String const& key, T const& t) {
  setValue(key, toString(t));
}

}
