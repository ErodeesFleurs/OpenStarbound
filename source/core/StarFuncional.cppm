export module star.functional;

import std;

export namespace star {

enum class func_errc : std::uint8_t { invalid_target, call_failed };

template <typename ErasedSignature> class move_only_function;

template <typename R, typename E, typename... Args>
class move_only_function<std::expected<R, E>(Args...)> {
    struct concept_t {
        concept_t(const concept_t&) = delete;
        concept_t(concept_t&&) = delete;
        auto operator=(const concept_t&) -> concept_t& = delete;
        auto operator=(concept_t&&) -> concept_t& = delete;
        virtual ~concept_t() = default;
        virtual auto call(Args&&... args) -> std::expected<R, E> = 0;
    };

    template <typename F> struct model_t final : concept_t {
        F m_callable;
        explicit model_t(F&& f) : m_callable(std::move(f)) {}

        auto call(Args&&... args) -> std::expected<R, E> override {
            return std::invoke(m_callable, std::forward<Args>(args)...);
        }
    };

    std::unique_ptr<concept_t> m_ptr;

  public:
    move_only_function() noexcept = default;
    explicit move_only_function(std::unique_ptr<concept_t> m_ptr) : m_ptr(std::move(m_ptr)) {}
    template <typename F>
        requires(!std::is_same_v<std::decay_t<F>, move_only_function>
                 && std::is_invocable_r_v<std::expected<R, E>, F, Args...>)
    explicit move_only_function(F&& f)
        : m_ptr(std::make_unique<model_t<std::decay_t<F>>>(std::forward<F>(f))) {}

    move_only_function(const move_only_function&) = delete;
    auto operator=(const move_only_function&) -> move_only_function& = delete;

    move_only_function(move_only_function&&) noexcept = default;
    auto operator=(move_only_function&&) noexcept -> move_only_function& = default;

    auto operator()(Args... args) const -> std::expected<R, E> {
        if (!m_ptr) {
            return std::unexpected(E(func_errc::invalid_target));
        }
        return m_ptr->call(std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept { return static_cast<bool>(m_ptr); }
};

template <typename R, typename E, typename... Args>
class move_only_function<std::expected<R, E>(Args...) const>
    : public move_only_function<std::expected<R, E>(Args...)> {

    using base = move_only_function<std::expected<R, E>(Args...)>;

  public:
    using base::base;

    auto operator()(Args... args) const -> std::expected<R, E> {
        return base::operator()(std::forward<Args>(args)...);
    }
};

}// namespace star
