#pragma once

#include "StarConfig.hpp"
#include "StarRenderer.hpp"
#include "StarTextureAtlas.hpp"

#include "GL/glew.h"

import std;

namespace Star {

constexpr std::size_t FrameBufferCount = 1;

// OpenGL 2.0 implementation of Renderer.  OpenGL context must be created and
// active during construction, destruction, and all method calls.
class OpenGlRenderer : public Renderer {
public:
  OpenGlRenderer();
  ~OpenGlRenderer() override;

  [[nodiscard]] auto rendererId() const -> String override;
  [[nodiscard]] auto screenSize() const -> Vec2U override;

  void loadConfig(Json const& config) override;
  void loadEffectConfig(String const& name, Json const& effectConfig, StringMap<String> const& shaders) override;

  void setEffectParameter(String const& parameterName, RenderEffectParameter const& parameter) override;
  void setEffectScriptableParameter(String const& effectName, String const& parameterName, RenderEffectParameter const& parameter) override;
  auto getEffectScriptableParameter(String const& effectName, String const& parameterName) -> std::optional<RenderEffectParameter> override;
  auto getEffectScriptableParameterType(String const& effectName, String const& parameterName) -> std::optional<VariantTypeIndex> override;
  void setEffectTexture(String const& textureName, ImageView const& image) override;

  void setScissorRect(std::optional<RectI> const& scissorRect) override;

  auto switchEffectConfig(String const& name) -> bool override;

  auto createTexture(Image const& texture, TextureAddressing addressing, TextureFiltering filtering) -> RefPtr<Texture> override;
  void setSizeLimitEnabled(bool enabled) override;
  void setMultiTexturingEnabled(bool enabled) override;
  void setMultiSampling(unsigned multiSampling) override;
  auto createTextureGroup(TextureGroupSize size, TextureFiltering filtering) -> Ptr<TextureGroup> override;
  auto createRenderBuffer() -> Ptr<RenderBuffer> override;

  auto immediatePrimitives() -> List<RenderPrimitive>& override;
  void render(RenderPrimitive primitive) override;
  void renderBuffer(Ptr<RenderBuffer> const& renderBuffer, Mat3F const& transformation) override;

  void flush(Mat3F const& transformation) override;

  void setScreenSize(Vec2U screenSize);

  void startFrame();
  void finishFrame();

private:
  struct GlTextureAtlasSet : public TextureAtlasSet<GLuint> {
  public:
    GlTextureAtlasSet(unsigned atlasNumCells);

    auto createAtlasTexture(Vec2U const& size, PixelFormat pixelFormat) -> GLuint override;
    void destroyAtlasTexture(GLuint const& glTexture) override;
    void copyAtlasPixels(GLuint const& glTexture, Vec2U const& bottomLeft, Image const& image) override;

    TextureFiltering textureFiltering;
  };

  struct GlTextureGroup : std::enable_shared_from_this<GlTextureGroup>, public TextureGroup {
    GlTextureGroup(unsigned atlasNumCells);
    ~GlTextureGroup() override;

    auto filtering() const -> TextureFiltering override;
    auto create(Image const& texture) -> RefPtr<Texture> override;

    GlTextureAtlasSet textureAtlasSet;
  };

  struct GlTexture : public Texture {
    [[nodiscard]] virtual auto glTextureId() const -> GLuint = 0;
    [[nodiscard]] virtual auto glTextureSize() const -> Vec2U = 0;
    [[nodiscard]] virtual auto glTextureCoordinateOffset() const -> Vec2U = 0;
  };

  struct GlGroupedTexture : public GlTexture {
    ~GlGroupedTexture() override;

    [[nodiscard]] auto size() const -> Vec2U override;
    [[nodiscard]] auto filtering() const -> TextureFiltering override;
    [[nodiscard]] auto addressing() const -> TextureAddressing override;

    [[nodiscard]] auto glTextureId() const -> GLuint override;
    [[nodiscard]] auto glTextureSize() const -> Vec2U override;
    [[nodiscard]] auto glTextureCoordinateOffset() const -> Vec2U override;

    void incrementBufferUseCount();
    void decrementBufferUseCount();

    unsigned bufferUseCount = 0;
    std::shared_ptr<GlTextureGroup> parentGroup;
    GlTextureAtlasSet::TextureHandle parentAtlasTexture = nullptr;
  };

  struct GlLoneTexture : public GlTexture {
    ~GlLoneTexture() override;

    [[nodiscard]] auto size() const -> Vec2U override;
    [[nodiscard]] auto filtering() const -> TextureFiltering override;
    [[nodiscard]] auto addressing() const -> TextureAddressing override;

    [[nodiscard]] auto glTextureId() const -> GLuint override;
    [[nodiscard]] auto glTextureSize() const -> Vec2U override;
    [[nodiscard]] auto glTextureCoordinateOffset() const -> Vec2U override;

    GLuint textureId = 0;
    Vec2U textureSize;
    TextureAddressing textureAddressing = TextureAddressing::Clamp;
    TextureFiltering textureFiltering = TextureFiltering::Nearest;
  };

  struct GlPackedVertexData {
    uint32_t textureIndex : 2;
    uint32_t fullbright : 1;
    uint32_t rX : 1;
    uint32_t rY : 1;
    uint32_t unused : 27;
  };

  struct GlRenderVertex {
    Vec2F pos;
    Vec2F uv;
    Vec4B color;
    union Packed {
      uint32_t packed;
      GlPackedVertexData vars;
    } pack;
  };

  struct GlRenderBuffer : public RenderBuffer {
    struct GlVertexBufferTexture {
      GLuint texture;
      Vec2U size;
    };

    struct GlVertexBuffer {
      List<GlVertexBufferTexture> textures;
      GLuint vertexBuffer = 0;
      size_t vertexCount = 0;
    };

    GlRenderBuffer();
    ~GlRenderBuffer() override;

    void set(List<RenderPrimitive>& primitives) override;

    RefPtr<GlTexture> whiteTexture;
    ByteArray accumulationBuffer;

    HashSet<RefPtr<Texture>> usedTextures;
    List<GlVertexBuffer> vertexBuffers;
    GLuint vertexArray = 0;

    bool useMultiTexturing{true};
  };

  struct EffectParameter {
    GLint parameterUniform = -1;
    VariantTypeIndex parameterType = 0;
    std::optional<RenderEffectParameter> parameterValue;
  };

  struct EffectTexture {
    GLint textureUniform = -1;
    unsigned textureUnit = 0;
    TextureAddressing textureAddressing = TextureAddressing::Clamp;
    TextureFiltering textureFiltering = TextureFiltering::Linear;
    GLint textureSizeUniform = -1;
    RefPtr<GlLoneTexture> textureValue;
  };

  struct GlFrameBuffer : RefCounter {
    GLuint id = 0;
    RefPtr<GlLoneTexture> texture;

    Json config;
    bool blitted = false;
    unsigned multisample = 0;
    unsigned sizeDiv = 1;

    GlFrameBuffer(Json const& config);
    ~GlFrameBuffer() override;
  };

  class Effect {
  public:
    GLuint program = 0;
    Json config;
    StringMap<EffectParameter> parameters;
    StringMap<EffectParameter> scriptables;// scriptable parameters which can be changed when the effect is not loaded
    StringMap<EffectTexture> textures;

    StringMap<GLuint> attributes;
    StringMap<GLuint> uniforms;

    auto getAttribute(String const& name) -> GLuint;
    auto getUniform(String const& name) -> GLuint;
    bool includeVBTextures;
  };

  static auto logGlErrorSummary(String prefix) -> bool;
  static void uploadTextureImage(PixelFormat pixelFormat, Vec2U size, uint8_t const* data);

  static auto createGlTexture(ImageView const& image, TextureAddressing addressing, TextureFiltering filtering) -> RefPtr<GlLoneTexture>;

  auto createGlRenderBuffer() -> std::shared_ptr<GlRenderBuffer>;

  void flushImmediatePrimitives(Mat3F const& transformation = Mat3F::identity());

  void renderGlBuffer(GlRenderBuffer const& renderBuffer, Mat3F const& transformation);

  void setupGlUniforms(Effect& effect, Vec2U screenSize);

  auto getGlFrameBuffer(String const& id) -> RefPtr<OpenGlRenderer::GlFrameBuffer>;
  void blitGlFrameBuffer(RefPtr<OpenGlRenderer::GlFrameBuffer> const& frameBuffer);
  void switchGlFrameBuffer(RefPtr<OpenGlRenderer::GlFrameBuffer> const& frameBuffer);

  Vec2U m_screenSize;

  GLuint m_program = 0;

  GLint m_positionAttribute = -1;
  GLint m_colorAttribute = -1;
  GLint m_texCoordAttribute = -1;
  GLint m_dataAttribute = -1;
  List<GLint> m_textureUniforms = {};
  List<GLint> m_textureSizeUniforms = {};
  GLint m_screenSizeUniform = -1;
  GLint m_vertexTransformUniform = -1;

  Json m_config;

  StringMap<Effect> m_effects;
  Effect* m_currentEffect;

  StringMap<RefPtr<GlFrameBuffer>> m_frameBuffers;
  RefPtr<GlFrameBuffer> m_currentFrameBuffer;

  RefPtr<GlTexture> m_whiteTexture;

  std::optional<RectI> m_scissorRect;

  bool m_limitTextureGroupSize;
  bool m_useMultiTexturing;
  unsigned m_multiSampling;// if non-zero, is enabled and acts as sample count
  List<std::shared_ptr<GlTextureGroup>> m_liveTextureGroups;

  List<RenderPrimitive> m_immediatePrimitives;
  std::shared_ptr<GlRenderBuffer> m_immediateRenderBuffer;
};

}// namespace Star
