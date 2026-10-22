module;
import std;
import star.format;
import star.type_identity;

namespace Star {

struct PointerConvertExceptionTag {
  static constexpr char const* name() { return "PointerConvertException"; }
};
using PointerConvertException = StarError<PointerConvertExceptionTag, StarException>;

template <typename Target, typename Source>
Target* queryCast(Source* p) noexcept {
  if (!p)
    return nullptr;
  if constexpr (std::is_convertible_v<Source*, Target*>) {
    return p;
  } else {
    auto result = static_cast<std::remove_const_t<Target> const*>(p->queryType(typeKey<Target>()));
    if constexpr (std::is_const_v<Target>)
      return result;
    else {
      static_assert(!std::is_const_v<Source>);
      return const_cast<Target*>(result);
    }
  }
}

template <typename Type1, typename Type2>
Type1* as(Type2* p) {
  return queryCast<Type1>(p);
}

template <typename Type1, typename Type2>
Type1 const* as(Type2 const* p) {
  return queryCast<Type1 const>(p);
}

template <typename Type1, typename Type2>
std::shared_ptr<Type1> as(std::shared_ptr<Type2> const& p) {
  if (auto adjusted = as<Type1>(p.get()))
    return std::shared_ptr<Type1>(p, adjusted);
  return {};
}

template <typename Type1, typename Type2>
std::shared_ptr<Type1 const> as(std::shared_ptr<Type2 const> const& p) {
  if (auto adjusted = as<Type1>(p.get()))
    return std::shared_ptr<Type1 const>(p, adjusted);
  return {};
}

template <typename Type1, typename Type2>
bool is(Type2* p) {
  return as<Type1>(p) != nullptr;
}

template <typename Type1, typename Type2>
bool is(Type2 const* p) {
  return as<Type1>(p) != nullptr;
}

template <typename Type1, typename Type2>
bool is(std::shared_ptr<Type2> const& p) {
  return is<Type1>(p.get());
}

template <typename Type1, typename Type2>
bool is(std::shared_ptr<Type2 const> const& p) {
  return is<Type1>(p.get());
}

template <typename Type1, typename Type2>
bool ris(Type2& r) {
  return is<Type1>(&r);
}

template <typename Type1, typename Type2>
bool ris(Type2 const& r) {
  return is<Type1>(&r);
}

template <typename Type, typename Ptr>
[[nodiscard]] auto convert(Ptr const& p) -> std::expected<decltype(as<Type>(p)), StarException> {
  if (!p)
    return std::unexpected(PointerConvertException::format("Could not convert from nullptr to {}", typeName<Type>()));
  else if (auto a = as<Type>(p))
    return a;
  else
    return std::unexpected(PointerConvertException::format("Could not convert from {} to {}", p->dynamicTypeName(), typeName<Type>()));
}

template <typename Type1, typename Type2>
[[nodiscard]] std::expected<Type1*, StarException> rconvert(Type2& r) {
  return convert<Type1>(&r);
}

template <typename Type1, typename Type2>
[[nodiscard]] std::expected<Type1 const*, StarException> rconvert(Type2 const& r) {
  return convert<Type1 const>(&r);
}

template <typename Type>
std::weak_ptr<Type> asWeak(std::shared_ptr<Type> const& p) {
  return std::weak_ptr<Type>(p);
}

template <typename Type>
std::weak_ptr<Type const> asWeak(std::shared_ptr<Type const> const& p) {
  return std::weak_ptr<Type>(p);
}

}

export module star.casting;

export namespace Star {
  using ::Star::PointerConvertExceptionTag;
  using ::Star::PointerConvertException;
  using ::Star::is;
  using ::Star::ris;
  using ::Star::as;
  using ::Star::convert;
  using ::Star::rconvert;
  using ::Star::asWeak;
}
