#pragma once

// Network-related ECS components for OpenStarbound
// These components handle network synchronization and interpolation

#include "StarVector.hpp"
#include "StarByteArray.hpp"

namespace Star {
namespace ECS {

//=============================================================================
// Network Components
//=============================================================================

// Network synchronization state
struct NetworkSyncComponent {
  uint64_t netVersion = 0;
  bool isDirty = false;
  bool isMaster = false;
  ConnectionId ownerConnection = 0;
  
  void markDirty() {
    isDirty = true;
  }
  
  void incrementVersion() {
    netVersion++;
    isDirty = false;
  }
};

// Network entity identity
struct NetworkIdentityComponent {
  EntityId networkId = 0;
  Maybe<String> uniqueId = {};
  EntityType entityType = EntityType::Object;
  
  bool hasUniqueId() const {
    return uniqueId.isValid();
  }
};

// Interpolation for smooth network updates
struct InterpolationComponent {
  bool enabled = true;
  float interpolationTime = 0.0f;
  float extrapolationHint = 0.0f;
  
  // Position interpolation
  Vec2F previousPosition = {};
  Vec2F targetPosition = {};
  float positionBlend = 1.0f;
  
  // Rotation interpolation
  float previousRotation = 0.0f;
  float targetRotation = 0.0f;
  
  Vec2F interpolatedPosition() const {
    return previousPosition + (targetPosition - previousPosition) * positionBlend;
  }
  
  float interpolatedRotation() const {
    // Handle angle wrapping for smooth rotation
    float diff = targetRotation - previousRotation;
    while (diff > M_PI) diff -= 2 * M_PI;
    while (diff < -M_PI) diff += 2 * M_PI;
    return previousRotation + diff * positionBlend;
  }
  
  void setTarget(Vec2F position, float rotation = 0.0f) {
    previousPosition = interpolatedPosition();
    previousRotation = interpolatedRotation();
    targetPosition = position;
    targetRotation = rotation;
    positionBlend = 0.0f;
  }
  
  void update(float dt, float interpolationRate = 10.0f) {
    positionBlend = std::min(positionBlend + dt * interpolationRate, 1.0f);
  }
};

// Client entity mode
struct ClientEntityModeComponent {
  ClientEntityMode mode = ClientEntityMode::ClientSlaveOnly;
  bool presenceMaster = false;
  
  bool isSlave() const {
    return mode == ClientEntityMode::ClientSlaveOnly;
  }
  
  bool canBeMaster() const {
    return mode == ClientEntityMode::ClientMasterAllowed || 
           mode == ClientEntityMode::ClientPresenceMaster;
  }
};

// Network message queue
struct NetworkMessageComponent {
  struct Message {
    ConnectionId sender = 0;
    String message;
    JsonArray args;
  };
  
  List<Message> pendingMessages = {};
  List<Message> outgoingMessages = {};
  
  void queueMessage(ConnectionId sender, String msg, JsonArray args = {}) {
    pendingMessages.append({sender, std::move(msg), std::move(args)});
  }
  
  void sendMessage(String msg, JsonArray args = {}) {
    outgoingMessages.append({0, std::move(msg), std::move(args)});
  }
  
  List<Message> pullIncoming() {
    List<Message> result;
    std::swap(result, pendingMessages);
    return result;
  }
  
  List<Message> pullOutgoing() {
    List<Message> result;
    std::swap(result, outgoingMessages);
    return result;
  }
};

// Entity replication settings
struct ReplicationComponent {
  float updateInterval = 0.05f; // 20 Hz default
  float updateTimer = 0.0f;
  bool forceUpdate = false;
  bool positionDirty = false;
  bool stateDirty = false;
  
  bool needsUpdate() {
    if (forceUpdate || updateTimer >= updateInterval) {
      updateTimer = 0.0f;
      forceUpdate = false;
      return true;
    }
    return false;
  }
  
  void tick(float dt) {
    updateTimer += dt;
  }
  
  void markPositionDirty() {
    positionDirty = true;
  }
  
  void markStateDirty() {
    stateDirty = true;
  }
};

} // namespace ECS
} // namespace Star
