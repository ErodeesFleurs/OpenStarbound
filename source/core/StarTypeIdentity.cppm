module;
import std;

namespace Star {

using TypeKey = void const*;

namespace TypeIdentityDetail {
  template <typename T>
  inline constexpr unsigned char Token = 0;
}

// An address identity within one linked image, never a persisted or wire ID.
// Match typeid(T)'s treatment of references and top-level cv qualifiers.
template <typename T>
constexpr TypeKey typeKey() noexcept {
  return &TypeIdentityDetail::Token<std::remove_cvref_t<T>>;
}

// Diagnostic text only: identity must not depend on compiler spelling or hashes.
template <typename T>
consteval std::string_view typeName() noexcept {
  constexpr std::string_view signature = std::source_location::current().function_name();
  constexpr auto begin = signature.find("T = ");
  constexpr auto end = signature.rfind(']');
  static_assert(begin != std::string_view::npos && end > begin + 4);
  return signature.substr(begin + 4, end - begin - 4);
}

// Explicit capabilities of the project's class graphs, not an implementation
// of arbitrary C++ dynamic_cast source-subobject disambiguation.
template <typename Derived, typename... Bases>
void const* queryTypeOf(Derived const* self, TypeKey key) noexcept {
  if (key == typeKey<Derived>())
    return self;
  if constexpr (sizeof...(Bases) == 0) {
    return nullptr;
  } else {
    void const* result = nullptr;
    bool ambiguous = false;
    auto merge = [&](void const* candidate) {
      if (candidate) {
        if (result && result != candidate)
          ambiguous = true;
        else
          result = candidate;
      }
    };
    (merge(static_cast<Bases const*>(self)->Bases::queryType(key)), ...);
    return ambiguous ? nullptr : result;
  }
}

}

export module star.type_identity;

export namespace Star {
  using ::Star::TypeKey;
  using ::Star::typeKey;
  using ::Star::typeName;
  using ::Star::queryTypeOf;
}
