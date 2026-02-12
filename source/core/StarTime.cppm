export module star.time;

import std;

export namespace star::time {

auto time_since_epoch() -> double;
auto milliseconds_since_epoch() -> std::int64_t;

auto monotonic_time() -> double;
auto monotonic_milliseconds() -> std::int64_t;
auto monotonic_microseconds() -> std::int64_t;

auto print_duration(double seconds) -> std::string;

auto print_date_and_time(
  std::int64_t epoch_ticks,
  std::string_view format = "<year>-<month>-<day> <hours>:<minutes>:<seconds>.<millis>")
  -> std::string;
auto print_current_date_and_time(
  std::string_view format = "<year>-<month>-<day> <hours>:<minutes>:<seconds>.<millis>")
  -> std::string;

auto epoch_ticks() -> std::int64_t;
auto epoch_tick_frequency() -> std::int64_t;

auto monotonic_ticks() -> std::int64_t;
auto monotonic_tick_frequency() -> std::int64_t;

auto ticks_to_seconds(std::int64_t ticks, std::int64_t tick_frequency) -> double;
auto ticks_to_milliseconds(std::int64_t ticks, std::int64_t tick_frequency) -> std::int64_t;
auto ticks_to_microseconds(std::int64_t ticks, std::int64_t tick_frequency) -> std::int64_t;
auto seconds_to_ticks(double seconds, std::int64_t tick_frequency) -> std::int64_t;
auto milliseconds_to_ticks(std::int64_t milliseconds, std::int64_t tick_frequency) -> std::int64_t;
auto microseconds_to_ticks(std::int64_t microseconds, std::int64_t tick_frequency) -> std::int64_t;

}// namespace star::time

export namespace star {

class clock {
  public:
    explicit clock(bool start_active = true);

    clock(clock const& other);
    auto operator=(clock const& other) -> clock&;

    clock(clock&& other) noexcept;
    auto operator=(clock&& other) noexcept -> clock&;

    virtual ~clock() = default;

    void reset();
    void stop();
    void start();

    auto is_running() const -> bool;

    auto time() const -> double;
    auto milliseconds() const -> std::int64_t;

    void set_time(double seconds);
    void set_milliseconds(std::int64_t millis);

    void adjust_time(double adjustment);
    void adjust_milliseconds(std::int64_t adjustment);

  protected:
    void update_elapsed() const;

    mutable std::recursive_mutex m_mutex;
    mutable std::int64_t m_elapsed_ticks = 0;
    mutable std::optional<std::int64_t> m_last_ticks;
    bool m_running = false;
};

class timer : private clock {
  public:
    static auto with_time(double time_left, bool start_active = true) -> timer;
    static auto with_milliseconds(std::int64_t millis, bool start_active = true) -> timer;

    timer();
    timer(timer&&) = default;
    auto operator=(timer&&) -> timer& = delete;
    timer(timer const& other);
    auto operator=(timer const& other) -> timer&;

    void restart(double time_left);
    void restart_with_milliseconds(std::int64_t milliseconds_left);

    auto time_left(bool allow_negative = false) const -> double;
    auto milliseconds_left(bool allow_negative = false) const -> std::int64_t;

    auto is_time_up() const -> bool;

    using clock::is_running;
    using clock::start;
    using clock::stop;
};

}// namespace star
