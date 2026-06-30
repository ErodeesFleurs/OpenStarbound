#pragma once

#include "StarEntity.hpp"
#include "StarParticle.hpp"
#include "StarStatusTypes.hpp"
#include "StarInteractionTypes.hpp"
#include "StarActorEntity.hpp"
#include "StarToolHandInterface.hpp"

namespace Star {

class Item;
using ItemPtr = SharedPtr<Item>;
class ToolUserEntity;

// FIXME: This interface is a complete mess.
// Tool hand methods extracted to ToolHandInterface.
class ToolUserEntity : public virtual ActorEntity, public virtual ToolHandInterface {
public:
  // Arm/hand methods inherited from ToolHandInterface.

  [[nodiscard]] virtual bool isAdmin() const = 0;
  [[nodiscard]] virtual Color favoriteColor() const = 0;
  [[nodiscard]] virtual String species() const = 0;

  virtual void requestEmote(String const& emote) = 0;

  // FIXME: This was used for an Item to get an ItemPtr to itself, which was
  // super bad and weird, but it COULD be used to get the item in the owner's
  // other hand, which is LESS bad.
  [[nodiscard]] virtual ItemPtr handItem(ToolHand hand) const = 0;

  // FIXME: What is the difference between interactRadius (which defines a tool
  // range) and inToolRange (which also defines a tool range indirectly).
  // inToolRange() implements based on the center of the tile of the aim
  // position (NOT the aim position!) but inToolRange(Vec2F) uses the given
  // position, which is again redundant.  Also, what is beamGunRadius and why
  // is it different than interact radius?  Can different tools have a
  // different interact radius?
  [[nodiscard]] virtual float interactRadius() const = 0;
  [[nodiscard]] virtual bool inToolRange() const = 0;
  [[nodiscard]] virtual bool inToolRange(Vec2F const& position) const = 0;
  [[nodiscard]] virtual float beamGunRadius() const = 0;

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
  [[nodiscard]] virtual bool instrumentPlaying() = 0;
  virtual void instrumentEquipped(String const& instrumentKind) = 0;

  // FIXME: These were all fine, just need to be fixed because now we have the
  // movement controller itself and can use that directly
  [[nodiscard]] virtual Vec2F position() const = 0;
  [[nodiscard]] virtual Vec2F velocity() const = 0;
  [[nodiscard]] virtual Direction facingDirection() const = 0;
  [[nodiscard]] virtual Direction walkingDirection() const = 0;

  // FIXME: Ditto here, except we now have the status controller directly.
  [[nodiscard]] virtual float powerMultiplier() const = 0;
  [[nodiscard]] virtual bool fullEnergy() const = 0;
  [[nodiscard]] virtual float energy() const = 0;
  [[nodiscard]] virtual bool consumeEnergy(float energy) = 0;
  [[nodiscard]] virtual bool energyLocked() const = 0;
  virtual void addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) = 0;
  [[nodiscard]] virtual ActiveUniqueStatusEffectSummary activeUniqueStatusEffectSummary() const = 0;

  // FIXME: This is a dumb way of getting limited animation support
  virtual void addEffectEmitters(StringSet const& emitters) = 0;
  virtual void addParticles(List<Particle> const& particles) = 0;
  virtual void addSound(String const& sound, float volume = 1.0f, float pitch = 1.0f) = 0;

  virtual void setCameraFocusEntity(Maybe<EntityId> const& cameraFocusEntity) = 0;
};

}
