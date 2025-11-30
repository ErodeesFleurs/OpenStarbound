#pragma once

#include "ecs/StarEcs.hpp"
#include "StarGameComponents.hpp"
#include "StarDrawable.hpp"

namespace Star {
namespace ECS {

// Sprite component for simple sprite rendering
struct SpriteComponent {
  String imagePath = "";
  Directives directives = {};
  float animationTimer = 0.0f;
  int currentFrame = 0;
  bool visible = true;
  float zLevel = 0.0f;
  bool fullbright = false;
};

// Animation component for animated entities
struct AnimationComponent {
  String animationState = "idle";
  float animationTime = 0.0f;
  bool animationLooping = true;
  float animationSpeed = 1.0f;
};

// Render system - prepares render data for all visible entities
// Priority: 10 (runs late, after game logic)
class RenderSystem : public System {
public:
  RenderSystem();
  
  void update(float dt) override;
  int priority() const override { return 10; }
  
  // Get the collected drawables for rendering
  List<Drawable> const& drawables() const { return m_drawables; }
  
  // Get light sources for rendering
  List<LightSource> const& lightSources() const { return m_lightSources; }
  
  // Clear render data (call before next frame)
  void clearRenderData();

private:
  void collectSprites();
  void collectLightSources();
  void updateAnimations(float dt);
  
  Drawable createDrawable(TransformComponent const& transform, SpriteComponent const& sprite) const;
  
  List<Drawable> m_drawables;
  List<LightSource> m_lightSources;
};

} // namespace ECS
} // namespace Star
