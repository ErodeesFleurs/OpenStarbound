#pragma once

#include "StarHumanoid.hpp"

namespace Star {

class EmoteProcessor;
using EmoteProcessorPtr = SharedPtr<EmoteProcessor>;
using EmoteProcessorConstPtr = SharedPtr<EmoteProcessor const>;

class EmoteProcessor {
public:
  EmoteProcessor();

  HumanoidEmote detectEmotes(String const& chatter) const;

private:
  struct EmoteBinding {
    EmoteBinding() : emote() {}
    String text;
    HumanoidEmote emote;
  };
  List<EmoteBinding> m_emoteBindings;
};

}
