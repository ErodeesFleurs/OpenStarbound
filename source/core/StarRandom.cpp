#include "StarRandom.hpp"

#include "StarThread.hpp"
#include "StarTime.hpp"

import std;

namespace Star {

RandomSource::RandomSource() {
  init(Random::randu64());
}

void RandomSource::init() {
  init(Random::randu64());
}

RandomSource::RandomSource(std::uint64_t seed) {
  init(seed);
}

void RandomSource::init(std::uint64_t seed) {
  /* choose random initial m_carry < 809430660 and */
  /* 256 random 32-bit integers for m_data[]    */
  m_carry = seed % 809430660;

  m_data[0] = seed;
  m_data[1] = seed >> 32;

  for (std::size_t i = 2; i < 256; ++i)
    m_data[i] = 69069 * m_data[i - 2] + 362437;

  m_index = 255;

  // Hard-coded initial skip of random values, to get the random generator
  // going.
  const unsigned RandomInitialSkip = 32;
  for (unsigned i = 0; i < RandomInitialSkip; ++i)
    gen32();
}

void RandomSource::addEntropy() {
  addEntropy(Random::randu64());
}

void RandomSource::addEntropy(std::uint64_t seed) {
  // to avoid seed aliasing
  seed ^= randu64();

  // Same algo as init, but bitwise xor with existing data

  m_carry = (m_carry ^ seed) % 809430660;

  m_data[0] ^= seed;
  m_data[1] ^= (seed >> 32) ^ seed;

  for (std::size_t i = 2; i < 256; ++i)
    m_data[i] ^= 69069 * m_data[i - 2] + 362437;
}

auto RandomSource::randu32() -> std::uint32_t {
  return gen32();
}

auto RandomSource::randu64() -> std::uint64_t {
  std::uint64_t r = randu32();
  r = r << 32;
  r = r | randu32();
  return r;
}

auto RandomSource::randi32() -> std::int32_t {
  return (std::int32_t)(randu32());
}

auto RandomSource::randi64() -> std::int64_t {
  return (std::int64_t)(randu64());
}

auto RandomSource::randf() -> float {
  return (randu32() & 0x7fffffff) / 2147483648.0f;
}

auto RandomSource::randd() -> double {
  return (randu64() & 0x7fffffffffffffff) / 9223372036854775808.0;
}

auto RandomSource::randInt(std::int64_t max) -> std::int64_t {
  return randUInt(max);
}

auto RandomSource::randUInt(std::uint64_t max) -> std::uint64_t {
  std::uint64_t denom = (std::uint64_t)(-1) / ((std::uint64_t)max + 1);
  return randu64() / denom;
}

auto RandomSource::randInt(std::int64_t min, std::int64_t max) -> std::int64_t {
  if (max < min)
    throw StarException("Maximum bound in randInt must be >= minimum bound!");
  return randInt(max - min) + min;
}

auto RandomSource::randUInt(std::uint64_t min, std::uint64_t max) -> std::uint64_t {
  if (max < min)
    throw StarException("Maximum bound in randUInt must be >= minimum bound!");
  return randUInt(max - min) + min;
}

auto RandomSource::randf(float min, float max) -> float {
  if (max < min)
    throw StarException("Maximum bound in randf must be >= minimum bound!");
  return randf() * (max - min) + min;
}

auto RandomSource::randd(double min, double max) -> double {
  if (max < min)
    throw StarException("Maximum bound in randd must be >= minimum bound!");
  return randd() * (max - min) + min;
}

auto RandomSource::randb() -> bool {
  std::uint32_t v = gen32();
  bool parity = false;
  while (v) {
    parity = !parity;
    v = v & (v - 1);
  }
  return parity;
}

void RandomSource::randBytes(char* buf, std::size_t len) {
  while (len) {
    std::uint32_t ui = gen32();
    for (std::size_t i = 0; i < 4; ++i) {
      if (len) {
        *buf = (char)(ui >> (i * 8));
        --len;
        ++buf;
      }
    }
  }
}

auto RandomSource::randBytes(std::size_t len) -> ByteArray {
  ByteArray array(len, 0);
  randBytes(array.ptr(), len);
  return array;
}

// normal distribution via Box-Muller
auto RandomSource::nrandf(float stddev, float mean) -> float {
  float rand1, rand2, distSqr;
  do {
    rand1 = 2 * randf() - 1;
    rand2 = 2 * randf() - 1;
    distSqr = rand1 * rand1 + rand2 * rand2;
  } while (distSqr >= 1);

  float mapping = std::sqrt(-2 * std::log(distSqr) / distSqr);
  return (rand1 * mapping * stddev + mean);
}

auto RandomSource::nrandd(double stddev, double mean) -> double {
  double rand1, rand2, distSqr;
  do {
    rand1 = 2 * randd() - 1;
    rand2 = 2 * randd() - 1;
    distSqr = rand1 * rand1 + rand2 * rand2;
  } while (distSqr >= 1);

  double mapping = std::sqrt(-2 * std::log(distSqr) / distSqr);
  return (rand1 * mapping * stddev + mean);
}

auto RandomSource::stochasticRound(double val) -> std::int64_t {
  double fpart = val - std::floor(val);
  if (randd() < fpart)
    return std::ceil(val);
  else
    return std::floor(val);
}

auto RandomSource::gen32() -> std::uint32_t {
  std::uint64_t a = 809430660;
  std::uint64_t t = a * m_data[++m_index] + m_carry;

  m_carry = (t >> 32);
  m_data[m_index] = t;

  return t;
}

namespace Random {
static std::optional<RandomSource> g_randSource;
static Mutex g_randMutex;

static auto produceRandomSeed() -> std::uint64_t {
  std::int64_t seed = Time::monotonicTicks();
  seed *= 1099511628211;
  seed ^= (((std::int64_t)std::rand()) << 32) | ((std::int64_t)std::rand());
  return seed;
}

void doInit(std::uint64_t seed) {
  g_randSource = RandomSource(seed);
  // Also set the C stdlib random seed
  std::srand(seed);
}

void checkInit() {
  // Mutex must already be held
  if (!g_randSource) {
    doInit(produceRandomSeed());
  }
}

void init() {
  MutexLocker locker(g_randMutex);
  doInit(produceRandomSeed());
}

void init(std::uint64_t seed) {
  MutexLocker locker(g_randMutex);
  doInit(seed);
}

void addEntropy() {
  MutexLocker locker(g_randMutex);
  checkInit();
  g_randSource->addEntropy(produceRandomSeed());
}

void addEntropy(std::uint64_t seed) {
  MutexLocker locker(g_randMutex);
  checkInit();
  g_randSource->addEntropy(seed);
}

auto randu32() -> std::uint32_t {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randu32();
}

auto randu64() -> std::uint64_t {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randu64();
}

auto randi32() -> std::int32_t {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randi32();
}

auto randi64() -> std::int64_t {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randi64();
}

auto randf() -> float {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randf();
}

auto randd() -> double {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randd();
}

auto randf(float min, float max) -> float {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randf(min, max);
}

auto randd(double min, double max) -> double {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randd(min, max);
}

auto randb() -> bool {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randb();
}

auto randInt(long long max) -> long long {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randInt(max);
}

auto randUInt(unsigned long long max) -> unsigned long long {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randUInt(max);
}

auto randInt(long long min, long long max) -> long long {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randInt(min, max);
}

auto randUInt(unsigned long long min, unsigned long long max) -> unsigned long long {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randUInt(min, max);
}

auto nrandf(float stddev, float mean) -> float {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->nrandf(stddev, mean);
}

auto nrandd(double stddev, double mean) -> double {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->nrandd(stddev, mean);
}

auto stochasticRound(double val) -> std::int64_t {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->stochasticRound(val);
}

void randBytes(char* buf, std::size_t len) {
  MutexLocker locker(g_randMutex);
  checkInit();
  g_randSource->randBytes(buf, len);
}

auto randBytes(std::size_t len) -> ByteArray {
  MutexLocker locker(g_randMutex);
  checkInit();
  return g_randSource->randBytes(len);
}
}// namespace Random

}// namespace Star
