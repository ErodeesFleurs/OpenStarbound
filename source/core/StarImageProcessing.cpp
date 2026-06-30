#include "StarImageProcessing.hpp"
#include "StarImageScaling.hpp"
#include "StarMatrix3.hpp"
#include "StarInterpolation.hpp"
#include "StarLexicalCast.hpp"
#include "StarColor.hpp"
#include "StarImage.hpp"
#include "StarStringView.hpp"
#include "StarEncode.hpp"
#include "StarLogging.hpp"

namespace Star {

StringList colorDirectivesFromConfig(JsonArray const& directives) {
  List<String> result;

  for (auto entry : directives) {
    if (entry.type() == Json::Type::String) {
      result.append(entry.toString());
    } else if (entry.type() == Json::Type::Object) {
      result.append(paletteSwapDirectivesFromConfig(entry));
    } else {
      throw StarException("Malformed color directives list.");
    }
  }
  return result;
}

String paletteSwapDirectivesFromConfig(Json const& swaps) {
  ColorReplaceImageOperation paletteSwaps;
  for (auto const& [sourceColor, replacementColor] : swaps.iterateObject())
    paletteSwaps.colorReplaceMap[Color::fromHex(sourceColor).toRgba()] = Color::fromHex(replacementColor.toString()).toRgba();
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
  try {
    std::string_view view = string.utf8();
    //double time = view.size() > 10000 ? Time::monotonicTime() : 0.0;
    auto firstBitEnd = view.find_first_of("=;");
    if (view.substr(0, firstBitEnd).compare("replace") == 0 && (firstBitEnd + 1) != view.size()) {
      //Perform optimized replace parse
      ColorReplaceImageOperation operation;

      std::string_view bits = view.substr(firstBitEnd + 1);
      operation.colorReplaceMap.reserve(bits.size() / 8);

      char const* hexPtr = nullptr;
      unsigned int hexLen = 0;

      char const* ptr = bits.data();
      char const* end = ptr + bits.size();

      char a[4]{}, b[4]{};
      bool which = true;
      auto colorBytes = [](char const bytes[4]) {
        return Vec4B(
            static_cast<uint8_t>(bytes[0]),
            static_cast<uint8_t>(bytes[1]),
            static_cast<uint8_t>(bytes[2]),
            static_cast<uint8_t>(bytes[3]));
      };

      while (true) {
        char ch = *ptr;

        if (ch == '=' || ch == ';' || ptr == end) {
          if (hexLen != 0) {
            char* c = which ? a : b;

            if (hexLen == 3) {
              [[maybe_unused]] size_t decoded = nibbleDecode(hexPtr, 3, c, 4);
              assert(decoded == 3);
              c[0] |= (c[0] << 4);
              c[1] |= (c[1] << 4);
              c[2] |= (c[2] << 4);
              c[3] = static_cast<char>(255);
            }
            else if (hexLen == 4) {
              [[maybe_unused]] size_t decoded = nibbleDecode(hexPtr, 4, c, 4);
              assert(decoded == 4);
              c[0] |= (c[0] << 4);
              c[1] |= (c[1] << 4);
              c[2] |= (c[2] << 4);
              c[3] |= (c[3] << 4);
            }
            else if (hexLen == 6) {
              [[maybe_unused]] size_t decoded = hexDecode(hexPtr, 6, c, 4);
              assert(decoded == 3);
              c[3] = static_cast<char>(255);
            }
            else if (hexLen == 8) {
              [[maybe_unused]] size_t decoded = hexDecode(hexPtr, 8, c, 4);
              assert(decoded == 4);
            }
            else if (!which || (ptr != end && ++ptr != end))
                return ErrorImageOperation{strf("Improper size for hex string '{}'", StringView(hexPtr, hexLen))};
            else // we're in A of A=B. In vanilla only A=B pairs are evaluated, so only throw an error if B is also there.
                return operation;

            if ((which = !which))
              operation.colorReplaceMap[colorBytes(a)] = colorBytes(b);

            hexLen = 0;
          }
        }
        else if (!hexLen++)
          hexPtr = ptr;

        if (ptr++ == end)
          break;
      }

      //if (time != 0.0)
      //  Logger::logf(LogLevel::Debug, "Parsed %u long directives to %u replace operations in %fs", view.size(), operation.colorReplaceMap.size(), Time::monotonicTime() - time);
      return operation;
    }

    List<StringView> bits;

    string.forEachSplitAnyView("=;", [&](StringView split, size_t, size_t) {
      if (!split.empty())
        bits.emplace_back(split);
    });

    StringView const& type = bits.at(0);

    if (type == "hueshift") {
      return HueShiftImageOperation::hueShiftDegrees(lexicalCast<float>(bits.at(1)));

    } else if (type == "saturation") {
      return SaturationShiftImageOperation::saturationShift100(lexicalCast<float>(bits.at(1)));

    } else if (type == "brightness") {
      return BrightnessMultiplyImageOperation::brightnessMultiply100(lexicalCast<float>(bits.at(1)));

    } else if (type == "fade") {
      return FadeToColorImageOperation(Color::fromHex(bits.at(1)).toRgb(), lexicalCast<float>(bits.at(2)));

    } else if (type == "scanlines") {
      return ScanLinesImageOperation{
          FadeToColorImageOperation(Color::fromHex(bits.at(1)).toRgb(), lexicalCast<float>(bits.at(2))),
          FadeToColorImageOperation(Color::fromHex(bits.at(3)).toRgb(), lexicalCast<float>(bits.at(4)))};

    } else if (type == "setcolor") {
      return SetColorImageOperation{Color::fromHex(bits.at(1)).toRgb()};

    } else if (type == "replace") {
      ColorReplaceImageOperation operation;
      for (size_t i = 0; i < (bits.size() - 1) / 2; ++i)
        operation.colorReplaceMap[Color::hexToVec4B(bits[i * 2 + 1])] = Color::hexToVec4B(bits[i * 2 + 2]);

      return operation;

    } else if (type == "addmask" || type == "submask") {
      AlphaMaskImageOperation operation;
      if (type == "addmask")
        operation.mode = AlphaMaskImageOperation::Additive;
      else
        operation.mode = AlphaMaskImageOperation::Subtractive;

      operation.maskImages = String(bits.at(1)).split('+');

      if (bits.size() > 2)
        operation.offset[0] = lexicalCast<int>(bits.at(2));

      if (bits.size() > 3)
        operation.offset[1] = lexicalCast<int>(bits.at(3));

      return operation;

    } else if (type == "blendmult" || type == "blendscreen") {
      BlendImageOperation operation;

      if (type == "blendmult")
        operation.mode = BlendImageOperation::Multiply;
      else
        operation.mode = BlendImageOperation::Screen;

      operation.blendImages = String(bits.at(1)).split('+');

      if (bits.size() > 2)
        operation.offset[0] = lexicalCast<int>(bits.at(2));

      if (bits.size() > 3)
        operation.offset[1] = lexicalCast<int>(bits.at(3));

      return operation;

    } else if (type == "multiply") {
      return MultiplyImageOperation{Color::fromHex(bits.at(1)).toRgba()};

    } else if (type == "border" || type == "outline") {
      BorderImageOperation operation;
      operation.pixels = lexicalCast<unsigned>(bits.at(1));
      operation.startColor = Color::fromHex(bits.at(2)).toRgba();
      if (bits.size() > 3)
        operation.endColor = Color::fromHex(bits.at(3)).toRgba();
      else
        operation.endColor = operation.startColor;
      operation.outlineOnly = type == "outline";
      operation.includeTransparent = false; // Currently just here for anti-aliased fonts

      return operation;

    } else if (type == "scalenearest" || type == "scalebilinear" || type == "scalebicubic" || type == "scale") {
      Vec2F scale;
      if (bits.size() == 2)
        scale = Vec2F::filled(lexicalCast<float>(bits.at(1)));
      else
        scale = Vec2F(lexicalCast<float>(bits.at(1)), lexicalCast<float>(bits.at(2)));

      ScaleImageOperation::Mode mode;
      if (type == "scalenearest")
        mode = ScaleImageOperation::Nearest;
      else if (type == "scalebicubic")
        mode = ScaleImageOperation::Bicubic;
      else
        mode = ScaleImageOperation::Bilinear;

      return ScaleImageOperation{mode, scale};

    } else if (type == "crop") {
      return CropImageOperation{RectI(lexicalCast<float>(bits.at(1)), lexicalCast<float>(bits.at(2)),
          lexicalCast<float>(bits.at(3)), lexicalCast<float>(bits.at(4)))};

    } else if (type == "flipx") {
      return FlipImageOperation{FlipImageOperation::FlipX};

    } else if (type == "flipy") {
      return FlipImageOperation{FlipImageOperation::FlipY};

    } else if (type == "flipxy") {
      return FlipImageOperation{FlipImageOperation::FlipXY};

    } else {
      return NullImageOperation();
    }
  } catch (OutOfRangeException const& e) {
    return ErrorImageOperation{std::string(e.what())};
  } catch (BadLexicalCast const& e) {
    return ErrorImageOperation{std::string(e.what())};
  } catch (StarException const& e) {
    return ErrorImageOperation{std::string(e.what())};
  }
}

String imageOperationToString(ImageOperation const& operation) {
  if (auto hueShiftOp = operation.ptr<HueShiftImageOperation>()) {
    return strf("hueshift={}", hueShiftOp->hueShiftAmount * 360.0f);
  } else if (auto saturationOp = operation.ptr<SaturationShiftImageOperation>()) {
    return strf("saturation={}", saturationOp->saturationShiftAmount * 100.0f);
  } else if (auto brightnessOp = operation.ptr<BrightnessMultiplyImageOperation>()) {
    return strf("brightness={}", (brightnessOp->brightnessMultiply - 1.0f) * 100.0f);
  } else if (auto fadeOp = operation.ptr<FadeToColorImageOperation>()) {
    return strf("fade={}={}", Color::rgb(fadeOp->color).toHex(), fadeOp->amount);
  } else if (auto scanLinesOp = operation.ptr<ScanLinesImageOperation>()) {
    return strf("scanlines={}={}={}={}",
        Color::rgb(scanLinesOp->fade1.color).toHex(),
        scanLinesOp->fade1.amount,
        Color::rgb(scanLinesOp->fade2.color).toHex(),
        scanLinesOp->fade2.amount);
  } else if (auto setColorOp = operation.ptr<SetColorImageOperation>()) {
    return strf("setcolor={}", Color::rgb(setColorOp->color).toHex());
  } else if (auto colorReplaceOp = operation.ptr<ColorReplaceImageOperation>()) {
    String str = "replace";
    for (auto const& [fromColor, toColor] : colorReplaceOp->colorReplaceMap)
      str += strf(";{}={}", Color::rgba(fromColor).toHex(), Color::rgba(toColor).toHex());
    return str;
  } else if (auto alphaMaskOp = operation.ptr<AlphaMaskImageOperation>()) {
    if (alphaMaskOp->mode == AlphaMaskImageOperation::Additive)
      return strf("addmask={};{};{}", alphaMaskOp->maskImages.join("+"), alphaMaskOp->offset[0], alphaMaskOp->offset[1]);
    else if (alphaMaskOp->mode == AlphaMaskImageOperation::Subtractive)
      return strf("submask={};{};{}", alphaMaskOp->maskImages.join("+"), alphaMaskOp->offset[0], alphaMaskOp->offset[1]);
  } else if (auto blendOp = operation.ptr<BlendImageOperation>()) {
    if (blendOp->mode == BlendImageOperation::Multiply)
      return strf("blendmult={};{};{}", blendOp->blendImages.join("+"), blendOp->offset[0], blendOp->offset[1]);
    else if (blendOp->mode == BlendImageOperation::Screen)
      return strf("blendscreen={};{};{}", blendOp->blendImages.join("+"), blendOp->offset[0], blendOp->offset[1]);
  } else if (auto multiplyOp = operation.ptr<MultiplyImageOperation>()) {
    return strf("multiply={}", Color::rgba(multiplyOp->color).toHex());
  } else if (auto borderOp = operation.ptr<BorderImageOperation>()) {
    if (borderOp->outlineOnly)
      return strf("outline={};{};{}", borderOp->pixels, Color::rgba(borderOp->startColor).toHex(), Color::rgba(borderOp->endColor).toHex());
    else
      return strf("border={};{};{}", borderOp->pixels, Color::rgba(borderOp->startColor).toHex(), Color::rgba(borderOp->endColor).toHex());
  } else if (auto scaleOp = operation.ptr<ScaleImageOperation>()) {
    if (scaleOp->mode == ScaleImageOperation::Nearest)
      return strf("scalenearest={}", scaleOp->scale);
    else if (scaleOp->mode == ScaleImageOperation::Bilinear)
      return strf("scalebilinear={}", scaleOp->scale);
    else if (scaleOp->mode == ScaleImageOperation::Bicubic)
      return strf("scalebicubic={}", scaleOp->scale);
  } else if (auto cropOp = operation.ptr<CropImageOperation>()) {
    return strf("crop={};{};{};{}", cropOp->subset.xMin(), cropOp->subset.xMax(), cropOp->subset.yMin(), cropOp->subset.yMax());
  } else if (auto flipOp = operation.ptr<FlipImageOperation>()) {
    if (flipOp->mode == FlipImageOperation::FlipX)
      return "flipx";
    else if (flipOp->mode == FlipImageOperation::FlipY)
      return "flipy";
    else if (flipOp->mode == FlipImageOperation::FlipXY)
      return "flipxy";
  }

  return "";
}

void parseImageOperations(StringView params, function<void(ImageOperation&&)> outputter) {
  params.forEachSplitView("?", [&](StringView op, size_t, size_t) {
    if (!op.empty())
      outputter(imageOperationFromString(op));
  });
}

List<ImageOperation> parseImageOperations(StringView params) {
  List<ImageOperation> operations;
  params.forEachSplitView("?", [&](StringView op, size_t, size_t) {
    if (!op.empty())
      operations.append(imageOperationFromString(op));
    });

  return operations;
}

String printImageOperations(List<ImageOperation> const& list) {
  return StringList(list.transformed(imageOperationToString)).join("?");
}

void addImageOperationReferences(ImageOperation const& operation, StringList& out) {
  if (auto alphaMaskOp = operation.ptr<AlphaMaskImageOperation>())
    out.appendAll(alphaMaskOp->maskImages);
  else if (auto blendOp = operation.ptr<BlendImageOperation>())
    out.appendAll(blendOp->blendImages);
}

StringList imageOperationReferences(List<ImageOperation> const& operations) {
  StringList references;
  for (auto const& operation : operations)
    addImageOperationReferences(operation, references);
  return references;
}

#ifdef STAR_COMPILER_GNU
#pragma GCC push_options
#pragma GCC optimize("-fno-fast-math", "-fassociative-math", "-freciprocal-math")
#endif
static void processSaturationShift(Image& image, SaturationShiftImageOperation const* op) {
  image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) {
    if (pixel[3] != 0) {
      Color color = Color::rgba(pixel);
      color.setSaturation(clamp(color.saturation() + op->saturationShiftAmount, 0.0f, 1.0f));
      pixel = color.toRgba();
    }
  });
}
#ifdef STAR_COMPILER_GNU
#pragma GCC pop_options
#endif



void processImageOperation(ImageOperation const& operation, Image& image, ImageReferenceCallback refCallback) {
  if (image.bytesPerPixel() == 3) {
    // Convert to an image format that has alpha so certain operations function properly
    image = image.convert(image.pixelFormat() == PixelFormat::BGR24 ? PixelFormat::BGRA32 : PixelFormat::RGBA32);
  }
  if (auto hueShiftOp = operation.ptr<HueShiftImageOperation>()) {
    image.forEachPixel([&hueShiftOp](unsigned, unsigned, Vec4B& pixel) {
      if (pixel[3] != 0)
        pixel = Color::hueShiftVec4B(pixel, hueShiftOp->hueShiftAmount);
    });
  } else if (auto saturationOp = operation.ptr<SaturationShiftImageOperation>()) {
    processSaturationShift(image, saturationOp);
  } else if (auto brightnessOp = operation.ptr<BrightnessMultiplyImageOperation>()) {
    image.forEachPixel([&brightnessOp](unsigned, unsigned, Vec4B& pixel) {
      if (pixel[3] != 0) {
        Color color = Color::rgba(pixel);
        color.setValue(clamp(color.value() * brightnessOp->brightnessMultiply, 0.0f, 1.0f));
        pixel = color.toRgba();
      }
    });
  } else if (auto fadeOp = operation.ptr<FadeToColorImageOperation>()) {
    image.forEachPixel([&fadeOp](unsigned, unsigned, Vec4B& pixel) {
      pixel[0] = fadeOp->rTable[pixel[0]];
      pixel[1] = fadeOp->gTable[pixel[1]];
      pixel[2] = fadeOp->bTable[pixel[2]];
    });
  } else if (auto scanLinesOp = operation.ptr<ScanLinesImageOperation>()) {
    image.forEachPixel([&scanLinesOp](unsigned, unsigned y, Vec4B& pixel) {
      if (y % 2 == 0) {
        pixel[0] = scanLinesOp->fade1.rTable[pixel[0]];
        pixel[1] = scanLinesOp->fade1.gTable[pixel[1]];
        pixel[2] = scanLinesOp->fade1.bTable[pixel[2]];
      } else {
        pixel[0] = scanLinesOp->fade2.rTable[pixel[0]];
        pixel[1] = scanLinesOp->fade2.gTable[pixel[1]];
        pixel[2] = scanLinesOp->fade2.bTable[pixel[2]];
      }
    });
  } else if (auto setColorOp = operation.ptr<SetColorImageOperation>()) {
    image.forEachPixel([&setColorOp](unsigned, unsigned, Vec4B& pixel) {
      pixel[0] = setColorOp->color[0];
      pixel[1] = setColorOp->color[1];
      pixel[2] = setColorOp->color[2];
    });
  } else if (auto colorReplaceOp = operation.ptr<ColorReplaceImageOperation>()) {
    image.forEachPixel([&colorReplaceOp](unsigned, unsigned, Vec4B& pixel) {
      if (auto m = colorReplaceOp->colorReplaceMap.maybe(pixel))
        pixel = *m;
    });

  } else if (auto alphaMaskOp = operation.ptr<AlphaMaskImageOperation>()) {
    if (alphaMaskOp->maskImages.empty())
      return;

    if (!refCallback)
      throw StarException("Missing image ref callback during AlphaMaskImageOperation in ImageProcessor::process");

    List<Image const*> maskImages;
    for (auto const& reference : alphaMaskOp->maskImages)
      maskImages.append(refCallback(reference));

    image.forEachPixel([&alphaMaskOp, &maskImages](unsigned x, unsigned y, Vec4B& pixel) {
      uint8_t maskAlpha = 0;
      Vec2U pos = Vec2U(Vec2I(x, y) + alphaMaskOp->offset);
      for (auto mask : maskImages) {
        if (pos[0] < mask->width() && pos[1] < mask->height()) {
          if (alphaMaskOp->mode == AlphaMaskImageOperation::Additive) {
            // We produce our mask alpha from the maximum alpha of any of
            // the
            // mask images.
            maskAlpha = std::max(maskAlpha, mask->get(pos)[3]);
          } else if (alphaMaskOp->mode == AlphaMaskImageOperation::Subtractive) {
            // We produce our mask alpha from the minimum alpha of any of
            // the
            // mask images.
            maskAlpha = std::min(maskAlpha, mask->get(pos)[3]);
          }
        }
      }
      pixel[3] = std::min(pixel[3], maskAlpha);
    });

  } else if (auto blendOp = operation.ptr<BlendImageOperation>()) {
    if (blendOp->blendImages.empty())
      return;

    if (!refCallback)
      throw StarException("Missing image ref callback during BlendImageOperation in ImageProcessor::process");

    List<Image const*> blendImages;
    for (auto const& reference : blendOp->blendImages)
      blendImages.append(refCallback(reference));

    image.forEachPixel([&blendOp, &blendImages](unsigned x, unsigned y, Vec4B& pixel) {
      Vec2U pos = Vec2U(Vec2I(x, y) + blendOp->offset);
      Vec4F fpixel = Color::v4bToFloat(pixel);
      for (auto blend : blendImages) {
        if (pos[0] < blend->width() && pos[1] < blend->height()) {
          Vec4F blendPixel = Color::v4bToFloat(blend->get(pos));
          if (blendOp->mode == BlendImageOperation::Multiply)
            fpixel = fpixel.piecewiseMultiply(blendPixel);
          else if (blendOp->mode == BlendImageOperation::Screen)
            fpixel = Vec4F::filled(1.0f) - (Vec4F::filled(1.0f) - fpixel).piecewiseMultiply(Vec4F::filled(1.0f) - blendPixel);
        }
      }
      pixel = Color::v4fToByte(fpixel);
    });

  } else if (auto multiplyOp = operation.ptr<MultiplyImageOperation>()) {
    image.forEachPixel([&multiplyOp](unsigned, unsigned, Vec4B& pixel) {
      pixel = pixel.combine(multiplyOp->color, [](uint8_t a, uint8_t b) -> uint8_t {
          return static_cast<uint8_t>((static_cast<int>(a) * static_cast<int>(b)) / 255);
        });
    });

  } else if (auto borderOp = operation.ptr<BorderImageOperation>()) {
    Image borderImage(image.size() + Vec2U::filled(borderOp->pixels * 2), PixelFormat::RGBA32);
    borderImage.copyInto(Vec2U::filled(borderOp->pixels), image);
    Vec2I borderImageSize = Vec2I(borderImage.size());

    borderImage.forEachPixel([&borderOp, &image, &borderImageSize](int x, int y, Vec4B& pixel) {
      int pixels = borderOp->pixels;
      bool includeTransparent = borderOp->includeTransparent;
      if (pixel[3] == 0 || (includeTransparent && pixel[3] != 255)) {
        int dist = std::numeric_limits<int>::max();
        for (int j = -pixels; j < pixels + 1; j++) {
          for (int i = -pixels; i < pixels + 1; i++) {
            if (i + x >= pixels && j + y >= pixels && i + x < borderImageSize[0] - pixels && j + y < borderImageSize[1] - pixels) {
              Vec4B remotePixel = image.get(i + x - pixels, j + y - pixels);
              if (remotePixel[3] != 0) {
                dist = std::min(dist, abs(i) + abs(j));
                if (dist == 1) // Early out, if dist is 1 it ain't getting shorter
                  break;
              }
            }
          }
        }

        if (dist < std::numeric_limits<int>::max()) {
          float percent = (dist - 1) / (2.0f * pixels - 1);
          if (pixel[3] != 0) {
            Color color = Color::rgba(borderOp->startColor).mix(Color::rgba(borderOp->endColor), percent);
            if (borderOp->outlineOnly) {
              float pixelA = byteToFloat(pixel[3]);
              color.setAlphaF((1.0f - pixelA) * fminf(pixelA, 0.5f) * 2.0f);
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
            pixel = Vec4B(Vec4F(borderOp->startColor) * (1 - percent) + Vec4F(borderOp->endColor) * percent);
          }
        }
      } else if (borderOp->outlineOnly) {
        pixel = Vec4B(0, 0, 0, 0);
      }
    });

    image = borderImage;

  } else if (auto scaleOp = operation.ptr<ScaleImageOperation>()) {
    auto scale = scaleOp->scale;
    if (scale[0] < 0.0f || scale[1] < 0.0f) {
      Logger::warn("Negative scale in ScaleImageOperation ({})", scale);
      scale = scale.piecewiseMax(Vec2F::filled(0.f));
    }
    if (scaleOp->mode == ScaleImageOperation::Nearest)
      image = scaleNearest(image, scale);
    else if (scaleOp->mode == ScaleImageOperation::Bilinear)
      image = scaleBilinear(image, scale);
    else if (scaleOp->mode == ScaleImageOperation::Bicubic)
      image = scaleBicubic(image, scale);

  } else if (auto cropOp = operation.ptr<CropImageOperation>()) {
    image = image.subImage(Vec2U(cropOp->subset.min()), Vec2U(cropOp->subset.size()));

  } else if (auto flipOp = operation.ptr<FlipImageOperation>()) {
    if (flipOp->mode == FlipImageOperation::FlipX || flipOp->mode == FlipImageOperation::FlipXY) {
      for (size_t y = 0; y < image.height(); ++y) {
        for (size_t xLeft = 0; xLeft < image.width() / 2; ++xLeft) {
          size_t xRight = image.width() - 1 - xLeft;

          auto left = image.get(xLeft, y);
          auto right = image.get(xRight, y);

          image.set(xLeft, y, right);
          image.set(xRight, y, left);
        }
      }
    }

    if (flipOp->mode == FlipImageOperation::FlipY || flipOp->mode == FlipImageOperation::FlipXY) {
      for (size_t x = 0; x < image.width(); ++x) {
        for (size_t yTop = 0; yTop < image.height() / 2; ++yTop) {
          size_t yBottom = image.height() - 1 - yTop;

          auto top = image.get(x, yTop);
          auto bottom = image.get(x, yBottom);

          image.set(x, yTop, bottom);
          image.set(x, yBottom, top);
        }
      }
    }
  }
}

Image processImageOperations(List<ImageOperation> const& operations, Image image, ImageReferenceCallback refCallback) {
  for (auto const& operation : operations)
    processImageOperation(operation, image, refCallback);

  return image;
}

}
