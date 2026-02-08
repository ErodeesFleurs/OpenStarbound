#pragma once

#include "StarActorEntity.hpp"
#include "StarConfig.hpp"
#include "StarInteractionTypes.hpp"
#include "StarParticle.hpp"
#include "StarStatusTypes.hpp"

namespace Star {

// FIXME: This interface is a complete mess.
class ToolUserEntity : public virtual ActorEntity {
public:
  // Translates the given arm position into it's final entity space position
  // based on the given facing direction, and arm angle, and an offset from the
  // rotation center of the arm.
  [[nodiscard]] virtual auto armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const -> Vec2F = 0;
  // The offset to give to armPosition to get the position of the hand.
  [[nodiscard]] virtual auto handOffset(ToolHand hand, Direction facingDirection) const -> Vec2F = 0;

  // Gets the world position of the current aim point.
  [[nodiscard]] virtual auto aimPosition() const -> Vec2F = 0;

  [[nodiscard]] virtual auto isAdmin() const -> bool = 0;
  [[nodiscard]] virtual auto favoriteColor() const -> Color = 0;
  [[nodiscard]] virtual auto species() const -> String = 0;

  virtual void requestEmote(String const& emote) = 0;

  // FIXME: This is effectively unusable, because since tool user items control
  // the angle and facing direction of the owner, and this uses the facing
  // direction and angle as input, the result will always be behind.
  [[nodiscard]] virtual auto handPosition(ToolHand hand, Vec2F const& handOffset = Vec2F()) const -> Vec2F = 0;

  // FIXME: This was used for an Item to get an ItemPtr to itself, which was
  // super bad and weird, but it COULD be used to get the item in the owner's
  // other hand, which is LESS bad.
  [[nodiscard]] virtual auto handItem(ToolHand hand) const -> Ptr<Item> = 0;

  // FIXME: What is the difference between interactRadius (which defines a tool
  // range) and inToolRange (which also defines a tool range indirectly).
  // inToolRange() implements based on the center of the tile of the aim
  // position (NOT the aim position!) but inToolRange(Vec2F) uses the given
  // position, which is again redundant.  Also, what is beamGunRadius and why
  // is it different than interact radius?  Can different tools have a
  // different interact radius?
  [[nodiscard]] virtual auto interactRadius() const -> float = 0;
  [[nodiscard]] virtual auto inToolRange() const -> bool = 0;
  [[nodiscard]] virtual auto inToolRange(Vec2F const& position) const -> bool = 0;
  [[nodiscard]] virtual auto beamGunRadius() const -> float = 0;

  // FIXME: Too specific to Player, just cast to Player if you have to and do
  // that, NPCs cannot possibly implement these properly (and do not implement
  // them at all).
  virtual void queueUIMessage(String const& message) = 0;
  virtual void interact(InteractAction const& action) = 0;

  // FIXME: Ditto here, instrumentPlaying() is just an accessor to the songbook
  // for when the songbook has had a song selected, and the instrument decides
  // when to cancel music anyway, also instrumentEquipped(String) is a straight
  // up ridiculous way of notifying the Player that the player itself is
  // holding an instrument, which it already knows.
  virtual auto instrumentPlaying() -> bool = 0;
  virtual void instrumentEquipped(String const& instrumentKind) = 0;

  // FIXME: how is this related to the hand position and isn't it already
  // included in the hand position and why is it necessary?
  [[nodiscard]] virtual auto armAdjustment() const -> Vec2F = 0;

  // FIXME: These were all fine, just need to be fixed because now we have the
  // movement controller itself and can use that directly
  [[nodiscard]] auto position() const -> Vec2F override = 0;
  [[nodiscard]] virtual auto velocity() const -> Vec2F = 0;
  [[nodiscard]] virtual auto facingDirection() const -> Direction = 0;
  [[nodiscard]] virtual auto walkingDirection() const -> Direction = 0;

  // FIXME: Ditto here, except we now have the status controller directly.
  [[nodiscard]] virtual auto powerMultiplier() const -> float = 0;
  [[nodiscard]] virtual auto fullEnergy() const -> bool = 0;
  [[nodiscard]] virtual auto energy() const -> float = 0;
  virtual auto consumeEnergy(float energy) -> bool = 0;
  [[nodiscard]] virtual auto energyLocked() const -> bool = 0;
  virtual void addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) = 0;
  [[nodiscard]] virtual auto activeUniqueStatusEffectSummary() const -> ActiveUniqueStatusEffectSummary = 0;

  // FIXME: This is a dumb way of getting limited animation support
  virtual void addEffectEmitters(StringSet const& emitters) = 0;
  virtual void addParticles(List<Particle> const& particles) = 0;
  virtual void addSound(String const& sound, float volume = 1.0f, float pitch = 1.0f) = 0;

  virtual void setCameraFocusEntity(std::optional<EntityId> const& cameraFocusEntity) = 0;
};

}// namespace Star
