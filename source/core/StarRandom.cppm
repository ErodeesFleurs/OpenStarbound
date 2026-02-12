export module star.random;

import star.byte_array;
import star.exception;

import std;

export namespace star {

using random_exception = exception_derived<"random_exception">;

class random_source {
public:
  using result_type = std::uint32_t;

  static constexpr auto min() -> result_type { return 0; }
  static constexpr auto max() -> result_type { return std::numeric_limits<result_type>::max(); }

  auto operator()() -> result_type { return gen_32(); }

  random_source();
  explicit random_source(std::uint64_t seed);

  void init();
  void init(std::uint64_t seed);

  void add_entropy();
  void add_entropy(std::uint64_t seed);

  [[nodiscard]] auto rand_u32() -> std::uint32_t;
  [[nodiscard]] auto rand_u64() -> std::uint64_t;
  [[nodiscard]] auto rand_i32() -> std::int32_t;
  [[nodiscard]] auto rand_i64() -> std::int64_t;

  [[nodiscard]] auto rand_f32() -> float;
  [[nodiscard]] auto rand_f64() -> double;

  [[nodiscard]] auto rand_int(std::int64_t max) -> std::int64_t;
  [[nodiscard]] auto rand_uint(std::uint64_t max) -> std::uint64_t;

  [[nodiscard]] auto rand_int(std::int64_t min, std::int64_t max) -> std::int64_t;
  [[nodiscard]] auto rand_uint(std::uint64_t min, std::uint64_t max) -> std::uint64_t;

  [[nodiscard]] auto rand_f32(float min, float max) -> float;
  [[nodiscard]] auto rand_f64(double min, double max) -> double;

  [[nodiscard]] auto rand_bool() -> bool;

  [[nodiscard]] auto nrand_f32(float stddev = 1.0f, float mean = 0.0f) -> float;
  [[nodiscard]] auto nrand_f64(double stddev = 1.0, double mean = 0.0) -> double;

  [[nodiscard]] auto stochastic_round(double val) -> std::int64_t;

  void rand_bytes(std::span<std::byte> buf);
  void rand_bytes(char* buf, std::size_t len);
  [[nodiscard]] auto rand_bytes(std::size_t len) -> byte_array;

  template <typename C>
  auto rand_from(C const& container) -> typename C::value_type const&;

  template <typename C>
  auto rand_from(C& container) -> typename C::value_type&;

  template <typename C>
  auto rand_value_from(C const& container) -> typename C::value_type;

  template <typename C>
  auto rand_value_from(C const& container, typename C::value_type const& default_val) -> typename C::value_type;

  template <typename C>
  void shuffle(C& container);

private:
  auto gen_32() -> std::uint32_t;

  std::array<std::uint32_t, 256> data_ = {};
  std::uint32_t carry_ = 0;
  std::uint8_t index_ = 255;
};

namespace random {
  void init();
  void init(std::uint64_t seed);

  void add_entropy();
  void add_entropy(std::uint64_t seed);

  auto rand_u32() -> std::uint32_t;
  auto rand_u64() -> std::uint64_t;
  auto rand_i32() -> std::int32_t;
  auto rand_i64() -> std::int64_t;
  auto rand_f32() -> float;
  auto rand_f64() -> double;
  auto rand_int(std::int64_t max) -> std::int64_t;
  auto rand_uint(std::uint64_t max) -> std::uint64_t;
  auto rand_int(std::int64_t min, std::int64_t max) -> std::int64_t;
  auto rand_uint(std::uint64_t min, std::uint64_t max) -> std::uint64_t;
  auto rand_f32(float min, float max) -> float;
  auto rand_f64(double min, double max) -> double;
  auto rand_bool() -> bool;

  auto nrand_f32(float stddev = 1.0f, float mean = 0.0f) -> float;
  auto nrand_f64(double stddev = 1.0, double mean = 0.0) -> double;

  auto stochastic_round(double val) -> std::int64_t;

  void rand_bytes(std::span<std::byte> buf);
  void rand_bytes(char* buf, std::size_t len);
  auto rand_bytes(std::size_t len) -> byte_array;

  template <typename C>
  auto rand_from(C const& container) -> typename C::value_type const&;
  template <typename C>
  auto rand_from(C& container) -> typename C::value_type&;
  template <typename C>
  auto rand_value_from(C const& container) -> typename C::value_type;
  template <typename C>
  auto rand_value_from(C const& container, typename C::value_type const& default_val) -> typename C::value_type;

  template <typename C>
  void shuffle(C& container);
}

template <typename C>
auto random_source::rand_from(C const& container) -> typename C::value_type const& {
  if (container.empty())
    throw random_exception("Empty container in rand_from");
  auto dist = std::uniform_int_distribution<std::size_t>(0, container.size() - 1);
  return *std::next(container.begin(), dist(*this));
}

template <typename C>
auto random_source::rand_from(C& container) -> typename C::value_type& {
  if (container.empty())
    throw random_exception("Empty container in rand_from");
  auto dist = std::uniform_int_distribution<std::size_t>(0, container.size() - 1);
  return *std::next(container.begin(), dist(*this));
}

template <typename C>
auto random_source::rand_value_from(C const& container) -> typename C::value_type {
  return rand_value_from(container, typename C::value_type());
}

template <typename C>
auto random_source::rand_value_from(C const& container, typename C::value_type const& default_val) -> typename C::value_type {
  if (container.empty())
    return default_val;
  auto dist = std::uniform_int_distribution<std::size_t>(0, container.size() - 1);
  return *std::next(container.begin(), dist(*this));
}

template <typename C>
void random_source::shuffle(C& container) {
  std::shuffle(container.begin(), container.end(), *this);
}

template <typename C>
auto random::rand_from(C const& container) -> typename C::value_type const& {
  if (container.empty())
    throw random_exception("Empty container in rand_from");
  return *std::next(container.begin(), random::rand_uint(container.size() - 1));
}

template <typename C>
auto random::rand_from(C& container) -> typename C::value_type& {
  if (container.empty())
    throw random_exception("Empty container in rand_from");
  return *std::next(container.begin(), random::rand_uint(container.size() - 1));
}

template <typename C>
auto random::rand_value_from(C const& container) -> typename C::value_type {
  return rand_value_from(container, typename C::value_type());
}

template <typename C>
auto random::rand_value_from(C const& container, typename C::value_type const& default_val) -> typename C::value_type {
  if (container.empty())
    return default_val;
  return *std::next(container.begin(), random::rand_uint(container.size() - 1));
}

template <typename C>
void random::shuffle(C& container) {
  random_source source(random::rand_u64());
  std::shuffle(container.begin(), container.end(), source);
}

} // namespace star
