#pragma once

#include "StarVector.hpp"

namespace Star {

class Image;

auto scaleNearest(Image const& srcImage, Vec2F const& scale) -> Image;
auto scaleBilinear(Image const& srcImage, Vec2F const& scale) -> Image;
auto scaleBicubic(Image const& srcImage, Vec2F const& scale) -> Image;

}// namespace Star
