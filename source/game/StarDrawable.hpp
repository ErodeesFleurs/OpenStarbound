#pragma once

#include "StarAssetPath.hpp"
#include "StarColor.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"

import std;

namespace Star {

struct Drawable {
  struct LinePart {
    Line2F line;
    float width;
    std::optional<Color> endColor;
  };

  struct PolyPart {
    PolyF poly;
  };

  struct ImagePart {
    AssetPath image;
    // Transformation of the image in pixel space (0, 0) - (width, height) to
    // the final drawn space
    Mat3F transformation;

    // Add directives to this ImagePart, while optionally keeping the
    // transformed center of the image the same if the directives change the
    // image size.
    auto addDirectives(Directives const& directives, bool keepImageCenterPosition = false) -> ImagePart&;
    auto addDirectivesGroup(DirectivesGroup const& directivesGroup, bool keepImageCenterPosition = false) -> ImagePart&;

    // Remove directives from this ImagePart, while optionally keeping the
    // transformed center of the image the same if the directives change the
    // image size.
    auto removeDirectives(bool keepImageCenterPosition = false) -> ImagePart&;
  };

  static auto makeLine(Line2F const& line, float lineWidth, Color const& color, Vec2F const& position = Vec2F()) -> Drawable;
  static auto makePoly(PolyF poly, Color const& color, Vec2F const& position = Vec2F()) -> Drawable;
  static auto makeImage(AssetPath image, float pixelSize, bool centered, Vec2F const& position, Color const& color = Color::White) -> Drawable;

  template <typename DrawablesContainer>
  static void translateAll(DrawablesContainer& drawables, Vec2F const& translation);

  template <typename DrawablesContainer>
  static void rotateAll(DrawablesContainer& drawables, float rotation, Vec2F const& rotationCenter = Vec2F());

  template <typename DrawablesContainer>
  static void scaleAll(DrawablesContainer& drawables, float scaling, Vec2F const& scaleCenter = Vec2F());

  template <typename DrawablesContainer>
  static void scaleAll(DrawablesContainer& drawables, Vec2F const& scaling, Vec2F const& scaleCenter = Vec2F());

  template <typename DrawablesContainer>
  static void transformAll(DrawablesContainer& drawables, Mat3F const& transformation);

  template <typename DrawablesContainer>
  static void rebaseAll(DrawablesContainer& drawables, Vec2F const& newBase = Vec2F());

  template <typename DrawablesContainer>
  static auto boundBoxAll(DrawablesContainer const& drawables, bool cropImages) -> RectF;

  Drawable();
  explicit Drawable(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  void translate(Vec2F const& translation);
  void rotate(float rotation, Vec2F const& rotationCenter = Vec2F());
  void scale(float scaling, Vec2F const& scaleCenter = Vec2F());
  void scale(Vec2F const& scaling, Vec2F const& scaleCenter = Vec2F());
  void transform(Mat3F const& transformation);

  // Change the base position of a drawable without changing the position that
  // the drawable appears, useful to re-base a set of drawables at the same
  // position so that they will be transformed together with minimal drift
  // between them.
  void rebase(Vec2F const& newBase = Vec2F());

  [[nodiscard]] auto boundBox(bool cropImages) const -> RectF;

  [[nodiscard]] auto isLine() const -> bool;
  auto linePart() -> LinePart&;
  [[nodiscard]] auto linePart() const -> LinePart const&;

  [[nodiscard]] auto isPoly() const -> bool;
  auto polyPart() -> PolyPart&;
  [[nodiscard]] auto polyPart() const -> PolyPart const&;

  [[nodiscard]] auto isImage() const -> bool;
  auto imagePart() -> ImagePart&;
  [[nodiscard]] auto imagePart() const -> ImagePart const&;

  MVariant<LinePart, PolyPart, ImagePart> part;

  Vec2F position;
  Color color;
  bool fullbright;
};

auto operator>>(DataStream& ds, Drawable& drawable) -> DataStream&;
auto operator<<(DataStream& ds, Drawable const& drawable) -> DataStream&;

template <typename DrawablesContainer>
void Drawable::translateAll(DrawablesContainer& drawables, Vec2F const& translation) {
  for (auto& drawable : drawables)
    drawable.translate(translation);
}

template <typename DrawablesContainer>
void Drawable::rotateAll(DrawablesContainer& drawables, float rotation, Vec2F const& rotationCenter) {
  for (auto& drawable : drawables)
    drawable.rotate(rotation, rotationCenter);
}

template <typename DrawablesContainer>
void Drawable::scaleAll(DrawablesContainer& drawables, float scaling, Vec2F const& scaleCenter) {
  for (auto& drawable : drawables)
    drawable.scale(scaling, scaleCenter);
}

template <typename DrawablesContainer>
void Drawable::scaleAll(DrawablesContainer& drawables, Vec2F const& scaling, Vec2F const& scaleCenter) {
  for (auto& drawable : drawables)
    drawable.scale(scaling, scaleCenter);
}

template <typename DrawablesContainer>
void Drawable::transformAll(DrawablesContainer& drawables, Mat3F const& transformation) {
  for (auto& drawable : drawables)
    drawable.transform(transformation);
}

template <typename DrawablesContainer>
void Drawable::rebaseAll(DrawablesContainer& drawables, Vec2F const& newBase) {
  for (auto& drawable : drawables)
    drawable.rebase(newBase);
}

template <typename DrawablesContainer>
auto Drawable::boundBoxAll(DrawablesContainer const& drawables, bool cropImages) -> RectF {
  RectF boundBox = RectF::null();
  for (auto const& drawable : drawables)
    boundBox.combine(drawable.boundBox(cropImages));
  return boundBox;
}

inline auto Drawable::isLine() const -> bool {
  return part.is<LinePart>();
}

inline auto Drawable::linePart() -> Drawable::LinePart& {
  return part.get<LinePart>();
}

inline auto Drawable::linePart() const -> Drawable::LinePart const& {
  return part.get<LinePart>();
}

inline auto Drawable::isPoly() const -> bool {
  return part.is<PolyPart>();
}

inline auto Drawable::polyPart() -> Drawable::PolyPart& {
  return part.get<PolyPart>();
}

inline auto Drawable::polyPart() const -> Drawable::PolyPart const& {
  return part.get<PolyPart>();
}

inline auto Drawable::isImage() const -> bool {
  return part.is<ImagePart>();
}

inline auto Drawable::imagePart() -> Drawable::ImagePart& {
  return part.get<ImagePart>();
}

inline auto Drawable::imagePart() const -> Drawable::ImagePart const& {
  return part.get<ImagePart>();
}

}// namespace Star
