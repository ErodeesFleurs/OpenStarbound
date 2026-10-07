module;

#include "StarList.hpp"
#include "StarRect.hpp"
#include "StarMap.hpp"
#include "StarVariant.hpp"
#include "StarString.hpp"

namespace Star {

class Json;
class StringView;
typedef List<Json> JsonArray;

STAR_CLASS(Image);

struct ImageOperationExceptionTag {
  static constexpr char const* name() { return "ImageOperationException"; }
};
using ImageOperationException = StarError<ImageOperationExceptionTag, StarException>;

StringList colorDirectivesFromConfig(JsonArray const& directives);
String paletteSwapDirectivesFromConfig(Json const& swaps);

struct NullImageOperation {
  bool unloaded = false;
};

struct ErrorImageOperation {
  Variant<std::string, std::exception_ptr> cause;
};

struct HueShiftImageOperation {
  // Specify hue shift angle as -360 to 360 rather than -1 to 1
  static HueShiftImageOperation hueShiftDegrees(float degrees);

  // value here is normalized to 1.0
  float hueShiftAmount;
};

struct SaturationShiftImageOperation {
  // Specify saturation shift as amount normalized to 100
  static SaturationShiftImageOperation saturationShift100(float amount);

  // value here is normalized to 1.0
  float saturationShiftAmount;
};

struct BrightnessMultiplyImageOperation {
  // Specify brightness multiply as amount where 0 means "no change" and 100
  // means "x2" and -100 means "x0"
  static BrightnessMultiplyImageOperation brightnessMultiply100(float amount);

  float brightnessMultiply;
};

// Fades R G and B channels to the given color by the given amount, ignores A
struct FadeToColorImageOperation {
  FadeToColorImageOperation(Vec3B color, float amount);

  Vec3B color;
  float amount;

  Array<uint8_t, 256> rTable;
  Array<uint8_t, 256> gTable;
  Array<uint8_t, 256> bTable;
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

typedef HashMap<Vec4B, Vec4B> ColorReplaceMap;

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

typedef Variant<NullImageOperation, ErrorImageOperation, HueShiftImageOperation, SaturationShiftImageOperation, BrightnessMultiplyImageOperation, FadeToColorImageOperation,
  ScanLinesImageOperation, SetColorImageOperation, ColorReplaceImageOperation, AlphaMaskImageOperation, BlendImageOperation,
  MultiplyImageOperation, BorderImageOperation, ScaleImageOperation, CropImageOperation, FlipImageOperation> ImageOperation;

ImageOperation imageOperationFromString(StringView string);
String imageOperationToString(ImageOperation const& operation);

void parseImageOperations(StringView params, function<void(ImageOperation&&)> outputter);

// Each operation is assumed to be separated by '?', with parameters
// separated by ';' or '='
List<ImageOperation> parseImageOperations(StringView params);

// Each operation separated by '?', returns string with leading '?'
String printImageOperations(List<ImageOperation> const& operations);

void addImageOperationReferences(ImageOperation const& operation, StringList& out);

StringList imageOperationReferences(List<ImageOperation> const& operations);

typedef function<Image const*(String const& refName)> ImageReferenceCallback;

void processImageOperation(ImageOperation const& operation, Image& input, ImageReferenceCallback refCallback = {});

Image processImageOperations(List<ImageOperation> const& operations, Image input, ImageReferenceCallback refCallback = {});

}

export module star.image_processing;

export namespace Star {
  using ::Star::Image;
  using ::Star::ImagePtr;
  using ::Star::ImageConstPtr;
  using ::Star::ImageUPtr;
  using ::Star::ImageConstUPtr;
  using ::Star::ImageWeakPtr;
  using ::Star::ImageConstWeakPtr;
  using ::Star::ImageOperationExceptionTag;
  using ::Star::ImageOperationException;
  using ::Star::colorDirectivesFromConfig;
  using ::Star::paletteSwapDirectivesFromConfig;
  using ::Star::NullImageOperation;
  using ::Star::ErrorImageOperation;
  using ::Star::HueShiftImageOperation;
  using ::Star::SaturationShiftImageOperation;
  using ::Star::BrightnessMultiplyImageOperation;
  using ::Star::FadeToColorImageOperation;
  using ::Star::ScanLinesImageOperation;
  using ::Star::SetColorImageOperation;
  using ::Star::ColorReplaceMap;
  using ::Star::ColorReplaceImageOperation;
  using ::Star::AlphaMaskImageOperation;
  using ::Star::BlendImageOperation;
  using ::Star::MultiplyImageOperation;
  using ::Star::BorderImageOperation;
  using ::Star::ScaleImageOperation;
  using ::Star::CropImageOperation;
  using ::Star::FlipImageOperation;
  using ::Star::ImageOperation;
  using ::Star::imageOperationFromString;
  using ::Star::imageOperationToString;
  using ::Star::parseImageOperations;
  using ::Star::printImageOperations;
  using ::Star::addImageOperationReferences;
  using ::Star::imageOperationReferences;
  using ::Star::ImageReferenceCallback;
  using ::Star::processImageOperation;
  using ::Star::processImageOperations;
}
