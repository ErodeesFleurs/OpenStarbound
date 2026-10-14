module;

import std;
import star.thread;
import star.set;
import star.string;
import star.map;
import star.list;
import star.format;
import star.poly;
import star.bi_map;
import star.file;

import star.array;
import star.vector;
import star.line;
namespace Star {

enum class LogLevel {
  Debug,
  Info,
  Warn,
  Error
};
extern EnumMap<LogLevel> const LogLevelNames;

class LogSink;
using LogSinkPtr = std::shared_ptr<LogSink>;
using LogSinkConstPtr = std::shared_ptr<const LogSink>;
using LogSinkWeakPtr = std::weak_ptr<LogSink>;
using LogSinkConstWeakPtr = std::weak_ptr<const LogSink>;
using LogSinkUPtr = std::unique_ptr<LogSink>;
using LogSinkConstUPtr = std::unique_ptr<const LogSink>;

// A sink for Logger messages.
class LogSink {
public:
  LogSink();
  virtual ~LogSink();

  virtual void log(char const* msg, LogLevel level) = 0;

  void setLevel(LogLevel level);
  LogLevel level();

private:
  std::atomic<LogLevel> m_level;
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
  static LogSinkPtr stdoutSink();
  // Don't use the stdout sink.
  static void removeStdoutSink();

  static void log(LogLevel level, char const* msg);

  template <typename... Args>
  static void logf(LogLevel level, std::format_string<Args...> msg, Args const&... args);
  template <typename S, typename... Args>
  requires RuntimeFormatString<S>
  static void logf(LogLevel level, S const& msg, Args const&... args);

  template <typename... Args>
  static void debug(std::format_string<Args...> msg, Args const&... args);
  template <typename S, typename... Args>
  requires RuntimeFormatString<S>
  static void debug(S const& msg, Args const&... args);
  template <typename... Args>
  static void info(std::format_string<Args...> msg, Args const&... args);
  template <typename S, typename... Args>
  requires RuntimeFormatString<S>
  static void info(S const& msg, Args const&... args);
  template <typename... Args>
  static void warn(std::format_string<Args...> msg, Args const&... args);
  template <typename S, typename... Args>
  requires RuntimeFormatString<S>
  static void warn(S const& msg, Args const&... args);
  template <typename... Args>
  static void error(std::format_string<Args...> msg, Args const&... args);
  template <typename S, typename... Args>
  requires RuntimeFormatString<S>
  static void error(S const& msg, Args const&... args);

  static bool loggable(LogLevel level);
  static void refreshLoggable();
private:

  static std::shared_ptr<StdoutLogSink> s_stdoutSink;
  static HashSet<LogSinkPtr> s_sinks;
  static Array<bool, 4> s_loggable;
  static Mutex s_mutex;
};

// For logging data that is very high frequency. It is a map of debug values to
// be displayed every frame, or in a debug output window, etc.
class LogMap {
public:
  static String getValue(String const& key);
  static void setValue(String const& key, String const& value);

  // Shorthand, converts given type to string using std::ostream.
  template <typename T>
  static void set(String const& key, T const& t);

  static Map<String, String> getValues();
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

  static Deque<Line> getLines(char const* space, bool andClear);
  static Deque<Point> getPoints(char const* space, bool andClear);
  static Deque<LogText> getText(char const* space, bool andClear);

  static void clear();

  static bool observed();
  static void setObserved(bool observed);

private:
  static Mutex s_mutex;
  static StringMap<Deque<Line>> s_lines;
  static StringMap<Deque<Point>> s_points;
  static StringMap<Deque<LogText>> s_logText;
  static bool s_observed;
};

template <typename... Args>
void Logger::logf(LogLevel level, std::format_string<Args...> msg, Args const&... args) {
  if (loggable(level)) {
    std::string output = strfChecked(msg, args...);
    MutexLocker locker(s_mutex);
    for (auto const& l : s_sinks) {
      if (l->level() <= level) {
        l->log(output.c_str(), level);
      }
    }
  }
}

template <typename S, typename... Args>
  requires RuntimeFormatString<S>
void Logger::logf(LogLevel level, S const& msg, Args const&... args) {
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
void Logger::debug(std::format_string<Args...> msg, Args const&... args) {
  logf(LogLevel::Debug, msg, args...);
}

template <typename S, typename... Args>
  requires RuntimeFormatString<S>
void Logger::debug(S const& msg, Args const&... args) {
  logf(LogLevel::Debug, msg, args...);
}

template <typename... Args>
void Logger::info(std::format_string<Args...> msg, Args const&... args) {
  logf(LogLevel::Info, msg, args...);
}

template <typename S, typename... Args>
  requires RuntimeFormatString<S>
void Logger::info(S const& msg, Args const&... args) {
  logf(LogLevel::Info, msg, args...);
}

template <typename... Args>
void Logger::warn(std::format_string<Args...> msg, Args const&... args) {
  logf(LogLevel::Warn, msg, args...);
}

template <typename S, typename... Args>
  requires RuntimeFormatString<S>
void Logger::warn(S const& msg, Args const&... args) {
  logf(LogLevel::Warn, msg, args...);
}

template <typename... Args>
void Logger::error(std::format_string<Args...> msg, Args const&... args) {
  logf(LogLevel::Error, msg, args...);
}

template <typename S, typename... Args>
  requires RuntimeFormatString<S>
void Logger::error(S const& msg, Args const&... args) {
  logf(LogLevel::Error, msg, args...);
}

template <typename T>
void LogMap::set(String const& key, T const& t) {
  setValue(key, toString(t));
}

}

export module star.logging;

export namespace Star {
  using ::Star::LogLevel;
  using ::Star::LogLevelNames;
  using ::Star::LogSink;
  using ::Star::LogSinkPtr;
  using ::Star::LogSinkConstPtr;
  using ::Star::LogSinkWeakPtr;
  using ::Star::LogSinkConstWeakPtr;
  using ::Star::LogSinkUPtr;
  using ::Star::LogSinkConstUPtr;
  using ::Star::StdoutLogSink;
  using ::Star::FileLogSink;
  using ::Star::Logger;
  using ::Star::LogMap;
  using ::Star::SpatialLogger;
}
