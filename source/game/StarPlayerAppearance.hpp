#pragma once

#include "StarEntityRendering.hpp"
#include "StarHumanoid.hpp"
#include "StarNetElementSystem.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarPlayerTypes.hpp"

namespace Star {

class Player;
struct PlayerConfig;
using PlayerConfigPtr = SharedPtr<PlayerConfig>;
class ArmorWearer;
using ArmorWearerPtr = SharedPtr<ArmorWearer>;
class ActorMovementController;
using ActorMovementControllerPtr = SharedPtr<ActorMovementController>;
class StatusController;
using StatusControllerPtr = SharedPtr<StatusController>;
class World;

class PlayerAppearance {
public:
  explicit PlayerAppearance(Player& player);
  void init();

  [[nodiscard]] String name() const;
  void setName(String const& name);
  [[nodiscard]] String species() const;
  void setSpecies(String const& species);
  [[nodiscard]] Gender gender() const;
  void setGender(Gender const& gender);
  void setPersonality(Personality const& personality);
  void setImagePath(Maybe<String> const& imagePath);
  [[nodiscard]] HumanoidIdentity const& identity() const;
  void setIdentity(HumanoidIdentity identity);
  void updateIdentity();
  [[nodiscard]] HumanoidPtr humanoid();
  [[nodiscard]] HumanoidPtr humanoid() const;

  void setHumanoidParameter(String key, Maybe<Json> value);
  [[nodiscard]] Maybe<Json> getHumanoidParameter(String key);
  void setHumanoidParameters(JsonObject parameters);
  [[nodiscard]] JsonObject getHumanoidParameters();
  void refreshHumanoidParameters();

  void setBodyDirectives(String const& directives);
  void setEmoteDirectives(String const& directives);
  void setHairGroup(String const& group);
  void setHairType(String const& type);
  void setHairDirectives(String const& directives);
  void setFacialHairGroup(String const& group);
  void setFacialHairType(String const& type);
  void setFacialHairDirectives(String const& directives);
  void setFacialMaskGroup(String const& group);
  void setFacialMaskType(String const& type);
  void setFacialMaskDirectives(String const& directives);
  void setHair(String const& group, String const& type, String const& directives);
  void setFacialHair(String const& group, String const& type, String const& directives);
  void setFacialMask(String const& group, String const& type, String const& directives);

  [[nodiscard]] bool displayNametag() const;
  [[nodiscard]] Vec3B nametagColor() const;
  [[nodiscard]] Vec2F nametagOrigin() const;
  [[nodiscard]] String nametag() const;
  void setNametag(Maybe<String> nametag);
  [[nodiscard]] Maybe<String> statusText() const;

  [[nodiscard]] List<Drawable> portrait(PortraitMode mode) const;
  [[nodiscard]] bool underwater() const;
  void animatePortrait(float dt);

  [[nodiscard]] Color favoriteColor() const;
  void setFavoriteColor(Color color);

  void setAnimationParameter(String name, Json value);

  [[nodiscard]] Maybe<String> inspectionLogName() const;
  [[nodiscard]] Maybe<String> inspectionDescription(String const& species) const;

  [[nodiscard]] NetElementDynamicGroup<NetHumanoid>& netHumanoid();
  [[nodiscard]] NetElementData<Maybe<String>>& deathParticleBurst();
  [[nodiscard]] NetElementHashMap<String, Json>& scriptedAnimationParameters();
  [[nodiscard]] NetElementEvent& refreshedHumanoidParameters();
  NetworkedAnimator::DynamicTarget& humanoidDynamicTarget();
  [[nodiscard]] NetElementData<Maybe<String>>& humanoidDanceNetState();
  [[nodiscard]] NetElementData<HumanoidIdentity>& identityNetState();

  [[nodiscard]] bool& identityUpdated();

  HumanoidIdentity m_identity;
  JsonObject m_humanoidParameters;

private:
  Player& m_player;

  bool m_identityUpdated = true;

  NetElementDynamicGroup<NetHumanoid> m_netHumanoid;
  NetElementData<Maybe<String>> m_deathParticleBurst;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
  NetElementEvent m_refreshedHumanoidParameters;
  NetworkedAnimator::DynamicTarget m_humanoidDynamicTarget;
  NetElementData<Maybe<String>> m_humanoidDanceNetState;
  NetElementData<HumanoidIdentity> m_identityNetState;
};

}
