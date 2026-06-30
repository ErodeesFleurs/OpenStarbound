#pragma once

#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {
class DataStream;

struct Drawable {
  struct LinePart {
    Line2F line;
    float width;
    Maybe<Color> endColor;
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
    [[nodiscard]] ImagePart& addDirectives(Directives const& directives, bool keepImageCenterPosition = false, ImageMetadataDatabaseConstPtr imageMetadata = {});
    [[nodiscard]] ImagePart& addDirectivesGroup(DirectivesGroup const& directivesGroup, bool keepImageCenterPosition = false, ImageMetadataDatabaseConstPtr imageMetadata = {});

    // Remove directives from this ImagePart, while optionally keeping the
    // transformed center of the image the same if the directives change the
    // image size.
    [[nodiscard]] ImagePart& removeDirectives(bool keepImageCenterPosition = false, ImageMetadataDatabaseConstPtr imageMetadata = {});
  };

  [[nodiscard]] static Drawable makeLine(Line2F const& line, float lineWidth, Color const& color, Vec2F const& position = Vec2F());
  [[nodiscard]] static Drawable makePoly(PolyF poly, Color const& color, Vec2F const& position = Vec2F());
  [[nodiscard]] static Drawable makeImage(AssetPath image, float pixelSize, bool centered, Vec2F const& position, Color const& color = Color::White, ImageMetadataDatabaseConstPtr imageMetadata = {});
  [[nodiscard]] static Drawable makeImage(AssetPath image, float pixelSize, bool centered, Vec2F const& position, ImageMetadataDatabaseConstPtr imageMetadata);

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
  [[nodiscard]] static RectF boundBoxAll(DrawablesContainer const& drawables, bool cropImages, ImageMetadataDatabaseConstPtr imageMetadata = {});

  Drawable();
  explicit Drawable(Json const& json, ImageMetadataDatabaseConstPtr imageMetadata = {});

  [[nodiscard]] Json toJson() const;

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

  [[nodiscard]] RectF boundBox(bool cropImages, ImageMetadataDatabaseConstPtr imageMetadata = {}) const;

  [[nodiscard]] bool isLine() const;
  [[nodiscard]] LinePart& linePart();
  [[nodiscard]] LinePart const& linePart() const;

  [[nodiscard]] bool isPoly() const;
  [[nodiscard]] PolyPart& polyPart();
  [[nodiscard]] PolyPart const& polyPart() const;

  [[nodiscard]] bool isImage() const;
  [[nodiscard]] ImagePart& imagePart();
  [[nodiscard]] ImagePart const& imagePart() const;

  MVariant<LinePart, PolyPart, ImagePart> part;

  Vec2F position;
  Color color;
  bool fullbright;
};

DataStream& operator>>(DataStream& ds, Drawable& drawable);
DataStream& operator<<(DataStream& ds, Drawable const& drawable);

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
[[nodiscard]] RectF Drawable::boundBoxAll(DrawablesContainer const& drawables, bool cropImages, ImageMetadataDatabaseConstPtr imageMetadata) {
  RectF boundBox = RectF::null();
  for (auto const& drawable : drawables)
    boundBox.combine(drawable.boundBox(cropImages, imageMetadata));
  return boundBox;
}

[[nodiscard]] inline bool Drawable::isLine() const {
  return part.is<LinePart>();
}

inline Drawable::LinePart& Drawable::linePart() {
  return part.get<LinePart>();
}

[[nodiscard]] inline Drawable::LinePart const& Drawable::linePart() const {
  return part.get<LinePart>();
}

[[nodiscard]] inline bool Drawable::isPoly() const {
  return part.is<PolyPart>();
}

inline Drawable::PolyPart& Drawable::polyPart() {
  return part.get<PolyPart>();
}

[[nodiscard]] inline Drawable::PolyPart const& Drawable::polyPart() const {
  return part.get<PolyPart>();
}

[[nodiscard]] inline bool Drawable::isImage() const {
  return part.is<ImagePart>();
}

inline Drawable::ImagePart& Drawable::imagePart() {
  return part.get<ImagePart>();
}

[[nodiscard]] inline Drawable::ImagePart const& Drawable::imagePart() const {
  return part.get<ImagePart>();
}

}
