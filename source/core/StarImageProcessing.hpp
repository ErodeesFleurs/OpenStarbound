#pragma once

#include "StarException.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
#include "StarJson.hpp"
#include "StarImage.hpp"

import std;

namespace Star {

using ImageOperationException = ExceptionDerived<"ImageOperationException">;

auto colorDirectivesFromConfig(JsonArray const& directives) -> StringList;
auto paletteSwapDirectivesFromConfig(Json const& swaps) -> String;

struct NullImageOperation {
  bool unloaded = false;
};

struct ErrorImageOperation {
  Variant<std::string, std::exception_ptr> cause;
};

struct HueShiftImageOperation {
  // Specify hue shift angle as -360 to 360 rather than -1 to 1
  static auto hueShiftDegrees(float degrees) -> HueShiftImageOperation;

  // value here is normalized to 1.0
  float hueShiftAmount;
};

struct SaturationShiftImageOperation {
  // Specify saturation shift as amount normalized to 100
  static auto saturationShift100(float amount) -> SaturationShiftImageOperation;

  // value here is normalized to 1.0
  float saturationShiftAmount;
};

struct BrightnessMultiplyImageOperation {
  // Specify brightness multiply as amount where 0 means "no change" and 100
  // means "x2" and -100 means "x0"
  static auto brightnessMultiply100(float amount) -> BrightnessMultiplyImageOperation;

  float brightnessMultiply;
};

// Fades R G and B channels to the given color by the given amount, ignores A
struct FadeToColorImageOperation {
  FadeToColorImageOperation(Vec3B color, float amount);

  Vec3B color;
  float amount;

  Array<std::uint8_t, 256> rTable;
  Array<std::uint8_t, 256> gTable;
  Array<std::uint8_t, 256> bTable;
};

// Applies two FadeToColor operations in alternating rows to produce a scanline effect
struct ScanLinesImageOperation {
  FadeToColorImageOperation fade1;
  FadeToColorImageOperation fade2;
};

// Sets RGB values to the given color, and ignores the alpha channel
struct SetColorImageOperation {
  Vec3B color;
};

using ColorReplaceMap = HashMap<Vec4B, Vec4B>;

struct ColorReplaceImageOperation {
  ColorReplaceMap colorReplaceMap;
};

struct AlphaMaskImageOperation {
  enum MaskMode {
    Additive,
    Subtractive
  };

  MaskMode mode;
  StringList maskImages;
  Vec2I offset;
};

struct BlendImageOperation {
  enum BlendMode {
    Multiply,
    Screen
  };

  BlendMode mode;
  StringList blendImages;
  Vec2I offset;
};

struct MultiplyImageOperation {
  Vec4B color;
};

struct BorderImageOperation {
  unsigned pixels;
  Vec4B startColor;
  Vec4B endColor;
  bool outlineOnly;
  bool includeTransparent;
};

struct ScaleImageOperation {
  enum Mode {
    Nearest,
    Bilinear,
    Bicubic
  };

  Mode mode;
  Vec2F scale;
};

struct CropImageOperation {
  RectI subset;
};

struct FlipImageOperation {
  enum Mode {
    FlipX,
    FlipY,
    FlipXY
  };
  Mode mode;
};

using ImageOperation = Variant<NullImageOperation, ErrorImageOperation, HueShiftImageOperation, SaturationShiftImageOperation, BrightnessMultiplyImageOperation, FadeToColorImageOperation,
  ScanLinesImageOperation, SetColorImageOperation, ColorReplaceImageOperation, AlphaMaskImageOperation, BlendImageOperation,
  MultiplyImageOperation, BorderImageOperation, ScaleImageOperation, CropImageOperation, FlipImageOperation>;

auto imageOperationFromString(StringView string) -> ImageOperation;
auto imageOperationToString(ImageOperation const& operation) -> String;

void parseImageOperations(StringView params, std::function<void(ImageOperation&&)> outputter);

// Each operation is assumed to be separated by '?', with parameters
// separated by ';' or '='
auto parseImageOperations(StringView params) -> List<ImageOperation>;

// Each operation separated by '?', returns string with leading '?'
auto printImageOperations(List<ImageOperation> const& operations) -> String;

void addImageOperationReferences(ImageOperation const& operation, StringList& out);

auto imageOperationReferences(List<ImageOperation> const& operations) -> StringList;

using ImageReferenceCallback = std::function<Image const*(String const& refName)>;

void processImageOperation(ImageOperation const& operation, Image& input, ImageReferenceCallback refCallback = {});

auto processImageOperations(List<ImageOperation> const& operations, Image input, ImageReferenceCallback refCallback = {}) -> Image;

}
