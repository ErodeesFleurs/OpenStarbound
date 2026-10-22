module;
import std;

import star.format;

namespace Star {

template <typename Result>
struct UniqueFunctionInvocationResult {
  using Type = std::expected<std::conditional_t<std::is_reference_v<Result>,
      std::add_pointer_t<std::remove_reference_t<Result>>, Result>, StarException>;
};

template <typename Value, typename Error>
struct UniqueFunctionInvocationResult<std::expected<Value, Error>> {
  static_assert(std::same_as<Error, StarException>,
      "UniqueFunction callbacks must use StarException errors");
  using Type = std::expected<Value, StarException>;
};

template <typename Signature>
class UniqueFunction;

template <typename Result, typename... Args>
class UniqueFunction<Result(Args...)> {
  static constexpr std::size_t InlineSize = 3 * sizeof(void*);
  struct Storage {
    alignas(std::max_align_t) std::byte bytes[InlineSize];
  } m_storage;

  struct Operations {
    Result (*invoke)(Storage&, Args&&...);
    void (*destroy)(Storage&) noexcept;
    void (*move)(Storage&, Storage&) noexcept;
  };
  Operations const* m_operations = nullptr;

  template <typename Function>
  static constexpr bool Inline = sizeof(Function) <= InlineSize
      && alignof(Function) <= alignof(Storage) && std::is_nothrow_move_constructible_v<Function>;

  template <typename Function>
  static Function* target(Storage& storage) noexcept {
    if constexpr (Inline<Function>)
      return std::launder(reinterpret_cast<Function*>(storage.bytes));
    else
      return *std::launder(reinterpret_cast<Function**>(storage.bytes));
  }

  template <typename Function>
  static Operations const& operations() {
    static constexpr Operations value{
      [](Storage& storage, Args&&... args) -> Result {
        if constexpr (std::is_void_v<Result>)
          std::invoke(*target<Function>(storage), std::forward<Args>(args)...);
        else
          return std::invoke(*target<Function>(storage), std::forward<Args>(args)...);
      },
      [](Storage& storage) noexcept {
        if constexpr (Inline<Function>)
          std::destroy_at(target<Function>(storage));
        else
          delete target<Function>(storage);
      },
      [](Storage& destination, Storage& source) noexcept {
        if constexpr (Inline<Function>) {
          std::construct_at(reinterpret_cast<Function*>(destination.bytes), std::move(*target<Function>(source)));
          std::destroy_at(target<Function>(source));
        } else {
          std::construct_at(reinterpret_cast<Function**>(destination.bytes), target<Function>(source));
        }
      }
    };
    return value;
  }

  void reset() noexcept {
    if (m_operations) {
      m_operations->destroy(m_storage);
      m_operations = nullptr;
    }
  }

public:
  // Preserve an error-valued task's result instead of nesting expected values.
  // Ordinary reference results use pointers so invocation failure is explicit.
  using InvocationResult = typename UniqueFunctionInvocationResult<Result>::Type;

  UniqueFunction() noexcept = default;
  UniqueFunction(std::nullptr_t) noexcept {}
  UniqueFunction(UniqueFunction const&) = delete;
  UniqueFunction& operator=(UniqueFunction const&) = delete;

  template <typename Callable>
    requires (!std::same_as<std::remove_cvref_t<Callable>, UniqueFunction>
        && std::is_constructible_v<std::decay_t<Callable>, Callable&&>
        && std::is_invocable_r_v<Result, std::decay_t<Callable>&, Args...>
        && (!std::is_void_v<Result>
            || !requires { typename std::remove_cvref_t<std::invoke_result_t<std::decay_t<Callable>&, Args...>>::error_type; })
        && (!std::is_reference_v<Result>
            || (std::is_reference_v<std::invoke_result_t<std::decay_t<Callable>&, Args...>>
                && std::is_convertible_v<
                    std::add_pointer_t<std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>&, Args...>>>,
                    std::add_pointer_t<std::remove_reference_t<Result>>>)))
  UniqueFunction(Callable&& callable) {
    using Function = std::decay_t<Callable>;
    if constexpr (std::is_pointer_v<Function> || std::is_member_pointer_v<Function>) {
      if (callable == nullptr)
        return;
    }
    if constexpr (Inline<Function>)
      std::construct_at(reinterpret_cast<Function*>(m_storage.bytes), std::forward<Callable>(callable));
    else
      std::construct_at(reinterpret_cast<Function**>(m_storage.bytes), new Function(std::forward<Callable>(callable)));
    m_operations = &operations<Function>();
  }

  UniqueFunction(UniqueFunction&& source) noexcept {
    if (source.m_operations) {
      source.m_operations->move(m_storage, source.m_storage);
      m_operations = std::exchange(source.m_operations, nullptr);
    }
  }

  UniqueFunction& operator=(UniqueFunction&& source) noexcept {
    if (this != &source) {
      reset();
      if (source.m_operations) {
        source.m_operations->move(m_storage, source.m_storage);
        m_operations = std::exchange(source.m_operations, nullptr);
      }
    }
    return *this;
  }

  UniqueFunction& operator=(std::nullptr_t) noexcept { reset(); return *this; }
  ~UniqueFunction() { reset(); }
  explicit operator bool() const noexcept { return m_operations != nullptr; }

  [[nodiscard]] InvocationResult operator()(Args... args) {
    if (!m_operations)
      return std::unexpected(StarException("Cannot invoke an empty UniqueFunction"));
    if constexpr (std::is_void_v<Result>) {
      m_operations->invoke(m_storage, std::forward<Args>(args)...);
      return {};
    } else if constexpr (std::is_reference_v<Result>) {
      auto&& result = m_operations->invoke(m_storage, std::forward<Args>(args)...);
      return std::addressof(result);
    } else {
      return m_operations->invoke(m_storage, std::forward<Args>(args)...);
    }
  }
};

}

export module star.unique_function;
export namespace Star {
  using ::Star::UniqueFunction;
}
