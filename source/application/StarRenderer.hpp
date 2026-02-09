#pragma once

#include "StarBiMap.hpp"
#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarImage.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarRefPtr.hpp"
#include "StarVariant.hpp"

import std;

namespace Star {

using RendererException = ExceptionDerived<"RendererException">;

class Texture;

enum class TextureAddressing {
  Clamp,
  Wrap
};
extern EnumMap<TextureAddressing> const TextureAddressingNames;

enum class TextureFiltering {
  Nearest,
  Linear
};
extern EnumMap<TextureFiltering> const TextureFilteringNames;

// Medium is the maximum guaranteed texture group size
// Where a Medium sized texture group is expected to fill a single page Large can be used,
// but is not guaranteed to be supported by all systems.
// Where Large sized textures are not supported, a Medium one is used
enum class TextureGroupSize {
  Small,
  Medium,
  Large
};

// Both screen coordinates and texture coordinates are in pixels from the
// bottom left to top right.
struct RenderVertex {
  Vec2F screenCoordinate;
  Vec2F textureCoordinate;
  Vec4B color;
  float param1;
};

class RenderTriangle {
public:
  RenderTriangle() = default;
  RenderTriangle(Vec2F posA, Vec2F posB, Vec2F posC, Vec4B color = Vec4B::filled(255), float param1 = 0.0f);
  RenderTriangle(RefPtr<Texture> tex, Vec2F posA, Vec2F uvA, Vec2F posB, Vec2F uvB, Vec2F posC, Vec2F uvC, Vec4B color = Vec4B::filled(255), float param1 = 0.0f);

  RefPtr<Texture> texture;
  RenderVertex a, b, c;
};

class RenderQuad {
public:
  RenderQuad() = default;
  RenderQuad(Vec2F posA, Vec2F posB, Vec2F posC, Vec2F posD, Vec4B color = Vec4B::filled(255), float param1 = 0.0f);
  RenderQuad(RefPtr<Texture> tex, Vec2F minScreen, float textureScale = 1.0f, Vec4B color = Vec4B::filled(255), float param1 = 0.0f);
  RenderQuad(RefPtr<Texture> tex, RectF const& screenCoords, Vec4B color = Vec4B::filled(255), float param1 = 0.0f);
  RenderQuad(RefPtr<Texture> tex, Vec2F posA, Vec2F uvA, Vec2F posB, Vec2F uvB, Vec2F posC, Vec2F uvC, Vec2F posD, Vec2F uvD, Vec4B color = Vec4B::filled(255), float param1 = 0.0f);
  RenderQuad(RefPtr<Texture> tex, RenderVertex vA, RenderVertex vB, RenderVertex vC, RenderVertex vD);
  RenderQuad(RectF const& rect, Vec4B color = Vec4B::filled(255), float param1 = 0.0f);

  RefPtr<Texture> texture;
  RenderVertex a, b, c, d;
};

class RenderPoly {
public:
  RenderPoly() = default;
  RenderPoly(List<Vec2F> const& verts, Vec4B color, float param1 = 0.0f);

  RefPtr<Texture> texture;
  List<RenderVertex> vertexes;
};

auto renderTexturedRect(RefPtr<Texture> texture, Vec2F minScreen, float textureScale = 1.0f, Vec4B color = Vec4B::filled(255), float param1 = 0.0f) -> RenderQuad;
auto renderTexturedRect(RefPtr<Texture> texture, RectF const& screenCoords, Vec4B color = Vec4B::filled(255), float param1 = 0.0f) -> RenderQuad;
auto renderFlatRect(RectF const& rect, Vec4B color, float param1 = 0.0f) -> RenderQuad;
auto renderFlatPoly(PolyF const& poly, Vec4B color, float param1 = 0.0f) -> RenderPoly;

using RenderPrimitive = Variant<RenderTriangle, RenderQuad, RenderPoly>;

class Texture : public RefCounter {
public:
  ~Texture() override = default;

  [[nodiscard]] virtual auto size() const -> Vec2U = 0;
  [[nodiscard]] virtual auto filtering() const -> TextureFiltering = 0;
  [[nodiscard]] virtual auto addressing() const -> TextureAddressing = 0;
};

// Textures may be created individually, or in a texture group.  Textures in
// a texture group will be faster to render when rendered together, and will
// use less texture memory when many small textures are in a common group.
// Texture groups must all have the same texture parameters, and will always
// use clamped texture addressing.
class TextureGroup {
public:
  virtual ~TextureGroup() = default;

  [[nodiscard]] virtual auto filtering() const -> TextureFiltering = 0;
  virtual auto create(Image const& texture) -> RefPtr<Texture> = 0;
};

class RenderBuffer {
public:
  virtual ~RenderBuffer() = default;

  // Transforms the given primitives into a form suitable for the underlying
  // graphics system and stores it for fast replaying.
  virtual void set(List<RenderPrimitive>& primitives) = 0;
};

using RenderEffectParameter = Variant<float, int, Vec4F, Vec3F, Vec2F, bool>;

class Renderer {
public:
  virtual ~Renderer() = default;

  [[nodiscard]] virtual auto rendererId() const -> String = 0;
  [[nodiscard]] virtual auto screenSize() const -> Vec2U = 0;

  virtual void loadConfig(Json const& config) = 0;

  // The actual shaders used by this renderer will be in a default no effects
  // state when constructed, but can be overridden here.  This config will be
  // specific to each type of renderer, so it will be necessary to key the
  // configuration off of the renderId string.  This should not be called every
  // frame, because it will result in a recompile of the underlying shader set.
  virtual void loadEffectConfig(String const& name, Json const& effectConfig, StringMap<String> const& shaders) = 0;

  // The effect config will specify named parameters and textures which can be
  // set here.
  virtual void setEffectParameter(String const& parameterName, RenderEffectParameter const& parameter) = 0;
  virtual void setEffectScriptableParameter(String const& effectName, String const& parameterName, RenderEffectParameter const& parameter) = 0;
  virtual auto getEffectScriptableParameter(String const& effectName, String const& parameterName) -> std::optional<RenderEffectParameter> = 0;
  virtual auto getEffectScriptableParameterType(String const& effectName, String const& parameterName) -> std::optional<VariantTypeIndex> = 0;
  virtual void setEffectTexture(String const& textureName, ImageView const& image) = 0;
  virtual auto switchEffectConfig(String const& name) -> bool = 0;

  // Any further rendering will be scissored based on this rect, specified in
  // pixels
  virtual void setScissorRect(std::optional<RectI> const& scissorRect) = 0;

  virtual auto createTexture(Image const& texture,
                             TextureAddressing addressing = TextureAddressing::Clamp,
                             TextureFiltering filtering = TextureFiltering::Nearest) -> RefPtr<Texture> = 0;
  virtual void setSizeLimitEnabled(bool enabled) = 0;
  virtual void setMultiTexturingEnabled(bool enabled) = 0;
  virtual void setMultiSampling(unsigned multiSampling) = 0;
  virtual auto createTextureGroup(TextureGroupSize size = TextureGroupSize::Medium, TextureFiltering filtering = TextureFiltering::Nearest) -> Ptr<TextureGroup> = 0;
  virtual auto createRenderBuffer() -> Ptr<RenderBuffer> = 0;

  virtual auto immediatePrimitives() -> List<RenderPrimitive>& = 0;
  virtual void render(RenderPrimitive primitive) = 0;
  virtual void renderBuffer(Ptr<RenderBuffer> const& renderBuffer, Mat3F const& transformation = Mat3F::identity()) = 0;

  virtual void flush(Mat3F const& transformation = Mat3F::identity()) = 0;
};

}// namespace Star
