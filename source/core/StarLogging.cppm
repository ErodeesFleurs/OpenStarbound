export module star.logging;

import std;
import star.bi_map;
import star.vector;
import star.file;
import star.io_device;
import star.array;
import star.string;
import star.poly;
import star.line;

export namespace star {

enum class log_level : std::uint8_t { debug, info, warn, error };

extern const enum_map<log_level> log_level_names;

class log_sink {
  public:
    log_sink();
    log_sink(const log_sink&) = delete;
    log_sink(log_sink&&) = delete;
    auto operator=(const log_sink&) -> log_sink& = delete;
    auto operator=(log_sink&&) -> log_sink& = delete;
    virtual ~log_sink();

    // Must be implemented by derived classes
    virtual void log(std::u8string_view msg, log_level level) = 0;

    void set_level(log_level level) noexcept;
    [[nodiscard]] auto level() const noexcept -> log_level;

  private:
    std::atomic<log_level> level_{log_level::info};
};

class stdout_log_sink : public log_sink {
  public:
    void log(std::u8string_view msg, log_level level) override;

  private:
    std::mutex mutex_;
};

class file_log_sink : public log_sink {
  public:
    file_log_sink(std::u8string_view filename, log_level level, bool truncate);

    void log(std::u8string_view msg, log_level level) override;

  private:
    std::shared_ptr<star::file> output_;
    std::mutex mutex_;
};

class logger {
  public:
    // Sink management
    static void add_sink(std::shared_ptr<log_sink> sink);
    static void remove_sink(const std::shared_ptr<log_sink>& sink);

    // Default stdout sink that outputs to standard output
    [[nodiscard]] static auto stdout_sink() -> std::shared_ptr<log_sink>;

    // Remove the stdout sink from active sinks
    static void remove_stdout_sink();

    // Core logging function
    static void log(log_level level, std::u8string_view msg);

    // Formatted logging with compile-time format string checking
    template <typename... Args>
    static void logf(log_level level, std::format_string<Args...> fmt, Args&&... args);

    // Runtime format string version (for dynamic format strings)
    template <typename... Args>
    static void vlogf(log_level level, std::u8string_view fmt, Args&&... args);

    // Convenience functions for each log level with compile-time format checking
    template <typename... Args> static void debug(std::format_string<Args...> fmt, Args&&... args);

    template <typename... Args> static void vdebug(std::u8string_view fmt, Args&&... args);

    template <typename... Args> static void info(std::format_string<Args...> fmt, Args&&... args);

    template <typename... Args> static void vinfo(std::u8string_view fmt, Args&&... args);

    template <typename... Args> static void warn(std::format_string<Args...> fmt, Args&&... args);

    template <typename... Args> static void vwarn(std::u8string_view fmt, Args&&... args);

    template <typename... Args> static void error(std::format_string<Args...> fmt, Args&&... args);

    template <typename... Args> static void verror(std::u8string_view fmt, Args&&... args);

    // Check if a log level is currently active
    [[nodiscard]] static auto loggable(log_level level) noexcept -> bool;

    // Refresh the loggable cache based on current sinks
    static void refresh_loggable();

  private:
    static std::shared_ptr<stdout_log_sink> stdout_sink_;
    static std::flat_set<std::shared_ptr<log_sink>> sinks_;
    static star::array<bool, 4> loggable_;
    static std::mutex mutex_;
};

class log_map {
  public:
    [[nodiscard]] static auto get_value(std::u8string_view key) -> std::u8string;
    static void set_value(std::u8string_view key, std::u8string_view value);

    // Shorthand that converts given type to string
    template <typename T> static void set(std::u8string_view key, T const& value);

    [[nodiscard]] static auto get_values() -> std::unordered_map<std::u8string, std::u8string>;
    static void clear();

  private:
    static std::flat_map<std::u8string, std::u8string> map_;
    static std::mutex mutex_;
};

class spatial_logger {
  public:
    // Maximum count of objects stored per space
    static constexpr std::size_t maximum_lines = 200'000;
    static constexpr std::size_t maximum_points = 200'000;
    static constexpr std::size_t maximum_text = 10'000;

    struct line_ {
        vec_2f begin;
        vec_2f end;
        vec_4b color;
    };

    struct point_ {
        vec_2f position;
        vec_4b color;
    };

    struct log_text_ {
        std::u8string text;
        vec_2f position;
        vec_4b color;
    };

    // Log geometric primitives to a named space
    static void log_poly(std::u8string_view space, poly_f const& poly, vec_4b const& color);

    static void log_line(std::u8string_view space, line_2f const& line, vec_4b const& color);

    static void log_line(std::u8string_view space, vec_2f const& begin, vec_2f const& end,
                         vec_4b const& color);

    static void log_point(std::u8string_view space, vec_2f const& position, vec_4b const& color);

    static void log_text(std::u8string_view space, std::u8string_view text, vec_2f const& position,
                         vec_4b const& color);

    // Retrieve logged data from a named space
    [[nodiscard]] static auto get_lines(std::u8string_view space, bool and_clear)
      -> std::deque<line_>;

    [[nodiscard]] static auto get_points(std::u8string_view space, bool and_clear)
      -> std::deque<point_>;

    [[nodiscard]] static auto get_text(std::u8string_view space, bool and_clear)
      -> std::deque<log_text_>;

    // Clear all logged spatial data
    static void clear();

    // Check if spatial logging is being observed (enables/disables logging)
    [[nodiscard]] static auto observed() noexcept -> bool;
    static void set_observed(bool observed) noexcept;

  private:
    static std::mutex mutex_;
    static std::flat_map<std::u8string, std::deque<line_>> lines_;
    static std::flat_map<std::u8string, std::deque<point_>> points_;
    static std::flat_map<std::u8string, std::deque<log_text_>> text_;
    static std::atomic<bool> observed_;
};

template <typename... Args>
void logger::logf(log_level level, std::format_string<Args...> fmt, Args&&... args) {
    if (!loggable(level)) [[unlikely]] {
        return;
    }

    auto output = std::format(fmt, std::forward<Args>(args)...);
    std::lock_guard lock{mutex_};

    for (auto const& sink : sinks_) {
        if (sink->level() <= level) {
            sink->log(string_cast<std::u8string_view>(output), level);
        }
    }
}

template <typename... Args>
void logger::vlogf(log_level level, std::u8string_view fmt, Args&&... args) {
    if (!loggable(level)) [[unlikely]] {
        return;
    }

    auto output = vstrf(fmt, std::forward<Args>(args)...);
    std::lock_guard lock{mutex_};

    for (auto const& sink : sinks_) {
        if (sink->level() <= level) {
            sink->log(output, level);
        }
    }
}

template <typename... Args> void logger::debug(std::format_string<Args...> fmt, Args&&... args) {
    logf(log_level::debug, fmt, std::forward<Args>(args)...);
}

template <typename... Args> void logger::vdebug(std::u8string_view fmt, Args&&... args) {
    vlogf(log_level::debug, fmt, std::forward<Args>(args)...);
}

template <typename... Args> void logger::info(std::format_string<Args...> fmt, Args&&... args) {
    logf(log_level::info, fmt, std::forward<Args>(args)...);
}

template <typename... Args> void logger::vinfo(std::u8string_view fmt, Args&&... args) {
    vlogf(log_level::info, fmt, std::forward<Args>(args)...);
}

template <typename... Args> void logger::warn(std::format_string<Args...> fmt, Args&&... args) {
    logf(log_level::warn, fmt, std::forward<Args>(args)...);
}

template <typename... Args> void logger::vwarn(std::u8string_view fmt, Args&&... args) {
    vlogf(log_level::warn, fmt, std::forward<Args>(args)...);
}

template <typename... Args> void logger::error(std::format_string<Args...> fmt, Args&&... args) {
    logf(log_level::error, fmt, std::forward<Args>(args)...);
}

template <typename... Args> void logger::verror(std::u8string_view fmt, Args&&... args) {
    vlogf(log_level::error, fmt, std::forward<Args>(args)...);
}

template <typename T> void log_map::set(std::u8string_view key, T const& value) {
    set_value(key, to_string(value));
}

}// namespace star
