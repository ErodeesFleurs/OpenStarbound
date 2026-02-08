#include "StarDrawable.hpp"
#include "StarColor.hpp"
#include "StarConfig.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

auto Drawable::ImagePart::addDirectives(Directives const& directives, bool keepImageCenterPosition) -> Drawable::ImagePart& {
  if (!directives)
    return *this;

  if (keepImageCenterPosition) {
    ConstPtr<ImageMetadataDatabase> imageMetadata = Root::singleton().imageMetadataDatabase();
    Vec2F imageSize = Vec2F(imageMetadata->imageSize(image));
    image.directives += directives;
    Vec2F newImageSize = Vec2F(imageMetadata->imageSize(image));

    // If we are trying to maintain the image center, PRE translate the image by
    // the change in size / 2
    transformation *= Mat3F::translation((imageSize - newImageSize) / 2);
  } else {
    image.directives += directives;
  }

  return *this;
}

auto Drawable::ImagePart::addDirectivesGroup(DirectivesGroup const& directivesGroup, bool keepImageCenterPosition) -> Drawable::ImagePart& {
  if (directivesGroup.empty())
    return *this;

  if (keepImageCenterPosition) {
    auto imageMetadata = Root::singleton().imageMetadataDatabase();
    Vec2F imageSize = Vec2F(imageMetadata->imageSize(image));
    for (Directives const& directives : directivesGroup.list())
      image.directives += directives;
    Vec2F newImageSize = Vec2F(imageMetadata->imageSize(image));

    // If we are trying to maintain the image center, PRE translate the image by
    // the change in size / 2
    transformation *= Mat3F::translation((imageSize - newImageSize) / 2);
  } else {
    for (Directives const& directives : directivesGroup.list())
      image.directives += directives;
  }

  return *this;
}

auto Drawable::ImagePart::removeDirectives(bool keepImageCenterPosition) -> Drawable::ImagePart& {
  if (keepImageCenterPosition) {
    auto imageMetadata = Root::singleton().imageMetadataDatabase();
    Vec2F imageSize = Vec2F(imageMetadata->imageSize(image));
    image.directives.clear();
    Vec2F newImageSize = Vec2F(imageMetadata->imageSize(image));

    // If we are trying to maintain the image center, PRE translate the image by
    // the change in size / 2
    transformation *= Mat3F::translation((imageSize - newImageSize) / 2);
  } else {
    image.directives.clear();
  }

  return *this;
}

auto Drawable::makeLine(Line2F const& line, float lineWidth, Color const& color, Vec2F const& position) -> Drawable {
  Drawable drawable;
  drawable.part = LinePart{.line = std::move(line), .width = lineWidth, .endColor = {}};
  drawable.color = color;
  drawable.position = position;

  return drawable;
}

auto Drawable::makePoly(PolyF poly, Color const& color, Vec2F const& position) -> Drawable {
  Drawable drawable;
  drawable.part = PolyPart{std::move(poly)};
  drawable.color = color;
  drawable.position = position;

  return drawable;
}

auto Drawable::makeImage(AssetPath image, float pixelSize, bool centered, Vec2F const& position, Color const& color) -> Drawable {
  Drawable drawable;
  Mat3F transformation = Mat3F::identity();
  if (centered) {
    auto imageMetadata = Root::singleton().imageMetadataDatabase();
    Vec2F imageSize = Vec2F(imageMetadata->imageSize(image));
    transformation.translate(-imageSize / 2);
  }

  if (pixelSize != 1.0f)
    transformation.scale(pixelSize);

  drawable.part = ImagePart{.image = std::move(image), .transformation = std::move(transformation)};
  drawable.position = position;
  drawable.color = color;

  return drawable;
}

Drawable::Drawable()
    : color(Color::White), fullbright(false) {}

Drawable::Drawable(Json const& json) {
  if (auto line = json.opt("line")) {
    part = LinePart{.line = jsonToLine2F(*line), .width = json.getFloat("width"), .endColor = {}};
  } else if (auto poly = json.opt("poly")) {
    part = PolyPart{jsonToPolyF(*poly)};
  } else if (auto image = json.opt("image")) {
    auto imageString = image->toString();
    Mat3F transformation = Mat3F::identity();
    if (auto transformationConfig = json.opt("transformation")) {
      transformation = jsonToMat3F(*transformationConfig);
    } else {
      if (json.getBool("centered", true)) {
        auto imageMetadata = Root::singleton().imageMetadataDatabase();
        Vec2F imageSize = Vec2F(imageMetadata->imageSize(imageString));
        transformation.translate(-imageSize / 2);
      }
      if (auto rotation = json.optFloat("rotation"))
        transformation.rotate(*rotation);
      if (json.getBool("mirrored", false))
        transformation.scale(Vec2F(-1, 1));
      if (auto scale = json.optFloat("scale"))
        transformation.scale(*scale);
    }

    part = ImagePart{.image = std::move(imageString), .transformation = std::move(transformation)};
  }
  position = json.opt("position").transform(jsonToVec2F).value();
  color = json.opt("color").transform(jsonToColor).value_or(Color::White);
  fullbright = json.getBool("fullbright", false);
}

auto Drawable::toJson() const -> Json {
  JsonObject json;
  if (auto line = part.ptr<LinePart>()) {
    json.set("line", jsonFromLine2F(line->line));
    json.set("width", line->width);
  } else if (auto poly = part.ptr<PolyPart>()) {
    json.set("poly", jsonFromPolyF(poly->poly));
  } else if (auto image = part.ptr<ImagePart>()) {
    json.set("image", AssetPath::join(image->image));
    json.set("transformation", jsonFromMat3F(image->transformation));
  }

  json.set("position", jsonFromVec2F(position));
  json.set("color", jsonFromColor(color));
  json.set("fullbright", fullbright);

  return json;
}

void Drawable::translate(Vec2F const& translation) {
  position += translation;
}

void Drawable::rotate(float rotation, Vec2F const& rotationCenter) {
  if (auto line = part.ptr<LinePart>())
    line->line.rotate(rotation);
  else if (auto poly = part.ptr<PolyPart>())
    poly->poly.rotate(rotation);
  else if (auto image = part.ptr<ImagePart>())
    image->transformation.rotate(rotation);

  position = (position - rotationCenter).rotate(rotation) + rotationCenter;
}

void Drawable::scale(float scaling, Vec2F const& scaleCenter) {
  scale(Vec2F::filled(scaling), scaleCenter);
}

void Drawable::scale(Vec2F const& scaling, Vec2F const& scaleCenter) {
  if (auto line = part.ptr<LinePart>())
    line->line.scale(scaling);
  else if (auto poly = part.ptr<PolyPart>())
    poly->poly.scale(scaling);
  else if (auto image = part.ptr<ImagePart>())
    image->transformation.scale(scaling);

  position = (position - scaleCenter).piecewiseMultiply(scaling) + scaleCenter;
}

void Drawable::transform(Mat3F const& transformation) {
  Vec2F localTranslation = transformation.transformVec2(Vec2F());
  Mat3F localTransform = Mat3F::translation(-localTranslation) * transformation;

  if (auto line = part.ptr<LinePart>())
    line->line.transform(localTransform);
  else if (auto poly = part.ptr<PolyPart>())
    poly->poly.transform(localTransform);
  else if (auto image = part.ptr<ImagePart>())
    image->transformation = localTransform * image->transformation;

  position = transformation.transformVec2(position);
}

void Drawable::rebase(Vec2F const& newBase) {
  if (auto line = part.ptr<LinePart>())
    line->line.translate(position - newBase);
  else if (auto poly = part.ptr<PolyPart>())
    poly->poly.translate(position - newBase);
  else if (auto image = part.ptr<ImagePart>())
    image->transformation.translate(position - newBase);

  position = newBase;
}

auto Drawable::boundBox(bool cropImages) const -> RectF {
  RectF boundBox = RectF::null();
  if (auto line = part.ptr<LinePart>()) {
    boundBox.combine(line->line.min());
    boundBox.combine(line->line.max());

  } else if (auto poly = part.ptr<PolyPart>()) {
    boundBox.combine(poly->poly.boundBox());

  } else if (auto image = part.ptr<ImagePart>()) {
    auto imageMetadata = Root::singleton().imageMetadataDatabase();
    RectF imageRegion = RectF::null();
    if (cropImages) {
      RectU nonEmptyRegion = imageMetadata->nonEmptyRegion(image->image);
      if (!nonEmptyRegion.isNull())
        imageRegion = RectF(nonEmptyRegion);
    } else {
      imageRegion = RectF::withSize(Vec2F(), Vec2F(imageMetadata->imageSize(image->image)));
    }

    if (!imageRegion.isNull()) {
      boundBox.combine(image->transformation.transformVec2(Vec2F(imageRegion.xMin(), imageRegion.yMin())));
      boundBox.combine(image->transformation.transformVec2(Vec2F(imageRegion.xMax(), imageRegion.yMin())));
      boundBox.combine(image->transformation.transformVec2(Vec2F(imageRegion.xMin(), imageRegion.yMax())));
      boundBox.combine(image->transformation.transformVec2(Vec2F(imageRegion.xMax(), imageRegion.yMax())));
    }
  }

  if (!boundBox.isNull())
    boundBox.translate(position);

  return boundBox;
}

auto operator>>(DataStream& ds, Drawable::LinePart& line) -> DataStream& {
  ds >> line.line;
  ds >> line.width;
  return ds;
}

auto operator<<(DataStream& ds, Drawable::LinePart const& line) -> DataStream& {
  ds << line.line;
  ds << line.width;
  return ds;
}

auto operator>>(DataStream& ds, Drawable::PolyPart& poly) -> DataStream& {
  ds >> poly.poly;
  return ds;
}

auto operator<<(DataStream& ds, Drawable::PolyPart const& poly) -> DataStream& {
  ds << poly.poly;
  return ds;
}

// I need to find out if this is for network serialization or not eventually
auto operator>>(DataStream& ds, Drawable::ImagePart& image) -> DataStream& {
  ds >> image.image;
  ds >> image.transformation;
  return ds;
}

auto operator<<(DataStream& ds, Drawable::ImagePart const& image) -> DataStream& {
  ds << image.image;
  ds << image.transformation;
  return ds;
}

auto operator>>(DataStream& ds, Drawable& drawable) -> DataStream& {
  ds >> drawable.part;
  ds >> drawable.position;
  ds >> drawable.color;
  ds >> drawable.fullbright;
  return ds;
}

auto operator<<(DataStream& ds, Drawable const& drawable) -> DataStream& {
  ds << drawable.part;
  ds << drawable.position;
  ds << drawable.color;
  ds << drawable.fullbright;
  return ds;
}

}// namespace Star
