#include "StarLogging.hpp"
#include "StarConfig.hpp"
#include "StarTime.hpp"

import std;

namespace Star {

EnumMap<LogLevel> const LogLevelNames{
  {LogLevel::Debug, "Debug"},
  {LogLevel::Info, "Info"},
  {LogLevel::Warn, "Warn"},
  {LogLevel::Error, "Error"}
};

LogSink::LogSink()
  : m_level(LogLevel::Info) {}

LogSink::~LogSink() = default;

void LogSink::setLevel(LogLevel level) {
  m_level = level;
  Logger::refreshLoggable();
}

auto LogSink::level() -> LogLevel {
  return m_level;
}

void StdoutLogSink::log(char const* msg, LogLevel level) {
  MutexLocker locker(m_logMutex);
  coutf("[{}] {}\n", LogLevelNames.getRight(level), msg);
}

FileLogSink::FileLogSink(String const& filename, LogLevel level, bool truncate) {
  if (truncate)
    m_output = File::open(filename, IOMode::Write | IOMode::Append | IOMode::Truncate);
  else
    m_output = File::open(filename, IOMode::Write | IOMode::Append);
  setLevel(level);
}

void FileLogSink::log(char const* msg, LogLevel level) {
  MutexLocker locker(m_logMutex);
  auto line = strf("[{}] [{}] {}\n", Time::printCurrentDateAndTime("<hours>:<minutes>:<seconds>.<millis>"), LogLevelNames.getRight(level), msg);
  m_output->write(line.data(), line.size());
}

void Logger::addSink(Ptr<LogSink> s) {
  MutexLocker locker(s_mutex);
  s_sinks.insert(s);
  refreshLoggable();
}

void Logger::removeSink(Ptr<LogSink> s) {
  MutexLocker locker(s_mutex);
  s_sinks.erase(s);
  refreshLoggable();
}

auto Logger::stdoutSink() -> Ptr<LogSink> {
  MutexLocker locker(s_mutex);
  return s_stdoutSink;
}

void Logger::removeStdoutSink() {
  MutexLocker locker(s_mutex);
  s_sinks.erase(s_stdoutSink);
  refreshLoggable();
}

void Logger::log(LogLevel level, char const* msg) {
  if (loggable(level)) {
    MutexLocker locker(s_mutex);
    for (auto const& l : s_sinks) {
      if (l->level() <= level)
        l->log(msg, level);
    }
  }
}

auto Logger::loggable(LogLevel level) -> bool {
  return s_loggable[(int)level];
}

void Logger::refreshLoggable() {
  s_loggable = Array<bool, 4>::filled(false);
  for (auto const& l : s_sinks) {
    for (auto i = (size_t)l->level(); i != s_loggable.size(); ++i)
      s_loggable[i] = true;
  }
}

std::shared_ptr<StdoutLogSink> Logger::s_stdoutSink = std::make_shared<StdoutLogSink>();
HashSet<Ptr<LogSink>> Logger::s_sinks{s_stdoutSink};
Array<bool, 4> Logger::s_loggable = Array<bool, 4>{false, true, true, true};
Mutex Logger::s_mutex;

auto LogMap::getValue(String const& key) -> String {
  MutexLocker locker(s_logMapMutex);
  return s_logMap.value(key);
}

void LogMap::setValue(String const& key, String const& value) {
  MutexLocker locker(s_logMapMutex);
  s_logMap[key] = value;
}

auto LogMap::getValues() -> Map<String, String> {
  MutexLocker locker(s_logMapMutex);
  return Map<String, String>::from(s_logMap);
}

void LogMap::clear() {
  MutexLocker locker(s_logMapMutex);
  s_logMap.clear();
}

HashMap<String, String> LogMap::s_logMap;
Mutex LogMap::s_logMapMutex;

size_t const SpatialLogger::MaximumLines;
size_t const SpatialLogger::MaximumPoints;
size_t const SpatialLogger::MaximumText;

void SpatialLogger::logPoly(char const* space, PolyF const& poly, Vec4B const& color) {
  if (!observed()) return;

  MutexLocker locker(s_mutex);
  auto& lines = s_lines[space];

  for (size_t i = 0; i < poly.sides(); ++i) {
    auto side = poly.side(i);
    lines.append(Line{.begin=side.min(), .end=side.max(), .color=color});
  }

  while (lines.size() > MaximumLines)
    lines.removeFirst();
}

void SpatialLogger::logLine(char const* space, Line2F const& line, Vec4B const& color) {
  if (!observed()) return;

  MutexLocker locker(s_mutex);
  auto& lines = s_lines[space];

  lines.append(Line{.begin=line.min(), .end=line.max(), .color=color});

  while (lines.size() > MaximumLines)
    lines.removeFirst();
}

void SpatialLogger::logLine(char const* space, Vec2F const& begin, Vec2F const& end, Vec4B const& color) {
  if (!observed()) return;

  MutexLocker locker(s_mutex);
  auto& lines = s_lines[space];

  lines.append(Line{.begin=begin, .end=end, .color=color});

  while (lines.size() > MaximumLines)
    lines.removeFirst();
}

void SpatialLogger::logPoint(char const* space, Vec2F const& position, Vec4B const& color) {
  if (!observed()) return;

  MutexLocker locker(s_mutex);
  auto& points = s_points[space];

  points.append(Point{.position=position, .color=color});

  while (points.size() > MaximumPoints)
    points.removeFirst();
}

void SpatialLogger::logText(char const* space, String text, Vec2F const& position, Vec4B const& color) {
  if (!observed()) return;

  MutexLocker locker(s_mutex);
  auto& texts = s_logText[space];

  texts.append(LogText{.text=text, .position=position, .color=color});

  while (texts.size() > MaximumText)
    texts.removeFirst();
}

auto SpatialLogger::getLines(char const* space, bool andClear) -> Deque<SpatialLogger::Line> {
  MutexLocker locker(s_mutex);
  if (andClear)
    return take(s_lines[space]);
  else
    return s_lines[space];
}

auto SpatialLogger::getPoints(char const* space, bool andClear) -> Deque<SpatialLogger::Point> {
  MutexLocker locker(s_mutex);
  if (andClear)
    return take(s_points[space]);
  else
    return s_points[space];
}

auto SpatialLogger::getText(char const* space, bool andClear) -> Deque<SpatialLogger::LogText> {
  MutexLocker locker(s_mutex);
  if (andClear)
    return take(s_logText[space]);
  else
    return s_logText[space];
}

void SpatialLogger::clear() {
  decltype(s_lines) lines;
  decltype(s_points) points;
  decltype(s_logText) logText;
  {
    MutexLocker locker(s_mutex);
    lines = std::move(s_lines);
    points = std::move(s_points);
    logText = std::move(s_logText);
  } // Move while locked to deallocate contents while unlocked.
}

auto SpatialLogger::observed() -> bool {
  return s_observed;
}

void SpatialLogger::setObserved(bool observed) {
  s_observed = observed;
}


Mutex SpatialLogger::s_mutex;
StringMap<Deque<SpatialLogger::Line>> SpatialLogger::s_lines;
StringMap<Deque<SpatialLogger::Point>> SpatialLogger::s_points;
StringMap<Deque<SpatialLogger::LogText>> SpatialLogger::s_logText;
bool SpatialLogger::s_observed = false;
}
