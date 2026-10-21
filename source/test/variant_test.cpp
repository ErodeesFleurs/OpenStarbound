import std;
import star.optional;
import star.data_stream_devices;
import star.data_stream_extra;

#include "gtest/gtest.h"

using namespace Star;

namespace {
struct OptionalThrowingMove {
  std::shared_ptr<int> value;
  bool fail;

  OptionalThrowingMove(std::shared_ptr<int> value, bool fail)
    : value(std::move(value)), fail(fail) {}
  OptionalThrowingMove(OptionalThrowingMove const&) = default;
  OptionalThrowingMove(OptionalThrowingMove&& other) {
    if (other.fail)
      throw std::runtime_error("move");
    value = std::move(other.value);
    fail = other.fail;
  }
  OptionalThrowingMove& operator=(OptionalThrowingMove const&) = default;
  OptionalThrowingMove& operator=(OptionalThrowingMove&&) noexcept = default;
};

template <typename T, std::size_t N>
void expectVariantWire(T const& value, std::array<unsigned char, N> const& expected) {
  DataStreamBuffer stream;
  stream << value;
  ASSERT_EQ(stream.size(), expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i)
    EXPECT_EQ(static_cast<unsigned char>(stream.ptr()[i]), expected[i]);

  stream.seek(0);
  T restored{};
  stream >> restored;
  EXPECT_EQ(restored, value);
  EXPECT_TRUE(stream.atEnd());
}
}

TEST(OptionalOperations, TakePreservesSourceOnFailureAndTransfersOwnership) {
  auto owner = std::make_shared<int>(42);
  std::weak_ptr<int> weak = owner;
  std::optional<OptionalThrowingMove> source(std::in_place, owner, true);
  owner.reset();
  EXPECT_THROW(takeOptional(source), std::runtime_error);
  ASSERT_TRUE(source);
  EXPECT_EQ(*source->value, 42);
  source->fail = false;
  auto extracted = takeOptional(source);
  EXPECT_FALSE(source);
  EXPECT_EQ(*extracted.value, 42);
  EXPECT_THROW(takeOptional(source), std::bad_optional_access);
  EXPECT_FALSE(weak.expired());
  extracted.value.reset();
  EXPECT_TRUE(weak.expired());
}

TEST(VariantSerialization, DiscriminantAndPayloadBytes) {
  using Value = std::variant<std::uint16_t, std::uint32_t>;
  expectVariantWire(Value(std::in_place_index<0>, 0x1234),
      std::array<unsigned char, 3>{0x00, 0x12, 0x34});
  expectVariantWire(Value(std::in_place_index<1>, 0x89abcdefu),
      std::array<unsigned char, 5>{0x01, 0x89, 0xab, 0xcd, 0xef});
}

TEST(VariantSerialization, MonostateAndDuplicateAlternativeBytes) {
  using Value = std::variant<std::monostate, std::uint16_t, std::uint16_t>;
  expectVariantWire(Value{}, std::array<unsigned char, 1>{0x00});
  expectVariantWire(Value(std::in_place_index<1>, 0x1234),
      std::array<unsigned char, 3>{0x01, 0x12, 0x34});
  expectVariantWire(Value(std::in_place_index<2>, 0x1234),
      std::array<unsigned char, 3>{0x02, 0x12, 0x34});
}
