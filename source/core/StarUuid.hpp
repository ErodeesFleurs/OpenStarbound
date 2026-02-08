#pragma once

#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarException.hpp"

import std;

namespace Star {

using UuidException = ExceptionDerived<"UuidException">;

std::size_t const UuidSize = 16;

class Uuid {
public:
  Uuid();
  explicit Uuid(ByteArray const& bytes);
  explicit Uuid(String const& hex);

  [[nodiscard]] auto ptr() const -> char const*;
  [[nodiscard]] auto bytes() const -> ByteArray;
  [[nodiscard]] auto hex() const -> String;

  auto operator==(Uuid const& u) const -> bool;
  auto operator!=(Uuid const& u) const -> bool;
  auto operator<(Uuid const& u) const -> bool;
  auto operator<=(Uuid const& u) const -> bool;
  auto operator>(Uuid const& u) const -> bool;
  auto operator>=(Uuid const& u) const -> bool;

private:
  Array<char, UuidSize> m_data;
};

template <>
struct hash<Uuid> {
  auto operator()(Uuid const& u) const -> std::size_t;
};

auto operator>>(DataStream& ds, Uuid& uuid) -> DataStream&;
auto operator<<(DataStream& ds, Uuid const& uuid) -> DataStream&;

}// namespace Star
