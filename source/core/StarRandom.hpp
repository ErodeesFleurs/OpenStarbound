#pragma once

#include "StarByteArray.hpp"
#include "StarException.hpp"
#include "StarStaticRandom.hpp"

import std;

namespace Star {

using RandomException = ExceptionDerived<"RandomException">;

// Deterministic random number source.  Uses multiply-with-carry algorithm.
// Much higher quality than the predictable random number generators.  Not
// thread safe (won't crash or anything, but might return less than optimal
// values).
class RandomSource {
public:
  // Generates a RandomSource with a seed from Random::randu64()
  RandomSource();
  RandomSource(std::uint64_t seed);

  // Re-initializes the random number generator using the given seed.  It is
  // exactly equivalent to constructing a new RandomSource, just using the same
  // buffer.
  void init();
  void init(std::uint64_t seed);

  void addEntropy();
  void addEntropy(std::uint64_t seed);

  auto randu32() -> std::uint32_t;
  auto randu64() -> std::uint64_t;

  auto randi32() -> std::int32_t;
  auto randi64() -> std::int64_t;

  // Generates values in the range [0.0, 1.0]
  auto randf() -> float;
  // Generates values in the range [0.0, 1.0]
  auto randd() -> double;

  // Random integer from [0, max], max must be >= 0
  auto randInt(std::int64_t max) -> std::int64_t;
  auto randUInt(std::uint64_t max) -> std::uint64_t;

  // Random integer from [min, max]
  auto randInt(std::int64_t min, std::int64_t max) -> std::int64_t;
  auto randUInt(std::uint64_t min, std::uint64_t max) -> std::uint64_t;

  auto randf(float min, float max) -> float;
  auto randd(double min, double max) -> double;

  auto randb() -> bool;

  // Generates values via normal distribution with box-muller algorithm
  auto nrandf(float stddev = 1.0f, float mean = 0.0f) -> float;
  auto nrandd(double stddev = 1.0, double mean = 0.0) -> double;

  // Round a fractional value statistically towards the floor or ceiling.  For
  // example, if a value is 5.2, 80% of the time it will round to 5, but 20% of
  // the time it will round to 6.
  auto stochasticRound(double val) -> std::int64_t;

  void randBytes(char* buf, std::size_t len);
  auto randBytes(std::size_t len) -> ByteArray;

  // Pick a random value out of a container
  template <typename Container>
  auto randFrom(Container const& container) -> typename Container::value_type const&;
  template <typename Container>
  auto randFrom(Container& container) -> typename Container::value_type&;
  template <typename Container>
  auto randValueFrom(Container const& container) -> typename Container::value_type;
  template <typename Container>
  auto randValueFrom(Container const& container, typename Container::value_type const& defaultVal) -> typename Container::value_type;

  template <typename Container>
  void shuffle(Container& container);

private:
  auto gen32() -> std::uint32_t;

  std::array<std::uint32_t, 256> m_data;
  std::uint32_t m_carry;
  std::uint8_t m_index;
};

// Global static versions of the methods in RandomSource.  It is not necessary
// to initialize the global RandomSource manually, it will be automatically
// initialized with a random seed on first use if it is not already initialized.
namespace Random {
void init();
void init(std::uint64_t seed);

void addEntropy();
void addEntropy(std::uint64_t seed);

auto randu32() -> std::uint32_t;
auto randu64() -> std::uint64_t;
auto randi32() -> std::int32_t;
auto randi64() -> std::int64_t;
auto randf() -> float;
auto randd() -> double;
auto randInt(long long max) -> long long;
auto randUInt(unsigned long long max) -> unsigned long long;
auto randInt(long long min, long long max) -> long long;
auto randUInt(unsigned long long min, unsigned long long max) -> unsigned long long;
auto randf(float min, float max) -> float;
auto randd(double min, double max) -> double;
auto randb() -> bool;

auto nrandf(float stddev = 1.0f, float mean = 0.0f) -> float;
auto nrandd(double stddev = 1.0, double mean = 0.0) -> double;

auto stochasticRound(double val) -> std::int64_t;

void randBytes(char* buf, std::size_t len);
auto randBytes(std::size_t len) -> ByteArray;

template <typename Container>
auto randFrom(Container const& container) -> typename Container::value_type const&;
template <typename Container>
auto randFrom(Container& container) -> typename Container::value_type&;
template <typename Container>
auto randValueFrom(Container const& container) -> typename Container::value_type;
template <typename Container>
auto randValueFrom(Container const& container, typename Container::value_type const& defaultVal) -> typename Container::value_type;

template <typename Container>
void shuffle(Container& container);
}// namespace Random

template <typename Container>
auto RandomSource::randFrom(Container const& container) -> typename Container::value_type const& {
  if (container.empty())
    throw RandomException("Empty container in randFrom");

  auto i = container.begin();
  std::advance(i, randUInt(container.size() - 1));
  return *i;
}

template <typename Container>
auto RandomSource::randFrom(Container& container) -> typename Container::value_type& {
  if (container.empty())
    throw RandomException("Empty container in randFrom");

  auto i = container.begin();
  std::advance(i, randUInt(container.size() - 1));
  return *i;
}

template <typename Container>
auto Random::randFrom(Container const& container) -> typename Container::value_type const& {
  if (container.empty())
    throw RandomException("Empty container in randFrom");

  auto i = container.begin();
  std::advance(i, Random::randUInt(container.size() - 1));
  return *i;
}

template <typename Container>
auto Random::randFrom(Container& container) -> typename Container::value_type& {
  if (container.empty())
    throw RandomException("Empty container in randFrom");

  auto i = container.begin();
  std::advance(i, Random::randUInt(container.size() - 1));
  return *i;
}

template <typename Container>
auto RandomSource::randValueFrom(Container const& container) -> typename Container::value_type {
  return randValueFrom(container, typename Container::value_type());
}

template <typename Container>
auto RandomSource::randValueFrom(
  Container const& container, typename Container::value_type const& defaultVal) -> typename Container::value_type {
  if (container.empty())
    return defaultVal;

  auto i = container.begin();
  std::advance(i, randInt(container.size() - 1));
  return *i;
}

template <typename Container>
void RandomSource::shuffle(Container& container) {
  std::size_t max = container.size();
  std::shuffle(container.begin(), container.end(), URBG<std::size_t>([this, max]() -> auto { return randUInt(max - 1); }));
}

template <typename Container>
auto Random::randValueFrom(Container const& container) -> typename Container::value_type {
  return randValueFrom(container, typename Container::value_type());
}

template <typename Container>
auto Random::randValueFrom(
  Container const& container, typename Container::value_type const& defaultVal) -> typename Container::value_type {
  if (container.empty())
    return defaultVal;

  auto i = container.begin();
  std::advance(i, Random::randInt(container.size() - 1));
  return *i;
}

template <typename Container>
void Random::shuffle(Container& container) {
  RandomSource random;
  std::shuffle(container.begin(), container.end(), URBG<std::size_t>([&]() -> auto { return static_cast<std::size_t>(random.randu64()); }));
}

}// namespace Star
