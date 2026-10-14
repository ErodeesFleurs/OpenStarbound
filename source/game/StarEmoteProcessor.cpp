import star.memory;
import star.json;
#include "StarJsonExtra.hpp"
import star.data_stream;
import star.game_types;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.bi_map;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.audio;
import star.map;
import star.set;
import star.maybe;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;

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
