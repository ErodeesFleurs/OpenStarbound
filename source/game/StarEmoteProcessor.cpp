#include "StarEmoteProcessor.hpp"

#include "StarRoot.hpp"

import std;

namespace Star {

EmoteProcessor::EmoteProcessor() {
  auto assets = Root::singleton().assets();

  m_emoteBindings.clear();
  auto cfg = assets->json("/emotes.config");
  for (auto binding : cfg.get("emoteBindings").iterateObject()) {
    for (auto text : binding.second.toArray()) {
      EmoteBinding emoteBinding;
      emoteBinding.emote = HumanoidEmoteNames.getLeft(binding.first);
      emoteBinding.text = text.toString();
      m_emoteBindings.append(emoteBinding);
    }
  }
}

auto EmoteProcessor::detectEmotes(String const& chatter) const -> HumanoidEmote {
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
    auto p = chatter.find(option.text);
    if (p == std::numeric_limits<std::size_t>::max())
      continue;
    float r = p + (float)option.text.length() * 0.01f;
    if (r > bestMatch) {
      bestMatch = r;
      result = option.emote;
    }
  }
  return result;
}

}// namespace Star
