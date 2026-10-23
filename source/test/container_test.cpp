import std;
import star.set;
import star.list;
import star.format;

#include "gtest/gtest.h"

using namespace Star;

TEST(SetTest, All) {
  Set<int> a = {1, 2, 3, 4};
  Set<int> b = {2, 4, 5};
  EXPECT_EQ(a.difference(b), Set<int>({1, 3}));
  EXPECT_EQ(a.intersection(b), Set<int>({2, 4}));
  EXPECT_EQ(a.combination(b), Set<int>({1, 2, 3, 4, 5}));
}

namespace {

template <typename Container>
void expectFilteredCopyContract(Container const& input) {
  std::vector<int> const original{1, 2, 1, 3};
  std::vector<int> visited;
  auto filtered = input.filtered([&](int value) {
    visited.push_back(value);
    return value == 1;
  });
  static_assert(std::same_as<decltype(filtered), Container>);
  EXPECT_EQ(std::vector<int>(filtered.begin(), filtered.end()), (std::vector<int>{1, 1}));
  EXPECT_EQ(visited, original);
  EXPECT_EQ(std::vector<int>(input.begin(), input.end()), original);

  visited.clear();
  auto accepted = input.filtered([&](int value) -> std::expected<bool, StarException> {
    visited.push_back(value);
    return value == 1;
  });
  static_assert(std::same_as<decltype(accepted), std::expected<Container, StarException>>);
  ASSERT_TRUE(accepted);
  EXPECT_EQ(std::vector<int>(accepted->begin(), accepted->end()), (std::vector<int>{1, 1}));
  EXPECT_EQ(visited, original);
  EXPECT_EQ(std::vector<int>(input.begin(), input.end()), original);

  visited.clear();
  auto failed = input.filtered([&](int& value) -> std::expected<bool, StarException> {
    visited.push_back(value);
    if (value == 2)
      return std::unexpected(OutOfRangeException("filtered first error", false));
    if (value == 3)
      return std::unexpected(OutOfRangeException("filtered later error", false));
    value += 10;
    return false;
  });
  ASSERT_FALSE(failed);
  EXPECT_TRUE(failed.error().template is<OutOfRangeException>());
  EXPECT_NE(std::string_view(failed.error().what()).find("filtered first error"), std::string_view::npos);
  EXPECT_EQ(visited, (std::vector<int>{1, 2}));
  EXPECT_EQ(std::vector<int>(input.begin(), input.end()), original);

  Container const empty;
  int calls = 0;
  auto emptyFiltered = empty.filtered([&](int) {
    ++calls;
    return true;
  });
  EXPECT_TRUE(emptyFiltered.empty());
  auto emptyAccepted = empty.filtered([&](int) -> std::expected<bool, StarException> {
    ++calls;
    return std::unexpected(OutOfRangeException("empty predicate must not run", false));
  });
  ASSERT_TRUE(emptyAccepted);
  EXPECT_TRUE(emptyAccepted->empty());
  EXPECT_EQ(calls, 0);
}

}

TEST(ListTest, FilteredCopyPreservesContainerAndPredicateContract) {
  expectFilteredCopyContract(List<int>{1, 2, 1, 3});
  auto bounded = StaticList<int, 4>::create({1, 2, 1, 3});
  ASSERT_TRUE(bounded);
  expectFilteredCopyContract(*bounded);
  expectFilteredCopyContract(SmallList<int, 2>{1, 2, 1, 3});
  expectFilteredCopyContract(Deque<int>{1, 2, 1, 3});
  expectFilteredCopyContract(LinkedList<int>{1, 2, 1, 3});
}
