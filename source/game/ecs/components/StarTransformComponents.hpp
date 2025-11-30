#pragma once

// Transform-related ECS components for OpenStarbound
// These components handle position, velocity, and bounds

#include "StarVector.hpp"
#include "StarRect.hpp"

namespace Star {
namespace ECS {

//=============================================================================
// Transform Components
//=============================================================================

// Basic position and rotation
struct TransformComponent {
  Vec2F position = {};
  float rotation = 0.0f;
  Vec2F scale = {1.0f, 1.0f};
  
  // Convenience methods
  void setPosition(Vec2F const& pos) { position = pos; }
  void move(Vec2F const& delta) { position += delta; }
  
  // Get position with rotation applied
  Vec2F transformPoint(Vec2F const& point) const {
    if (rotation == 0.0f && scale == Vec2F{1.0f, 1.0f}) {
      return point + position;
    }
    // Apply scale
    Vec2F scaled = {point[0] * scale[0], point[1] * scale[1]};
    // Apply rotation
    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);
    Vec2F rotated = {
      scaled[0] * cosR - scaled[1] * sinR,
      scaled[0] * sinR + scaled[1] * cosR
    };
    return rotated + position;
  }
};

// Velocity for moving entities
struct VelocityComponent {
  Vec2F velocity = {};
  Vec2F acceleration = {};
  
  void setVelocity(Vec2F const& vel) { velocity = vel; }
  void addVelocity(Vec2F const& delta) { velocity += delta; }
  void setAcceleration(Vec2F const& acc) { acceleration = acc; }
  
  float speed() const { return vmag(velocity); }
  Vec2F direction() const { return vnorm(velocity); }
};

// Bounding box for spatial queries
struct BoundsComponent {
  RectF metaBoundBox = {};
  RectF collisionArea = {};
  
  RectF worldBounds(Vec2F const& position) const {
    return metaBoundBox.translated(position);
  }
  
  RectF worldCollisionArea(Vec2F const& position) const {
    return collisionArea.translated(position);
  }
  
  bool overlaps(BoundsComponent const& other, Vec2F const& thisPos, Vec2F const& otherPos) const {
    return worldBounds(thisPos).intersects(other.worldBounds(otherPos));
  }
};

// Anchor component for entities attached to other entities
struct AnchorComponent {
  Entity anchoredTo = 0; // NullEntity
  Vec2F anchorOffset = {};
  bool inheritRotation = true;
  bool inheritVelocity = false;
};

} // namespace ECS
} // namespace Star
