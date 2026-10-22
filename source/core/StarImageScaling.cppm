module;

import std;
import star.format;
import star.interpolation;
import star.vector;
import star.math_common;
import star.image;

export module star.image_scaling;

export namespace Star {

[[nodiscard]] std::expected<Image, StarException> scaleNearest(Image const& srcImage, Vec2F const& scale);
[[nodiscard]] std::expected<Image, StarException> scaleBilinear(Image const& srcImage, Vec2F const& scale);
[[nodiscard]] std::expected<Image, StarException> scaleBicubic(Image const& srcImage, Vec2F const& scale);

}

namespace Star {

std::expected<Image, StarException> scaleNearest(Image const& srcImage, Vec2F const& scale) {
  Vec2U srcSize = srcImage.size();
  Vec2U destSize = Vec2U::round(vmult(Vec2F(srcSize), scale));
  destSize[0] = std::max(destSize[0], 1u);
  destSize[1] = std::max(destSize[1], 1u);

  Image destImage(destSize, srcImage.pixelFormat());

  for (unsigned y = 0; y < destSize[1]; ++y) {
    for (unsigned x = 0; x < destSize[0]; ++x) {
      auto assigned = destImage.set({x, y}, srcImage.clamp(Vec2I::round(vdiv(Vec2F(x, y), scale))));
      if (!assigned)
        return std::unexpected(std::move(assigned.error()));
    }
  }
  return destImage;
}

std::expected<Image, StarException> scaleBilinear(Image const& srcImage, Vec2F const& scale) {
  Vec2U srcSize = srcImage.size();
  Vec2U destSize = Vec2U::round(vmult(Vec2F(srcSize), scale));
  destSize[0] = std::max(destSize[0], 1u);
  destSize[1] = std::max(destSize[1], 1u);

  Image destImage(destSize, srcImage.pixelFormat());

  for (unsigned y = 0; y < destSize[1]; ++y) {
    for (unsigned x = 0; x < destSize[0]; ++x) {
      auto pos = vdiv(Vec2F(x, y), scale);
      auto ipart = Vec2I::floor(pos);
      auto fpart = pos - Vec2F(ipart);

      auto result = lerp(fpart[1], lerp(fpart[0], Vec4F(srcImage.clamp(ipart[0], ipart[1])), Vec4F(srcImage.clamp(ipart[0] + 1, ipart[1]))), lerp(fpart[0], Vec4F(srcImage.clamp(ipart[0], ipart[1] + 1)), Vec4F(srcImage.clamp(ipart[0] + 1, ipart[1] + 1))));

      auto assigned = destImage.set({x, y}, Vec4B(result));
      if (!assigned)
        return std::unexpected(std::move(assigned.error()));
    }
  }

  return destImage;
}

std::expected<Image, StarException> scaleBicubic(Image const& srcImage, Vec2F const& scale) {
  Vec2U srcSize = srcImage.size();
  Vec2U destSize = Vec2U::round(vmult(Vec2F(srcSize), scale));
  destSize[0] = std::max(destSize[0], 1u);
  destSize[1] = std::max(destSize[1], 1u);

  Image destImage(destSize, srcImage.pixelFormat());

  for (unsigned y = 0; y < destSize[1]; ++y) {
    for (unsigned x = 0; x < destSize[0]; ++x) {
      auto pos = vdiv(Vec2F(x, y), scale);
      auto ipart = Vec2I::floor(pos);
      auto fpart = pos - Vec2F(ipart);

      Vec4F a = cubic4(fpart[0],
          Vec4F(srcImage.clamp(ipart[0], ipart[1])),
          Vec4F(srcImage.clamp(ipart[0] + 1, ipart[1])),
          Vec4F(srcImage.clamp(ipart[0] + 2, ipart[1])),
          Vec4F(srcImage.clamp(ipart[0] + 3, ipart[1])));

      Vec4F b = cubic4(fpart[0],
          Vec4F(srcImage.clamp(ipart[0], ipart[1] + 1)),
          Vec4F(srcImage.clamp(ipart[0] + 1, ipart[1] + 1)),
          Vec4F(srcImage.clamp(ipart[0] + 2, ipart[1] + 1)),
          Vec4F(srcImage.clamp(ipart[0] + 3, ipart[1] + 1)));

      Vec4F c = cubic4(fpart[0],
          Vec4F(srcImage.clamp(ipart[0], ipart[1] + 2)),
          Vec4F(srcImage.clamp(ipart[0] + 1, ipart[1] + 2)),
          Vec4F(srcImage.clamp(ipart[0] + 2, ipart[1] + 2)),
          Vec4F(srcImage.clamp(ipart[0] + 3, ipart[1] + 2)));

      Vec4F d = cubic4(fpart[0],
          Vec4F(srcImage.clamp(ipart[0], ipart[1] + 3)),
          Vec4F(srcImage.clamp(ipart[0] + 1, ipart[1] + 3)),
          Vec4F(srcImage.clamp(ipart[0] + 2, ipart[1] + 3)),
          Vec4F(srcImage.clamp(ipart[0] + 3, ipart[1] + 3)));

      auto result = cubic4(fpart[1], a, b, c, d);

      auto assigned = destImage.set({x, y}, Vec4B(
          clamp(result[0], 0.0f, 255.0f),
          clamp(result[1], 0.0f, 255.0f),
          clamp(result[2], 0.0f, 255.0f),
          clamp(result[3], 0.0f, 255.0f)
        ));
      if (!assigned)
        return std::unexpected(std::move(assigned.error()));
    }
  }

  return destImage;
}

}
