import std;
import star.multi_array;
import star.interpolation;
import star.multi_array_interpolator;
import star.multi_table;

#include "gtest/gtest.h"

import star.array;
import star.math_common;
using namespace Star;

TEST(MultiArrayTest, All) {
  MultiArray<int, 2> table(10, 12);
  table.forEach([](Array2S const& index, int& val) {
      val = (index[0] + 1) * index[1];
    });

  EXPECT_EQ(table(3, 4), 16);
  EXPECT_EQ(table(5, 2), 12);
  EXPECT_EQ(table(0, 9), 9);
  EXPECT_EQ(table(8, 1), 9);
  EXPECT_EQ(table(0, 1), 1);
  EXPECT_EQ(table(8, 9), 81);

  MultiArray<int, 3> table3(5, 6, 7);
  table3.forEach([](Array3S const& index, int& val) {
      val = index[0] + index[1] + index[2];
    });

  EXPECT_EQ(table3(0, 0, 0), 0);
  EXPECT_EQ(table3(1, 1, 0), 2);
  EXPECT_EQ(table3(2, 0, 2), 4);
  EXPECT_EQ(table3(1, 1, 1), 3);
  EXPECT_EQ(table3(0, 1, 0), 1);
  EXPECT_EQ(table3(2, 2, 2), 6);
  EXPECT_EQ(table3(3, 3, 3), 9);
  EXPECT_EQ(table3(4, 4, 4), 12);

  table3.forEach({3, 3, 3}, {2, 2, 2}, [](Array3S const&, int& val) {
      val = 42;
    });

  EXPECT_EQ(table3(2, 2, 2), 6);
  EXPECT_EQ(table3(3, 3, 4), 42);
  EXPECT_EQ(table3(4, 4, 4), 42);
}

TEST(MultiTableTest, All) {
  MultiTable2F table;
  table.setRanges({MultiTable2F::Range{0, 2, 4, 6, 8, 10}, MultiTable2F::Range{0, 5, 10}});
  table.setInterpolationMode(InterpolationMode::Linear);
  table.setBoundMode(BoundMode::Clamp);
  table.eval([](Array2F const& index) {
      return index[0] * index[1];
    });

  auto interpolated1 = table.interpolate({1.0f, 1.0f});
  ASSERT_TRUE(interpolated1);
  EXPECT_LT(std::fabs(*interpolated1 - 1.0f), 0.001f);
  auto interpolated2 = table.interpolate({9.0f, 9.0f});
  ASSERT_TRUE(interpolated2);
  EXPECT_LT(std::fabs(*interpolated2 - 81.0f), 0.001f);
  auto interpolated3 = table.interpolate({6.0f, 10.0f});
  ASSERT_TRUE(interpolated3);
  EXPECT_LT(std::fabs(*interpolated3 - 60.0f), 0.001f);
  auto interpolated4 = table.interpolate({6.0f, 11.0f});
  ASSERT_TRUE(interpolated4);
  EXPECT_LT(std::fabs(*interpolated4 - 60.0f), 0.001f);
  EXPECT_LT(std::fabs(table.get({1, 1}) - 10.0f), 0.001f);

  table.setInterpolationMode(InterpolationMode::HalfStep);
  auto halfStep1 = table.interpolate({0.5f, 0.5f});
  ASSERT_TRUE(halfStep1);
  EXPECT_LT(std::fabs(*halfStep1 - 0.0f), 0.001f);
  auto halfStep2 = table.interpolate({4.0, 4.0});
  ASSERT_TRUE(halfStep2);
  EXPECT_LT(std::fabs(*halfStep2 - 20.0f), 0.001f);
}
