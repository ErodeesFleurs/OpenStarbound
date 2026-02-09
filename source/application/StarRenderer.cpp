#include "StarRenderer.hpp"

import std;

namespace Star {

EnumMap<TextureAddressing> const TextureAddressingNames{
  {TextureAddressing::Clamp, "Clamp"},
  {TextureAddressing::Wrap, "Wrap"}};

EnumMap<TextureFiltering> const TextureFilteringNames{
  {TextureFiltering::Nearest, "Nearest"},
  {TextureFiltering::Linear, "Linear"}};

RenderQuad::RenderQuad(Vec2F posA, Vec2F posB, Vec2F posC, Vec2F posD, Vec4B color, float param1) : texture() {
  a = {.screenCoordinate = posA, .textureCoordinate = {0, 0}, .color = color, .param1 = param1};
  b = {.screenCoordinate = posB, .textureCoordinate = {0, 0}, .color = color, .param1 = param1};
  c = {.screenCoordinate = posC, .textureCoordinate = {0, 0}, .color = color, .param1 = param1};
  d = {.screenCoordinate = posD, .textureCoordinate = {0, 0}, .color = color, .param1 = param1};
}

RenderQuad::RenderQuad(RefPtr<Texture> tex, Vec2F minPosition, float textureScale, Vec4B color, float param1) : texture(std::move(tex)) {
  Vec2F size = Vec2F(texture->size());
  a = {.screenCoordinate = minPosition, .textureCoordinate = {0, 0}, .color = color, .param1 = param1};
  b = {.screenCoordinate = {(minPosition[0] + size[0] * textureScale), minPosition[1]}, .textureCoordinate = {size[0], 0}, .color = color, .param1 = param1};
  c = {.screenCoordinate = {(minPosition[0] + size[0] * textureScale), (minPosition[1] + size[1] * textureScale)}, .textureCoordinate = size, .color = color, .param1 = param1};
  d = {.screenCoordinate = {minPosition[0], (minPosition[1] + size[1] * textureScale)}, .textureCoordinate = {0, size[1]}, .color = color, .param1 = param1};
}

RenderQuad::RenderQuad(RefPtr<Texture> tex, RectF const& screenCoords, Vec4B color, float param1) : texture(std::move(tex)) {
  Vec2F size = Vec2F(texture->size());
  a = {.screenCoordinate = screenCoords.min(), .textureCoordinate = {0, 0}, .color = color, .param1 = param1};
  b = {.screenCoordinate = {
         screenCoords.xMax(),
         screenCoords.yMin(),
       },
       .textureCoordinate = {size[0], 0.f},
       .color = color,
       .param1 = param1};
  c = {.screenCoordinate = screenCoords.max(), .textureCoordinate = size, .color = color, .param1 = param1};
  d = {.screenCoordinate = {
         screenCoords.xMin(),
         screenCoords.yMax(),
       },
       .textureCoordinate = {0.f, size[1]},
       .color = color,
       .param1 = param1};
}

RenderQuad::RenderQuad(RefPtr<Texture> tex, Vec2F posA, Vec2F uvA, Vec2F posB, Vec2F uvB, Vec2F posC, Vec2F uvC, Vec2F posD, Vec2F uvD, Vec4B color, float param1) : texture(std::move(tex)) {
  a = {.screenCoordinate = posA, .textureCoordinate = uvA, .color = color, .param1 = param1};
  b = {.screenCoordinate = posB, .textureCoordinate = uvB, .color = color, .param1 = param1};
  c = {.screenCoordinate = posC, .textureCoordinate = uvC, .color = color, .param1 = param1};
  d = {.screenCoordinate = posD, .textureCoordinate = uvD, .color = color, .param1 = param1};
}

RenderQuad::RenderQuad(RefPtr<Texture> tex, RenderVertex vA, RenderVertex vB, RenderVertex vC, RenderVertex vD)
    : texture(std::move(tex)), a(std::move(vA)), b(std::move(vB)), c(std::move(vC)), d(std::move(vD)) {}

RenderQuad::RenderQuad(RectF const& rect, Vec4B color, float param1)
    : a{.screenCoordinate = rect.min(), .textureCoordinate = {}, .color = color, .param1 = param1}, b{.screenCoordinate = {rect.xMax(), rect.yMin()}, .textureCoordinate = {}, .color = color, .param1 = param1}, c{.screenCoordinate = rect.max(), .textureCoordinate = {}, .color = color, .param1 = param1}, d{.screenCoordinate = {rect.xMin(), rect.yMax()}, .textureCoordinate = {}, .color = color, .param1 = param1} {};

RenderPoly::RenderPoly(List<Vec2F> const& verts, Vec4B color, float param1) {
  vertexes.reserve(verts.size());
  for (Vec2F const& v : verts)
    vertexes.append({v, {0, 0}, color, param1});
}

RenderTriangle::RenderTriangle(Vec2F posA, Vec2F posB, Vec2F posC, Vec4B color, float param1) : texture() {
  a = {.screenCoordinate = posA, .textureCoordinate = {0, 0}, .color = color, .param1 = param1};
  b = {.screenCoordinate = posB, .textureCoordinate = {0, 0}, .color = color, .param1 = param1};
  c = {.screenCoordinate = posC, .textureCoordinate = {0, 0}, .color = color, .param1 = param1};
}

RenderTriangle::RenderTriangle(RefPtr<Texture> tex, Vec2F posA, Vec2F uvA, Vec2F posB, Vec2F uvB, Vec2F posC, Vec2F uvC, Vec4B color, float param1) : texture(std::move(tex)) {
  a = {.screenCoordinate = posA, .textureCoordinate = uvA, .color = color, .param1 = param1};
  b = {.screenCoordinate = posB, .textureCoordinate = uvB, .color = color, .param1 = param1};
  c = {.screenCoordinate = posC, .textureCoordinate = uvC, .color = color, .param1 = param1};
}

auto renderTexturedRect(RefPtr<Texture> texture, Vec2F minPosition, float textureScale, Vec4B color, float param1) -> RenderQuad {
  return {std::move(texture), minPosition, textureScale, color, param1};
}

auto renderTexturedRect(RefPtr<Texture> texture, RectF const& screenCoords, Vec4B color, float param1) -> RenderQuad {
  return {std::move(texture), screenCoords, color, param1};
}

auto renderFlatRect(RectF const& rect, Vec4B color, float param1) -> RenderQuad {
  return {rect, color, param1};
}

auto renderFlatPoly(PolyF const& poly, Vec4B color, float param1) -> RenderPoly {
  return {poly.vertexes(), color, param1};
}

}// namespace Star
