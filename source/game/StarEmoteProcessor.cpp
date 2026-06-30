#include "StarEmoteProcessor.hpp"
#include "StarAlgorithm.hpp"
#include "StarJsonExtra.hpp"

namespace Star {

EmoteProcessor::EmoteProcessor(AssetsConstPtr assets) {
  assets = requireServiceValueAs<StarException>(std::move(assets), "EmoteProcessor", "assets");

  m_emoteBindings.clear();
  auto cfg = assets->json("/emotes.config");
  for (auto [emoteName, textOptions] : cfg.get("emoteBindings").iterateObject()) {
    for (auto text : textOptions.toArray()) {
      EmoteBinding emoteBinding;
      emoteBinding.emote = HumanoidEmoteNames.getLeft(emoteName);
      emoteBinding.text = text.toString();
      m_emoteBindings.append(emoteBinding);
    }
  }
}

HumanoidEmote EmoteProcessor::detectEmotes(String const& chatter) const {
  auto isShouty = [](String const& text) -> bool {
    int caps = 0;
    int noCaps = 0;
    for (auto c : text) {
      if (String::toUpper(c) != String::toLower(c)) {
        if (String::toUpper(c) == c)
          caps++;
        else
          noCaps++;
      }
    }
    return caps > noCaps;
  };

  HumanoidEmote result = HumanoidEmote::Idle;
  if (!chatter.empty()) {
    if (isShouty(chatter))
      result = HumanoidEmote::Shouting;
    else
      result = HumanoidEmote::Blabbering;
  }

  float bestMatch = -1;

  for (auto option : m_emoteBindings) {
    auto matchPosition = chatter.find(option.text);
    if (matchPosition == NPos)
      continue;
    float r = matchPosition + static_cast<float>(option.text.length()) * 0.01f;
    if (r > bestMatch) {
      bestMatch = r;
      result = option.emote;
    }
  }
  return result;
}

}
