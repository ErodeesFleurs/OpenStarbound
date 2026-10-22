import std;
import star.time;
import star.format;
import star.json;
import star.list;
import star.rect;
import star.image_processing;
import star.lexical_cast;
import star.color;
import star.image;
import star.string_view;
import star.string;
import star.byte_array;
import star.encode;
import star.logging;

import star.image_scaling;

import star.vector;
import star.math_common;
namespace Star {

std::expected<StringList, StarException> colorDirectivesFromConfig(JsonArray const& directives) {
  StringList result;
  for (auto const& entry : directives) {
    auto text = entry.type() == Json::Type::Object ? paletteSwapDirectivesFromConfig(entry) : entry.toString();
    if (!text)
      return std::unexpected(std::move(text.error()));
    result.append(std::move(*text));
  }
  return result;
}

std::expected<String, StarException> paletteSwapDirectivesFromConfig(Json const& swaps) {
  ColorReplaceImageOperation paletteSwaps;
  auto object = swaps.iterateObject();
  if (!object)
    return std::unexpected(std::move(object.error()));
  for (auto const& swap : *object) {
    auto from = Color::fromHex(swap.first);
    if (!from)
      return std::unexpected(std::move(from.error()));
    auto text = swap.second.toString();
    if (!text)
      return std::unexpected(std::move(text.error()));
    auto to = Color::fromHex(*text);
    if (!to)
      return std::unexpected(std::move(to.error()));
    paletteSwaps.colorReplaceMap[from->toRgba()] = to->toRgba();
  }
  return "?" + imageOperationToString(paletteSwaps);
}

HueShiftImageOperation HueShiftImageOperation::hueShiftDegrees(float degrees) {
  return HueShiftImageOperation{degrees / 360.0f};
}

SaturationShiftImageOperation SaturationShiftImageOperation::saturationShift100(float amount) {
  return SaturationShiftImageOperation{amount / 100.0f};
}

BrightnessMultiplyImageOperation BrightnessMultiplyImageOperation::brightnessMultiply100(float amount) {
  return BrightnessMultiplyImageOperation{amount / 100.0f + 1.0f};
}

FadeToColorImageOperation::FadeToColorImageOperation(Vec3B color, float amount) {
  this->color = color;
  this->amount = amount;

  auto fcl = Color::rgb(color).toLinear();
  for (int i = 0; i <= 255; ++i) {
    auto r = Color::rgb(Vec3B(i, i, i)).toLinear().mix(fcl, amount).toSRGB().toRgb();
    rTable[i] = r[0];
    gTable[i] = r[1];
    bTable[i] = r[2];
  }
}

ImageOperation imageOperationFromString(StringView string) {
  // Directives have an ASCII grammar; preserve non-ASCII reference bytes.
  std::string_view text = string.utf8();
  auto firstDelimiter = text.find_first_of("=;");
  if (text.substr(0, firstDelimiter) == "replace") {
    ColorReplaceImageOperation operation;
    if (firstDelimiter == std::string_view::npos)
      return operation;
    operation.colorReplaceMap.reserve((text.size() - firstDelimiter) / 8);
    std::optional<StringView> source;
    for (std::size_t begin = firstDelimiter + 1; begin < text.size();) {
      auto end = text.find_first_of("=;", begin);
      if (end == std::string_view::npos)
        end = text.size();
      if (end != begin) {
        StringView hex(text.substr(begin, end - begin));
        if (!source)
          source = hex;
        else {
          auto from = Color::hexToVec4B(*source);
          if (!from)
            return ErrorImageOperation{std::move(from.error())};
          auto to = Color::hexToVec4B(hex);
          if (!to)
            return ErrorImageOperation{std::move(to.error())};
          operation.colorReplaceMap[*from] = *to;
          source.reset();
        }
      }
      begin = end + 1;
    }
    return operation;
  }
  List<StringView> bits;
  for (std::size_t begin = 0; begin < text.size();) {
    auto end = text.find_first_of("=;", begin);
    if (end == std::string_view::npos)
      end = text.size();
    if (end != begin)
      bits.emplace_back(text.substr(begin, end - begin));
    begin = end + 1;
  }
  auto error = [](StarException cause) -> ImageOperation { return ErrorImageOperation{std::move(cause)}; };
  if (bits.empty())
    return error(ImageOperationException("Missing image operation"));
  auto type = bits[0];
  std::size_t required = 0;
  if (type == "hueshift" || type == "saturation" || type == "brightness" || type == "setcolor" || type == "multiply"
      || type == "addmask" || type == "submask" || type == "blendmult" || type == "blendscreen"
      || type == "scale" || type == "scalenearest" || type == "scalebilinear" || type == "scalebicubic")
    required = 1;
  else if (type == "fade" || type == "border" || type == "outline")
    required = 2;
  else if (type == "scanlines" || type == "crop")
    required = 4;
  if (bits.size() <= required)
    return error(ImageOperationException::format("Missing parameters for image operation '{}'", type));

  if (type == "flipx")
    return FlipImageOperation{FlipImageOperation::FlipX};
  if (type == "flipy")
    return FlipImageOperation{FlipImageOperation::FlipY};
  if (type == "flipxy")
    return FlipImageOperation{FlipImageOperation::FlipXY};
  if (type == "setcolor" || type == "multiply") {
    auto color = Color::fromHex(bits[1]);
    if (!color)
      return error(std::move(color.error()));
    if (type == "setcolor")
      return SetColorImageOperation{color->toRgb()};
    return MultiplyImageOperation{color->toRgba()};
  }
  if (type == "fade" || type == "scanlines") {
    auto color = Color::fromHex(bits[1]);
    if (!color)
      return error(std::move(color.error()));
    auto amount = lexicalCast<float>(bits[2]);
    if (!amount)
      return error(std::move(amount.error()));
    FadeToColorImageOperation fade(color->toRgb(), *amount);
    if (type == "fade")
      return fade;
    auto secondColor = Color::fromHex(bits[3]);
    if (!secondColor)
      return error(std::move(secondColor.error()));
    auto secondAmount = lexicalCast<float>(bits[4]);
    if (!secondAmount)
      return error(std::move(secondAmount.error()));
    return ScanLinesImageOperation{fade, FadeToColorImageOperation(secondColor->toRgb(), *secondAmount)};
  }
  if (type == "addmask" || type == "submask" || type == "blendmult" || type == "blendscreen") {
    StringList references;
    auto names = bits[1].utf8();
    for (std::size_t begin = 0; begin <= names.size();) {
      auto end = names.find('+', begin);
      if (end == std::string_view::npos)
        end = names.size();
      if (end != begin)
        references.emplace_back(names.substr(begin, end - begin));
      begin = end + 1;
    }
    Vec2I offset;
    for (std::size_t i = 2; i < std::min<std::size_t>(bits.size(), 4); ++i) {
      auto value = lexicalCast<int>(bits[i]);
      if (!value)
        return error(std::move(value.error()));
      offset[i - 2] = *value;
    }
    if (type == "addmask" || type == "submask")
      return AlphaMaskImageOperation{type == "addmask" ? AlphaMaskImageOperation::Additive : AlphaMaskImageOperation::Subtractive, std::move(references), offset};
    return BlendImageOperation{type == "blendmult" ? BlendImageOperation::Multiply : BlendImageOperation::Screen, std::move(references), offset};
  }
  if (type == "border" || type == "outline") {
    auto pixels = lexicalCast<unsigned>(bits[1]);
    if (!pixels)
      return error(std::move(pixels.error()));
    auto start = Color::fromHex(bits[2]);
    if (!start)
      return error(std::move(start.error()));
    auto end = bits.size() > 3 ? Color::fromHex(bits[3]) : start;
    if (!end)
      return error(std::move(end.error()));
    return BorderImageOperation{*pixels, start->toRgba(), end->toRgba(), type == "outline", false};
  }
  if (type == "crop") {
    float values[4]{};
    for (unsigned i = 0; i < 4; ++i) {
      auto value = lexicalCast<float>(bits[i + 1]);
      if (!value)
        return error(std::move(value.error()));
      values[i] = *value;
    }
    return CropImageOperation{RectI(values[0], values[1], values[2], values[3])};
  }
  if (type == "hueshift" || type == "saturation" || type == "brightness" || type == "scale"
      || type == "scalenearest" || type == "scalebilinear" || type == "scalebicubic") {
    auto value = lexicalCast<float>(bits[1]);
    if (!value)
      return error(std::move(value.error()));
    if (type == "hueshift")
      return HueShiftImageOperation::hueShiftDegrees(*value);
    if (type == "saturation")
      return SaturationShiftImageOperation::saturationShift100(*value);
    if (type == "brightness")
      return BrightnessMultiplyImageOperation::brightnessMultiply100(*value);
    auto second = bits.size() > 2 ? lexicalCast<float>(bits[2]) : value;
    if (!second)
      return error(std::move(second.error()));
    auto mode = type == "scalenearest" ? ScaleImageOperation::Nearest
      : type == "scalebicubic" ? ScaleImageOperation::Bicubic : ScaleImageOperation::Bilinear;
    return ScaleImageOperation{mode, Vec2F(*value, *second)};
  }
  return NullImageOperation{};
}

String imageOperationToString(ImageOperation const& operation) {
  if (auto op = std::get_if<HueShiftImageOperation>(&operation)) {
    return strf("hueshift={}", op->hueShiftAmount * 360.0f);
  } else if (auto op = std::get_if<SaturationShiftImageOperation>(&operation)) {
    return strf("saturation={}", op->saturationShiftAmount * 100.0f);
  } else if (auto op = std::get_if<BrightnessMultiplyImageOperation>(&operation)) {
    return strf("brightness={}", (op->brightnessMultiply - 1.0f) * 100.0f);
  } else if (auto op = std::get_if<FadeToColorImageOperation>(&operation)) {
    return strf("fade={}={}", Color::rgb(op->color).toHex(), op->amount);
  } else if (auto op = std::get_if<ScanLinesImageOperation>(&operation)) {
    return strf("scanlines={}={}={}={}",
        Color::rgb(op->fade1.color).toHex(),
        op->fade1.amount,
        Color::rgb(op->fade2.color).toHex(),
        op->fade2.amount);
  } else if (auto op = std::get_if<SetColorImageOperation>(&operation)) {
    return strf("setcolor={}", Color::rgb(op->color).toHex());
  } else if (auto op = std::get_if<ColorReplaceImageOperation>(&operation)) {
    String str = "replace";
    for (auto const& pair : op->colorReplaceMap)
      str += strf(";{}={}", Color::rgba(pair.first).toHex(), Color::rgba(pair.second).toHex());
    return str;
  } else if (auto op = std::get_if<AlphaMaskImageOperation>(&operation)) {
    if (op->mode == AlphaMaskImageOperation::Additive)
      return strf("addmask={};{};{}", op->maskImages.join("+"), op->offset[0], op->offset[1]);
    else if (op->mode == AlphaMaskImageOperation::Subtractive)
      return strf("submask={};{};{}", op->maskImages.join("+"), op->offset[0], op->offset[1]);
  } else if (auto op = std::get_if<BlendImageOperation>(&operation)) {
    if (op->mode == BlendImageOperation::Multiply)
      return strf("blendmult={};{};{}", op->blendImages.join("+"), op->offset[0], op->offset[1]);
    else if (op->mode == BlendImageOperation::Screen)
      return strf("blendscreen={};{};{}", op->blendImages.join("+"), op->offset[0], op->offset[1]);
  } else if (auto op = std::get_if<MultiplyImageOperation>(&operation)) {
    return strf("multiply={}", Color::rgba(op->color).toHex());
  } else if (auto op = std::get_if<BorderImageOperation>(&operation)) {
    if (op->outlineOnly)
      return strf("outline={};{};{}", op->pixels, Color::rgba(op->startColor).toHex(), Color::rgba(op->endColor).toHex());
    else
      return strf("border={};{};{}", op->pixels, Color::rgba(op->startColor).toHex(), Color::rgba(op->endColor).toHex());
  } else if (auto op = std::get_if<ScaleImageOperation>(&operation)) {
    if (op->mode == ScaleImageOperation::Nearest)
      return strf("scalenearest={}", op->scale);
    else if (op->mode == ScaleImageOperation::Bilinear)
      return strf("scalebilinear={}", op->scale);
    else if (op->mode == ScaleImageOperation::Bicubic)
      return strf("scalebicubic={}", op->scale);
  } else if (auto op = std::get_if<CropImageOperation>(&operation)) {
    return strf("crop={};{};{};{}", op->subset.xMin(), op->subset.xMax(), op->subset.yMin(), op->subset.yMax());
  } else if (auto op = std::get_if<FlipImageOperation>(&operation)) {
    if (op->mode == FlipImageOperation::FlipX)
      return "flipx";
    else if (op->mode == FlipImageOperation::FlipY)
      return "flipy";
    else if (op->mode == FlipImageOperation::FlipXY)
      return "flipxy";
  }

  return "";
}

std::expected<void, StarException> parseImageOperations(StringView params, std::function<std::expected<void, StarException>(ImageOperation&&)> outputter) {
  if (!outputter)
    return std::unexpected(ImageOperationException("Missing image operation output callback"));
  auto text = params.utf8();
  for (std::size_t begin = 0; begin < text.size();) {
    auto end = text.find('?', begin);
    if (end == std::string_view::npos)
      end = text.size();
    if (end != begin) {
      auto result = outputter(imageOperationFromString(text.substr(begin, end - begin)));
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
    begin = end + 1;
  }
  return {};
}

List<ImageOperation> parseImageOperations(StringView params) {
  List<ImageOperation> operations;
  auto text = params.utf8();
  for (std::size_t begin = 0; begin < text.size();) {
    auto end = text.find('?', begin);
    if (end == std::string_view::npos)
      end = text.size();
    if (end != begin)
      operations.append(imageOperationFromString(text.substr(begin, end - begin)));
    begin = end + 1;
  }
  return operations;
}

String printImageOperations(List<ImageOperation> const& list) {
  return StringList(list.transformed(imageOperationToString)).join("?");
}

void addImageOperationReferences(ImageOperation const& operation, StringList& out) {
  if (auto op = std::get_if<AlphaMaskImageOperation>(&operation))
    out.appendAll(op->maskImages);
  else if (auto op = std::get_if<BlendImageOperation>(&operation))
    out.appendAll(op->blendImages);
}

StringList imageOperationReferences(List<ImageOperation> const& operations) {
  StringList references;
  for (auto const& operation : operations)
    addImageOperationReferences(operation, references);
  return references;
}

static void processSaturationShift(Image& image, SaturationShiftImageOperation const* op) {
  image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) {
    if (pixel[3] != 0) {
      Color color = Color::rgba(pixel);
      color.setSaturation(clamp(color.saturation() + op->saturationShiftAmount, 0.0f, 1.0f));
      pixel = color.toRgba();
    }
  });
}



std::expected<void, StarException> processImageOperation(ImageOperation const& operation, Image& image, ImageReferenceCallback refCallback) {
  if (auto error = std::get_if<ErrorImageOperation>(&operation))
    return std::unexpected(error->cause);
  if (image.bytesPerPixel() == 3) {
    // Convert to an image format that has alpha so certain operations function properly
    image = image.convert(image.pixelFormat() == PixelFormat::BGR24 ? PixelFormat::BGRA32 : PixelFormat::RGBA32);
  }
  if (auto op = std::get_if<HueShiftImageOperation>(&operation)) {
    image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) {
      if (pixel[3] != 0)
        pixel = Color::hueShiftVec4B(pixel, op->hueShiftAmount);
    });
  } else if (auto op = std::get_if<SaturationShiftImageOperation>(&operation)) {
    processSaturationShift(image, op);
  } else if (auto op = std::get_if<BrightnessMultiplyImageOperation>(&operation)) {
    image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) {
      if (pixel[3] != 0) {
        Color color = Color::rgba(pixel);
        color.setValue(clamp(color.value() * op->brightnessMultiply, 0.0f, 1.0f));
        pixel = color.toRgba();
      }
    });
  } else if (auto op = std::get_if<FadeToColorImageOperation>(&operation)) {
    image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) {
      pixel[0] = op->rTable[pixel[0]];
      pixel[1] = op->gTable[pixel[1]];
      pixel[2] = op->bTable[pixel[2]];
    });
  } else if (auto op = std::get_if<ScanLinesImageOperation>(&operation)) {
    image.forEachPixel([&op](unsigned, unsigned y, Vec4B& pixel) {
      if (y % 2 == 0) {
        pixel[0] = op->fade1.rTable[pixel[0]];
        pixel[1] = op->fade1.gTable[pixel[1]];
        pixel[2] = op->fade1.bTable[pixel[2]];
      } else {
        pixel[0] = op->fade2.rTable[pixel[0]];
        pixel[1] = op->fade2.gTable[pixel[1]];
        pixel[2] = op->fade2.bTable[pixel[2]];
      }
    });
  } else if (auto op = std::get_if<SetColorImageOperation>(&operation)) {
    image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) {
      pixel[0] = op->color[0];
      pixel[1] = op->color[1];
      pixel[2] = op->color[2];
    });
  } else if (auto op = std::get_if<ColorReplaceImageOperation>(&operation)) {
    image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) {
      if (auto m = op->colorReplaceMap.maybe(pixel))
        pixel = *m;
    });

  } else if (auto op = std::get_if<AlphaMaskImageOperation>(&operation)) {
    if (op->maskImages.empty())
      return {};

    if (!refCallback)
      return std::unexpected(ImageOperationException("Missing image ref callback during AlphaMaskImageOperation"));

    List<Image const*> maskImages;
    for (auto const& reference : op->maskImages) {
      auto mask = refCallback(reference);
      if (!mask)
        return std::unexpected(std::move(mask.error()));
      if (!*mask)
        return std::unexpected(ImageOperationException::format("Missing mask image '{}'", reference));
      maskImages.append(*mask);
    }

    image.forEachPixel([&op, &maskImages](unsigned x, unsigned y, Vec4B& pixel) {
      std::uint8_t maskAlpha = 0;
      Vec2U pos = Vec2U(Vec2I(x, y) + op->offset);
      for (auto mask : maskImages) {
        if (pos[0] < mask->width() && pos[1] < mask->height()) {
          if (op->mode == AlphaMaskImageOperation::Additive) {
            // We produce our mask alpha from the maximum alpha of any of
            // the
            // mask images.
            maskAlpha = std::max(maskAlpha, mask->clamp(Vec2I(pos))[3]);
          } else if (op->mode == AlphaMaskImageOperation::Subtractive) {
            // We produce our mask alpha from the minimum alpha of any of
            // the
            // mask images.
            maskAlpha = std::min(maskAlpha, mask->clamp(Vec2I(pos))[3]);
          }
        }
      }
      pixel[3] = std::min(pixel[3], maskAlpha);
    });

  } else if (auto op = std::get_if<BlendImageOperation>(&operation)) {
    if (op->blendImages.empty())
      return {};

    if (!refCallback)
      return std::unexpected(ImageOperationException("Missing image ref callback during BlendImageOperation"));

    List<Image const*> blendImages;
    for (auto const& reference : op->blendImages) {
      auto blend = refCallback(reference);
      if (!blend)
        return std::unexpected(std::move(blend.error()));
      if (!*blend)
        return std::unexpected(ImageOperationException::format("Missing blend image '{}'", reference));
      blendImages.append(*blend);
    }

    image.forEachPixel([&op, &blendImages](unsigned x, unsigned y, Vec4B& pixel) {
      Vec2U pos = Vec2U(Vec2I(x, y) + op->offset);
      Vec4F fpixel = Color::v4bToFloat(pixel);
      for (auto blend : blendImages) {
        if (pos[0] < blend->width() && pos[1] < blend->height()) {
          Vec4F blendPixel = Color::v4bToFloat(blend->clamp(Vec2I(pos)));
          if (op->mode == BlendImageOperation::Multiply)
            fpixel = fpixel.piecewiseMultiply(blendPixel);
          else if (op->mode == BlendImageOperation::Screen)
            fpixel = Vec4F::filled(1.0f) - (Vec4F::filled(1.0f) - fpixel).piecewiseMultiply(Vec4F::filled(1.0f) - blendPixel);
        }
      }
      pixel = Color::v4fToByte(fpixel);
    });

  } else if (auto op = std::get_if<MultiplyImageOperation>(&operation)) {
    image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) {
      pixel = pixel.combine(op->color, [](std::uint8_t a, std::uint8_t b) -> std::uint8_t {
          return (std::uint8_t)(((int)a * (int)b) / 255);
        });
    });

  } else if (auto op = std::get_if<BorderImageOperation>(&operation)) {
    Image borderImage(image.size() + Vec2U::filled(op->pixels * 2), PixelFormat::RGBA32);
    auto copied = borderImage.copyInto(Vec2U::filled(op->pixels), image);
    if (!copied)
      return std::unexpected(std::move(copied.error()));
    Vec2I borderImageSize = Vec2I(borderImage.size());

    borderImage.forEachPixel([&op, &image, &borderImageSize](int x, int y, Vec4B& pixel) {
      int pixels = op->pixels;
      bool includeTransparent = op->includeTransparent;
      if (pixel[3] == 0 || (includeTransparent && pixel[3] != 255)) {
        int dist = std::numeric_limits<int>::max();
        for (int j = -pixels; j < pixels + 1; j++) {
          for (int i = -pixels; i < pixels + 1; i++) {
            if (i + x >= pixels && j + y >= pixels && i + x < borderImageSize[0] - pixels && j + y < borderImageSize[1] - pixels) {
              Vec4B remotePixel = image.get32(i + x - pixels, j + y - pixels);
              if (remotePixel[3] != 0) {
                dist = std::min(dist, std::abs(i) + std::abs(j));
                if (dist == 1) // Early out, if dist is 1 it ain't getting shorter
                  break;
              }
            }
          }
        }

        if (dist < std::numeric_limits<int>::max()) {
          float percent = (dist - 1) / (2.0f * pixels - 1);
          if (pixel[3] != 0) {
            Color color = Color::rgba(op->startColor).mix(Color::rgba(op->endColor), percent);
            if (op->outlineOnly) {
              float pixelA = byteToFloat(pixel[3]);
              color.setAlphaF((1.0f - pixelA) * std::fminf(pixelA, 0.5f) * 2.0f);
            }
            else {
              Color pixelF = Color::rgba(pixel);
              float pixelA = pixelF.alphaF(), colorA = color.alphaF();
              colorA += pixelA * (1.0f - colorA);
              pixelF.convertToLinear(); //Mix in linear color space as it is more perceptually accurate
              color.convertToLinear();
              color = color.mix(pixelF, pixelA);
              color.convertToSRGB();
              color.setAlphaF(colorA);
            }
            pixel = color.toRgba();
          } else {
            pixel = Vec4B(Vec4F(op->startColor) * (1 - percent) + Vec4F(op->endColor) * percent);
          }
        }
      } else if (op->outlineOnly) {
        pixel = Vec4B(0, 0, 0, 0);
      }
    });

    image = borderImage;

  } else if (auto op = std::get_if<ScaleImageOperation>(&operation)) {
    auto scale = op->scale;
    if (scale[0] < 0.0f || scale[1] < 0.0f) {
      EmergencyLog::warn("Negative scale in ScaleImageOperation ({})", scale);
      scale = scale.piecewiseMax(Vec2F::filled(0.f));
    }
    auto scaled = op->mode == ScaleImageOperation::Nearest ? scaleNearest(image, scale)
      : op->mode == ScaleImageOperation::Bicubic ? scaleBicubic(image, scale) : scaleBilinear(image, scale);
    if (!scaled)
      return std::unexpected(std::move(scaled.error()));
    image = std::move(*scaled);

  } else if (auto op = std::get_if<CropImageOperation>(&operation)) {
    auto cropped = image.subImage(Vec2U(op->subset.min()), Vec2U(op->subset.size()));
    if (!cropped)
      return std::unexpected(std::move(cropped.error()));
    image = std::move(*cropped);

  } else if (auto op = std::get_if<FlipImageOperation>(&operation)) {
    if (op->mode == FlipImageOperation::FlipX || op->mode == FlipImageOperation::FlipXY) {
      for (std::size_t y = 0; y < image.height(); ++y) {
        for (std::size_t xLeft = 0; xLeft < image.width() / 2; ++xLeft) {
          std::size_t xRight = image.width() - 1 - xLeft;

          auto left = image.get32(xLeft, y);
          auto right = image.get32(xRight, y);

          image.set32(xLeft, y, right);
          image.set32(xRight, y, left);
        }
      }
    }

    if (op->mode == FlipImageOperation::FlipY || op->mode == FlipImageOperation::FlipXY) {
      for (std::size_t x = 0; x < image.width(); ++x) {
        for (std::size_t yTop = 0; yTop < image.height() / 2; ++yTop) {
          std::size_t yBottom = image.height() - 1 - yTop;

          auto top = image.get32(x, yTop);
          auto bottom = image.get32(x, yBottom);

          image.set32(x, yTop, bottom);
          image.set32(x, yBottom, top);
        }
      }
    }
  }
  return {};
}

std::expected<Image, StarException> processImageOperations(List<ImageOperation> const& operations, Image image, ImageReferenceCallback refCallback) {
  for (auto const& operation : operations) {
    auto processed = processImageOperation(operation, image, refCallback);
    if (!processed)
      return std::unexpected(std::move(processed.error()));
  }
  return image;
}

}
