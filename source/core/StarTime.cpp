module star.time;

import std;

namespace star::time {

static constexpr std::int64_t ticks_per_second = 1'000'000'000;

auto time_since_epoch() -> double {
    return ticks_to_seconds(epoch_ticks(), epoch_tick_frequency());
}

auto milliseconds_since_epoch() -> std::int64_t {
    return ticks_to_milliseconds(epoch_ticks(), epoch_tick_frequency());
}

auto monotonic_time() -> double {
    return ticks_to_seconds(monotonic_ticks(), monotonic_tick_frequency());
}

auto monotonic_milliseconds() -> std::int64_t {
    return ticks_to_milliseconds(monotonic_ticks(), monotonic_tick_frequency());
}

auto monotonic_microseconds() -> std::int64_t {
    return ticks_to_microseconds(monotonic_ticks(), monotonic_tick_frequency());
}

auto print_duration(double seconds) -> std::string {
    auto dur = std::chrono::duration<std::double_t>(seconds);

    auto h = duration_cast<std::chrono::hours>(dur);
    dur -= h;
    auto m = duration_cast<std::chrono::minutes>(dur);
    dur -= m;
    auto s = duration_cast<std::chrono::seconds>(dur);
    ;
    auto ms = duration_cast<std::chrono::milliseconds>(dur);

    std::vector<std::string> parts;
    if (h.count() > 0) {
        parts.push_back(std::format("{} hour{}", h.count(), h.count() == 1 ? "" : "s"));
    }
    if (m.count() > 0) {
        parts.push_back(std::format("{} minute{}", m.count(), m.count() == 1 ? "" : "s"));
    }
    if (s.count() > 0) {
        parts.push_back(std::format("{} second{}", s.count(), s.count() == 1 ? "" : "s"));
    }
    parts.push_back(std::format("{} millisecond{}", ms.count(), ms.count() == 1 ? "" : "s"));

    std::string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i < parts.size() - 1) {
            result += ", ";
        }
    }
    return result;
}

auto print_date_and_time(std::int64_t ticks, std::string_view format) -> std::string {
    using namespace std::chrono;
    using namespace std::literals;

    nanoseconds total_ns{ticks};
    auto sec = duration_cast<seconds>(total_ns);
    auto ms_part = duration_cast<milliseconds>(total_ns % 1s).count();

    auto t = static_cast<std::time_t>(sec.count());
    std::tm* lt = std::localtime(&t);

    struct TagMap {
        std::string_view tag;
        std::string value;
    };
    std::array replacements{
      TagMap{.tag = "year", .value = std::format("{:04d}", lt->tm_year + 1900)},
      TagMap{.tag = "month", .value = std::format("{:02d}", lt->tm_mon + 1)},
      TagMap{.tag = "day", .value = std::format("{:02d}", lt->tm_mday)},
      TagMap{.tag = "hours", .value = std::format("{:02d}", lt->tm_hour)},
      TagMap{.tag = "minutes", .value = std::format("{:02d}", lt->tm_min)},
      TagMap{.tag = "seconds", .value = std::format("{:02d}", lt->tm_sec)},
      TagMap{.tag = "millis", .value = std::format("{:03d}", ms_part)}};

    std::string res;
    res.reserve(format.size() + 16);
    for (std::size_t i = 0; i < format.size(); ++i) {
        if (format[i] == '<') {
            std::size_t end = format.find('>', i);
            if (end != std::string_view::npos) {
                std::string_view current_tag = format.substr(i + 1, end - i - 1);

                bool found = false;
                for (const auto& [tag, value] : replacements) {
                    if (current_tag == tag) {
                        res += value;
                        i = end;
                        found = true;
                        break;
                    }
                }
                if (found) {
                    continue;
                }
            }
        }
        res += format[i];
    }

    return res;
}

auto print_current_date_and_time(std::string_view format) -> std::string {
    return print_date_and_time(epoch_ticks(), format);
}

auto epoch_ticks() -> std::int64_t {
    return std::chrono::system_clock::now().time_since_epoch() / std::chrono::nanoseconds(1);
}

auto epoch_tick_frequency() -> std::int64_t { return ticks_per_second; }

auto monotonic_ticks() -> std::int64_t {
    return std::chrono::steady_clock::now().time_since_epoch() / std::chrono::nanoseconds(1);
}

auto monotonic_tick_frequency() -> std::int64_t { return ticks_per_second; }

auto ticks_to_seconds(std::int64_t ticks, std::int64_t freq) -> double {
    return static_cast<double>(ticks) / static_cast<double>(freq);
}

auto ticks_to_milliseconds(std::int64_t ticks, std::int64_t freq) -> std::int64_t {
    std::int64_t tpms = freq / 1000;
    return tpms == 0 ? 0 : (ticks + tpms / 2) / tpms;
}

auto ticks_to_microseconds(std::int64_t ticks, std::int64_t freq) -> std::int64_t {
    std::int64_t tpus = freq / 1'000'000;
    return tpus == 0 ? 0 : (ticks + tpus / 2) / tpus;
}

auto seconds_to_ticks(double seconds, std::int64_t freq) -> std::int64_t {
    return static_cast<std::int64_t>(std::round(seconds * static_cast<std::double_t>(freq)));
}

auto milliseconds_to_ticks(std::int64_t ms, std::int64_t freq) -> std::int64_t {
    return ms * (freq / 1000);
}

auto microseconds_to_ticks(std::int64_t us, std::int64_t freq) -> std::int64_t {
    return us * (freq / 1'000'000);
}

}// namespace star::time

namespace star {

clock::clock(bool start_active) {
    if (start_active) {
        start();
    }
}

clock::clock(clock const& other) { *this = other; }

auto clock::operator=(clock const& other) -> clock& {
    if (this != &other) {
        std::scoped_lock lock(m_mutex, other.m_mutex);
        m_elapsed_ticks = other.m_elapsed_ticks;
        m_last_ticks = other.m_last_ticks;
        m_running = other.m_running;
    }
    return *this;
}

clock::clock(clock&& other) noexcept { *this = std::move(other); }

auto clock::operator=(clock&& other) noexcept -> clock& {
    if (this != &other) {
        std::scoped_lock lock(m_mutex, other.m_mutex);
        m_elapsed_ticks = std::exchange(other.m_elapsed_ticks, 0);
        m_last_ticks = std::exchange(other.m_last_ticks, std::nullopt);
        m_running = std::exchange(other.m_running, false);
    }
    return *this;
}

void clock::reset() {
    std::lock_guard lock(m_mutex);
    m_elapsed_ticks = 0;
    m_last_ticks = m_running ? std::make_optional(time::monotonic_ticks()) : std::nullopt;
}

void clock::stop() {
    std::lock_guard lock(m_mutex);
    update_elapsed();
    m_last_ticks.reset();
    m_running = false;
}

void clock::start() {
    std::lock_guard lock(m_mutex);
    if (!m_running) {
        m_running = true;
        m_last_ticks = time::monotonic_ticks();
    }
}

auto clock::is_running() const -> bool {
    std::lock_guard lock(m_mutex);
    return m_running;
}

auto clock::time() const -> double {
    std::lock_guard lock(m_mutex);
    update_elapsed();
    return time::ticks_to_seconds(m_elapsed_ticks, time::monotonic_tick_frequency());
}

auto clock::milliseconds() const -> std::int64_t {
    std::lock_guard lock(m_mutex);
    update_elapsed();
    return time::ticks_to_milliseconds(m_elapsed_ticks, time::monotonic_tick_frequency());
}

void clock::set_time(double seconds) {
    std::lock_guard lock(m_mutex);
    m_elapsed_ticks = time::seconds_to_ticks(seconds, time::monotonic_tick_frequency());
    if (m_running) {
        m_last_ticks = time::monotonic_ticks();
    }
}

void clock::set_milliseconds(std::int64_t millis) {
    std::lock_guard lock(m_mutex);
    m_elapsed_ticks = time::milliseconds_to_ticks(millis, time::monotonic_tick_frequency());
    if (m_running) {
        m_last_ticks = time::monotonic_ticks();
    }
}

void clock::adjust_time(double adjustment) {
    std::lock_guard lock(m_mutex);
    set_time(std::max(0.0, time() + adjustment));
}

void clock::adjust_milliseconds(std::int64_t adjustment) {
    std::lock_guard lock(m_mutex);
    set_milliseconds(milliseconds() + adjustment);
}

void clock::update_elapsed() const {
    if (!m_running) {
        return;
    }
    std::int64_t current = time::monotonic_ticks();
    if (m_last_ticks) {
        m_elapsed_ticks += (current - *m_last_ticks);
    }
    m_last_ticks = current;
}

auto timer::with_time(double t_left, bool start) -> timer {
    timer t;
    t.set_time(-t_left);
    if (start) {
        t.start();
    }
    return t;
}

auto timer::with_milliseconds(std::int64_t ms_left, bool start) -> timer {
    timer t;
    t.set_milliseconds(-ms_left);
    if (start) {
        t.start();
    }
    return t;
}

timer::timer() : clock(false) { set_time(0.0); }

timer::timer(timer const& other) = default;
auto timer::operator=(timer const& other) -> timer& = default;

void timer::restart(double t_left) {
    clock::set_time(-t_left);
    clock::start();
}

void timer::restart_with_milliseconds(std::int64_t ms_left) {
    clock::set_milliseconds(-ms_left);
    clock::start();
}

auto timer::time_left(bool neg) const -> double {
    double left = -clock::time();
    return neg ? left : std::max(0.0, left);
}

auto timer::milliseconds_left(bool neg) const -> std::int64_t {
    std::int64_t left = -clock::milliseconds();
    return neg ? left : std::max<std::int64_t>(0, left);
}

auto timer::is_time_up() const -> bool { return clock::time() >= 0.0; }

}// namespace star
