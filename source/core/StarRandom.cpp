#include "StarRandom.hpp"
#include "StarThread.hpp"
#include "StarTime.hpp"
#include "StarMathCommon.hpp"
#include <random>

namespace Star {

RandomSource::RandomSource() {
  init(Random::randu64());
}

void RandomSource::init() {
  init(Random::randu64());
}

RandomSource::RandomSource(uint64_t seed) {
  init(seed);
}

void RandomSource::init(uint64_t seed) {
  /* choose random initial m_carry < 809430660 and */
  /* 256 random 32-bit integers for m_data[]    */
  m_carry = seed % 809430660;

  m_data[0] = seed;
  m_data[1] = seed >> 32;

  for (size_t i = 2; i < 256; ++i)
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

void RandomSource::addEntropy(uint64_t seed) {
  // to avoid seed aliasing
  seed ^= randu64();

  // Same algo as init, but bitwise xor with existing data

  m_carry = (m_carry ^ seed) % 809430660;

  m_data[0] ^= seed;
  m_data[1] ^= (seed >> 32) ^ seed;

  for (size_t i = 2; i < 256; ++i)
    m_data[i] ^= 69069 * m_data[i - 2] + 362437;
}

[[nodiscard]] uint32_t RandomSource::randu32() {
  return gen32();
}

[[nodiscard]] uint64_t RandomSource::randu64() {
  uint64_t r = randu32();
  r = r << 32;
  r = r | randu32();
  return r;
}

[[nodiscard]] int32_t RandomSource::randi32() {
  return static_cast<int32_t>(randu32());
}

[[nodiscard]] int64_t RandomSource::randi64() {
  return static_cast<int64_t>(randu64());
}

[[nodiscard]] float RandomSource::randf() {
  return (randu32() & 0x7fffffff) / 2147483648.0f;
}

[[nodiscard]] double RandomSource::randd() {
  return (randu64() & 0x7fffffffffffffff) / 9223372036854775808.0;
}

[[nodiscard]] int64_t RandomSource::randInt(int64_t max) {
  return randUInt(max);
}

[[nodiscard]] uint64_t RandomSource::randUInt(uint64_t max) {
  uint64_t denom = static_cast<uint64_t>(-1) / (static_cast<uint64_t>(max) + 1);
  return randu64() / denom;
}

[[nodiscard]] int64_t RandomSource::randInt(int64_t min, int64_t max) {
  if (max < min)
    throw StarException("Maximum bound in randInt must be >= minimum bound!");
  return randInt(max - min) + min;
}

[[nodiscard]] uint64_t RandomSource::randUInt(uint64_t min, uint64_t max) {
  if (max < min)
    throw StarException("Maximum bound in randUInt must be >= minimum bound!");
  return randUInt(max - min) + min;
}

[[nodiscard]] float RandomSource::randf(float min, float max) {
  if (max < min)
    throw StarException("Maximum bound in randf must be >= minimum bound!");
  return randf() * (max - min) + min;
}

[[nodiscard]] double RandomSource::randd(double min, double max) {
  if (max < min)
    throw StarException("Maximum bound in randd must be >= minimum bound!");
  return randd() * (max - min) + min;
}

[[nodiscard]] bool RandomSource::randb() {
  uint32_t v = gen32();
  bool parity = false;
  while (v) {
    parity = !parity;
    v = v & (v - 1);
  }
  return parity;
}

void RandomSource::randBytes(char* buf, size_t len) {
  while (len) {
    uint32_t ui = gen32();
    for (size_t i = 0; i < 4; ++i) {
      if (len) {
        *buf = static_cast<char>(ui >> (i * 8));
        --len;
        ++buf;
      }
    }
  }
}

[[nodiscard]] ByteArray RandomSource::randBytes(size_t len) {
  ByteArray array(len, 0);
  randBytes(array.ptr(), len);
  return array;
}

// normal distribution via Box-Muller
[[nodiscard]] float RandomSource::nrandf(float stddev, float mean) {
  float rand1, rand2, distSqr;
  do {
    rand1 = 2 * randf() - 1;
    rand2 = 2 * randf() - 1;
    distSqr = rand1 * rand1 + rand2 * rand2;
  } while (distSqr >= 1);

  float mapping = std::sqrt(-2 * std::log(distSqr) / distSqr);
  return (rand1 * mapping * stddev + mean);
}

[[nodiscard]] double RandomSource::nrandd(double stddev, double mean) {
  double rand1, rand2, distSqr;
  do {
    rand1 = 2 * randd() - 1;
    rand2 = 2 * randd() - 1;
    distSqr = rand1 * rand1 + rand2 * rand2;
  } while (distSqr >= 1);

  double mapping = std::sqrt(-2 * std::log(distSqr) / distSqr);
  return (rand1 * mapping * stddev + mean);
}

[[nodiscard]] int64_t RandomSource::stochasticRound(double val) {
  double fpart = val - floor(val);
  if (randd() < fpart)
    return ceil(val);
  else
    return floor(val);
}

uint32_t RandomSource::gen32() {
  uint64_t a = 809430660;
  uint64_t t = a * m_data[++m_index] + m_carry;

  m_carry = (t >> 32);
  m_data[m_index] = t;

  return t;
}

namespace Random {
  static Maybe<RandomSource> g_randSource;
  static Mutex g_randMutex;

  static uint64_t produceRandomSeed() {
    int64_t seed = Time::monotonicTicks();
    seed *= 1099511628211;
    seed ^= (static_cast<int64_t>(std::random_device{}()) << 32) | static_cast<int64_t>(std::random_device{}());
    return seed;
  }

  void doInit(uint64_t seed) {
    g_randSource = RandomSource(seed);
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

  void init(uint64_t seed) {
    MutexLocker locker(g_randMutex);
    doInit(seed);
  }

  void addEntropy() {
    MutexLocker locker(g_randMutex);
    checkInit();
    g_randSource->addEntropy(produceRandomSeed());
  }

  void addEntropy(uint64_t seed) {
    MutexLocker locker(g_randMutex);
    checkInit();
    g_randSource->addEntropy(seed);
  }

  [[nodiscard]] uint32_t randu32() {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randu32();
  }

  [[nodiscard]] uint64_t randu64() {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randu64();
  }

  [[nodiscard]] int32_t randi32() {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randi32();
  }

  [[nodiscard]] int64_t randi64() {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randi64();
  }

  [[nodiscard]] float randf() {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randf();
  }

  [[nodiscard]] double randd() {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randd();
  }

  [[nodiscard]] float randf(float min, float max) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randf(min, max);
  }

  [[nodiscard]] double randd(double min, double max) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randd(min, max);
  }

  [[nodiscard]] bool randb() {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randb();
  }

  [[nodiscard]] long long randInt(long long max) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randInt(max);
  }

  [[nodiscard]] unsigned long long randUInt(unsigned long long max) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randUInt(max);
  }

  [[nodiscard]] long long randInt(long long min, long long max) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randInt(min, max);
  }

  [[nodiscard]] unsigned long long randUInt(unsigned long long min, unsigned long long max) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randUInt(min, max);
  }

  [[nodiscard]] float nrandf(float stddev, float mean) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->nrandf(stddev, mean);
  }

  [[nodiscard]] double nrandd(double stddev, double mean) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->nrandd(stddev, mean);
  }

  [[nodiscard]] int64_t stochasticRound(double val) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->stochasticRound(val);
  }

  void randBytes(char* buf, size_t len) {
    MutexLocker locker(g_randMutex);
    checkInit();
    g_randSource->randBytes(buf, len);
  }

  [[nodiscard]] ByteArray randBytes(size_t len) {
    MutexLocker locker(g_randMutex);
    checkInit();
    return g_randSource->randBytes(len);
  }
}

}
