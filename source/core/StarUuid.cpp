import std;
import star.hash;
import star.data_stream;
import star.uuid;
import star.random;
import star.format;
import star.string;
import star.byte_array;
import star.encode;

namespace Star {

Uuid::Uuid(RandomService& random) {
  random.randBytes(m_data.ptr(), UuidSize);
}

std::expected<Uuid, StarException> Uuid::create(ByteArray const& bytes) {
  if (bytes.size() != UuidSize)
    return std::unexpected(UuidException(strf("Size mismatch in reading Uuid from ByteArray: {} vs {}", bytes.size(), UuidSize)));
  auto uuid = nil();
  bytes.copyTo(uuid.m_data.ptr(), UuidSize);
  return uuid;
}

std::expected<Uuid, StarException> Uuid::create(String const& hex) {
  auto bytes = hexDecode(hex);
  if (!bytes)
    return std::unexpected(std::move(bytes.error()));
  return create(*bytes);
}

char const* Uuid::ptr() const {
  return m_data.ptr();
}

ByteArray Uuid::bytes() const {
  return ByteArray(m_data.ptr(), UuidSize);
}

String Uuid::hex() const {
  return hexEncode(m_data.ptr(), UuidSize);
}

bool Uuid::operator==(Uuid const& u) const {
  return m_data == u.m_data;
}

bool Uuid::operator!=(Uuid const& u) const {
  return m_data != u.m_data;
}

bool Uuid::operator<(Uuid const& u) const {
  return m_data < u.m_data;
}

bool Uuid::operator<=(Uuid const& u) const {
  return m_data <= u.m_data;
}

bool Uuid::operator>(Uuid const& u) const {
  return m_data > u.m_data;
}

bool Uuid::operator>=(Uuid const& u) const {
  return m_data >= u.m_data;
}

std::size_t hash<Uuid>::operator()(Uuid const& u) const {
  std::size_t hashval = 0;
  for (std::size_t i = 0; i < UuidSize; ++i)
    hashCombine(hashval, u.ptr()[i]);
  return hashval;
}

std::expected<DataStream*, StarException> operator>>(DataStream& ds, Uuid& uuid) {
  auto result = ds.read<Uuid>();
  if (!result)
    return std::unexpected(std::move(result.error()));
  uuid = std::move(*result);
  return &ds;
}

template <>
std::expected<Uuid, StarException> DataStream::read<Uuid>() {
  auto bytes = readBytes(UuidSize);
  if (!bytes)
    return std::unexpected(std::move(bytes.error()));
  return Uuid::create(*bytes);
}

std::expected<DataStream*, StarException> operator<<(DataStream& ds, Uuid const& uuid) {
  auto result = ds.writeData(uuid.ptr(), UuidSize);
  if (!result)
    return std::unexpected(std::move(result.error()));
  return &ds;
}

}
