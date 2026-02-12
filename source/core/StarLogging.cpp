module star.logging;

import star.time;
import star.io_device;
import star.string;
import std;

namespace star {

// ============================================================================
// Log Level Names
// ============================================================================

enum_map<log_level> const log_level_names{{log_level::debug, "Debug"},
                                          {log_level::info, "Info"},
                                          {log_level::warn, "Warn"},
                                          {log_level::error, "Error"}};

log_sink::log_sink() = default;

log_sink::~log_sink() = default;

void log_sink::set_level(log_level level) noexcept {
    level_.store(level, std::memory_order_release);
    logger::refresh_loggable();
}

auto log_sink::level() const noexcept -> log_level {
    return level_.load(std::memory_order_acquire);
}

void stdout_log_sink::log(std::u8string_view msg, log_level level) {
    std::lock_guard lock{mutex_};
    std::print(std::cout, "[{}] {}\n", log_level_names.get_right(level), msg);
    std::cout.flush();
}

file_log_sink::file_log_sink(std::u8string_view filename, log_level level, bool truncate) {
    star::io_mode mode = star::io_mode::write | star::io_mode::append;
    if (truncate) {
        mode |= star::io_mode::truncate;
    }

    output_ = file::open(std::u8string{filename}, mode);
    set_level(level);
}

void file_log_sink::log(std::u8string_view msg, log_level level) {
    std::lock_guard lock{mutex_};

    auto timestamp = time::print_current_date_and_time("<hours>:<minutes>:<seconds>.<millis>");

    auto line = std::format("[{}] [{}] {}\n", timestamp, log_level_names.get_right(level), msg);

    auto _ = output_->write(std::as_bytes(std::span{line.data(), line.size()}));
}

// ============================================================================
// Logger Implementation
// ============================================================================

void logger::add_sink(std::shared_ptr<log_sink> sink) {
    if (!sink) [[unlikely]] {
        return;
    }

    std::lock_guard lock{mutex_};
    sinks_.insert(std::move(sink));
    refresh_loggable();
}

void logger::remove_sink(const std::shared_ptr<log_sink>& sink) {
    if (!sink) [[unlikely]] {
        return;
    }

    std::lock_guard lock{mutex_};
    sinks_.erase(sink);
    refresh_loggable();
}

auto logger::stdout_sink() -> std::shared_ptr<log_sink> {
    std::lock_guard lock{mutex_};
    return stdout_sink_;
}

void logger::remove_stdout_sink() {
    std::lock_guard lock{mutex_};
    sinks_.erase(stdout_sink_);
    refresh_loggable();
}

void logger::log(log_level level, std::u8string_view msg) {
    if (!loggable(level)) [[unlikely]] {
        return;
    }

    std::lock_guard lock{mutex_};
    for (auto const& sink : sinks_) {
        if (sink->level() <= level) {
            sink->log(msg, level);
        }
    }
}

auto logger::loggable(log_level level) noexcept -> bool {
    return loggable_[static_cast<std::size_t>(level)];
}

void logger::refresh_loggable() {
    loggable_ = star::array<bool, 4>::filled(false);

    for (auto const& sink : sinks_) {
        auto sink_level = static_cast<std::size_t>(sink->level());
        for (auto i = sink_level; i < loggable_.size(); ++i) {
            loggable_[i] = true;
        }
    }
}

// Static member initialization
std::shared_ptr<stdout_log_sink> logger::stdout_sink_ = std::make_shared<stdout_log_sink>();

std::flat_set<std::shared_ptr<log_sink>> logger::sinks_{stdout_sink_};

star::array<bool, 4> logger::loggable_ = star::array<bool, 4>{false, true, true, true};

std::mutex logger::mutex_;

// ============================================================================
// Log Map Implementation
// ============================================================================

auto log_map::get_value(std::u8string_view key) -> std::u8string {
    std::lock_guard lock{mutex_};

    auto it = map_.find(std::u8string{key});
    if (it != map_.end()) {
        return it->second;
    }

    return {};
}

void log_map::set_value(std::u8string_view key, std::u8string_view value) {
    std::lock_guard lock{mutex_};
    map_[std::u8string{key}] = value;
}

auto log_map::get_values() -> std::unordered_map<std::u8string, std::u8string> {
    std::lock_guard lock{mutex_};
    return std::unordered_map<std::u8string, std::u8string>{map_.begin(), map_.end()};
}

void log_map::clear() {
    std::lock_guard lock{mutex_};
    map_.clear();
}

// Static member initialization
std::flat_map<std::u8string, std::u8string> log_map::map_;
std::mutex log_map::mutex_;

// ============================================================================
// Spatial Logger Implementation
// ============================================================================

// Static constexpr member definitions (C++17+)
constexpr std::size_t spatial_logger::maximum_lines;
constexpr std::size_t spatial_logger::maximum_points;
constexpr std::size_t spatial_logger::maximum_text;

void spatial_logger::log_poly(std::u8string_view space, poly_f const& poly, vec_4b const& color) {
    if (!observed()) [[unlikely]] {
        return;
    }

    std::lock_guard lock{mutex_};
    auto& lines = lines_[std::u8string{space}];

    for (std::size_t i = 0; i < poly.sides(); ++i) {
        auto side = poly.side(i);
        lines.push_back(line_{.begin = side.min(), .end = side.max(), .color = color});
    }

    while (lines.size() > maximum_lines) {
        lines.pop_front();
    }
}

void spatial_logger::log_line(std::u8string_view space, line_2f const& line_obj,
                              vec_4b const& color) {
    if (!observed()) [[unlikely]] {
        return;
    }

    std::lock_guard lock{mutex_};
    auto& lines = lines_[std::u8string{space}];

    lines.push_back(line_{.begin = line_obj.min(), .end = line_obj.max(), .color = color});

    while (lines.size() > maximum_lines) {
        lines.pop_front();
    }
}

void spatial_logger::log_line(std::u8string_view space, vec_2f const& begin, vec_2f const& end,
                              vec_4b const& color) {
    if (!observed()) [[unlikely]] {
        return;
    }

    std::lock_guard lock{mutex_};
    auto& lines = lines_[std::u8string{space}];

    lines.push_back(line_{.begin = begin, .end = end, .color = color});

    while (lines.size() > maximum_lines) {
        lines.pop_front();
    }
}

void spatial_logger::log_point(std::u8string_view space, [[maybe_unused]] vec_2f const& position,
                               [[maybe_unused]] vec_4b const& color) {
    if (!observed()) [[unlikely]] {
        return;
    }

    std::lock_guard lock{mutex_};
    auto& points = points_[std::u8string{space}];

    points.push_back(point_{.position = position, .color = color});

    while (points.size() > maximum_points) {
        points.pop_front();
    }
}

void spatial_logger::log_text(std::u8string_view space, std::u8string_view text,
                              [[maybe_unused]] vec_2f const& position,
                              [[maybe_unused]] vec_4b const& color) {
    if (!observed()) [[unlikely]] {
        return;
    }

    std::lock_guard lock{mutex_};
    auto& texts = text_[std::u8string{space}];

    texts.push_back(log_text_{.text = std::u8string{text}, .position = position, .color = color});

    while (texts.size() > maximum_text) {
        texts.pop_front();
    }
}

auto spatial_logger::get_lines(std::u8string_view space, bool and_clear)
  -> std::deque<spatial_logger::line_> {
    std::lock_guard lock{mutex_};
    auto& lines = lines_[std::u8string{space}];

    if (and_clear) {
        return std::exchange(lines, {});
    } else {
        return lines;
    }
}

auto spatial_logger::get_points(std::u8string_view space, bool and_clear)
  -> std::deque<spatial_logger::point_> {
    std::lock_guard lock{mutex_};
    auto& points = points_[std::u8string{space}];

    if (and_clear) {
        return std::exchange(points, {});
    } else {
        return points;
    }
}

auto spatial_logger::get_text(std::u8string_view space, bool and_clear)
  -> std::deque<spatial_logger::log_text_> {
    std::lock_guard lock{mutex_};
    auto& texts = text_[std::u8string{space}];

    if (and_clear) {
        return std::exchange(texts, {});
    } else {
        return texts;
    }
}

void spatial_logger::clear() {
    // Move containers while locked to deallocate contents while unlocked
    decltype(lines_) lines;
    decltype(points_) points;
    decltype(text_) texts;

    {
        std::lock_guard lock{mutex_};
        lines = std::move(lines_);
        points = std::move(points_);
        texts = std::move(text_);
    }

    // Containers are destroyed here outside the lock
}

auto spatial_logger::observed() noexcept -> bool {
    return observed_.load(std::memory_order_relaxed);
}

void spatial_logger::set_observed(bool observed) noexcept {
    observed_.store(observed, std::memory_order_relaxed);
}

std::mutex spatial_logger::mutex_;
std::flat_map<std::u8string, std::deque<spatial_logger::line_>> spatial_logger::lines_;
std::flat_map<std::u8string, std::deque<spatial_logger::point_>> spatial_logger::points_;
std::flat_map<std::u8string, std::deque<spatial_logger::log_text_>> spatial_logger::text_;
std::atomic<bool> spatial_logger::observed_{false};

}// namespace star
