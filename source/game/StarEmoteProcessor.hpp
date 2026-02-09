#pragma once

#include "StarHumanoid.hpp"

namespace Star {

class EmoteProcessor {
public:
  EmoteProcessor();

  [[nodiscard]] auto detectEmotes(String const& chatter) const -> HumanoidEmote;

private:
  struct EmoteBinding {
    EmoteBinding() = default;
    String text;
    HumanoidEmote emote{};
  };
  List<EmoteBinding> m_emoteBindings;
};

}// namespace Star
