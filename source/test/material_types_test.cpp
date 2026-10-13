import star.material_types;

#include "gtest/gtest.h"

using namespace Star;

TEST(MaterialHueTest, FiniteDegreesTruncateAndWrap) {
  struct HueCase {
    float degrees;
    unsigned hue;
  };
  HueCase const cases[] = {
    {0.0f, 0}, {45.0f, 31}, {-45.0f, 225},
    {360.0f, 0}, {-360.0f, 0}, {405.0f, 31}, {-405.0f, 225},
    {359.0f, 254}, {-359.0f, 2}, {0.5f, 0}, {-0.5f, 0}
  };
  for (auto const& test : cases)
    EXPECT_EQ(static_cast<unsigned>(materialHueFromDegrees(test.degrees)), test.hue) << test.degrees;
}
