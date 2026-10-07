#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarAssetPath.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVector.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarMaybe.hpp"
#include "StarNetElement.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarListener.hpp"
#include "StarVersion.hpp"

import star.drawable;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.emote_processor;

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
    auto p = chatter.find(option.text);
    if (p == NPos)
      continue;
    float r = p + (float)option.text.length() * 0.01f;
    if (r > bestMatch) {
      bestMatch = r;
      result = option.emote;
    }
  }
  return result;
}

}
