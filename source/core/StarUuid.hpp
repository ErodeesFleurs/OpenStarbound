#pragma once

#include "StarArray.hpp"
#include "StarDataStream.hpp"

namespace Star {

struct UuidExceptionTag { static constexpr char const* typeName = "UuidException"; };
using UuidException = TypedException<StarException, UuidExceptionTag>;

size_t const UuidSize = 16;

class Uuid {
public:
  Uuid();
  explicit Uuid(ByteArray const& bytes);
  explicit Uuid(String const& hex);

  [[nodiscard]] char const* ptr() const;
  [[nodiscard]] ByteArray bytes() const;
  [[nodiscard]] String hex() const;

  auto operator<=>(Uuid const&) const = default;

private:
  Array<char, UuidSize> m_data;
};

template <>
struct hash<Uuid> {
  [[nodiscard]] size_t operator()(Uuid const& u) const;
};

DataStream& operator>>(DataStream& ds, Uuid& uuid);
DataStream& operator<<(DataStream& ds, Uuid const& uuid);

}
