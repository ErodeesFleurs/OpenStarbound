#include "StarPlayerAppearance.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerChatAndEmotes.hpp"
#include "StarPlayerFactory.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarArmorWearer.hpp"
#include "StarActorMovementController.hpp"
#include "StarStatusController.hpp"
#include "StarWorld.hpp"
#include "StarNetworkedAnimatorLuaBindings.hpp"
#include "StarScriptedAnimatorLuaBindings.hpp"
#include "StarEntityLuaBindings.hpp"
#include "StarEncode.hpp"
#include "StarXXHash.hpp"
#include "StarInspectionTool.hpp"

namespace Star {

PlayerAppearance::PlayerAppearance(Player* player)
  : m_player(player), m_identityUpdated(true) {}

void PlayerAppearance::init() {
  m_identity = m_player->m_config->defaultIdentity;
  m_identityUpdated = true;
  m_netHumanoid.addNetElement(make_shared<NetHumanoid>(m_identity, m_humanoidParameters, Json()));
}

String PlayerAppearance::name() const {
  return m_identity.name;
}

void PlayerAppearance::setName(String const& name) {
  m_identity.name = name;
  updateIdentity();
}

String PlayerAppearance::species() const {
  return m_identity.species;
}

void PlayerAppearance::setSpecies(String const& species) {
  m_identity.species = species;
  updateIdentity();
}

Gender PlayerAppearance::gender() const {
  return m_identity.gender;
}

void PlayerAppearance::setGender(Gender const& gender) {
  m_identity.gender = gender;
  updateIdentity();
}

void PlayerAppearance::setPersonality(Personality const& personality) {
  m_identity.personality = personality;
  updateIdentity();
}

void PlayerAppearance::setImagePath(Maybe<String> const& imagePath) {
  m_identity.imagePath = imagePath;
  updateIdentity();
}

HumanoidPtr PlayerAppearance::humanoid() {
  return m_netHumanoid.netElements().last()->humanoid();
}
HumanoidPtr PlayerAppearance::humanoid() const {
  return m_netHumanoid.netElements().last()->humanoid();
}

HumanoidIdentity const& PlayerAppearance::identity() const {
  return m_identity;
}

void PlayerAppearance::setIdentity(HumanoidIdentity identity) {
  m_identity = std::move(identity);
  updateIdentity();
}

void PlayerAppearance::updateIdentity() {
  m_identityUpdated = true;
  auto oldIdentity = humanoid()->identity();
  if ((m_identity.species != oldIdentity.species) || (m_identity.imagePath != oldIdentity.imagePath)) {
    refreshHumanoidParameters();
  } else {
    humanoid()->setIdentity(m_identity);
  }
}

void PlayerAppearance::setHumanoidParameter(String key, Maybe<Json> value) {
  if (value.isValid())
    m_humanoidParameters.set(key, value.value());
  else
    m_humanoidParameters.erase(key);

  m_netHumanoid.netElements().last()->setHumanoidParameters(m_humanoidParameters);
}

Maybe<Json> PlayerAppearance::getHumanoidParameter(String key) {
  return m_humanoidParameters.maybe(key);
}

void PlayerAppearance::setHumanoidParameters(JsonObject parameters) {
  m_humanoidParameters = parameters;
  m_netHumanoid.netElements().last()->setHumanoidParameters(m_humanoidParameters);
}

JsonObject PlayerAppearance::getHumanoidParameters() {
  return m_humanoidParameters;
}

void PlayerAppearance::refreshHumanoidParameters() {
  auto speciesDatabase = Root::singleton().speciesDatabase();
  auto speciesDef = speciesDatabase->species(m_identity.species);

  if (m_player->isMaster() || !m_player->inWorld()) {
    m_refreshedHumanoidParameters.trigger();
    m_netHumanoid.clearNetElements();
    m_netHumanoid.addNetElement(make_shared<NetHumanoid>(m_identity, m_humanoidParameters, Json()));
    m_player->m_effectsAnimator->setGlobalTag("effectDirectives", speciesDef->effectDirectives());
    m_deathParticleBurst.set(humanoid()->defaultDeathParticles());
    m_player->m_statusController->setStatusProperty("ouchNoise", speciesDef->ouchNoise(m_identity.gender));
    m_scriptedAnimationParameters.clear();
  } else {
    m_humanoidParameters = m_netHumanoid.netElements().last()->humanoidParameters();
  }
  auto armor = m_player->m_armor->diskStore();
  m_player->m_armor->reset();
  m_player->m_armor->diskLoad(armor);
  m_player->m_armor->setupHumanoid(*humanoid(), m_player->forceNude());

  m_player->m_movementController->resetBaseParameters(ActorMovementParameters(jsonMerge(humanoid()->defaultMovementParameters(), humanoid()->playerMovementParameters().value(m_player->m_config->movementParameters))));

  if (m_player->inWorld()) {
    if (m_player->isMaster()) {
      for (auto& p : m_player->m_genericScriptContexts) {
        if (p.second->initialized()) {
          p.second->removeCallbacks("animator");
          p.second->addCallbacks("animator", LuaBindings::makeNetworkedAnimatorCallbacks(humanoid()->networkedAnimator()));
          p.second->invoke("refreshHumanoidParameters");
        }
      }
    }
    if (m_player->world()->isClient() && m_player->m_scriptedAnimator.initialized()) {
      m_player->m_scriptedAnimator.uninit();
      m_player->m_scriptedAnimator.removeCallbacks("animationConfig");
      m_player->m_scriptedAnimator.removeCallbacks("entity");

      m_player->m_scriptedAnimator.setScripts(humanoid()->animationScripts());
      m_player->m_scriptedAnimator.addCallbacks("animationConfig", LuaBindings::makeScriptedAnimatorCallbacks(humanoid()->networkedAnimator(),
        [this](String const& name, Json const& defaultValue) -> Json {
          return m_scriptedAnimationParameters.value(name, defaultValue);
        }));
      m_player->m_scriptedAnimator.addCallbacks("entity", LuaBindings::makeEntityCallbacks(m_player));
      m_player->m_scriptedAnimator.init(m_player->world());
    }
  }
}

void PlayerAppearance::setBodyDirectives(String const& directives) { m_identity.bodyDirectives = directives; updateIdentity(); }
void PlayerAppearance::setEmoteDirectives(String const& directives) { m_identity.emoteDirectives = directives; updateIdentity(); }

void PlayerAppearance::setHairGroup(String const& group) { m_identity.hairGroup = group; updateIdentity(); }
void PlayerAppearance::setHairType(String const& type) { m_identity.hairType = type; updateIdentity(); }
void PlayerAppearance::setHairDirectives(String const& directives) { m_identity.hairDirectives = directives; updateIdentity(); }

void PlayerAppearance::setFacialHairGroup(String const& group) { m_identity.facialHairGroup = group; updateIdentity(); }
void PlayerAppearance::setFacialHairType(String const& type) { m_identity.facialHairType = type; updateIdentity(); }
void PlayerAppearance::setFacialHairDirectives(String const& directives) { m_identity.facialHairDirectives = directives; updateIdentity(); }

void PlayerAppearance::setFacialMaskGroup(String const& group) { m_identity.facialMaskGroup = group; updateIdentity(); }
void PlayerAppearance::setFacialMaskType(String const& type) { m_identity.facialMaskType = type; updateIdentity(); }
void PlayerAppearance::setFacialMaskDirectives(String const& directives) { m_identity.facialMaskDirectives = directives; updateIdentity(); }

void PlayerAppearance::setHair(String const& group, String const& type, String const& directives) {
  setHairGroup(group); setHairType(type); setHairDirectives(directives);
}

void PlayerAppearance::setFacialHair(String const& group, String const& type, String const& directives) {
  setFacialHairGroup(group); setFacialHairType(type); setFacialHairDirectives(directives);
}

void PlayerAppearance::setFacialMask(String const& group, String const& type, String const& directives) {
  setFacialMaskGroup(group); setFacialMaskType(type); setFacialMaskDirectives(directives);
}

bool PlayerAppearance::displayNametag() const {
  return true;
}

Vec3B PlayerAppearance::nametagColor() const {
  auto assets = Root::singleton().assets();
  return jsonToVec3B(assets->json("/player.config:nametagColor"));
}

Vec2F PlayerAppearance::nametagOrigin() const {
  return m_player->mouthPosition(false);
}

String PlayerAppearance::nametag() const {
  if (auto jNametag = m_player->getSecretProperty("nametag"); jNametag.isType(Json::Type::String))
    return jNametag.toString();
  else
    return name();
}

void PlayerAppearance::setNametag(Maybe<String> nametag) {
  m_player->setSecretProperty("nametag", nametag ? Json(*nametag) : Json());
}

Maybe<String> PlayerAppearance::statusText() const {
  return {};
}

List<Drawable> PlayerAppearance::portrait(PortraitMode mode) const {
  if (m_player->isPermaDead())
    return humanoid()->renderSkull();
  if (m_player->invisible())
    return {};
  if (!m_player->inWorld())
    const_cast<PlayerAppearance*>(this)->refreshHumanoidParameters();
  return humanoid()->renderPortrait(mode);
}

bool PlayerAppearance::underwater() const {
  if (!m_player->inWorld())
    return false;

  auto level = m_player->world()->liquidLevel(Vec2I(m_player->position() + m_player->m_config->underwaterSensor)).level;
  return level > 0.0f && (level > m_player->m_config->underwaterMinWaterLevel || !m_player->m_config->underwaterMinWaterLevel);
}

void PlayerAppearance::animatePortrait(float dt) {
  humanoid()->animate(dt, {});
  humanoid()->setEmoteState(m_player->m_chatAndEmotes->emoteState());
}

Color PlayerAppearance::favoriteColor() const {
  return Color::rgba(m_identity.color);
}

void PlayerAppearance::setFavoriteColor(Color color) {
  m_identity.color = color.toRgba();
  updateIdentity();
}

void PlayerAppearance::setAnimationParameter(String name, Json value) {
  m_scriptedAnimationParameters.set(std::move(name), std::move(value));
}

Maybe<String> PlayerAppearance::inspectionLogName() const {
  auto identifier = m_player->uniqueId();
  if (String* str = identifier.ptr()) {
    auto hash = XXH3_128bits(str->utf8Ptr(), str->utf8Size());
    return String("Player #") + hexEncode((const char*)&hash, sizeof(hash));
  }
  return identifier;
}

Maybe<String> PlayerAppearance::inspectionDescription(String const&) const {
  return m_player->m_description;
}

NetElementDynamicGroup<NetHumanoid>& PlayerAppearance::netHumanoid() {
  return m_netHumanoid;
}

NetElementData<Maybe<String>>& PlayerAppearance::deathParticleBurst() {
  return m_deathParticleBurst;
}

NetElementHashMap<String, Json>& PlayerAppearance::scriptedAnimationParameters() {
  return m_scriptedAnimationParameters;
}

NetElementEvent& PlayerAppearance::refreshedHumanoidParameters() {
  return m_refreshedHumanoidParameters;
}

NetworkedAnimator::DynamicTarget& PlayerAppearance::humanoidDynamicTarget() {
  return m_humanoidDynamicTarget;
}

NetElementData<Maybe<String>>& PlayerAppearance::humanoidDanceNetState() {
  return m_humanoidDanceNetState;
}

NetElementData<HumanoidIdentity>& PlayerAppearance::identityNetState() {
  return m_identityNetState;
}

bool& PlayerAppearance::identityUpdated() {
  return m_identityUpdated;
}

}

