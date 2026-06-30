#pragma once

#include "StarStaticRandom.hpp"
#include "StarByteArray.hpp"

namespace Star {

struct RandomExceptionTag { static constexpr char const* typeName = "RandomException"; };
using RandomException = TypedException<StarException, RandomExceptionTag>;

// Deterministic random number source.  Uses multiply-with-carry algorithm.
// Much higher quality than the predictable random number generators.  Not
// thread safe (won't crash or anything, but might return less than optimal
// values).
class RandomSource {
public:
  // Generates a RandomSource with a seed from Random::randu64()
  RandomSource();
  RandomSource(uint64_t seed);

  // Re-initializes the random number generator using the given seed.  It is
  // exactly equivalent to constructing a new RandomSource, just using the same
  // buffer.
  void init();
  void init(uint64_t seed);

  void addEntropy();
  void addEntropy(uint64_t seed);

  [[nodiscard]] uint32_t randu32();
  [[nodiscard]] uint64_t randu64();

  [[nodiscard]] int32_t randi32();
  [[nodiscard]] int64_t randi64();

  // Generates values in the range [0.0, 1.0]
  [[nodiscard]] float randf();
  // Generates values in the range [0.0, 1.0]
  [[nodiscard]] double randd();

  // Random integer from [0, max], max must be >= 0
  [[nodiscard]] int64_t randInt(int64_t max);
  [[nodiscard]] uint64_t randUInt(uint64_t max);

  // Random integer from [min, max]
  [[nodiscard]] int64_t randInt(int64_t min, int64_t max);
  [[nodiscard]] uint64_t randUInt(uint64_t min, uint64_t max);

  [[nodiscard]] float randf(float min, float max);
  [[nodiscard]] double randd(double min, double max);

  [[nodiscard]] bool randb();

  // Generates values via normal distribution with box-muller algorithm
  [[nodiscard]] float nrandf(float stddev = 1.0f, float mean = 0.0f);
  [[nodiscard]] double nrandd(double stddev = 1.0, double mean = 0.0);

  // Round a fractional value statistically towards the floor or ceiling.  For
  // example, if a value is 5.2, 80% of the time it will round to 5, but 20% of
  // the time it will round to 6.
  [[nodiscard]] int64_t stochasticRound(double val);

  void randBytes(char* buf, size_t len);
  [[nodiscard]] ByteArray randBytes(size_t len);

  // Pick a random value out of a container
  template <typename Container>
  [[nodiscard]] typename Container::value_type const& randFrom(Container const& container);
  template <typename Container>
  [[nodiscard]] typename Container::value_type& randFrom(Container& container);
  template <typename Container>
  [[nodiscard]] typename Container::value_type randValueFrom(Container const& container);
  template <typename Container>
  [[nodiscard]] typename Container::value_type randValueFrom(Container const& container, typename Container::value_type const& defaultVal);

  template <typename Container>
  void shuffle(Container& container);

private:
  [[nodiscard]] uint32_t gen32();

  uint32_t m_data[256];
  uint32_t m_carry;
  uint8_t m_index;
};

// Global static versions of the methods in RandomSource.  It is not necessary
// to initialize the global RandomSource manually, it will be automatically
// initialized with a random seed on first use if it is not already initialized.
namespace Random {
  void init();
  void init(uint64_t seed);

  void addEntropy();
  void addEntropy(uint64_t seed);

  [[nodiscard]] uint32_t randu32();
  [[nodiscard]] uint64_t randu64();
  [[nodiscard]] int32_t randi32();
  [[nodiscard]] int64_t randi64();
  [[nodiscard]] float randf();
  [[nodiscard]] double randd();
  [[nodiscard]] long long randInt(long long max);
  [[nodiscard]] unsigned long long randUInt(unsigned long long max);
  [[nodiscard]] long long randInt(long long min, long long max);
  [[nodiscard]] unsigned long long randUInt(unsigned long long min, unsigned long long max);
  [[nodiscard]] float randf(float min, float max);
  [[nodiscard]] double randd(double min, double max);
  [[nodiscard]] bool randb();

  [[nodiscard]] float nrandf(float stddev = 1.0f, float mean = 0.0f);
  [[nodiscard]] double nrandd(double stddev = 1.0, double mean = 0.0);

  [[nodiscard]] int64_t stochasticRound(double val);

  void randBytes(char* buf, size_t len);
  [[nodiscard]] ByteArray randBytes(size_t len);

  template <typename Container>
  [[nodiscard]] typename Container::value_type const& randFrom(Container const& container);
  template <typename Container>
  [[nodiscard]] typename Container::value_type& randFrom(Container& container);
  template <typename Container>
  [[nodiscard]] typename Container::value_type randValueFrom(Container const& container);
  template <typename Container>
  [[nodiscard]] typename Container::value_type randValueFrom(Container const& container, typename Container::value_type const& defaultVal);

  template <typename Container>
  void shuffle(Container& container);
}

template <typename Container>
[[nodiscard]] typename Container::value_type const& RandomSource::randFrom(Container const& container) {
  if (container.empty())
    throw RandomException("Empty container in randFrom");

  auto i = container.begin();
  std::advance(i, randUInt(container.size() - 1));
  return *i;
}

template <typename Container>
[[nodiscard]] typename Container::value_type& RandomSource::randFrom(Container& container) {
  if (container.empty())
    throw RandomException("Empty container in randFrom");

  auto i = container.begin();
  std::advance(i, randUInt(container.size() - 1));
  return *i;
}

template <typename Container>
[[nodiscard]] typename Container::value_type const& Random::randFrom(Container const& container) {
  if (container.empty())
    throw RandomException("Empty container in randFrom");

  auto i = container.begin();
  std::advance(i, Random::randUInt(container.size() - 1));
  return *i;
}

template <typename Container>
[[nodiscard]] typename Container::value_type& Random::randFrom(Container& container) {
  if (container.empty())
    throw RandomException("Empty container in randFrom");

  auto i = container.begin();
  std::advance(i, Random::randUInt(container.size() - 1));
  return *i;
}

template <typename Container>
[[nodiscard]] typename Container::value_type RandomSource::randValueFrom(Container const& container) {
  return randValueFrom(container, typename Container::value_type());
}

template <typename Container>
[[nodiscard]] typename Container::value_type RandomSource::randValueFrom(
    Container const& container, typename Container::value_type const& defaultVal) {
  if (container.empty())
    return defaultVal;

  auto i = container.begin();
  std::advance(i, randInt(container.size() - 1));
  return *i;
}

template <typename Container>
void RandomSource::shuffle(Container& container) {
  size_t max = container.size();
  std::shuffle(container.begin(), container.end(), URBG<size_t>([this, max]() { return randUInt(max - 1); }));
}

template <typename Container>
[[nodiscard]] typename Container::value_type Random::randValueFrom(Container const& container) {
  return randValueFrom(container, typename Container::value_type());
}

template <typename Container>
[[nodiscard]] typename Container::value_type Random::randValueFrom(
    Container const& container, typename Container::value_type const& defaultVal) {
  if (container.empty())
    return defaultVal;

  auto i = container.begin();
  std::advance(i, Random::randInt(container.size() - 1));
  return *i;
}

template <typename Container>
void Random::shuffle(Container& container) {
  RandomSource random;
  std::shuffle(container.begin(), container.end(), URBG<size_t>([&]() { return static_cast<size_t>(random.randu64()); }));
}

}
