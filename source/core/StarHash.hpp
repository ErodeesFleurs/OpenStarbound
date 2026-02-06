#pragma once

import std;

namespace Star {

// To avoid having to specialize std::hash in the std namespace, which is
// slightly annoying, Star type wrappers use Star::hash, which just defaults to
// std::hash.  Star::hash also enables template specialization with a dummy
// Enable parameter.
template <typename T, typename Enable = void>
struct hash : public std::hash<T> {};

template <class T>
inline void hashCombine(std::size_t& hash, const T& v) {
    std::hash<T> hasher;
    hash ^= hasher(v) * 2654435761 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

// Paul Larson hashing algorithm, very very *cheap* hashing function.
class PLHasher {
public:
  PLHasher(std::size_t initial = 0)
    : m_hash(initial) {}

  template <typename T>
  void put(T b) {
    m_hash = m_hash * 101 + (std::size_t)b;
  }

  std::size_t hash() const {
    return m_hash;
  }

private:
  std::size_t m_hash;
};

template <typename first_t, typename second_t>
class hash<std::pair<first_t, second_t>> {
private:
  Star::hash<first_t> firstHasher;
  Star::hash<second_t> secondHasher;

public:
  std::size_t operator()(std::pair<first_t, second_t> const& a) const {
    std::size_t hashval = firstHasher(a.first);
    hashCombine(hashval, secondHasher(a.second));
    return hashval;
  }
};

template <typename... TTypes>
class hash<std::tuple<TTypes...>> {
private:
  typedef std::tuple<TTypes...> Tuple;

  template <std::size_t N>
  std::size_t operator()(Tuple const&) const {
    return 0;
  }

  template <std::size_t N, typename THead, typename... TTail>
  std::size_t operator()(Tuple const& value) const {
    std::size_t hash = Star::hash<THead>()(std::get<N - sizeof...(TTail) - 1>(value));
    hashCombine(hash, operator()<N, TTail...>(value));
    return hash;
  }

public:
  std::size_t operator()(Tuple const& value) const {
    return operator()<sizeof...(TTypes), TTypes...>(value);
  }
};


template <typename T>
struct hash<std::optional<T>> {
  std::size_t operator()(std::optional<T> const& a) const {
    if (a)
      return Star::hash<T>()(*a);
    else
      return 0;
  }
};

template <typename EnumType>
struct hash<EnumType, typename std::enable_if<std::is_enum<EnumType>::value>::type> {
private:
  typedef typename std::underlying_type<EnumType>::type UnderlyingType;

public:
  std::size_t operator()(EnumType e) const {
    return std::hash<UnderlyingType>()((UnderlyingType)e);
  }
};

template <typename T>
std::size_t hashOf(T const& t) {
  return Star::hash<T>()(t);
}

template <typename T1, typename T2, typename... TL>
std::size_t hashOf(T1 const& t1, T2 const& t2, TL const&... rest) {
  std::size_t hash = hashOf(t1);
  hashCombine(hash, hashOf(t2, rest...));
  return hash;
};

}
