import std;
import star.math_common;
import star.matrix3;

#include "gtest/gtest.h"

using namespace Star;

TEST(MathTest, All) {
  EXPECT_EQ(countSetBits<std::uint32_t>(7), 3u);
  EXPECT_EQ(countSetBits<std::uint32_t>(15), 4u);
  EXPECT_EQ(countSetBits<std::uint32_t>(-1), 32u);
}

TEST(Math, CycleIncrement) {
  int a = 0;
  a = cycleIncrement(a, 10, 13);
  ASSERT_EQ(a, 10);
  a = cycleIncrement(a, 10, 13);
  ASSERT_EQ(a, 11);
  a = cycleIncrement(a, 10, 13);
  ASSERT_EQ(a, 12);
  a = cycleIncrement(a, 10, 13);
  ASSERT_EQ(a, 13);
  a = cycleIncrement(a, 10, 13);
  ASSERT_EQ(a, 10);
  int b = 14;
  b = cycleIncrement(b, 10, 13);
  ASSERT_EQ(b, 10);
}

TEST(Matrix3, RangeCopyPreservesRowMajorValuesAndDestinationBounds) {
  std::array<int, 9> input{1, 2, 3, 4, 5, 6, 7, 8, 9};
  Matrix3<int> matrix(input);
  input[3] = 99;
  EXPECT_EQ(2, matrix[0][1]);
  EXPECT_EQ(4, matrix[1][0]);

  std::array<int, 11> output;
  output.fill(-1);
  matrix.copy(std::span(output).subspan<1, 9>());
  EXPECT_EQ(output, (std::array<int, 11>{-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1}));
}
