#include "StarAiDatabase.hpp"
#include "StarLexicalCast.hpp"
#include "StarRoot.hpp"

namespace Star {

AiDatabase::AiDatabase() {
  auto assets = Root::singleton().assets();
  auto config = assets->json("/ai/ai.config");

  auto& missions = assets->scanExtension("aimission");
  assets->queueJsons(missions);

  for (auto const& file : missions) {
    if (auto config = assets->json(file)) {
      auto mission = parseMission(config);
      m_missions[mission.missionName] = mission;
    }
  }

  for (auto const& [species, speciesConfig] : config.get("species").iterateObject())
    m_speciesParameters[species] = parseSpeciesParameters(speciesConfig);

  for (auto const& [statusKey, statusConfig] : config.get("shipStatus").iterateObject())
    m_shipStatus[lexicalCast<unsigned>(statusKey)] = parseSpeech(statusConfig);

  m_noMissionsSpeech = parseSpeech(config.get("noMissionsSpeech"));
  m_noCrewSpeech = parseSpeech(config.get("noCrewSpeech"));

  m_animationConfig.charactersPerSecond = config.getFloat("charactersPerSecond");
  m_animationConfig.defaultAnimation = config.getString("defaultAnimation");
  m_animationConfig.staticAnimation = Animation("/ai/ai.config:staticAnimation");
  m_animationConfig.staticOpacity = config.getFloat("staticOpacity");
  m_animationConfig.scanlineAnimation = Animation("/ai/ai.config:scanlineAnimation");
  m_animationConfig.scanlineOpacity = config.getFloat("scanlineOpacity");

  for (auto const& [name, animConfig] : config.get("aiAnimations").iterateObject())
    m_animationConfig.aiAnimations[name] = Animation(animConfig, "/ai/");
}

auto AiDatabase::mission(String const& missionName) const -> AiMission {
  return m_missions.get(missionName);
}

auto AiDatabase::shipStatus(unsigned shipLevel) const -> AiSpeech {
  // Find the first open speech set at this ship level or below.
  auto i = m_shipStatus.upper_bound(shipLevel);
  if (i != m_shipStatus.begin() && (--i)->first <= shipLevel)
    return i->second;

  return {};
}

auto AiDatabase::noCrewSpeech() const -> AiSpeech {
  return m_noCrewSpeech;
}

auto AiDatabase::noMissionsSpeech() const -> AiSpeech {
  return m_noMissionsSpeech;
}

auto AiDatabase::portraitImage(String const& species, String const& frame) const -> String {
  return strf("/ai/{}:{}", m_speciesParameters.get(species).portraitFrames, frame);
}

auto AiDatabase::animation(String const& species, String const& animationName) const -> Animation {
  auto faceAnimation = m_animationConfig.aiAnimations.get(animationName);
  faceAnimation.setTag("image", m_speciesParameters.get(species).aiFrames);
  return faceAnimation;
}

auto AiDatabase::staticAnimation(String const& species) const -> Animation {
  auto staticAnimation = m_animationConfig.staticAnimation;
  staticAnimation.setTag("image", m_speciesParameters.get(species).staticFrames);
  staticAnimation.setColor(Color::rgbaf(1.0f, 1.0f, 1.0f, m_animationConfig.staticOpacity));
  return staticAnimation;
}

auto AiDatabase::scanlineAnimation() const -> Animation {
  auto animation = m_animationConfig.scanlineAnimation;
  animation.setColor(Color::rgbaf(1.0f, 1.0f, 1.0f, m_animationConfig.scanlineOpacity));
  return animation;
}

auto AiDatabase::charactersPerSecond() const -> float {
  return m_animationConfig.charactersPerSecond;
}

auto AiDatabase::defaultAnimation() const -> String {
  return m_animationConfig.defaultAnimation;
}

auto AiDatabase::parseSpeech(Json const& v) -> AiSpeech {
  return AiSpeech{.animation = v.getString("animation"), .text = v.getString("text"), .speedModifier = v.getFloat("speedModifier", 1.0f)};
}

auto AiDatabase::parseSpeciesParameters(Json const& v) -> AiDatabase::AiSpeciesParameters {
  AiSpeciesParameters species;
  species.aiFrames = v.getString("aiFrames");
  species.portraitFrames = v.getString("portraitFrames");
  species.staticFrames = v.getString("staticFrames");
  return species;
}

auto AiDatabase::parseSpeciesMissionText(Json const& vm) -> AiSpeciesMissionText {
  return AiSpeciesMissionText{
    .buttonText = vm.getString("buttonText"),
    .repeatButtonText = vm.getString("repeatButtonText"),
    .selectSpeech = parseSpeech(vm.get("selectSpeech", {}))};
}

auto AiDatabase::parseMission(Json const& vm) -> AiMission {
  AiMission mission;
  mission.missionName = vm.getString("missionName");
  mission.missionUniqueWorld = vm.getString("missionWorld");
  mission.warpAnimation = vm.optString("warpAnimation");
  mission.warpDeploy = vm.optBool("warpDeploy");
  mission.icon = AssetPath::relativeTo("/ai/", vm.getString("icon"));
  for (auto const& [species, textConfig] : vm.get("speciesText").iterateObject())
    mission.speciesText[species] = parseSpeciesMissionText(textConfig);
  return mission;
}

}// namespace Star
