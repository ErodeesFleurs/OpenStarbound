#include <stdio.h>
import std;
import star.logging;
import star.string;
import star.set;
import star.map;
import star.list;
import star.bi_map;
import star.file;
import star.io_device;
import star.format;
import star.time;

import star.array;
import star.vector;
import star.line;
import star.poly;
import star.algorithm;
namespace Star {

LogSink::LogSink()
  : m_level(LogLevel::Info) {}

LogSink::~LogSink() {}

void LogSink::setLevel(LogLevel level) {
  m_level = level;
}

LogLevel LogSink::level() {
  return m_level;
}

std::expected<void, StarException> StdoutLogSink::log(char const* msg, LogLevel level) {
  std::lock_guard locker(m_logMutex);
  auto name = enumName<LogLevelNames>(level);
  if (!name)
    return std::unexpected(std::move(name.error()));
  coutf("[{}] {}\n", *name, msg);
  return {};
}

std::expected<std::shared_ptr<FileLogSink>, StarException> FileLogSink::create(
    String const& filename, LogLevel level, bool truncate) {
  auto output = File::open(filename,
      truncate ? IOMode::Write | IOMode::Append | IOMode::Truncate : IOMode::Write | IOMode::Append);
  if (!output)
    return std::unexpected(std::move(output.error()));
  auto sink = std::shared_ptr<FileLogSink>(new FileLogSink());
  sink->m_output = std::move(*output);
  sink->setLevel(level);
  return sink;
}

std::expected<void, StarException> FileLogSink::log(char const* msg, LogLevel level) {
  std::lock_guard locker(m_logMutex);
  auto timestamp = Time::printCurrentDateAndTime("<hours>:<minutes>:<seconds>.<millis>");
  auto name = enumName<LogLevelNames>(level);
  if (!name)
    return std::unexpected(std::move(name.error()));
  auto line = strf("[{}] [{}] {}\n", timestamp, *name, msg);
  auto written = m_output->write(line.data(), line.size());
  if (!written)
    return std::unexpected(std::move(written.error()));
  return {};
}

void Logger::addSink(std::shared_ptr<LogSink> s) {
  std::lock_guard locker(s_mutex);
  s_sinks.insert(s);
}

void Logger::removeSink(std::shared_ptr<LogSink> s) {
  std::lock_guard locker(s_mutex);
  s_sinks.erase(s);
}

std::shared_ptr<LogSink> Logger::stdoutSink() {
  std::lock_guard locker(s_mutex);
  return s_stdoutSink;
}

void Logger::removeStdoutSink() {
  std::lock_guard locker(s_mutex);
  s_sinks.erase(s_stdoutSink);
}

std::expected<void, StarException> Logger::log(LogLevel level, char const* msg) {
  if (loggable(level)) {
    std::lock_guard locker(s_mutex);
    for (auto const& l : s_sinks) {
      if (l->level() <= level) {
        auto result = l->log(msg, level);
        if (!result)
          return std::unexpected(std::move(result.error()).withContext(strf("While writing log message: {}", msg)));
      }
    }
  }
  return {};
}

Logger::Logger()
  : s_stdoutSink(std::make_shared<StdoutLogSink>()), s_sinks{s_stdoutSink} {}

bool Logger::loggable(LogLevel level) {
  std::lock_guard locker(s_mutex);
  for (auto const& sink : s_sinks) {
    if (sink->level() <= level)
      return true;
  }
  return false;
}

void EmergencyLog::log(LogLevel, char const* message) {
  std::fputs(message, stderr);
  std::fputc('\n', stderr);
}

String LogMap::getValue(String const& key) {
  std::lock_guard locker(s_logMapMutex);
  return s_logMap.value(key);
}

void LogMap::setValue(String const& key, String const& value) {
  std::lock_guard locker(s_logMapMutex);
  s_logMap[key] = value;
}

Map<String, String> LogMap::getValues() {
  std::lock_guard locker(s_logMapMutex);
  return Map<String, String>::from(s_logMap);
}

void LogMap::clear() {
  std::lock_guard locker(s_logMapMutex);
  s_logMap.clear();
}


std::size_t const SpatialLogger::MaximumLines;
std::size_t const SpatialLogger::MaximumPoints;
std::size_t const SpatialLogger::MaximumText;

void SpatialLogger::logPoly(char const* space, PolyF const& poly, Vec4B const& color) {
  if (!observed()) return;

  std::lock_guard locker(s_mutex);
  auto& lines = s_lines[space];

  for (std::size_t i = 0; i < poly.sides(); ++i) {
    auto side = poly.side(i);
    lines.append(Line{side.min(), side.max(), color});
  }

  while (lines.size() > MaximumLines)
    lines.erase(lines.begin());
}

void SpatialLogger::logLine(char const* space, Line2F const& line, Vec4B const& color) {
  if (!observed()) return;

  std::lock_guard locker(s_mutex);
  auto& lines = s_lines[space];

  lines.append(Line{line.min(), line.max(), color});

  while (lines.size() > MaximumLines)
    lines.erase(lines.begin());
}

void SpatialLogger::logLine(char const* space, Vec2F const& begin, Vec2F const& end, Vec4B const& color) {
  if (!observed()) return;

  std::lock_guard locker(s_mutex);
  auto& lines = s_lines[space];

  lines.append(Line{begin, end, color});

  while (lines.size() > MaximumLines)
    lines.erase(lines.begin());
}

void SpatialLogger::logPoint(char const* space, Vec2F const& position, Vec4B const& color) {
  if (!observed()) return;

  std::lock_guard locker(s_mutex);
  auto& points = s_points[space];

  points.append(Point{position, color});
  while (points.size() > MaximumPoints)
    points.erase(points.begin());
}

void SpatialLogger::logText(char const* space, String text, Vec2F const& position, Vec4B const& color) {
  if (!observed()) return;

  std::lock_guard locker(s_mutex);
  auto& texts = s_logText[space];

  texts.append(LogText{text, position, color});

  while (texts.size() > MaximumText)
    texts.erase(texts.begin());
}

Deque<SpatialLogger::Line> SpatialLogger::getLines(char const* space, bool andClear) {
  std::lock_guard locker(s_mutex);
  if (andClear)
    return take(s_lines[space]);
  else
    return s_lines[space];
}

Deque<SpatialLogger::Point> SpatialLogger::getPoints(char const* space, bool andClear) {
  std::lock_guard locker(s_mutex);
  if (andClear)
    return take(s_points[space]);
  else
    return s_points[space];
}

Deque<SpatialLogger::LogText> SpatialLogger::getText(char const* space, bool andClear) {
  std::lock_guard locker(s_mutex);
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
    std::lock_guard locker(s_mutex);
    lines = std::move(s_lines);
    points = std::move(s_points);
    logText = std::move(s_logText);
  } // Move while locked to deallocate contents while unlocked.
}

bool SpatialLogger::observed() {
  return s_observed;
}

void SpatialLogger::setObserved(bool observed) {
  s_observed = observed;
}


}
