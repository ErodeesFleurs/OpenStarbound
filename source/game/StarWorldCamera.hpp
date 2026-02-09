#pragma once

#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"
#include "StarWorldGeometry.hpp"

import std;

namespace Star {

class WorldCamera {
public:
  void setScreenSize(Vec2U screenSize);
  [[nodiscard]] auto screenSize() const -> Vec2U;

  void setTargetPixelRatio(float targetPixelRatio);
  void setPixelRatio(float pixelRatio);
  [[nodiscard]] auto pixelRatio() const -> float;

  void setWorldGeometry(WorldGeometry geometry);
  [[nodiscard]] auto worldGeometry() const -> WorldGeometry;

  // Set the camera center position (in world space) to as close to the given
  // location as possible while keeping the screen within world bounds.
  void setCenterWorldPosition(Vec2F position, bool force = false);
  // Returns the actual camera position.
  [[nodiscard]] auto centerWorldPosition() const -> Vec2F;

  // Transforms world coordinates into one set of screen coordinates.  Since
  // the world is non-euclidean, one world coordinate can transform to
  // potentially an infinite number of screen coordinates.  This will retrun
  // the closest to the center of the screen.
  [[nodiscard]] auto worldToScreen(Vec2F worldCoord) const -> Vec2F;

  // Assumes top left corner of screen is (0, 0) in screen coordinates.
  [[nodiscard]] auto screenToWorld(Vec2F screen) const -> Vec2F;

  // Returns screen dimensions in world space.
  [[nodiscard]] auto worldScreenRect() const -> RectF;

  // Returns tile dimensions of the tiles that overlap with the screen
  [[nodiscard]] auto worldTileRect() const -> RectI;

  // Returns the position of the lower left corner of the lower left tile of
  // worldTileRect, in screen coordinates.
  [[nodiscard]] auto tileMinScreen() const -> Vec2F;

  void update(float dt);

private:
  WorldGeometry m_worldGeometry;
  Vec2U m_screenSize;
  float m_pixelRatio = 1.0f;
  float m_targetPixelRatio = 1.0f;
  Vec2F m_worldCenter;
  Vec2F m_rawWorldCenter;
};

inline void WorldCamera::setScreenSize(Vec2U screenSize) {
  m_screenSize = screenSize;
}

inline auto WorldCamera::screenSize() const -> Vec2U {
  return m_screenSize;
}

inline void WorldCamera::setTargetPixelRatio(float targetPixelRatio) {
  m_targetPixelRatio = targetPixelRatio;
}

inline void WorldCamera::setPixelRatio(float pixelRatio) {
  m_pixelRatio = m_targetPixelRatio = pixelRatio;
}

inline auto WorldCamera::pixelRatio() const -> float {
  return m_pixelRatio;
}

inline void WorldCamera::setWorldGeometry(WorldGeometry geometry) {
  m_worldGeometry = std::move(geometry);
}

inline auto WorldCamera::worldGeometry() const -> WorldGeometry {
  return m_worldGeometry;
}

inline auto WorldCamera::centerWorldPosition() const -> Vec2F {
  return {m_worldCenter};
}

inline auto WorldCamera::worldToScreen(Vec2F worldCoord) const -> Vec2F {
  Vec2F wrappedCoord = m_worldGeometry.nearestTo(Vec2F(m_worldCenter), worldCoord);
  return {
    static_cast<float>((wrappedCoord[0] - m_worldCenter[0]) * (TilePixels * m_pixelRatio) + (float)m_screenSize[0] / 2.0),
    (wrappedCoord[1] - m_worldCenter[1]) * (TilePixels * m_pixelRatio) + (float)m_screenSize[1] / 2.0};
}

inline auto WorldCamera::screenToWorld(Vec2F screen) const -> Vec2F {
  return {
    static_cast<float>(screen[0] - (float)m_screenSize[0] / 2.0) / (TilePixels * m_pixelRatio) + m_worldCenter[0],
    (screen[1] - (float)m_screenSize[1] / 2.0) / (TilePixels * m_pixelRatio) + m_worldCenter[1]};
}

inline auto WorldCamera::worldScreenRect() const -> RectF {
  // screen dimensions in world space
  float w = (float)m_screenSize[0] / (TilePixels * m_pixelRatio);
  float h = (float)m_screenSize[1] / (TilePixels * m_pixelRatio);
  return RectF::withSize(Vec2F(m_worldCenter[0] - w / 2, m_worldCenter[1] - h / 2), Vec2F(w, h));
}

inline auto WorldCamera::worldTileRect() const -> RectI {
  RectF screen = worldScreenRect();
  Vec2I min = Vec2I::floor(screen.min());
  Vec2I size = Vec2I::ceil(Vec2F(m_screenSize) / (TilePixels * m_pixelRatio) + (screen.min() - Vec2F(min)));
  return RectI::withSize(min, size);
}

inline auto WorldCamera::tileMinScreen() const -> Vec2F {
  RectF screenRect = worldScreenRect();
  RectI tileRect = worldTileRect();
  return (Vec2F(tileRect.min()) - screenRect.min()) * (TilePixels * m_pixelRatio);
}

inline void WorldCamera::update(float dt) {
  float newPixelRatio = lerp(std::exp(-20.0f * dt), m_targetPixelRatio, m_pixelRatio);
  if (std::abs(newPixelRatio - m_targetPixelRatio) < 0.0125f)
    newPixelRatio = m_targetPixelRatio;
  if (m_pixelRatio != newPixelRatio) {
    m_pixelRatio = newPixelRatio;
    setCenterWorldPosition(m_rawWorldCenter, true);
  }
}

}// namespace Star
