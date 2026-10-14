module;

import std;
import star.list;
import star.string;

namespace Star {

// Keep the game-heavy Humanoid definition out of interface BMIs.
enum class HumanoidEmote;

class EmoteProcessor;
using EmoteProcessorPtr = std::shared_ptr<EmoteProcessor>;
using EmoteProcessorConstPtr = std::shared_ptr<const EmoteProcessor>;
using EmoteProcessorWeakPtr = std::weak_ptr<EmoteProcessor>;
using EmoteProcessorConstWeakPtr = std::weak_ptr<const EmoteProcessor>;
using EmoteProcessorUPtr = std::unique_ptr<EmoteProcessor>;
using EmoteProcessorConstUPtr = std::unique_ptr<const EmoteProcessor>;

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
