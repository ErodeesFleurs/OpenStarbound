module;
import std;
import star.json;

import star.game_types;

import star.color;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.vector;
import star.text;






import star.world_geometry;
import star.chat_action;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.chat_bubble_separation;
import star.world_camera;

namespace Star {

class GuiContext;
class AssetTextureGroup;
class WorldClient;
class StoredFunction;

class ChatBubbleManager {
public:
  ChatBubbleManager();

  void setCamera(WorldCamera const& camera);

  void addChatActions(List<ChatAction> chatActions, bool silent = false);

  void update(float dt, std::shared_ptr<WorldClient> world);
  void render();

private:
  typedef std::tuple<String, Vec2F> BubbleImage;
  typedef std::tuple<String, TextStyle, bool, Vec2F> BubbleText;

  struct Bubble {
    EntityId entity;
    String text;
    Json config;
    float age;
    List<BubbleImage> backgroundImages;
    List<BubbleText> bubbleText;
    bool onscreen;
  };

  struct PortraitBubble {
    EntityId entity;
    String portrait;
    String text;
    Vec2F position;
    Json config;
    float age;
    List<BubbleImage> backgroundImages;
    List<BubbleText> bubbleText;
    bool onscreen;
  };

  // Calculate the alpha for a speech bubble based on distance from player to
  // edge of screen
  std::uint8_t calcDistanceFadeAlpha(Vec2F bubbleScreenPosition, std::shared_ptr<StoredFunction> fadeFunction) const;

  RectF bubbleImageRect(Vec2F screenPos, BubbleImage const& bubbleImage, float pixelRatio);
  void drawBubbleImage(Vec2F screenPos, BubbleImage const& bubbleImage, float pixelRatio, int alpha);
  void drawBubbleText(Vec2F screenPos, BubbleText const& bubbleText, float pixelRatio, int alpha, bool isPortrait);

  GuiContext* m_guiContext;

  WorldCamera m_camera;

  TextPositioning m_textTemplate;
  TextPositioning m_portraitTextTemplate;
  TextStyle m_textStyle;
  Vec2F m_textPadding;

  BubbleSeparator<Bubble> m_bubbles;
  int m_zoom;
  float m_cachedInterfaceScale;
  Vec2F m_bubbleOffset;
  float m_maxAge;
  float m_portraitMaxAge;
  float m_interBubbleMargin;
  int m_maxMessagePerEntity;

  Deque<PortraitBubble> m_portraitBubbles;
  String m_portraitBackgroundImage;
  String m_portraitMoreImage;
  Vec2I m_portraitMorePosition;
  Vec2I m_portraitBackgroundSize;
  Vec2I m_portraitPosition;
  Vec2I m_portraitSize;
  Vec2I m_portraitTextPosition;
  unsigned m_portraitTextWidth;
  float m_portraitChatterFramerate;
  float m_portraitChatterDuration;

  float m_furthestVisibleTextDistance; // 0.0 is directly over the player, 1.0
  // is the edge of the window
  std::shared_ptr<StoredFunction> m_textFadeFunction;
  std::shared_ptr<StoredFunction> m_bubbleFadeFunction;
};

}

export module star.chat_bubble_manager;

export namespace Star {
  using ::Star::GuiContext;
  using ::Star::AssetTextureGroup;
  using ::Star::WorldClient;
  using ::Star::ChatBubbleManager;
  using ::Star::StoredFunction;
}
