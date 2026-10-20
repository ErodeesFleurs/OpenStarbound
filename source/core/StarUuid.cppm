module;
import std;
import star.string;
import star.format;
import star.hash;
import star.byte_array;

import star.array;
import star.data_stream;
import star.random;

namespace Star {

struct UuidExceptionTag {
  static constexpr char const* name() { return "UuidException"; }
};
using UuidException = StarError<UuidExceptionTag, StarException>;

inline constexpr std::size_t UuidSize = 16;

class Uuid {
public:
  friend DataStream& operator>>(DataStream& ds, Uuid& uuid);
  friend DataStream& operator<<(DataStream& ds, Uuid const& uuid);
  friend struct hash<Uuid>;

  // An explicitly unfilled identity for wire staging, never identity generation.
  static Uuid nil() { return Uuid(NilTag{}); }

  explicit Uuid(RandomService& random);
  explicit Uuid(ByteArray const& bytes);
  explicit Uuid(String const& hex);

  char const* ptr() const;
  ByteArray bytes() const;
  String hex() const;

  bool operator==(Uuid const& u) const;
  bool operator!=(Uuid const& u) const;
  bool operator<(Uuid const& u) const;
  bool operator<=(Uuid const& u) const;
  bool operator>(Uuid const& u) const;
  bool operator>=(Uuid const& u) const;

private:
  struct NilTag {};
  explicit Uuid(NilTag) : m_data() {}

  Array<char, UuidSize> m_data;
};

template <>
struct hash<Uuid> {
  std::size_t operator()(Uuid const& u) const;
};

DataStream& operator>>(DataStream& ds, Uuid& uuid);
DataStream& operator<<(DataStream& ds, Uuid const& uuid);

// Decode directly without generating a throwaway random identity.
template <>
Uuid DataStream::read<Uuid>();

}

export module star.uuid;

export namespace Star {
  template <>
  struct hash<Uuid>;
}

export extern "C++" {
namespace Star {
  template <>
  Uuid DataStream::read<Uuid>();
}
}


export namespace Star {
  using ::Star::UuidExceptionTag;
  using ::Star::UuidException;
  using ::Star::UuidSize;
  using ::Star::Uuid;
}
