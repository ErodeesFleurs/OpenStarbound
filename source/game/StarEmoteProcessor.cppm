module;

#include "StarConfig.hpp"
import star.list;
import star.string;

namespace Star {

// Keep the game-heavy Humanoid definition out of interface BMIs.
enum class HumanoidEmote;

STAR_CLASS(EmoteProcessor);

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

export module star.emote_processor;

export namespace Star {
  using ::Star::HumanoidEmote;
  using ::Star::EmoteProcessor;
  using ::Star::EmoteProcessorPtr;
  using ::Star::EmoteProcessorConstPtr;
}
