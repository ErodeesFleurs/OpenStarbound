module star.random;

import std;

namespace star {

static auto produce_random_seed() -> std::uint64_t {
    std::random_device rd;
    std::uint64_t seed = rd();
    seed <<= 32;//NOLINT
    seed |= rd();
    return seed;
}

random_source::random_source() { init(produce_random_seed()); }

random_source::random_source(std::uint64_t seed) { init(seed); }

void random_source::init() { init(produce_random_seed()); }

void random_source::init(std::uint64_t seed) {
    carry_ = static_cast<std::uint32_t>(seed % 809430660);
    data_[0] = static_cast<std::uint32_t>(seed);
    data_[1] = static_cast<std::uint32_t>(seed >> 32);//NOLINT

    for (std::size_t i = 2; i < 256; ++i) {
        data_[i] = 69069 * data_[i - 2] + 362437;
    }

    index_ = 255;

    for (unsigned i = 0; i < 32; ++i) {
        gen_32();
    }
}

void random_source::add_entropy() { add_entropy(produce_random_seed()); }

void random_source::add_entropy(std::uint64_t seed) {
    seed ^= rand_u64();
    carry_ = (carry_ ^ static_cast<std::uint32_t>(seed)) % 809430660;
    data_[0] ^= static_cast<std::uint32_t>(seed);
    data_[1] ^= static_cast<std::uint32_t>(seed >> 32) ^ static_cast<std::uint32_t>(seed);//NOLINT

    for (std::size_t i = 2; i < 256; ++i) {
        data_[i] ^= 69069 * data_[i - 2] + 362437;
    }
}

auto random_source::rand_u32() -> std::uint32_t { return gen_32(); }

auto random_source::rand_u64() -> std::uint64_t {
    return (static_cast<std::uint64_t>(gen_32()) << 32) | gen_32();//NOLINT
}

auto random_source::rand_i32() -> std::int32_t { return static_cast<std::int32_t>(gen_32()); }

auto random_source::rand_i64() -> std::int64_t { return static_cast<std::int64_t>(rand_u64()); }

auto random_source::rand_f32() -> float {
    return std::uniform_real_distribution<float>(0.0F, 1.0F)(*this);
}

auto random_source::rand_f64() -> double {
    return std::uniform_real_distribution<double>(0.0, 1.0)(*this);
}

auto random_source::rand_int(std::int64_t max) -> std::int64_t { return rand_int(0, max); }

auto random_source::rand_uint(std::uint64_t max) -> std::uint64_t { return rand_uint(0, max); }

auto random_source::rand_int(std::int64_t min, std::int64_t max) -> std::int64_t {
    if (max < min) {
        throw random_exception("Maximum bound in rand_int must be >= minimum bound!");
    }
    return std::uniform_int_distribution<std::int64_t>(min, max)(*this);
}

auto random_source::rand_uint(std::uint64_t min, std::uint64_t max) -> std::uint64_t {
    if (max < min) {
        throw random_exception("Maximum bound in rand_uint must be >= minimum bound!");
    }
    return std::uniform_int_distribution<std::uint64_t>(min, max)(*this);
}

auto random_source::rand_f32(float min, float max) -> float {
    if (max < min) {
        throw random_exception("Maximum bound in rand_f32 must be >= minimum bound!");
    }
    return std::uniform_real_distribution<float>(min, max)(*this);
}

auto random_source::rand_f64(double min, double max) -> double {
    if (max < min) {
        throw random_exception("Maximum bound in rand_f64 must be >= minimum bound!");
    }
    return std::uniform_real_distribution<double>(min, max)(*this);
}

auto random_source::rand_bool() -> bool {
    // 使用 C++20 std::popcount 优化原本的奇偶校验位算法
    return (std::popcount(gen_32()) & 1) != 0;//NOLINT
}

void random_source::rand_bytes(std::span<std::byte> buf) {
    auto* ptr = reinterpret_cast<std::uint8_t*>(buf.data());
    auto len = buf.size();
    while (len >= 4) {
        std::uint32_t ui = gen_32();
        std::memcpy(ptr, &ui, 4);
        ptr += 4;
        len -= 4;
    }
    if (len > 0) {
        std::uint32_t ui = gen_32();
        std::memcpy(ptr, &ui, len);
    }
}

void random_source::rand_bytes(char* buf, std::size_t len) {
    rand_bytes(std::span(reinterpret_cast<std::byte*>(buf), len));
}

auto random_source::rand_bytes(std::size_t len) -> byte_array {
    byte_array array(len);
    rand_bytes(reinterpret_cast<char*>(array.data()), len);
    return array;
}

auto random_source::nrand_f32(float stddev, float mean) -> float {
    return std::normal_distribution<float>(mean, stddev)(*this);
}

auto random_source::nrand_f64(double stddev, double mean) -> double {
    return std::normal_distribution<double>(mean, stddev)(*this);
}

auto random_source::stochastic_round(double val) -> std::int64_t {
    double floor_val = std::floor(val);
    double fpart = val - floor_val;
    if (rand_f64() < fpart) {
        return static_cast<std::int64_t>(floor_val + 1.0);
    }
    return static_cast<std::int64_t>(floor_val);
}

auto random_source::gen_32() -> std::uint32_t {
    constexpr std::uint64_t a = 809430660;
    std::uint64_t t = a * data_[++index_] + carry_;
    carry_ = static_cast<std::uint32_t>(t >> 32);//NOLINT
    data_[index_] = static_cast<std::uint32_t>(t);
    return data_[index_];
}

namespace random {
    static std::mutex g_mutex;
    static std::optional<random_source> g_source;

    static void ensure_init() {
        if (!g_source) {
            g_source.emplace(produce_random_seed());
        }
    }

    void init() {
        std::lock_guard lock(g_mutex);
        g_source.emplace(produce_random_seed());
    }

    void init(std::uint64_t seed) {
        std::lock_guard lock(g_mutex);
        g_source.emplace(seed);
    }

    void add_entropy() {
        std::lock_guard lock(g_mutex);
        ensure_init();
        g_source->add_entropy(produce_random_seed());
    }

    void add_entropy(std::uint64_t seed) {
        std::lock_guard lock(g_mutex);
        ensure_init();
        g_source->add_entropy(seed);
    }

    auto rand_u32() -> std::uint32_t {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_u32();
    }

    auto rand_u64() -> std::uint64_t {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_u64();
    }

    auto rand_i32() -> std::int32_t {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_i32();
    }

    auto rand_i64() -> std::int64_t {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_i64();
    }

    auto rand_f32() -> float {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_f32();
    }

    auto rand_f64() -> double {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_f64();
    }

    auto rand_int(std::int64_t max) -> std::int64_t {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_int(max);
    }

    auto rand_uint(std::uint64_t max) -> std::uint64_t {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_uint(max);
    }

    auto rand_int(std::int64_t min, std::int64_t max) -> std::int64_t {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_int(min, max);
    }

    auto rand_uint(std::uint64_t min, std::uint64_t max) -> std::uint64_t {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_uint(min, max);
    }

    auto rand_f32(float min, float max) -> float {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_f32(min, max);
    }

    auto rand_f64(double min, double max) -> double {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_f64(min, max);
    }

    auto rand_bool() -> bool {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_bool();
    }

    auto nrand_f32(float stddev, float mean) -> float {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->nrand_f32(stddev, mean);
    }

    auto nrand_f64(double stddev, double mean) -> double {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->nrand_f64(stddev, mean);
    }

    auto stochastic_round(double val) -> std::int64_t {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->stochastic_round(val);
    }

    void rand_bytes(std::span<std::byte> buf) {
        std::lock_guard lock(g_mutex);
        ensure_init();
        g_source->rand_bytes(buf);
    }

    void rand_bytes(char* buf, std::size_t len) {
        std::lock_guard lock(g_mutex);
        ensure_init();
        g_source->rand_bytes(buf, len);
    }

    auto rand_bytes(std::size_t len) -> byte_array {
        std::lock_guard lock(g_mutex);
        ensure_init();
        return g_source->rand_bytes(len);
    }
}// namespace random

}// namespace star
