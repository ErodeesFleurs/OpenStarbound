#include "StarRenderSystem.hpp"

namespace Star {
namespace ECS {

RenderSystem::RenderSystem() = default;

void RenderSystem::update(float dt) {
  // Clear previous frame's render data
  clearRenderData();
  
  // Update animations
  updateAnimations(dt);
  
  // Collect all visible sprites
  collectSprites();
  
  // Collect all light sources
  collectLightSources();
}

void RenderSystem::clearRenderData() {
  m_drawables.clear();
  m_lightSources.clear();
}

void RenderSystem::updateAnimations(float dt) {
  for (auto [entity, animation] : m_world->view<AnimationComponent>()) {
    animation.animationTime += dt * animation.animationSpeed;
    
    // Handle looping animations
    // The actual animation frame calculation would depend on
    // the animation data from assets
  }
  
  // Update sprite animation frames
  for (auto [entity, sprite] : m_world->view<SpriteComponent>()) {
    sprite.animationTimer += dt;
    // Frame advancement would be based on animation data
  }
}

void RenderSystem::collectSprites() {
  for (auto [entity, transform, sprite] : 
       m_world->view<TransformComponent, SpriteComponent>()) {
    
    // Skip invisible sprites
    if (!sprite.visible || sprite.imagePath.empty()) {
      continue;
    }
    
    Drawable drawable = createDrawable(transform, sprite);
    m_drawables.append(std::move(drawable));
  }
  
  // Sort by z-level for proper rendering order
  std::stable_sort(m_drawables.begin(), m_drawables.end(),
    [](Drawable const& a, Drawable const& b) {
      return a.position[1] < b.position[1]; // Sort by Y for depth
    });
}

void RenderSystem::collectLightSources() {
  for (auto [entity, transform, lights] : 
       m_world->view<TransformComponent, LightSourceComponent>()) {
    
    for (auto const& light : lights.sources) {
      // Create a copy of the light with world position
      LightSource worldLight = light;
      worldLight.position += transform.position;
      m_lightSources.append(std::move(worldLight));
    }
  }
}

Drawable RenderSystem::createDrawable(TransformComponent const& transform, SpriteComponent const& sprite) const {
  Drawable drawable = Drawable::makeImage(sprite.imagePath, 1.0f, true, transform.position);
  
  // Apply directives if any
  if (!sprite.directives.empty()) {
    drawable.imagePart().addDirectives(sprite.directives, true);
  }
  
  // Apply rotation
  if (transform.rotation != 0.0f) {
    drawable.rotate(transform.rotation);
  }
  
  // Apply scale
  if (transform.scale[0] != 1.0f || transform.scale[1] != 1.0f) {
    drawable.scale(transform.scale);
  }
  
  // Set fullbright
  drawable.fullbright = sprite.fullbright;
  
  return drawable;
}

} // namespace ECS
} // namespace Star
