#pragma once

#include "StarOutputProxy.hpp"
#include "StarFormat.hpp"

import std;

namespace Star {

template <std::size_t N>
struct FixedString {
    std::array<char, N> data;
    constexpr FixedString(const char (&str)[N]) {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = str[i];
    }

    constexpr operator std::string_view() const { return {data.data(), N - 1}; }
    constexpr auto operator()() const -> std::string_view { return static_cast<std::string_view>(*this); }
    [[nodiscard]] constexpr auto c_str() const -> const char* { return data.data(); }
};

class StarException : public std::exception {
public:
    template <typename... Args>
    static auto format(std::format_string<Args...> fmt, Args&&... args) -> StarException;
    template <typename... Args>
    static auto vformat(std::string_view fmt, Args&&... args) -> StarException;

    StarException() noexcept;
    ~StarException() noexcept override;

    explicit StarException(std::string message, bool genStackTrace = true) noexcept;
    explicit StarException(std::exception const& cause) noexcept;
    StarException(std::string message, std::exception const& cause) noexcept;

    template <typename T, typename... Args>
    static auto create(std::format_string<Args...> fmt, Args&&... args) -> T {
        return T(strf(fmt, std::forward<Args>(args)...));
    }

    auto what() const noexcept -> char const* override;

    // If the given exception is really StarException, then this will call
    // StarException::printException, otherwise just prints std::exception::what.
    friend void printException(std::ostream& os, std::exception const& e, bool fullStacktrace);
    friend auto printException(std::exception const& e, bool fullStacktrace) -> std::string;
    friend auto outputException(std::exception const& e, bool fullStacktrace) -> OutputProxy;

protected:
    StarException(char const* type, std::string message, bool genStackTrace = true) noexcept;
    StarException(char const* type, std::string message, std::exception const& cause) noexcept;

private:
    // Takes the ostream to print to, whether to print the full stacktrace.  Must
    // not bind 'this', may outlive the exception in the case of chained
    // exception causes.
    std::function<void(std::ostream&, bool)> m_printException;

    // m_printException will be called without the stack-trace to print
    // m_whatBuffer, if the what() method is invoked.
    mutable std::string m_whatBuffer;
};

template <typename... Args>
auto StarException::format(std::format_string<Args...> fmt, Args&&... args) -> StarException {
  return StarException(strf(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
auto StarException::vformat(std::string_view fmt, Args&&... args) -> StarException {
  return StarException(vstrf(fmt, std::forward<Args>(args)...));
}


void printException(std::ostream& os, std::exception const& e, bool fullStacktrace);
auto printException(std::exception const& e, bool fullStacktrace) -> std::string;
auto outputException(std::exception const& e, bool fullStacktrace) -> OutputProxy;

void printStack(char const* message);

// Log error and stack-trace and possibly show a dialog box if available, then
// abort.
void fatalError(char const* message, bool showStackTrace);
void fatalException(std::exception const& e, bool showStackTrace);

template <typename T>
constexpr auto get_type_name() -> std::string_view {
    std::string_view name = __PRETTY_FUNCTION__;
    std::size_t start = name.find("T = ") + 4;
    std::size_t end = name.find_last_of("]");
    return name.substr(start, end - start);
}

template <FixedString Name, typename Base = StarException>
class ExceptionDerived : public Base {
public:
    // 自动继承所有基类构造函数（对别名同样有效）
    using Base::Base;

    static constexpr std::string_view ClassName = Name();

    ExceptionDerived()
        : Base(ClassName.data(), std::string()) {}

    explicit ExceptionDerived(std::string message, bool genStackTrace = true)
        : Base(ClassName.data(), std::move(message), genStackTrace) {}

    explicit ExceptionDerived(std::exception const& cause)
        : Base(ClassName.data(), std::string(), cause) {}

    template <typename... Args>
    static auto format(std::format_string<Args...> fmt, Args&&... args) -> ExceptionDerived {
        return ExceptionDerived(strf(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static auto vformat(std::string_view fmt, Args&&... args) -> ExceptionDerived {
        return ExceptionDerived(vstrf(fmt, std::forward<Args>(args)...));
    }
};


using OutOfRangeException = ExceptionDerived<"OutOfRangeException">;
using IOException = ExceptionDerived<"IOException">;
using MemoryException = ExceptionDerived<"MemoryException">;

}
