module;

export module star.exception;

import std;

export namespace star {

#if __has_include(<stacktrace>) && defined(__cpp_lib_stacktrace)
#include <stacktrace>
constexpr bool STAR_HAS_STACKTRACE = true;
#else
constexpr bool STAR_HAS_STACKTRACE = false;
#endif

template <std::size_t N> struct fixed_string {
    std::array<char, N> data{};

    constexpr fixed_string(  //NOLINT(hicpp-explicit-conversions)
      const char (&str)[N]) {//NOLINT(hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
        std::ranges::copy(str, data.begin());
    }

    constexpr explicit operator std::string_view() const noexcept {
        return {data.data(), N > 0 ? N - 1 : 0};
    }

    constexpr auto operator()() const noexcept { return static_cast<std::string_view>(*this); }
    [[nodiscard]] constexpr auto c_str() const noexcept -> const char* { return data.data(); }
};

class star_exception : public std::exception {
  public:
    star_exception() noexcept = default;
    star_exception(const star_exception&) = default;
    star_exception(star_exception&&) = delete;
    auto operator=(const star_exception&) -> star_exception& = default;
    auto operator=(star_exception&&) -> star_exception& = delete;
    ~star_exception() override = default;

    explicit star_exception(std::string message, bool genStackTrace = true,
                            std::source_location loc = std::source_location::current()) noexcept
        : m_message(std::move(message)), m_location(loc) {
        if (genStackTrace) {
            capture_stack();
        }
    }

    star_exception(std::string message, const std::exception& cause)
        : m_message(std::format("{}\nCause: {}", message, cause.what())) {}

    template <typename... Args>
    static auto format(std::format_string<Args...> fmt, Args&&... args) -> star_exception {
        return star_exception(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static auto vformat(std::string_view fmt, Args&&... args) -> star_exception {
        return star_exception(std::vformat(fmt, std::forward<Args>(args)...));
    }

    [[nodiscard]] auto what() const noexcept -> const char* override {
        if (m_what_buffer.empty()) {
            m_what_buffer =
              std::format("{}({}:{}) [{}] : {}", m_location.function_name(), m_location.file_name(),
                          m_location.line(), get_type_name(), m_message);
        }
        return m_what_buffer.c_str();
    }

    auto location() const noexcept -> const std::source_location& { return m_location; }

  protected:
    virtual auto get_type_name() const noexcept -> std::string_view { return "star_exception"; }

  private:
    void capture_stack() {
#if STAR_HAS_STACKTRACE
        m_stacktrace = std::stacktrace::current();
#endif
    }

    std::string m_message;
    std::source_location m_location = std::source_location::current();
    mutable std::string m_what_buffer;
#if STAR_HAS_STACKTRACE
    std::stacktrace m_stacktrace;
#endif
};

template <fixed_string Name, typename Base = star_exception> class exception_derived : public Base {
  public:
    using Base::Base;

    exception_derived() : Base(std::string(Name.c_str())) {}

    explicit exception_derived(std::string message, bool gen_stack_trace = true)
        : Base(std::move(message), gen_stack_trace) {}

    [[nodiscard]] auto get_type_name() const noexcept -> std::string_view override { return Name(); }

    template <typename... Args>
    static auto format(std::format_string<Args...> fmt, Args&&... args) -> exception_derived {
        return exception_derived(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static auto vformat(std::string_view fmt, Args&&... args) -> exception_derived {
        return exception_derived(std::vformat(fmt, std::forward<Args>(args)...));
    }
};

using out_of_range_exception = exception_derived<"out_of_range_exception">;
using io_exception = exception_derived<"io_exception">;
using memory_exception = exception_derived<"memory_exception">;

}// namespace star
