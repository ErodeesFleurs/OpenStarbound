#pragma once

#include "StarLua.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

namespace Detail {
  template <typename Tuple, size_t Skip>
  struct TupleTailImpl;

  template <typename... Ts, size_t Skip>
  struct TupleTailImpl<std::tuple<Ts...>, Skip> {
    static_assert(Skip <= sizeof...(Ts), "Not enough arguments to skip");

    template <size_t... Is>
    static auto make(std::index_sequence<Is...>)
      -> std::tuple<std::tuple_element_t<Skip + Is, std::tuple<Ts...>>...>;

    using type = decltype(make(std::make_index_sequence<sizeof...(Ts) - Skip>{}));
  };

  template <typename Tuple, size_t Skip>
  using TupleTail = typename TupleTailImpl<Tuple, Skip>::type;
}

// Helper to bind a free function as a Lua callback with bound arguments.
// Func is a function pointer, BoundArgs are captured by the lambda, and the
// remaining arguments are exposed to Lua.
//
// Usage:
//   bindCallback<&WorldCallbacks::entityPosition>(callbacks, "entityPosition", world);
template <auto Func, typename... BoundArgs>
void bindCallback(LuaCallbacks& callbacks, String name, BoundArgs&&... bound) {
  using Traits = FunctionTraits<decltype(Func)>;
  using ReturnType = typename Traits::Return;
  using AllArgs = typename Traits::ArgTuple;
  using LuaArgs = Detail::TupleTail<AllArgs, sizeof...(BoundArgs)>;

  auto binder = [&]<typename... LuaTs>(std::tuple<LuaTs...>*) {
    callbacks.registerCallbackWithSignature<ReturnType, LuaTs...>(
      name,
      [bound = std::tuple(std::forward<BoundArgs>(bound)...)](LuaTs... args) -> ReturnType {
        return std::apply([&](auto&&... b) {
          return Func(std::forward<decltype(b)>(b)..., std::forward<LuaTs>(args)...);
        }, bound);
      }
    );
  };

  binder(static_cast<LuaArgs*>(nullptr));
}

}
