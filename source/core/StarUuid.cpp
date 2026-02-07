#include "StarUuid.hpp"
#include "StarEncode.hpp"
#include "StarFormat.hpp"
#include "StarRandom.hpp"

namespace Star {

Uuid::Uuid() : Uuid(Random::randBytes(UuidSize)) {}

Uuid::Uuid(ByteArray const& bytes) {
  if (bytes.size() != UuidSize)
    throw UuidException(strf("Size mismatch in reading Uuid from ByteArray: {} vs {}", bytes.size(), UuidSize));

  bytes.copyTo(m_data.ptr(), UuidSize);
}

Uuid::Uuid(String const& hex) : Uuid(hexDecode(hex)) {}

auto Uuid::ptr() const -> char const* {
  return m_data.ptr();
}

auto Uuid::bytes() const -> ByteArray {
  return {m_data.ptr(), UuidSize};
}

auto Uuid::hex() const -> String {
  return hexEncode(m_data.ptr(), UuidSize);
}

auto Uuid::operator==(Uuid const& u) const -> bool {
  return m_data == u.m_data;
}

auto Uuid::operator!=(Uuid const& u) const -> bool {
  return m_data != u.m_data;
}

auto Uuid::operator<(Uuid const& u) const -> bool {
  return m_data < u.m_data;
}

auto Uuid::operator<=(Uuid const& u) const -> bool {
  return m_data <= u.m_data;
}

auto Uuid::operator>(Uuid const& u) const -> bool {
  return m_data > u.m_data;
}

auto Uuid::operator>=(Uuid const& u) const -> bool {
  return m_data >= u.m_data;
}

auto hash<Uuid>::operator()(Uuid const& u) const -> size_t {
  size_t hashval = 0;
  for (size_t i = 0; i < UuidSize; ++i)
    hashCombine(hashval, u.ptr()[i]);
  return hashval;
}

auto operator>>(DataStream& ds, Uuid& uuid) -> DataStream& {
  uuid = Uuid(ds.readBytes(UuidSize));
  return ds;
}

auto operator<<(DataStream& ds, Uuid const& uuid) -> DataStream& {
  ds.writeData(uuid.ptr(), UuidSize);
  return ds;
}

}// namespace Star
