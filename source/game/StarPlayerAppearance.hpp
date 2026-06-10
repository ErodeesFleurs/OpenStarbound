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
  explicit PlayerAppearance(Player* player);
  void init();

  String name() const;
  void setName(String const& name);
  String species() const;
  void setSpecies(String const& species);
  Gender gender() const;
  void setGender(Gender const& gender);
  void setPersonality(Personality const& personality);
  void setImagePath(Maybe<String> const& imagePath);
  HumanoidIdentity const& identity() const;
  void setIdentity(HumanoidIdentity identity);
  void updateIdentity();
  HumanoidPtr humanoid();
  HumanoidPtr humanoid() const;

  void setHumanoidParameter(String key, Maybe<Json> value);
  Maybe<Json> getHumanoidParameter(String key);
  void setHumanoidParameters(JsonObject parameters);
  JsonObject getHumanoidParameters();
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

  bool displayNametag() const;
  Vec3B nametagColor() const;
  Vec2F nametagOrigin() const;
  String nametag() const;
  void setNametag(Maybe<String> nametag);
  Maybe<String> statusText() const;

  List<Drawable> portrait(PortraitMode mode) const;
  bool underwater() const;
  void animatePortrait(float dt);

  Color favoriteColor() const;
  void setFavoriteColor(Color color);

  void setAnimationParameter(String name, Json value);

  Maybe<String> inspectionLogName() const;
  Maybe<String> inspectionDescription(String const& species) const;

  NetElementDynamicGroup<NetHumanoid>& netHumanoid();
  NetElementData<Maybe<String>>& deathParticleBurst();
  NetElementHashMap<String, Json>& scriptedAnimationParameters();
  NetElementEvent& refreshedHumanoidParameters();
  NetworkedAnimator::DynamicTarget& humanoidDynamicTarget();
  NetElementData<Maybe<String>>& humanoidDanceNetState();
  NetElementData<HumanoidIdentity>& identityNetState();

  bool& identityUpdated();

  HumanoidIdentity m_identity;
  JsonObject m_humanoidParameters;

private:
  Player* m_player;

  bool m_identityUpdated;

  NetElementDynamicGroup<NetHumanoid> m_netHumanoid;
  NetElementData<Maybe<String>> m_deathParticleBurst;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
  NetElementEvent m_refreshedHumanoidParameters;
  NetworkedAnimator::DynamicTarget m_humanoidDynamicTarget;
  NetElementData<Maybe<String>> m_humanoidDanceNetState;
  NetElementData<HumanoidIdentity> m_identityNetState;
};

}

