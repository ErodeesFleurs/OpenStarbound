#pragma once

namespace Star {

class Image;
[[nodiscard]] Image scaleNearest(Image const& srcImage, Vec2F const& scale);
[[nodiscard]] Image scaleBilinear(Image const& srcImage, Vec2F const& scale);
[[nodiscard]] Image scaleBicubic(Image const& srcImage, Vec2F const& scale);

}
