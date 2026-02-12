module star.image.processing;

import star.json;
import star.color;
import star.encode;
import star.format;
import star.lexical_cast;
import std;

namespace star {

auto color_directives_from_config(std::span<const json::json_value> directives)
  -> std::expected<std::vector<std::u8string>, image_operation_errc> {
    std::vector<std::u8string> result;
    result.reserve(directives.size());

    for (const auto& entry : directives) {
        if (entry.is<std::u8string>()) {
            result.push_back(entry.as<std::u8string>().value_or(u8""));
        } else if (entry.is<json::json_object>()) {
            result.push_back(palette_swap_directives_from_config(entry));
        } else {
            return std::unexpected(image_operation_errc::invalid_parameter);
        }
    }
    return result;
}

auto palette_swap_directives_from_config(const json::json_value& swaps) -> std::u8string {
    color_replace_image_operation palette_swaps;
    for (const auto& [from_hex, to_hex] :
         *(swaps.as<json::json_object>().value_or(std::make_shared<const json::json_object>()))) {
        palette_swaps.color_replace_map[color::from_hex(from_hex).to_rgba()] =
          color::from_hex(to_hex.as<std::u8string>().value_or(std::u8string{})).to_rgba();
    }
    return u8'?' + image_operation_to_string(palette_swaps);
}

constexpr auto hue_shift_image_operation::hue_shift_degrees(std::float_t degrees) noexcept
  -> hue_shift_image_operation {
    return hue_shift_image_operation{degrees / 360.0F};
}

constexpr auto saturation_shift_image_operation::saturation_shift_100(std::float_t amount) noexcept
  -> saturation_shift_image_operation {
    return saturation_shift_image_operation{amount / 100.0F};
}

constexpr auto
brightness_multiply_image_operation::brightness_multiply_100(std::float_t amount) noexcept
  -> brightness_multiply_image_operation {
    return brightness_multiply_image_operation{amount / 100.0F + 1.0F};
}

auto image_operation_from_string(std::u8string_view s) noexcept
  -> std::expected<image_operation, image_operation_errc> {
    std::size_t cursor = 0;

    auto next_bit = [&]() noexcept -> std::optional<std::u8string_view> {
        if (cursor > s.size()) {
            return std::nullopt;
        }

        std::size_t start = cursor;
        while (cursor < s.size() && s[cursor] != u8'=' && s[cursor] != u8';') {
            cursor++;
        }
        std::u8string_view bit(s.data() + start, cursor - start);
        cursor++;
        return bit;
    };

    auto next_float = [&]() noexcept -> std::optional<float> {
        return next_bit().and_then([](std::u8string_view sv) -> std::optional<float> {
            auto res = lexical_cast<float>(sv);
            return res ? std::optional<float>(*res) : std::nullopt;
        });
    };

    auto next_int = [&]() noexcept -> std::optional<int> {
        return next_bit().and_then([](std::u8string_view sv) -> std::optional<int> {
            auto res = lexical_cast<int>(sv);
            return res ? std::optional<std::int32_t>(*res) : std::nullopt;
        });
    };

    auto type = next_bit().value_or(u8"");
    if (type.empty()) {
        return std::unexpected(image_operation_errc::missing_parameter);
    }

    if (type == u8"hueshift") {
        if (auto val = next_float()) {
            return hue_shift_image_operation::hue_shift_degrees(*val);
        }
        return std::unexpected(image_operation_errc::invalid_parameter);

    } else if (type == u8"saturation") {
        if (auto val = next_float()) {
            return saturation_shift_image_operation::saturation_shift_100(*val);
        }
        return std::unexpected(image_operation_errc::invalid_parameter);

    } else if (type == u8"brightness") {
        if (auto val = next_float()) {
            return brightness_multiply_image_operation::brightness_multiply_100(*val);
        }
        return std::unexpected(image_operation_errc::invalid_parameter);

    } else if (type == u8"fade") {
        auto color_hex = next_bit();
        auto amount = next_float();
        if (!amount || !color_hex) {
            return std::unexpected(image_operation_errc::invalid_parameter);
        }
        return fade_to_color_image_operation{.color = color::from_hex(*color_hex).to_rgb(),
                                             .amount = *amount};

    } else if (type == u8"scanlines") {
        auto color_hex_1 = next_bit();
        auto amount_1 = next_float();
        auto color_hex_2 = next_bit();
        auto amount_2 = next_float();
        if (!color_hex_1 || !color_hex_2 || !amount_1 || !amount_2) {
            return std::unexpected(image_operation_errc::invalid_parameter);
        }
        return scan_lines_image_operation{
          .fade_1 = fade_to_color_image_operation{.color = color::from_hex(*color_hex_1).to_rgb(),
                                                  .amount = *amount_1},
          .fade_2 = fade_to_color_image_operation{.color = color::from_hex(*color_hex_2).to_rgb(),
                                                  .amount = *amount_2}};
    } else if (type == u8"setcolor") {
        if (auto color_hex = next_bit()) {
            return set_color_image_operation{color::from_hex(*color_hex).to_rgb()};
        }
        return std::unexpected(image_operation_errc::invalid_parameter);
    } else if (type == u8"replace") {
        color_replace_image_operation operation;
        while (true) {
            auto k = next_bit();
            auto v = next_bit();
            if (!k || !v) {
                break;
            }
            operation.color_replace_map[color::hex_to_vec_4b(*k)] = color::hex_to_vec_4b(*v);
        }
        return operation;
    } else if (type == u8"addmask" || type == u8"submask") {
        alpha_mask_image_operation operation;
        operation.mode = (type == u8"addmask") ? alpha_mask_image_operation::mask_mode::additive
                                               : alpha_mask_image_operation::mask_mode::subtractive;

        auto mask_images_bit = next_bit();
        if (!mask_images_bit) {
            return std::unexpected(image_operation_errc::invalid_parameter);
        }

        operation.mask_images = *mask_images_bit | std::views::split(u8'+')
          | std::ranges::to<std::vector<std::u8string>>();

        if (auto x = next_int()) {
            operation.offset[0] = *x;
        }

        if (auto y = next_int()) {
            operation.offset[1] = *y;
        }

        return operation;
    } else if (type == u8"blendmult" || type == u8"blendscreen") {
        blend_image_operation operation;
        operation.mode = (type == u8"blendmult") ? blend_image_operation::blend_mode::multiply
                                                 : blend_image_operation::blend_mode::screen;
        auto blend_images_bit = next_bit();
        if (!blend_images_bit) {
            return std::unexpected(image_operation_errc::invalid_parameter);
        }
        operation.blend_images = *blend_images_bit | std::views::split(u8'+')
          | std::ranges::to<std::vector<std::u8string>>();

        if (auto x = next_int()) {
            operation.offset[0] = *x;
        }
        if (auto y = next_int()) {
            operation.offset[1] = *y;
        }
        return operation;
    } else if (type == u8"multiply") {
        if (auto color_hex = next_bit()) {
            return multiply_image_operation{color::from_hex(*color_hex).to_rgba()};
        }
        return std::unexpected(image_operation_errc::invalid_parameter);
    } else if (type == u8"border" || type == u8"outline") {
        auto pixels = next_int();
        if (!pixels) {
            return std::unexpected(image_operation_errc::invalid_parameter);
        }
        border_image_operation operation;
        operation.pixels = *pixels;
        auto start_color_hex = next_bit();
        auto end_color_hex = next_bit();
        if (!start_color_hex) {
            return std::unexpected(image_operation_errc::invalid_parameter);
        }
        operation.start_color = color::from_hex(*start_color_hex).to_rgba();
        if (end_color_hex) {
            operation.end_color = color::from_hex(*end_color_hex).to_rgba();
        } else {
            operation.end_color = operation.start_color;
        }
        operation.outline_only = type == u8"outline";
        operation.include_transparent = false;// Currently just here for anti-aliased fonts
        return operation;
    } else if (type == u8"scalenearest" || type == u8"scalebilinear" || type == u8"scalebicubic"
               || type == u8"scale") {
        auto fill = next_float();
        if (!fill) {
            return std::unexpected(image_operation_errc::invalid_parameter);
        }
        auto rest = next_float();
        vec_2f scale;
        if (rest) {
            scale = vec_2f(*fill, *rest);
        } else {
            scale = vec_2f::filled(*fill);
        }

        enum scale_image_operation::mode mode;
        if (type == u8"scalenearest") {
            mode = scale_image_operation::mode::nearest;
        } else if (type == u8"scalebicubic") {
            mode = scale_image_operation::mode::bicubic;
        } else {
            mode = scale_image_operation::mode::bilinear;
        }

        return scale_image_operation{.mode = mode, .scale = scale};
    } else if (type == u8"crop") {
        auto pos_1 = next_float();
        auto pos_2 = next_float();
        auto pos_3 = next_float();
        auto pos_4 = next_float();
        if (!pos_1 || !pos_2 || !pos_3 || !pos_4) {
            return std::unexpected(image_operation_errc::invalid_parameter);
        }
        return crop_image_operation{
          rect_i(static_cast<std::int32_t>(*pos_1), static_cast<std::int32_t>(*pos_2),
                 static_cast<std::int32_t>(*pos_3), static_cast<std::int32_t>(*pos_4))};
    } else if (type == u8"flipx") {
        return flip_image_operation{flip_image_operation::mode::flip_x};
    } else if (type == u8"flipy") {
        return flip_image_operation{flip_image_operation::mode::flip_y};
    } else if (type == u8"flipxy") {
        return flip_image_operation{flip_image_operation::mode::flip_xy};
    } else {
        return null_image_operation();
    }
}

auto image_operation_to_string(const image_operation& op) -> std::u8string {
    return std::visit(
      [](const auto& arg) -> std::u8string {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, hue_shift_image_operation>) {
              return star::vformat(u8"hueshift={}", arg.hue_shift_amount * 360.0F);
          } else if constexpr (std::is_same_v<T, saturation_shift_image_operation>) {
              return star::vformat(u8"saturation={}", arg.saturation_shift_amount * 100.0F);
          } else if constexpr (std::is_same_v<T, brightness_multiply_image_operation>) {
              return star::vformat(u8"brightness={}", (arg.brightness_multiply - 1.0F) * 100.0F);
          } else if constexpr (std::is_same_v<T, fade_to_color_image_operation>) {
              return star::vformat(u8"fade={}={}", color::rgb(arg.color).to_hex(), arg.amount);
          } else if constexpr (std::is_same_v<T, scan_lines_image_operation>) {
              return star::vformat(u8"scanlines={}={}={}={}", color::rgb(arg.fade_1.color).to_hex(),
                                   arg.fade_1.amount, color::rgb(arg.fade_2.color).to_hex(),
                                   arg.fade_2.amount);
          } else if constexpr (std::is_same_v<T, set_color_image_operation>) {
              return star::vformat(u8"setcolor={}", color::rgb(arg.color).to_hex());
          } else if constexpr (std::is_same_v<T, color_replace_image_operation>) {
              auto parts =
                arg.color_replace_map | std::views::transform([](auto const& pair) -> auto {
                    return star::vformat(u8";{}={}", color::rgba(pair.first).to_hex(),
                                         color::rgba(pair.second).to_hex());
                });
              std::u8string result = u8"replace";
              for (auto const& segment : parts) {
                  result += segment;
              }
              return result;
          } else if constexpr (std::is_same_v<T, alpha_mask_image_operation>) {
              auto join_str =
                arg.mask_images | std::views::join_with(u8'+') | std::ranges::to<std::u8string>();
              if (arg.mode == alpha_mask_image_operation::mask_mode::additive) {
                  return star::vformat(u8"addmask={};{};{}", join_str, arg.offset[0],
                                       arg.offset[1]);
              } else if (arg.mode == alpha_mask_image_operation::mask_mode::subtractive) {
                  return star::vformat(u8"submask={};{};{}", join_str, arg.offset[0],
                                       arg.offset[1]);
              }
          } else if constexpr (std::is_same_v<T, blend_image_operation>) {
              auto join_str =
                arg.blend_images | std::views::join_with(u8'+') | std::ranges::to<std::u8string>();
              if (arg.mode == blend_image_operation::blend_mode::multiply) {
                  return star::vformat(u8"blendmult={};{};{}", join_str, arg.offset[0],
                                       arg.offset[1]);
              } else if (arg.mode == blend_image_operation::blend_mode::screen) {
                  return star::vformat(u8"blendscreen={};{};{}", join_str, arg.offset[0],
                                       arg.offset[1]);
              }
          } else if constexpr (std::is_same_v<T, multiply_image_operation>) {
              return star::vformat(u8"multiply={}", color::rgba(arg.color).to_hex());
          } else if constexpr (std::is_same_v<T, border_image_operation>) {
              auto type_str = arg.outline_only ? u8"outline" : u8"border";
              return star::vformat(u8"{}={};{};{};{}", type_str, arg.pixels,
                                   color::rgba(arg.start_color).to_hex(),
                                   color::rgba(arg.end_color).to_hex());
          } else if constexpr (std::is_same_v<T, scale_image_operation>) {
              auto mode_str = [&]() -> std::u8string_view {
                  switch (arg.mode) {
                  case scale_image_operation::mode::nearest: return u8"scalenearest";
                  case scale_image_operation::mode::bilinear: return u8"scalebilinear";
                  case scale_image_operation::mode::bicubic: return u8"scalebicubic";
                  default: return u8"scale";
                  }
              }();
              return star::vformat(u8"{}={};{}", mode_str, arg.scale.x(), arg.scale.y());
          } else if constexpr (std::is_same_v<T, crop_image_operation>) {
              return star::vformat(u8"crop={};{};{};{}", arg.subset.x_min(), arg.subset.x_max(),
                                   arg.subset.y_min(), arg.subset.y_max());
          } else if constexpr (std::is_same_v<T, flip_image_operation>) {
              auto mode_str = [&]() -> std::u8string {
                  switch (arg.mode) {
                  case flip_image_operation::mode::flip_x: return u8"flipx";
                  case flip_image_operation::mode::flip_y: return u8"flipy";
                  case flip_image_operation::mode::flip_xy: return u8"flipxy";
                  default: return u8"";
                  }
              }();
              return mode_str;
          }
          return u8"";
      },
      op);
    // } else if (auto op = operation.ptr<FlipImageOperation>()) {
    //     if (op->mode == FlipImageOperation::FlipX)
    //         return "flipx";
    //     else if (op->mode == FlipImageOperation::FlipY)
    //         return "flipy";
    //     else if (op->mode == FlipImageOperation::FlipXY)
    //         return "flipxy";
    // }
}

// void parseImageOperations(StringView params, std::function<void(ImageOperation&&)> outputter) {
//     params.forEachSplitView("?", [&](StringView op, std::size_t, std::size_t) -> void {
//         if (!op.empty())
//             outputter(imageOperationFromString(op));
//     });
// }

// auto parseImageOperations(StringView params) -> List<ImageOperation> {
//     List<ImageOperation> operations;
//     params.forEachSplitView("?", [&](StringView op, std::size_t, std::size_t) -> void {
//         if (!op.empty())
//             operations.append(imageOperationFromString(op));
//     });

//     return operations;
// }

// auto printImageOperations(List<ImageOperation> const& list) -> String {
//     return StringList(list.transformed(imageOperationToString)).join("?");
// }

// void addImageOperationReferences(ImageOperation const& operation, StringList& out) {
//     if (auto op = operation.ptr<AlphaMaskImageOperation>())
//         out.appendAll(op->maskImages);
//     else if (auto op = operation.ptr<BlendImageOperation>())
//         out.appendAll(op->blendImages);
// }

// auto imageOperationReferences(List<ImageOperation> const& operations) -> StringList {
//     StringList references;
//     for (auto const& operation : operations)
//         addImageOperationReferences(operation, references);
//     return references;
// }

// static void processSaturationShift(Image& image, SaturationShiftImageOperation const* op) {
//     image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) -> void {
//         if (pixel[3] != 0) {
//             Color color = Color::rgba(pixel);
//             color.setSaturation(clamp(color.saturation() + op->saturationShiftAmount, 0.0f, 1.0f));
//             pixel = color.toRgba();
//         }
//     });
// }

// void processImageOperation(ImageOperation const& operation, Image& image,
//                            ImageReferenceCallback refCallback) {
//     if (image.bytesPerPixel() == 3) {
//         // Convert to an image format that has alpha so certain operations function properly
//         image = image.convert(image.pixelFormat() == PixelFormat::BGR24 ? PixelFormat::BGRA32
//                                                                         : PixelFormat::RGBA32);
//     }
//     if (auto op = operation.ptr<HueShiftImageOperation>()) {
//         image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) -> void {
//             if (pixel[3] != 0)
//                 pixel = Color::hueShiftVec4B(pixel, op->hueShiftAmount);
//         });
//     } else if (auto op = operation.ptr<SaturationShiftImageOperation>()) {
//         processSaturationShift(image, op);
//     } else if (auto op = operation.ptr<BrightnessMultiplyImageOperation>()) {
//         image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) -> void {
//             if (pixel[3] != 0) {
//                 Color color = Color::rgba(pixel);
//                 color.setValue(clamp(color.value() * op->brightnessMultiply, 0.0f, 1.0f));
//                 pixel = color.toRgba();
//             }
//         });
//     } else if (auto op = operation.ptr<FadeToColorImageOperation>()) {
//         image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) -> void {
//             pixel[0] = op->rTable[pixel[0]];
//             pixel[1] = op->gTable[pixel[1]];
//             pixel[2] = op->bTable[pixel[2]];
//         });
//     } else if (auto op = operation.ptr<ScanLinesImageOperation>()) {
//         image.forEachPixel([&op](unsigned, unsigned y, Vec4B& pixel) -> void {
//             if (y % 2 == 0) {
//                 pixel[0] = op->fade1.rTable[pixel[0]];
//                 pixel[1] = op->fade1.gTable[pixel[1]];
//                 pixel[2] = op->fade1.bTable[pixel[2]];
//             } else {
//                 pixel[0] = op->fade2.rTable[pixel[0]];
//                 pixel[1] = op->fade2.gTable[pixel[1]];
//                 pixel[2] = op->fade2.bTable[pixel[2]];
//             }
//         });
//     } else if (auto op = operation.ptr<SetColorImageOperation>()) {
//         image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) -> void {
//             pixel[0] = op->color[0];
//             pixel[1] = op->color[1];
//             pixel[2] = op->color[2];
//         });
//     } else if (auto op = operation.ptr<ColorReplaceImageOperation>()) {
//         image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) -> void {
//             if (auto m = op->colorReplaceMap.maybe(pixel))
//                 pixel = *m;
//         });

//     } else if (auto op = operation.ptr<AlphaMaskImageOperation>()) {
//         if (op->maskImages.empty())
//             return;

//         if (!refCallback)
//             throw StarException("Missing image ref callback during AlphaMaskImageOperation in "
//                                 "ImageProcessor::process");

//         List<Image const*> maskImages;
//         for (auto const& reference : op->maskImages)
//             maskImages.append(refCallback(reference));

//         image.forEachPixel([&op, &maskImages](unsigned x, unsigned y, Vec4B& pixel) -> void {
//             std::uint8_t maskAlpha = 0;
//             Vec2U pos = Vec2U(Vec2I(x, y) + op->offset);
//             for (auto mask : maskImages) {
//                 if (pos[0] < mask->width() && pos[1] < mask->height()) {
//                     if (op->mode == AlphaMaskImageOperation::Additive) {
//                         // We produce our mask alpha from the maximum alpha of any of
//                         // the
//                         // mask images.
//                         maskAlpha = std::max(maskAlpha, mask->get(pos)[3]);
//                     } else if (op->mode == AlphaMaskImageOperation::Subtractive) {
//                         // We produce our mask alpha from the minimum alpha of any of
//                         // the
//                         // mask images.
//                         maskAlpha = std::min(maskAlpha, mask->get(pos)[3]);
//                     }
//                 }
//             }
//             pixel[3] = std::min(pixel[3], maskAlpha);
//         });

//     } else if (auto op = operation.ptr<BlendImageOperation>()) {
//         if (op->blendImages.empty())
//             return;

//         if (!refCallback)
//             throw StarException("Missing image ref callback during BlendImageOperation in "
//                                 "ImageProcessor::process");

//         List<Image const*> blendImages;
//         for (auto const& reference : op->blendImages)
//             blendImages.append(refCallback(reference));

//         image.forEachPixel([&op, &blendImages](unsigned x, unsigned y, Vec4B& pixel) -> void {
//             Vec2U pos = Vec2U(Vec2I(x, y) + op->offset);
//             Vec4F fpixel = Color::v4bToFloat(pixel);
//             for (auto blend : blendImages) {
//                 if (pos[0] < blend->width() && pos[1] < blend->height()) {
//                     Vec4F blendPixel = Color::v4bToFloat(blend->get(pos));
//                     if (op->mode == BlendImageOperation::Multiply)
//                         fpixel = fpixel.piecewiseMultiply(blendPixel);
//                     else if (op->mode == BlendImageOperation::Screen)
//                         fpixel = Vec4F::filled(1.0f)
//                           - (Vec4F::filled(1.0f) - fpixel)
//                               .piecewiseMultiply(Vec4F::filled(1.0f) - blendPixel);
//                 }
//             }
//             pixel = Color::v4fToByte(fpixel);
//         });

//     } else if (auto op = operation.ptr<MultiplyImageOperation>()) {
//         image.forEachPixel([&op](unsigned, unsigned, Vec4B& pixel) -> void {
//             pixel = pixel.combine(op->color, [](std::uint8_t a, std::uint8_t b) -> std::uint8_t {
//                 return (std::uint8_t)(((int)a * (int)b) / 255);
//             });
//         });

//     } else if (auto op = operation.ptr<BorderImageOperation>()) {
//         Image borderImage(image.size() + Vec2U::filled(op->pixels * 2), PixelFormat::RGBA32);
//         borderImage.copyInto(Vec2U::filled(op->pixels), image);
//         Vec2I borderImageSize = Vec2I(borderImage.size());

//         borderImage.forEachPixel([&op, &image, &borderImageSize](int x, int y,
//                                                                  Vec4B& pixel) -> void {
//             int pixels = op->pixels;
//             bool includeTransparent = op->includeTransparent;
//             if (pixel[3] == 0 || (includeTransparent && pixel[3] != 255)) {
//                 int dist = std::numeric_limits<int>::max();
//                 for (int j = -pixels; j < pixels + 1; j++) {
//                     for (int i = -pixels; i < pixels + 1; i++) {
//                         if (i + x >= pixels && j + y >= pixels
//                             && i + x < borderImageSize[0] - pixels
//                             && j + y < borderImageSize[1] - pixels) {
//                             Vec4B remotePixel = image.get(i + x - pixels, j + y - pixels);
//                             if (remotePixel[3] != 0) {
//                                 dist = std::min(dist, std::abs(i) + std::abs(j));
//                                 if (dist == 1)// Early out, if dist is 1 it ain't getting shorter
//                                     break;
//                             }
//                         }
//                     }
//                 }

//                 if (dist < std::numeric_limits<int>::max()) {
//                     float percent = (dist - 1) / (2.0f * pixels - 1);
//                     if (pixel[3] != 0) {
//                         Color color =
//                           Color::rgba(op->startColor).mix(Color::rgba(op->endColor), percent);
//                         if (op->outlineOnly) {
//                             float pixelA = byteToFloat(pixel[3]);
//                             color.setAlphaF((1.0f - pixelA) * std::fminf(pixelA, 0.5f) * 2.0f);
//                         } else {
//                             Color pixelF = Color::rgba(pixel);
//                             float pixelA = pixelF.alphaF(), colorA = color.alphaF();
//                             colorA += pixelA * (1.0f - colorA);
//                             pixelF
//                               .convertToLinear();//Mix in linear color space as it is more perceptually accurate
//                             color.convertToLinear();
//                             color = color.mix(pixelF, pixelA);
//                             color.convertToSRGB();
//                             color.setAlphaF(colorA);
//                         }
//                         pixel = color.toRgba();
//                     } else {
//                         pixel = Vec4B(Vec4F(op->startColor) * (1 - percent)
//                                       + Vec4F(op->endColor) * percent);
//                     }
//                 }
//             } else if (op->outlineOnly) {
//                 pixel = Vec4B(0, 0, 0, 0);
//             }
//         });

//         image = borderImage;

//     } else if (auto op = operation.ptr<ScaleImageOperation>()) {
//         auto scale = op->scale;
//         if (scale[0] < 0.0f || scale[1] < 0.0f) {
//             Logger::warn("Negative scale in ScaleImageOperation ({})", scale);
//             scale = scale.piecewiseMax(Vec2F::filled(0.f));
//         }
//         if (op->mode == ScaleImageOperation::Nearest)
//             image = scaleNearest(image, scale);
//         else if (op->mode == ScaleImageOperation::Bilinear)
//             image = scaleBilinear(image, scale);
//         else if (op->mode == ScaleImageOperation::Bicubic)
//             image = scaleBicubic(image, scale);

//     } else if (auto op = operation.ptr<CropImageOperation>()) {
//         image = image.subImage(Vec2U(op->subset.min()), Vec2U(op->subset.size()));

//     } else if (auto op = operation.ptr<FlipImageOperation>()) {
//         if (op->mode == FlipImageOperation::FlipX || op->mode == FlipImageOperation::FlipXY) {
//             for (std::size_t y = 0; y < image.height(); ++y) {
//                 for (std::size_t xLeft = 0; xLeft < image.width() / 2; ++xLeft) {
//                     std::size_t xRight = image.width() - 1 - xLeft;

//                     auto left = image.get(xLeft, y);
//                     auto right = image.get(xRight, y);

//                     image.set(xLeft, y, right);
//                     image.set(xRight, y, left);
//                 }
//             }
//         }

//         if (op->mode == FlipImageOperation::FlipY || op->mode == FlipImageOperation::FlipXY) {
//             for (std::size_t x = 0; x < image.width(); ++x) {
//                 for (std::size_t yTop = 0; yTop < image.height() / 2; ++yTop) {
//                     std::size_t yBottom = image.height() - 1 - yTop;

//                     auto top = image.get(x, yTop);
//                     auto bottom = image.get(x, yBottom);

//                     image.set(x, yTop, bottom);
//                     image.set(x, yBottom, top);
//                 }
//             }
//         }
//     }
// }

// auto processImageOperations(List<ImageOperation> const& operations, Image image,
//                             ImageReferenceCallback refCallback) -> Image {
//     for (auto const& operation : operations)
//         processImageOperation(operation, image, refCallback);

//     return image;
// }

}// namespace star
